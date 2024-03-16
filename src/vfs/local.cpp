#include "local.hpp"
#include <QDir>
#include <QFileInfo>

/**
 * @brief Converts a local file information to a qfcmd::IFileSystem::FileStat structure.
 * @param[in] info - The QFileInfo object containing the local file information.
 * @return A qfcmd::IFileSystem::FileStat structure representing the local file information.
 * @throws None
 */
static qfcmd_fs_stat_t _local_file_info_to_stat(const QFileInfo& info)
{
    qfcmd_fs_stat_t stat;
    memset(&stat, 0, sizeof(stat));

    if (info.isDir())
    {
        stat.st_mode |= QFCMD_FS_S_IFDIR;
    }
    if (info.isFile())
    {
        stat.st_mode |= QFCMD_FS_S_IFREG;
    }

    stat.st_size = info.size();
    stat.st_mtime = info.lastModified().toSecsSinceEpoch();

    return stat;
}

/**
 * @brief Generates the QIODeviceBase::OpenMode based on the given flags.
 * @param[in] flags - The flags used to determine the open mode
 * @return The QIODeviceBase::OpenMode based on the flags
 */
static QIODeviceBase::OpenMode _local_file_open_mode(uint64_t flags)
{
    QIODeviceBase::OpenMode mode = QIODeviceBase::NotOpen;
    if (flags & QFCMD_FS_O_RDONLY)
    {
        mode |= QIODeviceBase::ReadOnly;
    }
    if (flags & QFCMD_FS_O_WRONLY)
    {
        mode |= QIODeviceBase::WriteOnly;
    }
    if (flags & QFCMD_FS_O_RDWR)
    {
        mode |= QIODeviceBase::ReadWrite;
    }
    if (flags & QFCMD_FS_O_APPEND)
    {
        mode |= QIODeviceBase::Append;
    }
    if (flags & QFCMD_FS_O_TRUNCATE)
    {
        mode |= QIODeviceBase::Truncate;
    }
    if (!(flags & QFCMD_FS_O_CREAT))
    {
        mode |= QIODeviceBase::ExistingOnly;
    }
    return mode;
}

qfcmd::LocalFS::LocalFS(QObject* parent)
    : FileSystem(parent)
{
}

qfcmd::LocalFS::~LocalFS()
{
}

int qfcmd::LocalFS::mount(const QUrl& url, FsPtr& fs)
{
    (void)url;

    /*
     * For local file system, it should be safe to use singleton instance.
     */
    static qfcmd::FileSystem::FsPtr instance(new qfcmd::LocalFS);
    fs = instance;

    return 0;
}

int qfcmd::LocalFS::ls(const QUrl& url, FileInfoEntry* entry)
{
    const QString file_path = url.toLocalFile();
    QFileInfoList info_list = QDir(file_path).entryInfoList();

    for (QFileInfo& info : info_list)
    {
        const QString fileName = info.fileName();
        if (fileName == "." || fileName == "..")
        {
            continue;
        }

        entry->insert(fileName, _local_file_info_to_stat(info));
    }

    return 0;
}

int qfcmd::LocalFS::stat(const QUrl& url, qfcmd_fs_stat_t* stat)
{
    const QString file_path = url.toLocalFile();
    QFileInfo info(file_path);

    *stat = _local_file_info_to_stat(info);
    return 0;
}

int qfcmd::LocalFS::open(uintptr_t* fh, const QUrl& url, uint64_t flags)
{
    const QString path = url.toLocalFile();
    QFile* file = new QFile(path);

    QIODeviceBase::OpenMode mode = _local_file_open_mode(flags);
    if (!file->open(mode))
    {
        delete file;
        return -ENOENT;
    }

    *fh = reinterpret_cast<uintptr_t>(file);
    return 0;
}

void qfcmd::LocalFS::close(uintptr_t fh)
{
    QFile* file = reinterpret_cast<QFile*>(fh);
    file->close();
    delete file;
}

int qfcmd::LocalFS::read(uintptr_t fh, void* buf, size_t size)
{
    QFile* file = reinterpret_cast<QFile*>(fh);
    return file->read((char*)buf, (qint64)size);
}

int qfcmd::LocalFS::write(uintptr_t fh, const void* buf, size_t size)
{
    QFile* file = reinterpret_cast<QFile*>(fh);
    return file->write((const char*)buf, (qint64)size);
}
