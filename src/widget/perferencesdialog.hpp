#ifndef PERFERENCESDIALOG_HPP
#define PERFERENCESDIALOG_HPP

#include <QDialog>
#include <QListWidget>

namespace Ui {
class PerferencesDialog;
}

class PerferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PerferencesDialog(QWidget *parent = nullptr);
    virtual ~PerferencesDialog();

public:
    /**
     * Adds a configuration widget to the preferences dialog.
     *
     * @param widget the widget to be added
     *
     * @return void
     *
     * @throws None
     */
    void addConfigWidget(QWidget* widget);

private slots:
    void changePage(QListWidgetItem *current, QListWidgetItem *previous);

private:
    Ui::PerferencesDialog *ui;
};

#endif // PERFERENCESDIALOG_HPP
