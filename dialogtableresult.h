#ifndef DIALOGTABLERESULT_H
#define DIALOGTABLERESULT_H

#include <QDialog>

namespace Ui {
class DialogTableResult;
}

class DialogTableResult : public QDialog
{
    Q_OBJECT

public:
    explicit DialogTableResult(QWidget *parent = nullptr);
    ~DialogTableResult();
    void SetLabelText(QString labelText);


private:
    Ui::DialogTableResult *ui;

signals:
    void createTableReport();


};

#endif // DIALOGTABLERESULT_H
