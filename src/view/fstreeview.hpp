#ifndef FSTREEVIEW_HPP
#define FSTREEVIEW_HPP

#include <QTreeView>
#include <QFileSystemModel>

namespace qfcmd
{

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
    QFileSystemModel* m_model;
};

}; /* namespace qfcmd */

#endif // FSTREEVIEW_HPP
