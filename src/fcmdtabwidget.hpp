#ifndef FCMDTABWIDGET_HPP
#define FCMDTABWIDGET_HPP

#include <QTabWidget>

class FcmdTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    FcmdTabWidget(QWidget* parent = nullptr);
    virtual ~FcmdTabWidget();

public:
    void addTab(QWidget* widget, const QString& title);

public slots:
    void onUpdateTabTitle(const QString& title);
    void onTabCloseRequest(int index);
};

#endif // FCMDTABWIDGET_HPP
