#include "componentsetting.h"
#include "diffconfigdialog.h"  // 顶部加这个
#include "ui_componentsetting.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QInputDialog>
#include <QMessageBox>
#include <QUuid>

Componentsetting::Componentsetting(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Componentsetting)
{
    ui->setupUi(this);

    setWindowTitle("添加组件");
    setFixedSize(400,300);

    //代码创建控件
    editName = new QLineEdit(this);
    listMaterials = new QListWidget(this);
    btnAdd = new QPushButton("添加材质",this);
    btnEditName = new QPushButton("修改中文名", this);  // 新增
    btnEditVmt = new QPushButton("修改vmtName", this);   // 新增
    btnEdit = new QPushButton("配置差分",this);
    btnDelete = new QPushButton("删除",this);

    //竖直布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    //组件名水平分布
    QHBoxLayout *nameLayout = new QHBoxLayout();
    nameLayout->addWidget(new QLabel("组件名:"));
    nameLayout->addWidget(editName);
    mainLayout->addLayout(nameLayout);

    //材质列表
    mainLayout->addWidget(new QLabel("包含材质:"));
    mainLayout->addWidget(listMaterials);

    //按钮水平分布
    QHBoxLayout *btnLayout1 = new QHBoxLayout();
    btnLayout1->addWidget(btnAdd);
    btnLayout1->addWidget(btnEditName);
    btnLayout1->addWidget(btnEditVmt);

    QHBoxLayout *btnLayout2 = new QHBoxLayout();
    btnLayout2->addWidget(btnEdit);
    btnLayout2->addWidget(btnDelete);

    mainLayout->addLayout(btnLayout1);
    mainLayout->addLayout(btnLayout2);

    //确定取消按钮，连接本页面的ok和取消
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);

    //按钮信号连接
    connect(btnAdd, &QPushButton::clicked, this,&Componentsetting::onAddMaterial);
    connect(btnEditName, &QPushButton::clicked, this, &Componentsetting::onEditMaterialName);
    connect(btnEditVmt, &QPushButton::clicked, this, &Componentsetting::onEditMaterialVmt);
    connect(btnEdit, &QPushButton::clicked, this, &Componentsetting::onEditMaterial);
    connect(btnDelete, &QPushButton::clicked, this, &Componentsetting::onDeleteMaterial);

    //列表带复选框
    listMaterials->setSelectionMode(QAbstractItemView::SingleSelection);//选单个

}

Componentsetting::~Componentsetting()
{
    delete ui;
}

//获取本弹窗列表
QList<MaterialInComponent> Componentsetting::getMaterialList()
{
    return tempMaterials;
}


// 添加材质
void Componentsetting::onAddMaterial()
{
    QString name = QInputDialog::getText(this, "添加材质", "显示名(中文):");
    if (name.isEmpty()) return;

    QString vmtName = QInputDialog::getText(this, "添加材质", "文件名(vmtName):");
    if (vmtName.isEmpty()) return;

    // 检查中文名重复
    for (const auto &m : tempMaterials) {
        if (m.name == name) {
            QMessageBox::warning(this, "错误", "中文名已存在");
            return;
        }
    }

    // 检查英文名重复
    for (const auto &m : tempMaterials) {
        if (m.vmtName == vmtName) {
            QMessageBox::warning(this, "错误", "英文名已存在");
            return;
        }
    }

    MaterialInComponent mat;
    mat.matId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    mat.name = name;
    mat.vmtName = vmtName;
    mat.hasDiff = false;
    tempMaterials.append(mat);

    updateMaterialList();
}

//更新材质列表
void Componentsetting::updateMaterialList()
{
    listMaterials->clear();

    for (const auto& mat : tempMaterials) {
        QString displayText = mat.name + " (" + mat.vmtName + ")";

        if (mat.hasDiff) {
            displayText += QString(" - %1差分").arg(mat.diffNames.size());
        } else {
            displayText += " - 无差分";
        }

        listMaterials->addItem(displayText);
    }
}

// 修改材质中文名
void Componentsetting::onEditMaterialName()
{
    int row = listMaterials->currentRow();
    if(row < 0) {
        QMessageBox::information(this, "提示", "未选中任一项！");
        return;
    }

    MaterialInComponent &mat = tempMaterials[row];

    QString newName = QInputDialog::getText(this, "编辑中文名",
                                            "请输入材质中文名:",
                                            QLineEdit::Normal,
                                            mat.name);

    // 检查中文名重复
    for (const auto &m : tempMaterials) {
        if (m.name == newName) {
            QMessageBox::warning(this, "错误", "中文名已存在！");
            return;
        }
    }

    if(!newName.isEmpty() && newName != mat.name) {
        mat.name = newName;
        updateMaterialList();
    }
}

// 修改材质英文名
void Componentsetting::onEditMaterialVmt()
{
    int row = listMaterials->currentRow();
    if(row < 0) {
        QMessageBox::information(this, "提示", "未选中任一项！");
        return;
    }

    MaterialInComponent &mat = tempMaterials[row];

    QString newVmtName = QInputDialog::getText(this, "编辑英文名",
                                               "请输入材质英文名:",
                                               QLineEdit::Normal,
                                               mat.vmtName);

    if(!newVmtName.isEmpty() && newVmtName != mat.vmtName) {
        // 检查英文名是否重复
        bool exist = false;
        for(int i = 0; i < tempMaterials.size(); i++) {
            if(i != row && tempMaterials[i].vmtName == newVmtName) {
                exist = true;
                break;
            }
        }

        if(exist) {
            QMessageBox::warning(this, "重复警告", "该英文名已存在！");
            return;
        }

        mat.vmtName = newVmtName;
        updateMaterialList();
    }
}

// 删除勾选材质
void Componentsetting::onDeleteMaterial()
{
    int row = listMaterials->currentRow();
    if(row >= 0) {
        tempMaterials.removeAt(row);  //从数据列表中移除
        delete listMaterials->takeItem(row);  //从界面列表中移除并删除项
    }
}
// 配置差分按钮
void Componentsetting::onEditMaterial()
{
    int row = listMaterials->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "提示", "请先选中一个材质");
        return;
    }

    MaterialInComponent &mat = tempMaterials[row];

    qDebug() << "=== 打开差分配置前 ===";
    qDebug() << "材质名:" << mat.name;
    qDebug() << "vmtName:" << mat.vmtName;
    qDebug() << "当前 hasDiff:" << mat.hasDiff;
    qDebug() << "当前 diffNames:" << mat.diffNames;

    // 直接打开配置弹窗，不询问是否有差分
    DiffConfigDialog dlg(this);
    //窗口获取当前选中组件差分
    dlg.setDiffNames(mat.diffNames);

    if (dlg.exec() == QDialog::Accepted) {
        mat.diffNames = dlg.getDiffNames();
        mat.hasDiff = !mat.diffNames.isEmpty();  // 有差分就true，没有就false

        qDebug() << "=== 配置差分保存 ===";
        qDebug() << "材质名:" << mat.name;
        qDebug() << "vmtName:" << mat.vmtName;
        qDebug() << "hasDiff:" << mat.hasDiff;
        qDebug() << "diffNames:" << mat.diffNames;

        // 更新显示
        updateMaterialList();
    } else {
        qDebug() << "=== 配置差分取消 ===";
    }
}

//编辑已完成组件把数据加载对话框（window->componentsetting）
void Componentsetting::setComponentData(QString& name, QList<MaterialInComponent>& mats)
{
    editName->setText(name);
    tempMaterials = mats;
    updateMaterialList();
}


