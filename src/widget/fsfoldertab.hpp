#ifndef QFCMD_FS_FOLDERTAB_HPP
#define QFCMD_FS_FOLDERTAB_HPP

#include <QWidget>

namespace qfcmd {

struct FolderTabInner;

class FolderTab : public QWidget
{
    Q_OBJECT

public:
    explicit FolderTab(QWidget *parent = nullptr);
    explicit FolderTab(const QString& path, QWidget *parent = nullptr);
    ~FolderTab();

private slots:
    /**
     * @brief On double click event on `tableView`.
     */
    void onTableViewDoubleClicked(const QModelIndex &index);
    void onGoBackClicked();
    void onGoForwardClicked();
    void onGoUpClicked();
    void onTableViewContextMenuRequested(QPoint pos);
    void slotShowProperties();

private:
    FolderTabInner*     m_inner;     /* Internal. */
};

} /* namespace qfcmd */

#endif // QFCMD_FS_FOLDERTAB_HPP
