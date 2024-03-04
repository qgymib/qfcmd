#include <QDir>
#include <QMenu>
#include <QTabBar>
#include <QAction>
#include <QMouseEvent>

#include "fstabwidget.hpp"
#include "fsfoldertab.hpp"

qfcmd::FsTabWidget::FsTabWidget(QWidget* parent)
    : QTabWidget(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, &QTabWidget::tabCloseRequested, this, &FsTabWidget::slotTabCloseRequest);
    connect(this, &QWidget::customContextMenuRequested, this, &FsTabWidget::slotContextMenuRequest);

    slotOpenNewTab(QDir::homePath());
}

qfcmd::FsTabWidget::~FsTabWidget()
{
}

void qfcmd::FsTabWidget::mousePressEvent(QMouseEvent *event)
{
    int tab_idx = tabBar()->tabAt(event->pos());
    if (tab_idx < 0)
    {
        return;
    }

    if (event->button() != Qt::MiddleButton || tab_idx < 0)
    {
        QTabWidget::mousePressEvent(event);
        return;
    }

    event->accept();
    slotTabCloseRequest(tab_idx);
}

void qfcmd::FsTabWidget::slotUpdateTabTitle(const QString& title)
{
    QWidget* widget = qobject_cast<QWidget*>(sender());
    if (widget == NULL)
    {
        return;
    }

    int idx = indexOf(widget);
    if (idx < 0)
    {
        return;
    }

    setTabText(idx, title);
}

void qfcmd::FsTabWidget::slotTabCloseRequest(int index)
{
    if (count() <= 1)
    {
        return;
    }
    removeTab(index);
}

void qfcmd::FsTabWidget::slotOpenNewTab(const QString& path)
{
    qfcmd::FolderTab* tab = new qfcmd::FolderTab(path);

    int idx = QTabWidget::addTab(tab, tab->windowTitle());
    connect(tab, &QWidget::windowTitleChanged, this, &FsTabWidget::slotUpdateTabTitle);
    connect(tab, &qfcmd::FolderTab::signalOpenInNewTab, this, &FsTabWidget::slotOpenNewTab);
    setCurrentIndex(idx);
}

void qfcmd::FsTabWidget::slotContextMenuRequest(const QPoint &pos)
{
    int idx = tabBar()->tabAt(pos);
    if (idx < 0)
    {
        return;
    }

    QMenu* menu = new QMenu(this);

    if (count() > 1)
    {
        QAction* act = menu->addAction("Close", this, &FsTabWidget::slotTabCloseAction);
        act->setData(idx);
    }

    menu->exec(tabBar()->mapToGlobal(pos));
}

void qfcmd::FsTabWidget::slotTabCloseAction()
{
    QAction *act = qobject_cast<QAction *>(sender());
    int idx = act->data().toInt();
    slotTabCloseRequest(idx);
}
