#include "diffconfigdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include "ui_diffconfigdialog.h"
#include <QMessageBox>

DiffConfigDialog::DiffConfigDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DiffConfigDialog)
{
    ui->setupUi(this);

    setWindowTitle("配置差分");
    setFixedSize(300, 400);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 输入框和添加按钮
    QHBoxLayout *inputLayout = new QHBoxLayout();
    editDiffName = new QLineEdit(this);
    editDiffName->setPlaceholderText("输入差分名字");
    btnAdd = new QPushButton("添加", this);
    inputLayout->addWidget(editDiffName);
    inputLayout->addWidget(btnAdd);
    mainLayout->addLayout(inputLayout);

    // 差分列表
    mainLayout->addWidget(new QLabel("已有差分（选中删除）："));
    listDiffs = new QListWidget(this);
    listDiffs->setSelectionMode(QAbstractItemView::SingleSelection);
    mainLayout->addWidget(listDiffs);

    // 删除按钮
    btnDelete = new QPushButton("删除选中", this);
    mainLayout->addWidget(btnDelete);

    // 确定取消
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(buttonBox);

    connect(btnAdd, &QPushButton::clicked, this, &DiffConfigDialog::onAddDiff);
    connect(btnDelete, &QPushButton::clicked, this, &DiffConfigDialog::onDeleteDiff);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

DiffConfigDialog::~DiffConfigDialog()
{
    delete ui;
}

//编辑一个已有差分的材质时，调用此函数把已存在的差分名字显示在对话框中
void DiffConfigDialog::setDiffNames(const QStringList &names)
{
    listDiffs->clear();
    for (const QString &name : names) {
        listDiffs->addItem(name);
    }
}

//返回列表差分名字（所有）
QStringList DiffConfigDialog::getDiffNames() const
{
    QStringList names;
    for (int i = 0; i < listDiffs->count(); i++) {
        names << listDiffs->item(i)->text();
    }
    return names;
}

//增加差分
void DiffConfigDialog::onAddDiff()
{
    QString name = editDiffName->text().trimmed();
    if (!name.isEmpty()) {
        // 检查是否已存在（已生效）
        bool exists = false;
        for (int i = 0; i < listDiffs->count(); i++) {
            if (listDiffs->item(i)->text() == name) {
                exists = true;
                QMessageBox::information(this,"重复警告","你输入的差分名称已存在！");
                break;
            }
        }
        if (!exists) {
            listDiffs->addItem(name);
            editDiffName->clear();
        }
    }
}

void DiffConfigDialog::onDeleteDiff()
{
    int row = listDiffs->currentRow();
    if (row >= 0) {
        delete listDiffs->takeItem(row);
    }
}
