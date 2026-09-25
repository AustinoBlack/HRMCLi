#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <cjson/cJSON.h>

#include "hrmcli/node.h"
#include "hrmcli/node_config.h"


static char *read_file(const char *path)
{
    FILE *file;
    char *buffer;

    long size;
    size_t bytes_read;

    file = fopen(path, "rb");

    if (file == NULL) {
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }

    size = ftell(file);

    if (size < 0) {
        fclose(file);
        return NULL;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }

    buffer = malloc((size_t)size + 1);

    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }

    bytes_read = fread(
        buffer,
        1,
        (size_t)size,
        file
    );

    fclose(file);

    if (bytes_read != (size_t)size) {
        free(buffer);
        return NULL;
    }

    buffer[size] = '\0';

    return buffer;
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


int node_config_load(
    const char *path,
    Node *nodes,
    int max_nodes,
    int *node_count
)
{
    char *json_text;

    cJSON *root;
    cJSON *node_array;
    cJSON *item;

    int count = 0;

    if (
        path == NULL ||
        nodes == NULL ||
        node_count == NULL ||
        max_nodes <= 0
    ) {
        return -1;
    }

    *node_count = 0;

    json_text = read_file(path);

    if (json_text == NULL) {
        return -1;
    }

    root = cJSON_Parse(json_text);

    free(json_text);

    if (root == NULL) {
        return -1;
    }

    node_array = cJSON_GetObjectItemCaseSensitive(
        root,
        "nodes"
    );

    if (!cJSON_IsArray(node_array)) {
        cJSON_Delete(root);
        return -1;
    }

    cJSON_ArrayForEach(item, node_array) {
        cJSON *name;
        cJSON *address;
        cJSON *protocol_json;

        NodeProtocol protocol;
        Node *node;

        if (count >= max_nodes) {
            break;
        }

        if (!cJSON_IsObject(item)) {
            continue;
        }

        name = cJSON_GetObjectItemCaseSensitive(
            item,
            "name"
        );

        address = cJSON_GetObjectItemCaseSensitive(
            item,
            "address"
        );

        protocol_json = cJSON_GetObjectItemCaseSensitive(
            item,
            "protocol"
        );

        if (
            !cJSON_IsString(name) ||
            !cJSON_IsString(address) ||
            !cJSON_IsString(protocol_json)
        ) {
            continue;
        }

        if (
            parse_protocol(
                protocol_json->valuestring,
                &protocol
            ) != 0
        ) {
            continue;
        }

        node = &nodes[count];

        memset(
            node,
            0,
            sizeof(*node)
        );

        snprintf(
            node->name,
            sizeof(node->name),
            "%s",
            name->valuestring
        );

        snprintf(
            node->address,
            sizeof(node->address),
            "%s",
            address->valuestring
        );

        node->protocol = protocol;

        /*
         * Runtime BMC information has not yet
         * been queried.
         */
        node->status =
            NODE_STATUS_UNKNOWN;

        node->power =
            NODE_POWER_UNKNOWN;

        count++;
    }

    cJSON_Delete(root);

    *node_count = count;

    return 0;
}
