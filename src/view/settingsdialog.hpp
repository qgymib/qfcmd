#ifndef FCMD_SETTINGSDIALOG_HPP
#define FCMD_SETTINGSDIALOG_HPP

#include <QDialog>
#include <QLabel>
#include <QListView>
#include <QStackedLayout>
#include <QLineEdit>

namespace qfcmd
{

class IOptionPage
{
    Q_DISABLE_COPY_MOVE(IOptionPage);

public:
    IOptionPage();
    virtual ~IOptionPage();
};

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(const QString& title, QWidget* parent = nullptr);
    virtual ~SettingsDialog();

private:
    void createUi();

private:
    void action_apply();
    void action_accept();
    void action_reject();

private:
    QLineEdit*          m_filter_edit;
    QListView*          m_list_view;
    QLabel*             m_header_label;
    QStackedLayout*     m_stacked_layout;
};

};

#endif // FCMDSETTINGSDIALOG_HPP
