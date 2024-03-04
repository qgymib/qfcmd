#ifndef QFCMD_FS_TABWIDGET_HPP
#define QFCMD_FS_TABWIDGET_HPP

#include <QTabWidget>

namespace qfcmd {

class FsTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    FsTabWidget(QWidget* parent = nullptr);
    virtual ~FsTabWidget();

public slots:
    /**
     * @brief Add new tab with given path.
     * @param[in] path - path to folder.
     */
    void slotOpenNewTab(const QString& path);

private slots:
    void slotUpdateTabTitle(const QString& title);
    void slotTabCloseRequest(int index);
    void slotTabBarChangeTab(int index);
};

} /* namespace qfcmd */

#endif // QFCMD_FS_TABWIDGET_HPP
