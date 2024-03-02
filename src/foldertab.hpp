#ifndef FOLDERTAB_HPP
#define FOLDERTAB_HPP

#include <QWidget>
#include <QFileSystemModel>

namespace Ui {
class FolderTab;
}

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
    /**
     * @brief Set current folder.
     * @param[in] path - path to folder.
     */
    void cd(const QString& path);
    void cd_with_history(const QString& path);

    void update_ui_visibility();

private:
    size_t              m_cfg_path_max_history; /* Max number of path history.*/
    size_t              m_path_idx;             /* Current path. */
    QStringList         m_path_history;         /* Path history. */

private:
    Ui::FolderTab*      ui;         /* UI. */
    QFileSystemModel*   m_model;    /* File system model. */
};

#endif // FOLDERTAB_HPP
