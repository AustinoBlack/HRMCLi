#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "hrmcli/node.h"
#include "hrmcli/node_config.h"
#include "hrmcli/paths.h"
#include "hrmcli/startup.h"


#define STARTUP_NODE_TEST_MAX 64


static void startup_add_check(
    StartupState *state,
    const char *name,
    StartupStatus status,
    const char *message
)
{
    StartupCheck *check;

    if (
        state == NULL ||
        state->check_count >= STARTUP_MAX_CHECKS
    ) {
        return;
    }

    check =
        &state->checks[state->check_count];

    snprintf(
        check->name,
        sizeof(check->name),
        "%s",
        name != NULL ? name : ""
    );

    snprintf(
        check->message,
        sizeof(check->message),
        "%s",
        message != NULL ? message : ""
    );

    check->status = status;

    if (status == STARTUP_STATUS_WARN) {
        state->has_warnings = 1;
    }

    if (status == STARTUP_STATUS_FAIL) {
        state->has_failures = 1;
    }

    state->check_count++;
}


static void check_data_directory(
    StartupState *state
)
{
    struct stat info;
    char message[STARTUP_CHECK_MESSAGE_MAX];

    const char *path =
        hrmcli_data_dir();

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

        startup_add_check(
            state,
            "Data directory",
            STARTUP_STATUS_OK,
            message
        );

        return;
    }

    if (errno == ENOENT) {
        snprintf(
            message,
            sizeof(message),
            "Not present yet: %s",
            path
        );
    } else {
        snprintf(
            message,
            sizeof(message),
            "Unable to access %s",
            path
        );
    }

    /*
     * This is only a warning for now because
     * development builds currently load their
     * node configuration from the repository.
     */
    startup_add_check(
        state,
        "Data directory",
        STARTUP_STATUS_WARN,
        message
    );
}


static void check_node_configuration(
    StartupState *state
)
{
    Node nodes[STARTUP_NODE_TEST_MAX];

    int node_count = 0;
    int result;

    char message[STARTUP_CHECK_MESSAGE_MAX];

    const char *path =
        hrmcli_nodes_path();

    result = node_config_load(
        path,
        nodes,
        STARTUP_NODE_TEST_MAX,
        &node_count
    );

    if (result != 0) {
        snprintf(
            message,
            sizeof(message),
            "Unable to load %s",
            path
        );

        startup_add_check(
            state,
            "Node configuration",
            STARTUP_STATUS_WARN,
            message
        );

        startup_add_check(
            state,
            "Managed nodes",
            STARTUP_STATUS_WARN,
            "Node count unavailable"
        );

        return;
    }

    snprintf(
        message,
        sizeof(message),
        "%s",
        path
    );

    startup_add_check(
        state,
        "Node configuration",
        STARTUP_STATUS_OK,
        message
    );

    if (node_count == 0) {
        startup_add_check(
            state,
            "Managed nodes",
            STARTUP_STATUS_WARN,
            "No managed nodes configured"
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

    startup_add_check(
        state,
        "Managed nodes",
        STARTUP_STATUS_OK,
        message
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


void startup_run_checks(
    StartupState *state
)
{
    if (state == NULL) {
        return;
    }

    /*
     * Allow the startup checks to be run again
     * without appending duplicate results.
     */
    state->check_count = 0;
    state->has_warnings = 0;
    state->has_failures = 0;

    check_data_directory(state);
    check_node_configuration(state);
}
