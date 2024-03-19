#ifndef QFCMD_VFS_FILE_HPP
#define QFCMD_VFS_FILE_HPP

#include <QObject>
#include <QUrl>
#include "filesystem.hpp"

namespace qfcmd {

class FileInner;

class File : public QObject
{
    Q_OBJECT
    friend class FileInner;

public:
    File(QObject* parent = nullptr);
    virtual ~File();

public:
    static int ls(const QUrl &url, FileSystem::FileInfoEntry *entry);
    static int stat(const QUrl &url, qfcmd_fs_stat_t *stat);

public:
    virtual int open(const QUrl& url, uint64_t flags);
    virtual void close();
    virtual int read(void* buf, size_t size);
    virtual int write(const void* buf, size_t size);

private:
    FileInner* m_inner;
};

} /* namespace qfcmd */

#endif
