#include "fcmdtabwidget.hpp"

FcmdTabWidget::FcmdTabWidget(QWidget* parent) : QTabWidget(parent) {}

FcmdTabWidget::~FcmdTabWidget() {}

void FcmdTabWidget::addTab(QWidget* widget, const QString& title)
{
    QTabWidget::addTab(widget, title);
    connect(widget, &QWidget::windowTitleChanged, this, &FcmdTabWidget::onUpdateTabTitle);
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
