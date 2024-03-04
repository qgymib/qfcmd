#include <QDir>
#include <QToolButton>
#include <QLabel>
#include <QTabBar>

#include "fstabwidget.hpp"
#include "fsfoldertab.hpp"

qfcmd::FsTabWidget::FsTabWidget(QWidget* parent)
    : QTabWidget(parent)
{
    connect(this, &QTabWidget::tabCloseRequested, this, &FsTabWidget::slotTabCloseRequest);

    addTab(new QLabel("+"), QString("+"));
    connect(this, &QTabWidget::currentChanged, this, &qfcmd::FsTabWidget::slotTabBarChangeTab);

    slotOpenNewTab(QDir::homePath());
}

qfcmd::FsTabWidget::~FsTabWidget()
{
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
    if (index == count() - 1)
    {
        return;
    }
    removeTab(index);
    setTabsClosable(count() > 1);
}

void qfcmd::FsTabWidget::slotOpenNewTab(const QString& path)
{
    int position = count() - 1;
    qfcmd::FolderTab* tab = new qfcmd::FolderTab(path);

    int idx = QTabWidget::insertTab(position, tab, tab->windowTitle());
    connect(tab, &QWidget::windowTitleChanged, this, &FsTabWidget::slotUpdateTabTitle);
    connect(tab, &qfcmd::FolderTab::signalOpenInNewTab, this, &FsTabWidget::slotOpenNewTab);
    setCurrentIndex(idx);

    setTabsClosable(count() > 1);
}

void qfcmd::FsTabWidget::slotTabBarChangeTab(int index)
{
    if (index == count() - 1)
    {
        slotOpenNewTab(QDir::homePath());
    }
}
