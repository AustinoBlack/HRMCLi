#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "hrmcli/node.h"
#include "hrmcli/node_config.h"
#include "hrmcli/paths.h"
#include "hrmcli/startup.h"
#include "hrmcli/serial.h"

#define STARTUP_NODE_TEST_MAX 64


static int startup_begin_check(
    StartupState *state,
    const char *name,
    StartupProgressFn progress,
    void *userdata
)
{
    StartupCheck *check;
    int index;

    if (
        state == NULL ||
        state->check_count >= STARTUP_MAX_CHECKS
    ) {
        return -1;
    }

    index = state->check_count;
    check = &state->checks[index];

    snprintf(
        check->name,
        sizeof(check->name),
        "%s",
        name
    );

    check->message[0] = '\0';
    check->status = STARTUP_STATUS_PENDING;

    state->check_count++;

    if (progress != NULL) {
        progress(state, userdata);
    }

    return index;
}


static void startup_finish_check(
    StartupState *state,
    int index,
    StartupStatus status,
    const char *message,
    StartupProgressFn progress,
    void *userdata
)
{
    StartupCheck *check;

    if (
        state == NULL ||
        index < 0 ||
        index >= state->check_count
    ) {
        return;
    }

    check = &state->checks[index];

    check->status = status;

    snprintf(
        check->message,
        sizeof(check->message),
        "%s",
        message != NULL ? message : ""
    );

    if (status == STARTUP_STATUS_WARN) {
        state->has_warnings = 1;
    }

    if (status == STARTUP_STATUS_FAIL) {
        state->has_failures = 1;
    }

    if (progress != NULL) {
        progress(state, userdata);
    }
}

static void check_data_directory(
    StartupState *state,
    StartupProgressFn progress,
    void *userdata
)
{
    struct stat info;
    char message[STARTUP_CHECK_MESSAGE_MAX];

    const char *path = hrmcli_data_dir();

    int index = startup_begin_check(
        state,
        "Data directory",
        progress,
        userdata
    );

    if (
        stat(path, &info) == 0 &&
        S_ISDIR(info.st_mode)
    ) {
        snprintf(
            message,
            sizeof(message),
            "%s",
            path
        );

        startup_finish_check(
            state,
            index,
            STARTUP_STATUS_OK,
            message,
            progress,
            userdata
        );

        return;
    }

    snprintf(
        message,
        sizeof(message),
        "Not present yet: %s",
        path
    );

    startup_finish_check(
        state,
        index,
        STARTUP_STATUS_WARN,
        message,
        progress,
        userdata
    );
}

static void check_log_directory(
    StartupState *state,
    StartupProgressFn progress,
    void *userdata
)
{
    struct stat info;
    char message[STARTUP_CHECK_MESSAGE_MAX];

    const char *path = hrmcli_log_dir();

    int index = startup_begin_check(
        state,
        "Log directory",
        progress,
        userdata
    );

    if (
        stat(path, &info) == 0 &&
        S_ISDIR(info.st_mode)
    ) {
        startup_finish_check(
            state,
            index,
            STARTUP_STATUS_OK,
            path,
            progress,
            userdata
        );

        return;
    }

    snprintf(
        message,
        sizeof(message),
        "Not present yet: %s",
        path
    );

    startup_finish_check(
        state,
        index,
        STARTUP_STATUS_WARN,
        message,
        progress,
        userdata
    );
}

static void check_directory_writable(
    StartupState *state,
    const char *name,
    const char *path,
    StartupProgressFn progress,
    void *userdata
)
{
    int index;
    char message[STARTUP_CHECK_MESSAGE_MAX];

    index = startup_begin_check(
        state,
        name,
        progress,
        userdata
    );

    if (
        access(path, W_OK | X_OK) == 0
    ) {
        startup_finish_check(
            state,
            index,
            STARTUP_STATUS_OK,
            path,
            progress,
            userdata
        );

        return;
    }

    snprintf(
        message,
        sizeof(message),
        "Not writable: %s",
        path
    );

    startup_finish_check(
        state,
        index,
        STARTUP_STATUS_WARN,
        message,
        progress,
        userdata
    );
}

static void check_node_configuration(
    StartupState *state,
    StartupProgressFn progress,
    void *userdata
)
{
    Node nodes[STARTUP_NODE_TEST_MAX];

    int node_count = 0;

    int config_index;
    int nodes_index;

    char message[STARTUP_CHECK_MESSAGE_MAX];

    NodeConfigStatus result;

    const char *path = hrmcli_nodes_path();

    config_index = startup_begin_check(
        state,
        "Node configuration",
        progress,
        userdata
    );

    result = node_config_load(
        path,
        nodes,
        STARTUP_NODE_TEST_MAX,
        &node_count
    );

    switch (result) {
    case NODE_CONFIG_OK:
        startup_finish_check(
            state,
            config_index,
            STARTUP_STATUS_OK,
            path,
            progress,
            userdata
        );
        break;

    case NODE_CONFIG_INVALID_NODE:
        startup_finish_check(
            state,
            config_index,
            STARTUP_STATUS_WARN,
            "One or more invalid node entries were ignored",
            progress,
            userdata
        );
        break;

    case NODE_CONFIG_TOO_MANY_NODES:
        startup_finish_check(
            state,
            config_index,
            STARTUP_STATUS_WARN,
            "Node limit exceeded; extra entries were ignored",
            progress,
            userdata
        );
        break;

    case NODE_CONFIG_FILE_NOT_FOUND:
        snprintf(
            message,
            sizeof(message),
            "File not found: %s",
            path
        );

        startup_finish_check(
            state,
            config_index,
            STARTUP_STATUS_WARN,
            message,
            progress,
            userdata
        );

        nodes_index = startup_begin_check(
            state,
            "Managed nodes",
            progress,
            userdata
        );

        startup_finish_check(
            state,
            nodes_index,
            STARTUP_STATUS_WARN,
            "Node count unavailable",
            progress,
            userdata
        );

        return;

    case NODE_CONFIG_FILE_READ_ERROR:
        snprintf(
            message,
            sizeof(message),
            "Unable to read: %s",
            path
        );

        startup_finish_check(
            state,
            config_index,
            STARTUP_STATUS_WARN,
            message,
            progress,
            userdata
        );

        nodes_index = startup_begin_check(
            state,
            "Managed nodes",
            progress,
            userdata
        );

        startup_finish_check(
            state,
            nodes_index,
            STARTUP_STATUS_WARN,
            "Node count unavailable",
            progress,
            userdata
        );

        return;

    case NODE_CONFIG_PARSE_ERROR:
        startup_finish_check(
            state,
            config_index,
            STARTUP_STATUS_WARN,
            "JSON parse error",
            progress,
            userdata
        );

        nodes_index = startup_begin_check(
            state,
            "Managed nodes",
            progress,
            userdata
        );

        startup_finish_check(
            state,
            nodes_index,
            STARTUP_STATUS_WARN,
            "Node count unavailable",
            progress,
            userdata
        );

        return;

    case NODE_CONFIG_INVALID_ROOT:
        startup_finish_check(
            state,
            config_index,
            STARTUP_STATUS_WARN,
            "Invalid JSON structure",
            progress,
            userdata
        );

        nodes_index = startup_begin_check(
            state,
            "Managed nodes",
            progress,
            userdata
        );

        startup_finish_check(
            state,
            nodes_index,
            STARTUP_STATUS_WARN,
            "Node count unavailable",
            progress,
            userdata
        );

        return;

    case NODE_CONFIG_INVALID_ARGUMENT:
    default:
        startup_finish_check(
            state,
            config_index,
            STARTUP_STATUS_WARN,
            node_config_status_string(result),
            progress,
            userdata
        );

        nodes_index = startup_begin_check(
            state,
            "Managed nodes",
            progress,
            userdata
        );

        startup_finish_check(
            state,
            nodes_index,
            STARTUP_STATUS_WARN,
            "Node count unavailable",
            progress,
            userdata
        );

        return;
    }

    /*
     * If we got here, the config was usable,
     * even if one or more entries were skipped.
     */
    nodes_index = startup_begin_check(
        state,
        "Managed nodes",
        progress,
        userdata
    );

    if (node_count == 0) {
        startup_finish_check(
            state,
            nodes_index,
            STARTUP_STATUS_WARN,
            "No managed nodes configured",
            progress,
            userdata
        );

        return;
    }

    snprintf(
        message,
        sizeof(message),
        "%d managed node%s loaded",
        node_count,
        node_count == 1 ? "" : "s"
    );

    startup_finish_check(
        state,
        nodes_index,
        STARTUP_STATUS_OK,
        message,
        progress,
        userdata
    );
}

void startup_init(
    StartupState *state
)
{
    if (state == NULL) {
        return;
    }

    memset(
        state,
        0,
        sizeof(*state)
    );
}

static void check_serial_console(
    StartupState *state,
    StartupProgressFn progress,
    void *userdata
)
{
    SerialConfig config;
    SerialConfigStatus status;

    SerialDevice devices[SERIAL_MAX_DEVICES];

    int device_count = 0;
    int configured_present = 0;

    int index;

    char message[STARTUP_CHECK_MESSAGE_MAX];

    const char *path =
        hrmcli_serial_config_path();

    index = startup_begin_check(
        state,
        "Serial console",
        progress,
        userdata
    );

    status = serial_config_load(
        path,
        &config
    );

    switch (status) {
    case SERIAL_CONFIG_FILE_NOT_FOUND:
        startup_finish_check(
            state,
            index,
            STARTUP_STATUS_WARN,
            "Serial console not configured",
            progress,
            userdata
        );

        return;

    case SERIAL_CONFIG_OK:
        break;

    default:
        snprintf(
            message,
            sizeof(message),
            "%s",
            serial_config_status_string(status)
        );

        startup_finish_check(
            state,
            index,
            STARTUP_STATUS_WARN,
            message,
            progress,
            userdata
        );

        return;
    }

    /*
     * An empty device path is valid config data,
     * but means the console has not been selected yet.
     */
    if (config.device[0] == '\0') {
        startup_finish_check(
            state,
            index,
            STARTUP_STATUS_WARN,
            "Serial console not configured",
            progress,
            userdata
        );

        return;
    }

    /*
     * Enumerate currently-present serial devices and
     * look for the exact persisted device path.
     */
    if (
        serial_enumerate(
            devices,
            SERIAL_MAX_DEVICES,
            &device_count
        ) != 0
    ) {
        startup_finish_check(
            state,
            index,
            STARTUP_STATUS_WARN,
            "Unable to enumerate serial devices",
            progress,
            userdata
        );

        return;
    }

    for (int i = 0; i < device_count; i++) {
        if (
            strcmp(
                devices[i].path,
                config.device
            ) == 0
        ) {
            configured_present =
                devices[i].present;

            break;
        }
    }

    if (!configured_present) {
        snprintf(
            message,
            sizeof(message),
            "Configured device not present: %.80s",
            config.device
        );

        startup_finish_check(
            state,
            index,
            STARTUP_STATUS_WARN,
            message,
            progress,
            userdata
        );

        return;
    }

    snprintf(
        message,
        sizeof(message),
        "%.96s @ %d",
        config.device,
        config.baud
    );

    startup_finish_check(
        state,
        index,
        STARTUP_STATUS_OK,
        message,
        progress,
        userdata
    );
}

void startup_run_checks(
    StartupState *state,
    StartupProgressFn progress,
    void *userdata
)
{
    if (state == NULL) {
        return;
    }

    state->check_count = 0;
    state->has_warnings = 0;
    state->has_failures = 0;

    check_data_directory(
        state,
        progress,
        userdata
    );

    check_directory_writable(
        state,
        "Data directory writable",
        hrmcli_data_dir(),
        progress,
        userdata
    );

    check_log_directory(
        state,
        progress,
        userdata
    );

    check_directory_writable(
        state,
        "Log directory writable",
        hrmcli_log_dir(),
        progress,
        userdata
    );

    check_node_configuration(
        state,
        progress,
        userdata
    );

    check_serial_console(
        state,
        progress,
        userdata
    );
}
