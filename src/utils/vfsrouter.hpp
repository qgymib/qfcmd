#ifndef QFCMD_UTILS_VFS_ROUTER_HPP
#define QFCMD_UTILS_VFS_ROUTER_HPP

#include <functional>
#include <QJsonObject>

namespace qfcmd {

class VfsRouterInner;

/**
 * @brief A router for VFS.
 */
class VfsRouter
{
    friend class VfsRouterInner;

public:
    typedef std::function<QJsonObject(const QJsonObject&)> RouterCB;

public:
    VfsRouter();
    virtual ~VfsRouter();

public:
    /**
     * @brief Route a path to a callback.
     * @param[in] path - Route path.
     * @param[in] cb   - Callback function.
     */
    void route(const QString& path, RouterCB cb);

private:
    VfsRouterInner* m_inner;
};

} /* namespace qfcmd */

#endif
