#include "fcmdtabwidget.hpp"

FcmdTabWidget::FcmdTabWidget(QWidget* parent)
    : QTabWidget(parent)
{
    connect(this, &QTabWidget::tabCloseRequested, this, &FcmdTabWidget::onTabCloseRequest);
}

FcmdTabWidget::~FcmdTabWidget() {}

void FcmdTabWidget::addTab(QWidget* widget, const QString& title)
{
    QTabWidget::addTab(widget, title);
    connect(widget, &QWidget::windowTitleChanged, this, &FcmdTabWidget::onUpdateTabTitle);

    setTabsClosable(count() > 1);
}

void FcmdTabWidget::onUpdateTabTitle(const QString& title)
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

void FcmdTabWidget::onTabCloseRequest(int index)
{
    removeTab(index);
    setTabsClosable(count() > 1);
}
