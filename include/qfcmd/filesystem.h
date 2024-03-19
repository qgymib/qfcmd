#ifndef QFCMD_FILESYSTEM_H
#define QFCMD_FILESYSTEM_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum qfcmd_fs_open_flag
{
    QFCMD_FS_O_RDONLY   = 0x0001,   /**< Read only. */
    QFCMD_FS_O_WRONLY   = 0x0002,   /**< Write only. */
    QFCMD_FS_O_RDWR     = QFCMD_FS_O_RDONLY | QFCMD_FS_O_WRONLY,
    QFCMD_FS_O_APPEND   = 0x0004,   /**< Append to file. */
    QFCMD_FS_O_TRUNCATE = 0x0008,   /**< Truncate file to zero. */
    QFCMD_FS_O_CREAT    = 0x0100,   /**< Create file if not exist. */
} qfcmd_fs_open_flag_t;

typedef enum qfcmd_fs_stat_flag
{
    QFCMD_FS_S_IFDIR    = 0x4000,   /**< Directory. */
    QFCMD_FS_S_IFREG    = 0x8000,   /**< Regular file. */
} qfcmd_fs_stat_flag_t;

typedef struct qfcmd_fs_stat
{
    uint64_t st_mode;   /**< File mode. See #qfcmd_fs_stat_flag_t. */
    uint64_t st_size;   /**< File size in bytes. */
    uint64_t st_mtime;  /**< Last modified time in seconds in UTC. */
} qfcmd_fs_stat_t;

/**
 * @brief Callback function for listing files.
 * @param[in] name - name of the file. Encoding in UTF-8.
 * @param[in] stat - stat of the file.
 * @param[in] data - user data.
 * @return 0 on success, or non-zero to stop listing.
 */
typedef int (*qfcmd_fs_ls_cb)(const char* name, const qfcmd_fs_stat_t* stat, void* data);

/**
 * @brief Filesystem operations.
 *
 * The filesystem must provide a mount function to mount a file system, see
 * #qfcmd_fs_mount_fn.
 */
typedef struct qfcmd_filesystem
{
    /**
     * @brief Destroy the filesystem object.
     *
     * It is guarantee that there are no other references to the filesystem,
     * and no more calls to the filesystem will be made.
     *
     * @param[in] thiz - This object.
     */
    void (*destroy)(struct qfcmd_filesystem* thiz);

    /**
     * @brief (Optional) List items in directory.
     * @param[in] thiz - This object.
     * @param[in] url - URL of directory. Encoding in UTF-8.
     * @param[in] cb - Callback function.
     * @param[in] data - user data which must be passed to the callback.
     * @return 0 on success, or -errno on error.
     */
    int (*ls)(struct qfcmd_filesystem* thiz, const char* url, qfcmd_fs_ls_cb cb, void* data);

    /**
     * @brief (Optional) Get file status.
     * @param[in] thiz - This object.
     * @param[in] url - URL of file. Encoding in UTF-8.
     * @param[out] stat - File status.
     * @return 0 on success, or -errno on error.
     */
    int (*stat)(struct qfcmd_filesystem* thiz, const char* url, qfcmd_fs_stat_t* stat);

    /**
     * @brief (Optional) Open file in binary mode.
     * @param[in] thiz - This object.
     * @param[out] fh - File handle.
     * @param[in] url - URL of file. Encoding in UTF-8.
     * @param[in] flags - Open flags. See #qfcmd_fs_open_flag_t.
     * @return 0 on success, or -errno on error.
     */
    int (*open)(struct qfcmd_filesystem* thiz, uintptr_t* fh, const char* url, uint64_t flags);

    /**
     * @brief (Optional) Close file.
     * @param[in] thiz - This object.
     * @param[in] fh - File handle.
     * @return 0 on success, or -errno on error.
     */
    int (*close)(struct qfcmd_filesystem* thiz, uintptr_t fh);

    /**
     * @brief (Optional) Read data from file.
     * @param[in] thiz - This object.
     * @param[in] fh - File handle.
     * @param[out] buf - Buffer to store data.
     * @param[in] size - Buffer size.
     * @return Number of bytes read on success, or -errno on error.
     */
    int (*read)(struct qfcmd_filesystem* thiz, uintptr_t fh, void* buf, size_t size);

    /**
     * @brief (Optional) Write data to file.
     * @param[in] thiz - This object.
     * @param[in] fh - File handle.
     * @param[in] buf - Buffer containing data.
     * @param[in] size - Size of data.
     * @return Number of bytes written on success, or -errno on error.
     */
    int (*write)(struct qfcmd_filesystem* thiz, uintptr_t fh, const void* buf, size_t size);
} qfcmd_filesystem_t;

/**
 * @brief Mount file system.
 *
 * The mount function might be called multiple times, if there are multiple
 * mount points.
 *
 * The \p url is encoded in UTF-8, it looks like 'file:///path/to/file'
 * in Linux and 'file:///c:/path/to/file' in Windows.
 *
 * @param[in] url - URL of mount point. Encoding in UTF-8.
 * @param[out] fs - Filesystem instance.
 * @return 0 on success, or -errno on error.
 */
typedef int (*qfcmd_fs_mount_fn)(const char* url, qfcmd_filesystem_t** fs);

#ifdef __cplusplus
}
#endif
#endif
