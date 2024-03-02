#include <QDir>

#include "fstabwidget.hpp"
#include "fsfoldertab.hpp"

qfcmd::FsTabWidget::FsTabWidget(QWidget* parent)
    : QTabWidget(parent)
{
    connect(this, &QTabWidget::tabCloseRequested, this, &FsTabWidget::slotTabCloseRequest);

    addTab(QDir::homePath());
}

qfcmd::FsTabWidget::~FsTabWidget()
{
}

void qfcmd::FsTabWidget::addTab(const QString& path)
{
    qfcmd::FolderTab* tab = new qfcmd::FolderTab(path);

    QTabWidget::addTab(tab, tab->windowTitle());
    connect(tab, &QWidget::windowTitleChanged, this, &FsTabWidget::slotUpdateTabTitle);

    setTabsClosable(count() > 1);
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
