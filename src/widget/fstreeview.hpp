#ifndef QFCMD_FS_TREEVIEW_HPP
#define QFCMD_FS_TREEVIEW_HPP

#include <QTreeView>

namespace qfcmd {

struct FsTreeViewInner;

class FsTreeView : public QTreeView
{
    Q_OBJECT
public:
    FsTreeView(QWidget* parent = nullptr);
    virtual ~FsTreeView();

public slots:
    /**
     * @brief Change the current directory.
     * @param[in] path The path of the directory.
     */
	void slotChangeDirectory(const QString& path);

private:
    FsTreeViewInner* m_inner;
};

} /* namespace qfcmd */

#endif // QFCMD_FS_TREEVIEW_HPP
