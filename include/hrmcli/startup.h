#ifndef HRMCLI_STARTUP_H
#define HRMCLI_STARTUP_H

#define STARTUP_CHECK_NAME_MAX 64
#define STARTUP_CHECK_MESSAGE_MAX 128
#define STARTUP_MAX_CHECKS 16

typedef enum {
    STARTUP_STATUS_PENDING,
    STARTUP_STATUS_OK,
    STARTUP_STATUS_WARN,
    STARTUP_STATUS_FAIL
} StartupStatus;

typedef struct {
    char name[STARTUP_CHECK_NAME_MAX];
    char message[STARTUP_CHECK_MESSAGE_MAX];

    StartupStatus status;
} StartupCheck;

typedef struct {
    StartupCheck checks[STARTUP_MAX_CHECKS];

    int check_count;
    int has_warnings;
    int has_failures;
} StartupState;

void startup_init(
    StartupState *state
);

void startup_run_checks(
    StartupState *state
);

#endif
