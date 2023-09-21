#include "dialogtableresult.h"
#include "ui_dialogtableresult.h"

DialogTableResult::DialogTableResult(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogTableResult)
{
    ui->setupUi(this);

    connect(ui->buttonTableResultExit,SIGNAL(clicked()),this,SLOT(close()));
    connect(ui->buttonTableResultSave,QPushButton::clicked,[&](){emit createTableReport();});

}
//============================================================================
DialogTableResult::~DialogTableResult()
{
    delete ui;
}

//============================================================================
void DialogTableResult::SetLabelText(QString labelText)
{
    ui->labelTableResult->setText(labelText);
}
//============================================================================
