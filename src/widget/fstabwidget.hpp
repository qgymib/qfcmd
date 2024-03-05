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

public:
    void closeCurrentActiveTab();

protected:
    /**
     * @see https://doc.qt.io/qt-6/qwidget.html#mousePressEvent
     */
    virtual void mousePressEvent(QMouseEvent* event) override;

public slots:
    /**
     * @brief Add new tab with given path.
     * @param[in] path - path to folder.
     */
    void slotOpenNewTab(const QString& path);

private slots:

    /**
     * @brief Triggered when table change it's name.
     * @param[in] title The new title.
     */
    void slotUpdateTabTitle(const QString& title);

    /**
     * @brief Triggered when user click close button.
     * @param[in] index The table index to remove.
     */
    void slotTabCloseRequest(int index);

    /**
     * @brief Triggered when user choose close from menu.
     */
    void slotTabCloseAction();

    /**
     * @brief Custom menu request.
     * @param[in] pos   Position.
     */
    void slotContextMenuRequest(const QPoint& pos);
};

} /* namespace qfcmd */

#endif // QFCMD_FS_TABWIDGET_HPP
