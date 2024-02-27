#ifndef KEYBOARDSHORTCUTSFORM_HPP
#define KEYBOARDSHORTCUTSFORM_HPP

#include <QTableView>
#include "model/keyboardshortcuts.hpp"

namespace Ui {
class KeyboardShortcutsForm;
}

class KeyboardShortcutsForm : public QTableView
{
    Q_OBJECT

public:
    explicit KeyboardShortcutsForm(const KeyboardShortcuts::ShortcutMap& shortcuts,
        QWidget *parent = nullptr);
    ~KeyboardShortcutsForm();

private:
    Ui::KeyboardShortcutsForm *ui;
};

#endif // KEYBOARDSHORTCUTSFORM_HPP
