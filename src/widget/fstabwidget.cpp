#include <QDir>
#include <QMenu>
#include <QTabBar>
#include <QAction>
#include <QMouseEvent>

#include "fstabwidget.hpp"
#include "fsfoldertab.hpp"
#include "settings.hpp"

namespace qfcmd {
struct FsTabWidgetInner
{
    FsTabWidgetInner(FsTabWidget* parent);
    ~FsTabWidgetInner();

    FsTabWidget*                                    parent;
    std::function<void(const QStringList&, int)>    cb;
};
} /* namespace qfcmd */

qfcmd::FsTabWidgetInner::FsTabWidgetInner(FsTabWidget *parent)
{
    this->parent = parent;
}

qfcmd::FsTabWidgetInner::~FsTabWidgetInner()
{
}

qfcmd::FsTabWidget::FsTabWidget(QWidget* parent,
                                const QStringList& paths,
                                int activate_idx,
                                std::function<void(const QStringList&, int)> cb)
    : QTabWidget(parent)
    , m_inner(new FsTabWidgetInner(this))
{
    m_inner->cb = cb;
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, &QTabWidget::tabCloseRequested, this, &FsTabWidget::slotTabCloseRequest);
    connect(this, &QWidget::customContextMenuRequested, this, &FsTabWidget::slotContextMenuRequest);

    for(QString path : paths)
    {
        slotOpenNewTab(path);
    }
    if (paths.size() == 0)
    {
        slotOpenNewTab(QDir::homePath());
    }

    setCurrentIndex(activate_idx);
}

qfcmd::FsTabWidget::~FsTabWidget()
{
    QStringList tabs;

    for (int i = 0; i < count(); i++)
    {
        qfcmd::FolderTab* tab = qobject_cast<qfcmd::FolderTab*>(widget(i));
        if (tab == nullptr)
        {
            continue;
        }

        tabs << tab->path();
    }

    m_inner->cb(tabs, currentIndex());

    delete m_inner;
}

void qfcmd::FsTabWidget::closeCurrentActivateTab()
{
    int idx = currentIndex();
    slotTabCloseRequest(idx);
}

void qfcmd::FsTabWidget::duplicateCurrentActivateTab()
{
    qfcmd::FolderTab* tab = qobject_cast<qfcmd::FolderTab*>(currentWidget());
    const QString path = tab->path();
    slotOpenNewTab(path);
}

void qfcmd::FsTabWidget::goBack()
{
    qfcmd::FolderTab* tab = qobject_cast<qfcmd::FolderTab*>(currentWidget());
    tab->slotGoBack();
}

void qfcmd::FsTabWidget::goForward()
{
    qfcmd::FolderTab* tab = qobject_cast<qfcmd::FolderTab*>(currentWidget());
    tab->slotGoForward();
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

    /*
     * Re-focus on this widget, so user can press Ctrl+W again to close other tabs.
     */
    setFocus();
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
    qfcmd::FolderTab* tab = qobject_cast<qfcmd::FolderTab*>(widget(idx));

    QMenu* menu = new QMenu(this);
    if (tab != nullptr)
    {
        QAction* act = menu->addAction(tr("Duplicate tab"), this, &FsTabWidget::slotDuplicateSelectedTab);
        act->setShortcut(qfcmd::Settings::get<QKeySequence>(qfcmd::Settings::SHORTCUT_DUPLICATE_TAB));
        act->setData(tab->path());
    }

    if (count() > 1)
    {
        QAction* act = menu->addAction(tr("Close"), this, &FsTabWidget::slotTabCloseAction);
        act->setShortcut(qfcmd::Settings::get<QKeySequence>(qfcmd::Settings::SHORTCUT_CLOSE_TAB));
        act->setData(idx);
    }

    menu->exec(tabBar()->mapToGlobal(pos));
}

void qfcmd::FsTabWidget::slotDuplicateSelectedTab()
{
    QAction* act = qobject_cast<QAction *>(sender());
    QString path = act->data().toString();
    slotOpenNewTab(path);
}

void qfcmd::FsTabWidget::slotTabCloseAction()
{
    QAction* act = qobject_cast<QAction *>(sender());
    int idx = act->data().toInt();
    slotTabCloseRequest(idx);
}
