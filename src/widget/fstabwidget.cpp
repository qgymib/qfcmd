#include <QDir>

#include "fstabwidget.hpp"
#include "fsfoldertab.hpp"

qfcmd::FsTabWidget::FsTabWidget(QWidget* parent)
    : QTabWidget(parent)
{
    connect(this, &QTabWidget::tabCloseRequested, this, &FsTabWidget::slotTabCloseRequest);

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
    removeTab(index);
    setTabsClosable(count() > 1);
}

void qfcmd::FsTabWidget::slotOpenNewTab(const QString& path)
{
    qfcmd::FolderTab* tab = new qfcmd::FolderTab(path);

    int idx = QTabWidget::addTab(tab, tab->windowTitle());
    connect(tab, &QWidget::windowTitleChanged, this, &FsTabWidget::slotUpdateTabTitle);
    connect(tab, &qfcmd::FolderTab::signalOpenInNewTab, this, &FsTabWidget::slotOpenNewTab);
    setCurrentIndex(idx);

    setTabsClosable(count() > 1);
}
