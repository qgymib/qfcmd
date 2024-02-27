#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QSettings>

#include "fcmdshortcutmanager.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void setupActions();

private slots:
    void actionAboutDialog();
    void actionShowToolbarChange();
    void actionPerferences();

private:
    Ui::MainWindow*     ui;
    QSettings           settings;
    FcmdShortCutManager m_shortcut_mgr;
};
#endif // MAINWINDOW_H
