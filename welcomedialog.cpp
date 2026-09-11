#include "welcomedialog.h"
#include "ui_welcomedialog.h"

WelcomeDialog::WelcomeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WelcomeDialog)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("Bienvenido a Solverix"));
}

WelcomeDialog::~WelcomeDialog()
{
    delete ui;
}

WelcomeDialog::Choice WelcomeDialog::choice()
{
    return this->result;
}

void WelcomeDialog::on_quickModeButton_clicked()
{
    this->result = QuickMode;
    accept();
}

void WelcomeDialog::on_advancedButton_clicked()
{
    this->result = Advanced;
    accept();
}

void WelcomeDialog::on_practiceButton_clicked()
{
    this->result = Practice;
    accept();
}
