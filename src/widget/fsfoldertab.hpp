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

public:
    /**
     * @brief Get filesystem path.
     * @return Path.
     */
    QString path() const;

public slots:
    /**
     * @brief Go back to previous directory.
     */
    void slotGoBack();

    /**
     * @brief Go forward to next directory.
     */
    void slotGoForward();

signals:
    /**
     * @brief Open focus file in new tab.
     */
    void signalOpenInNewTab(const QString& path);

private slots:
    /**
     * @brief Open focus item.
     */
    void slotOpenItem();

    /**
     * @brief Open focus file with.
     */
    void slotOpenFileWith();

    /**
     * @brief Open focused directory in new tab.
     */
    void slotOpenInNewTab();

    void onGoUpClicked();
    void slotTableViewContextMenuRequested(QPoint pos);
    void slotShowProperties();

private:
    FolderTabInner*     m_inner;     /* Internal. */
};

} /* namespace qfcmd */

#endif // QFCMD_FS_FOLDERTAB_HPP
