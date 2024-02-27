#include "fcmdshortcutmanager.hpp"

FcmdShortCutManager::FcmdShortCutManager(QWidget *parent)
    : QObject{parent}
{
    m_parent = parent;
}

FcmdShortCutManager::~FcmdShortCutManager()
{
}

void FcmdShortCutManager::setShortcut(const QString& text, const QKeySequence& shortcut)
{
    FcmdActionMap::iterator it = m_actions.find(text);
    if (it == m_actions.end())
    {
        QErrorMessage dialog(m_parent);
        dialog.showMessage("Shortcut not found");
        dialog.exec();
        return;
    }

    QAction* action = it.value();
    action->setShortcut(shortcut);
}

KeyboardShortcuts::ShortcutMap FcmdShortCutManager::getShortcutMap()
{
    KeyboardShortcuts::ShortcutMap record;

    foreach (QAction* action, m_actions)
    {
        record[action->text()] = action->shortcut();
    }

    return record;
}
