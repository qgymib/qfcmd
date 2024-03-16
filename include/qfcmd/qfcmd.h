#ifndef QFCMD_H
#define QFCMD_H
#ifdef __cplusplus
extern "C" {
#endif

#include "filesystem.h"

#define QFCMD_API_VERSION   "1.0"

typedef struct qfcmd_host_api
{
    /**
     * @brief Setup plugin.
     *
     * This function must be called exactly once during plugin initialization,
     * before any other function can be called.
     *
     * The informations are given as key-value pair (case sensitive), for example:
     * ```c
     * const char* info[] = {
     *     "API_VERSION=" QFCMD_API_VERSION,
     *     "name=example_plugin",
     *     "version=1.0.0",
     * };
     * api->setup(api, 1, info);
     * ```
     *
     * The following keys must be set:
     * + `API_VERSION`: The API version. Always set to #QFCMD_API_VERSION.
     *
     * The following keys are optional:
     * + `name`: The name of the plugin. If not set, the name of the dll will be used.
     * + `version`: The version of the plugin.
     * + `author`: The author of the plugin.
     * + `email`: The email of the author.
     *
     * @param[in] thiz - This object.
     * @param[in] argc - Number of arguments.
     * @param[in] argv - Arguments.
     */
    void (*setup)(struct qfcmd_plugin_api* thiz, int argc, char* argv[]);

    /**
     * @brief Register Virual File System (VFS).
     * @param[in] thiz - This object.
     * @param[in] scheme - Scheme of VFS.
     * @param[in] fn - Callback function.
     * @return 0 on success, or -errno on error.
     */
    int (*register_vfs)(struct qfcmd_plugin_api* thiz, const char* scheme, qfcmd_fs_mount_fn fn);
} qfcmd_host_api_t;

/**
 * @brief Plugin entrypoint.
 * @param[in] api - Host API.
 * @return 0 on success, or -errno on error.
 */
typedef int (*qfcmd_plugin_entry_fn)(const qfcmd_host_api_t* api);

#ifdef __cplusplus
}
#endif
#endif
