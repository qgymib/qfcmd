#include <QApplication>
#include <QDialogButtonBox>
#include <QPushButton>
#include "settingsdialog.hpp"

using namespace qfcmd;

typedef QList<IOptionPage*> OptionPageList;

/**
 * @brief The list of all option pages.
 */
static OptionPageList s_option_pages;

IOptionPage::IOptionPage()
{
    s_option_pages.append(this);
}

IOptionPage::~IOptionPage()
{
    s_option_pages.removeOne(this);
}

SettingsDialog::SettingsDialog(const QString& title, QWidget* parent)
    : QDialog(parent)
    , m_filter_edit(new QLineEdit)
    , m_list_view(new QListView)
    , m_header_label(new QLabel)
    , m_stacked_layout(new QStackedLayout)
{
    createUi();
    setWindowTitle(title);
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::createUi()
{
    QGridLayout* main_layout = new QGridLayout;

    main_layout->addWidget(m_filter_edit, 0, 0, 1, 1);

    {
        QHBoxLayout* header_layout = new QHBoxLayout;
        int left_margin = QApplication::style()->pixelMetric(QStyle::PM_LayoutLeftMargin);
        header_layout->addSpacerItem(new QSpacerItem(left_margin, 0, QSizePolicy::Fixed, QSizePolicy::Ignored));
        header_layout->addWidget(m_header_label);
        main_layout->addLayout(header_layout, 0, 1, 1, 1);
    }
    
    main_layout->addWidget(m_list_view, 1, 0, 1, 1);

    {
        m_stacked_layout->setContentsMargins(0, 0, 0, 0);
        m_stacked_layout->addWidget(new QWidget(this));
        main_layout->addLayout(m_stacked_layout, 1, 1, 2, 1);
    }

    {
        QDialogButtonBox* button_box = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        button_box->button(QDialogButtonBox::Ok)->setDefault(true);
        connect(button_box->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, this, &SettingsDialog::action_apply);
        connect(button_box, &QDialogButtonBox::accepted, this, &SettingsDialog::action_accept);
        connect(button_box, &QDialogButtonBox::rejected, this, &SettingsDialog::action_reject);

        main_layout->addWidget(button_box, 3, 0, 1, 2);
    }

    main_layout->setColumnStretch(1, 4);
    main_layout->setSizeConstraint(QLayout::SetMinimumSize);
    setLayout(main_layout);
}

void SettingsDialog::action_apply()
{
}

void SettingsDialog::action_accept()
{
}

void SettingsDialog::action_reject()
{
}
