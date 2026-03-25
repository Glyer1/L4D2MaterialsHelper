#ifndef DIFFCONFIGDIALOG_H
#define DIFFCONFIGDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>

namespace Ui {
class DiffConfigDialog;
}

class DiffConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DiffConfigDialog(QWidget *parent = nullptr);
    ~DiffConfigDialog();

    void setDiffNames(const QStringList &names);
    QStringList getDiffNames() const;


private slots:
    void onAddDiff();//加差分
    void onDeleteDiff();//删差分
private:
    Ui::DiffConfigDialog *ui;

    QListWidget *listDiffs;//差分列表
    QLineEdit *editDiffName;//填在editLine的差分名字
    QPushButton *btnAdd;
    QPushButton *btnDelete;
};

#endif // DIFFCONFIGDIALOG_H
