#include <QAbstractTableModel>

#include "keyboardshortcutsform.hpp"
#include "ui_keyboardshortcutsform.h"

KeyboardShortcutsForm::KeyboardShortcutsForm(const KeyboardShortcuts::ShortcutMap& shortcuts,
    QWidget *parent) : QTableView(parent) , ui(new Ui::KeyboardShortcutsForm)
{
    ui->setupUi(this);

    KeyboardShortcuts* model = new KeyboardShortcuts(shortcuts, this);
    setModel(model);
}

KeyboardShortcutsForm::~KeyboardShortcutsForm()
{
    delete ui;
}
