#include "hrmcli/paths.h"

const char *hrmcli_nodes_path(void)
{
    /*return "/var/lib/hrmcli/nodes.json"; *//*PRODUCTION*/
    return "config/defaults/nodes.json";
}

const char *hrmcli_data_dir(void)
{
    return "/var/lib/hrmcli";
}

const char *hrmcli_log_dir(void)
{
    return "/var/log/hrmcli";
}
