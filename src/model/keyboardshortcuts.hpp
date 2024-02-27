#ifndef KEYBOARDSHORTCUTS_H
#define KEYBOARDSHORTCUTS_H

#include <QAbstractTableModel>
#include <QMap>
#include <QList>
#include <QKeySequence>

class KeyboardShortcuts : public QAbstractTableModel
{
    Q_OBJECT

public:
    typedef QMap<QString, QKeySequence> ShortcutMap;

public:
    explicit KeyboardShortcuts(const ShortcutMap& shortcuts, QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    struct ShortcutPair
    {
        QString         text;
        QKeySequence    shortcut;
    };
    typedef QList<struct ShortcutPair> ShortcutVec;

private:
    QStringList m_headers;
    ShortcutVec m_shortcuts;
};

#endif // KEYBOARDSHORTCUTS_H
