#include <QDir>
#include <QVariant>
#include <QAction>
#include <QApplication>
#include <QHeaderView>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QSplitter>
#include <QStatusBar>
#include <QToolBar>
#include <QSettings>
#include <QShortcut>

#include "mainwindow.hpp"
#include "aboutdialog.hpp"
#include "perferencesdialog.hpp"
#include "keyboardshortcutsform.hpp"
#include "fstabwidget.hpp"
#include "fstreeview.hpp"
#include "fcmdshortcutmanager.hpp"
#include "settings.hpp"

namespace qfcmd {
struct MainWindowInner
{
    MainWindowInner(MainWindow* parent);
    ~MainWindowInner();

    MainWindow*             parent;

    QAction*                actionAbout;
    QAction*                actionShowToolbar;
    QAction*                actionPerferences;
    QSplitter*              centralwidget;
    qfcmd::FsTreeView*      treeView;
    QSplitter*              pannel;
    qfcmd::FsTabWidget*     panel_0;
    qfcmd::FsTabWidget*     panel_1;
    QMenuBar*               menubar;
    QMenu*                  menuHelp;
    QMenu*                  menuView;
    QMenu*                  menuEdit;
    QStatusBar*             statusbar;
    QToolBar*               toolBar;

    QShortcut*              scCloseTab;
    QShortcut*              scDuplicateTab;

    QSettings*              settings;
    FcmdShortCutManager*    shortcut_mgr;
};
} /* namespace qfcmd */

static void _mainwindow_setupActions(qfcmd::MainWindowInner* inner)
{
	/* Register all known actions. */
	inner->shortcut_mgr->regAction(inner->actionPerferences, inner->parent, &qfcmd::MainWindow::actionPerferences);
	inner->shortcut_mgr->regAction(inner->actionShowToolbar, inner->parent, &qfcmd::MainWindow::actionShowToolbarChange);
	inner->shortcut_mgr->regAction(inner->actionAbout, inner->parent, &qfcmd::MainWindow::actionAboutDialog);
}

/**
 * @brief Get the current activated panel.
 */
static qfcmd::FsTabWidget* _get_activate_panel(qfcmd::MainWindowInner* inner)
{
    QWidget* w = QApplication::focusWidget();
    while (w != nullptr)
    {
        if (w == inner->panel_0)
        {
            break;
        }
        else if (w == inner->panel_1)
        {
            break;
        }

        w = qobject_cast<QWidget*>(w->parent());
    }

    if (w == nullptr)
    {
        return nullptr;
    }

    return qobject_cast<qfcmd::FsTabWidget*>(w);
}

qfcmd::MainWindowInner::MainWindowInner(MainWindow* parent)
{
    this->parent = parent;

	actionAbout = new QAction(parent);
	actionShowToolbar = new QAction(parent);
	actionShowToolbar->setCheckable(true);
	actionPerferences = new QAction(parent);
	centralwidget = new QSplitter(parent);
	centralwidget->setOrientation(Qt::Horizontal);
	treeView = new qfcmd::FsTreeView(centralwidget);
	centralwidget->addWidget(treeView);
	pannel = new QSplitter(centralwidget);
	pannel->setOrientation(Qt::Horizontal);
    panel_0 = new qfcmd::FsTabWidget(pannel,
                                     qfcmd::Settings::get<QStringList>(qfcmd::Settings::TABS_PANEL_0),
                                     qfcmd::Settings::get<int>(qfcmd::Settings::TABS_PANEL_0_ACTIVATE),
                                     [](const QStringList& tabs, int idx){
                                         qfcmd::Settings::set(qfcmd::Settings::TABS_PANEL_0, tabs);
                                         qfcmd::Settings::set(qfcmd::Settings::TABS_PANEL_0_ACTIVATE, idx);
                                     });
    panel_0->setMovable(true);
    pannel->addWidget(panel_0);
    panel_1 = new qfcmd::FsTabWidget(pannel,
                                     qfcmd::Settings::get<QStringList>(qfcmd::Settings::TABS_PANEL_1),
                                     qfcmd::Settings::get<int>(qfcmd::Settings::TABS_PANEL_1_ACTIVATE),
                                     [](const QStringList& tabs, int idx){
                                         qfcmd::Settings::set(qfcmd::Settings::TABS_PANEL_1, tabs);
                                         qfcmd::Settings::set(qfcmd::Settings::TABS_PANEL_1_ACTIVATE, idx);
                                     });
    panel_1->setMovable(true);
    pannel->addWidget(panel_1);
	centralwidget->addWidget(pannel);
	parent->setCentralWidget(centralwidget);
	menubar = new QMenuBar(parent);
	menubar->setGeometry(QRect(0, 0, 1366, 21));
	menuHelp = new QMenu(menubar);
	menuView = new QMenu(menubar);
	menuEdit = new QMenu(menubar);
    parent->setMenuBar(menubar);
	statusbar = new QStatusBar(parent);
    parent->setStatusBar(statusbar);
	toolBar = new QToolBar(parent);
    parent->addToolBar(Qt::TopToolBarArea, toolBar);

	menubar->addAction(menuEdit->menuAction());
	menubar->addAction(menuView->menuAction());
	menubar->addAction(menuHelp->menuAction());
	menuHelp->addAction(actionAbout);
	menuView->addAction(actionShowToolbar);
	menuEdit->addAction(actionPerferences);

	centralwidget->setStretchFactor(1, 1);

    parent->setWindowTitle(QCoreApplication::translate("MainWindow", "File Commander", nullptr));
	actionAbout->setText(QCoreApplication::translate("MainWindow", "About", nullptr));
	actionShowToolbar->setText(QCoreApplication::translate("MainWindow", "Show Toolbar", nullptr));
	actionPerferences->setText(QCoreApplication::translate("MainWindow", "Preferences", nullptr));
	menuHelp->setTitle(QCoreApplication::translate("MainWindow", "Help", nullptr));
	menuView->setTitle(QCoreApplication::translate("MainWindow", "View", nullptr));
	menuEdit->setTitle(QCoreApplication::translate("MainWindow", "Edit", nullptr));
	toolBar->setWindowTitle(QCoreApplication::translate("MainWindow", "toolBar", nullptr));

    parent->resize(1366, 768);

    scCloseTab = new QShortcut(qfcmd::Settings::get<QKeySequence>(qfcmd::Settings::CLOSE_TAB),
                               parent);
    scDuplicateTab = new QShortcut(qfcmd::Settings::get<QKeySequence>(qfcmd::Settings::DUPLICATE_TAB),
                                   parent);

    settings = new QSettings;
    shortcut_mgr = new FcmdShortCutManager(parent);
}

qfcmd::MainWindowInner::~MainWindowInner()
{
    delete settings;
    delete shortcut_mgr;
}

qfcmd::MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_inner(new MainWindowInner(this))
{
    _mainwindow_setupActions(m_inner);
    setWindowIcon(QIcon(":/app.ico"));

    connect(m_inner->scCloseTab, &QShortcut::activated, this, &MainWindow::slotShortcutCloseTab);
    connect(m_inner->scDuplicateTab, &QShortcut::activated, this, &MainWindow::slotShortcutDuplicateTab);

    {
        m_inner->treeView->slotChangeDirectory(QDir::currentPath());
    }

    {
        bool visible = m_inner->settings->value("view/ShowToolbar", true).toBool();
        m_inner->toolBar->setVisible(visible);
        m_inner->actionShowToolbar->setChecked(visible);
    }

    /* By default we focus on panel_0 */
    m_inner->panel_0->setFocus();
}

qfcmd::MainWindow::~MainWindow()
{
    delete m_inner;
}

void qfcmd::MainWindow::actionAboutDialog()
{
    AboutDialog dialog(this);
    dialog.exec();
}

void qfcmd::MainWindow::actionShowToolbarChange()
{
    bool need_visible = !m_inner->toolBar->isVisible();

    m_inner->settings->setValue("view/ShowToolbar", need_visible);

    m_inner->actionShowToolbar->setChecked(need_visible);
    m_inner->toolBar->setVisible(need_visible);
}

void qfcmd::MainWindow::actionPerferences()
{
    PerferencesDialog dialog(this);

    dialog.addConfigWidget(new KeyboardShortcutsForm(m_inner->shortcut_mgr->getShortcutMap()));

    dialog.exec();
}

void qfcmd::MainWindow::slotShortcutCloseTab()
{
    qfcmd::FsTabWidget* tabWidget =_get_activate_panel(m_inner);
    if (tabWidget == nullptr)
    {
        return;
    }
    tabWidget->closeCurrentActivateTab();
}

void qfcmd::MainWindow::slotShortcutDuplicateTab()
{
    qfcmd::FsTabWidget* tabWidget =_get_activate_panel(m_inner);
    if (tabWidget == nullptr)
    {
        return;
    }
    tabWidget->duplicateCurrentActivateTab();
}
