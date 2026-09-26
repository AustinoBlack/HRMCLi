#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <cjson/cJSON.h>

#include "hrmcli/node_config.h"

static const char *protocol_config_string(
    NodeProtocol protocol
)
{
    switch (protocol) {
    case NODE_PROTOCOL_IPMI:
        return "ipmi";

    case NODE_PROTOCOL_REDFISH:
        return "redfish";

    default:
        return NULL;
    }
}

static NodeConfigStatus read_file(
    const char *path,
    char **buffer
)
{
    FILE *file;
    long size;
    size_t bytes_read;
    char *data;

    if (
        path == NULL ||
        buffer == NULL
    ) {
        return NODE_CONFIG_INVALID_ARGUMENT;
    }

    *buffer = NULL;

    file = fopen(path, "rb");

    if (file == NULL) {
        if (errno == ENOENT) {
            return NODE_CONFIG_FILE_NOT_FOUND;
        }

        return NODE_CONFIG_FILE_READ_ERROR;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NODE_CONFIG_FILE_READ_ERROR;
    }

    size = ftell(file);

    if (size < 0) {
        fclose(file);
        return NODE_CONFIG_FILE_READ_ERROR;
    }

    rewind(file);

    data = malloc((size_t)size + 1);

    if (data == NULL) {
        fclose(file);
        return NODE_CONFIG_FILE_READ_ERROR;
    }

    bytes_read = fread(
        data,
        1,
        (size_t)size,
        file
    );

    if (
        bytes_read != (size_t)size &&
        ferror(file)
    ) {
        free(data);
        fclose(file);

        return NODE_CONFIG_FILE_READ_ERROR;
    }

    data[bytes_read] = '\0';

    fclose(file);

    *buffer = data;

    return NODE_CONFIG_OK;
}


static int parse_protocol(
    const char *text,
    NodeProtocol *protocol
)
{
    if (
        text == NULL ||
        protocol == NULL
    ) {
        return -1;
    }

    if (strcasecmp(text, "ipmi") == 0) {
        *protocol = NODE_PROTOCOL_IPMI;
        return 0;
    }

    if (strcasecmp(text, "redfish") == 0) {
        *protocol = NODE_PROTOCOL_REDFISH;
        return 0;
    }

    return -1;
}


NodeConfigStatus node_config_load(
    const char *path,
    Node *nodes,
    int max_nodes,
    int *node_count
)
{
    char *json_data;

    cJSON *root;
    cJSON *node_array;
    cJSON *entry;

    int count = 0;
    int invalid_node_found = 0;
    int too_many_nodes = 0;

    NodeConfigStatus status;

    if (
        path == NULL ||
        nodes == NULL ||
        max_nodes <= 0 ||
        node_count == NULL
    ) {
        return NODE_CONFIG_INVALID_ARGUMENT;
    }

    *node_count = 0;

    /*
     * Read the configuration file.
     */
    status = read_file(
        path,
        &json_data
    );

    if (status != NODE_CONFIG_OK) {
        return status;
    }

    /*
     * Parse the JSON document.
     */
    root = cJSON_Parse(json_data);

    free(json_data);

    if (root == NULL) {
        return NODE_CONFIG_PARSE_ERROR;
    }

    /*
     * Expected format:
     *
     * {
     *     "nodes": [...]
     * }
     */
    if (!cJSON_IsObject(root)) {
        cJSON_Delete(root);
        return NODE_CONFIG_INVALID_ROOT;
    }

    node_array = cJSON_GetObjectItemCaseSensitive(
        root,
        "nodes"
    );

    if (!cJSON_IsArray(node_array)) {
        cJSON_Delete(root);
        return NODE_CONFIG_INVALID_ROOT;
    }

    /*
     * Parse each configured node.
     */
    cJSON_ArrayForEach(entry, node_array) {
        cJSON *name;
        cJSON *address;
        cJSON *protocol;

        NodeProtocol parsed_protocol;

        if (!cJSON_IsObject(entry)) {
            invalid_node_found = 1;
            continue;
        }

        name = cJSON_GetObjectItemCaseSensitive(
            entry,
            "name"
        );

        address = cJSON_GetObjectItemCaseSensitive(
            entry,
            "address"
        );

        protocol = cJSON_GetObjectItemCaseSensitive(
            entry,
            "protocol"
        );

        /*
         * Required fields must all be strings.
         */
        if (
            !cJSON_IsString(name) ||
            !cJSON_IsString(address) ||
            !cJSON_IsString(protocol)
        ) {
            invalid_node_found = 1;
            continue;
        }

        /*
         * Required strings may not be empty.
         */
        if (
            name->valuestring[0] == '\0' ||
            address->valuestring[0] == '\0' ||
            protocol->valuestring[0] == '\0'
        ) {
            invalid_node_found = 1;
            continue;
        }

        /*
         * Reject values that do not fit into
         * the fixed-size Node structure rather
         * than silently truncating them.
         */
        if (
            strlen(name->valuestring) >= NODE_NAME_MAX ||
            strlen(address->valuestring) >= NODE_ADDRESS_MAX
        ) {
            invalid_node_found = 1;
            continue;
        }

        if (
            parse_protocol(
                protocol->valuestring,
                &parsed_protocol
            ) != 0
        ) {
            invalid_node_found = 1;
            continue;
        }

        /*
         * Continue validating entries even after
         * reaching capacity, but do not store
         * additional nodes.
         */
        if (count >= max_nodes) {
            too_many_nodes = 1;
            continue;
        }

        snprintf(
            nodes[count].name,
            sizeof(nodes[count].name),
            "%s",
            name->valuestring
        );

        snprintf(
            nodes[count].address,
            sizeof(nodes[count].address),
            "%s",
            address->valuestring
        );

        nodes[count].protocol =
            parsed_protocol;

        nodes[count].status =
            NODE_STATUS_UNKNOWN;

        nodes[count].power =
            NODE_POWER_UNKNOWN;

        count++;
    }

    cJSON_Delete(root);

    *node_count = count;

    /*
     * These are warning conditions rather than
     * complete load failures. Valid nodes have
     * still been returned to the caller.
     */
    if (too_many_nodes) {
        return NODE_CONFIG_TOO_MANY_NODES;
    }

    if (invalid_node_found) {
        return NODE_CONFIG_INVALID_NODE;
    }

    return NODE_CONFIG_OK;
}

NodeConfigStatus node_config_save(
    const char *path,
    const Node *nodes,
    int node_count
)
{
    cJSON *root;
    cJSON *node_array;

    char *json_data;
    char *tmp_path;

    FILE *file;

    size_t path_length;
    size_t json_length;
    size_t bytes_written;

    if (
        path == NULL ||
        nodes == NULL ||
        node_count < 0
    ) {
        return NODE_CONFIG_INVALID_ARGUMENT;
    }

    root = cJSON_CreateObject();

    if (root == NULL) {
        return NODE_CONFIG_SERIALIZE_ERROR;
    }

    node_array = cJSON_CreateArray();

    if (node_array == NULL) {
        cJSON_Delete(root);
        return NODE_CONFIG_SERIALIZE_ERROR;
    }

    if (
        !cJSON_AddItemToObject(
            root,
            "nodes",
            node_array
        )
    ) {
        cJSON_Delete(node_array);
        cJSON_Delete(root);

        return NODE_CONFIG_SERIALIZE_ERROR;
    }

    for (
        int i = 0;
        i < node_count;
        i++
    ) {
        cJSON *entry;

        const char *protocol;

        protocol =
            protocol_config_string(
                nodes[i].protocol
            );

        /*
         * Reject invalid node data rather than
         * writing a broken configuration.
         */
        if (
            nodes[i].name[0] == '\0' ||
            nodes[i].address[0] == '\0' ||
            protocol == NULL
        ) {
            cJSON_Delete(root);

            return NODE_CONFIG_INVALID_NODE;
        }

        entry = cJSON_CreateObject();

        if (entry == NULL) {
            cJSON_Delete(root);

            return NODE_CONFIG_SERIALIZE_ERROR;
        }

        if (
            cJSON_AddStringToObject(
                entry,
                "name",
                nodes[i].name
            ) == NULL ||
            cJSON_AddStringToObject(
                entry,
                "address",
                nodes[i].address
            ) == NULL ||
            cJSON_AddStringToObject(
                entry,
                "protocol",
                protocol
            ) == NULL
        ) {
            cJSON_Delete(entry);
            cJSON_Delete(root);

            return NODE_CONFIG_SERIALIZE_ERROR;
        }

        if (
            !cJSON_AddItemToArray(
                node_array,
                entry
            )
        ) {
            cJSON_Delete(entry);
            cJSON_Delete(root);

            return NODE_CONFIG_SERIALIZE_ERROR;
        }
    }

    /*
     * Pretty-print the file so nodes.json remains
     * readable by a human when troubleshooting.
     */
    json_data = cJSON_Print(root);

    cJSON_Delete(root);

    if (json_data == NULL) {
        return NODE_CONFIG_SERIALIZE_ERROR;
    }

    /*
     * Write to a temporary file first.
     *
     * This prevents a failed write from destroying
     * the currently valid nodes.json.
     */
    path_length = strlen(path);

    tmp_path = malloc(
        path_length + 5
    );

    if (tmp_path == NULL) {
        cJSON_free(json_data);

        return NODE_CONFIG_FILE_WRITE_ERROR;
    }

    snprintf(
        tmp_path,
        path_length + 5,
        "%s.tmp",
        path
    );

    file = fopen(
        tmp_path,
        "wb"
    );

    if (file == NULL) {
        free(tmp_path);
        cJSON_free(json_data);

        return NODE_CONFIG_FILE_WRITE_ERROR;
    }

    json_length = strlen(json_data);

    bytes_written = fwrite(
        json_data,
        1,
        json_length,
        file
    );

    if (
        bytes_written != json_length ||
        fflush(file) != 0
    ) {
        fclose(file);
        remove(tmp_path);

        free(tmp_path);
        cJSON_free(json_data);

        return NODE_CONFIG_FILE_WRITE_ERROR;
    }

    if (fclose(file) != 0) {
        remove(tmp_path);

        free(tmp_path);
        cJSON_free(json_data);

        return NODE_CONFIG_FILE_WRITE_ERROR;
    }

    /*
     * Rename replaces the live configuration only
     * after the temporary file was written fully.
     */
    if (
        rename(
            tmp_path,
            path
        ) != 0
    ) {
        remove(tmp_path);

        free(tmp_path);
        cJSON_free(json_data);

        return NODE_CONFIG_FILE_WRITE_ERROR;
    }

    free(tmp_path);
    cJSON_free(json_data);

    return NODE_CONFIG_OK;
}

const char *node_config_status_string(
    NodeConfigStatus status
)
{
    switch (status) {
    case NODE_CONFIG_OK:
        return "OK";

    case NODE_CONFIG_FILE_NOT_FOUND:
        return "File not found";

    case NODE_CONFIG_FILE_READ_ERROR:
        return "File read error";

    case NODE_CONFIG_PARSE_ERROR:
        return "JSON parse error";

    case NODE_CONFIG_INVALID_ROOT:
        return "Invalid JSON structure";

    case NODE_CONFIG_INVALID_NODE:
        return "Invalid node entry";

    case NODE_CONFIG_TOO_MANY_NODES:
        return "Too many nodes";

    case NODE_CONFIG_INVALID_ARGUMENT:
        return "Invalid argument";

    case NODE_CONFIG_FILE_WRITE_ERROR:
        return "File write error";

    case NODE_CONFIG_SERIALIZE_ERROR:
        return "JSON serialization error";

    default:
        return "Unknown error";
    }
}
