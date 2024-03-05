#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>

namespace qfcmd {

struct MainWindowInner;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void actionAboutDialog();
    void actionShowToolbarChange();
    void actionPerferences();

    /**
     * @brief Close current tab.
     * @see qfcmd::Settings::CLOSE_CURRENT_TAB
     */
    void slotCloseCurrentTab();

private:
    MainWindowInner*    m_inner;
};

} /* namespace qfcmd */

#endif // MAINWINDOW_H
