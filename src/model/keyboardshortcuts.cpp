#include "keyboardshortcuts.hpp"

KeyboardShortcuts::KeyboardShortcuts(const ShortcutMap& shortcuts, QObject *parent)
    : QAbstractTableModel(parent)
{
    m_headers << tr("Command") << tr("Shortcut");

    foreach(const QString& key, shortcuts.keys())
    {
        m_shortcuts << ShortcutPair{key, shortcuts[key]};
    }
}

QVariant KeyboardShortcuts::headerData(int section,
                        Qt::Orientation orientation,
                        int role) const
{
    if (orientation != Qt::Horizontal)
    {
        return QVariant();
    }

    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    if (section >= m_headers.size())
    {
        return QVariant();
    }
    return m_headers[section];
}

int KeyboardShortcuts::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_shortcuts.size();
}

int KeyboardShortcuts::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_headers.size();
}

QVariant KeyboardShortcuts::data(const QModelIndex &index, int role) const
{
    (void)role;
    if (!index.isValid())
        return QVariant();

    struct ShortcutPair item = m_shortcuts[index.row()];
    if (index.column() == 0)
    {
        return item.text;
    }
    return item.shortcut.toString();
}

bool KeyboardShortcuts::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value) {
        // FIXME: Implement me!
        emit dataChanged(index, index, {role});
        return true;
    }
    return false;
}

Qt::ItemFlags KeyboardShortcuts::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable; // FIXME: Implement me!
}
