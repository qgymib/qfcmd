#include "perferencesdialog.hpp"
#include "ui_perferencesdialog.h"

PerferencesDialog::PerferencesDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PerferencesDialog)
{
    ui->setupUi(this);
    connect(ui->listWidget, &QListWidget::currentItemChanged, this, &PerferencesDialog::changePage);
}

PerferencesDialog::~PerferencesDialog()
{
    delete ui;
}

void PerferencesDialog::addConfigWidget(QWidget* widget)
{
    QListWidgetItem* item = new QListWidgetItem;
    item->setText(widget->windowTitle());
    item->setTextAlignment(Qt::AlignHCenter);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    ui->listWidget->addItem(item);

    ui->stackedWidget->addWidget(widget);
}

void PerferencesDialog::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (current == nullptr)
    {
        current = previous;
    }
    ui->stackedWidget->setCurrentIndex(ui->listWidget->row(current));
}
