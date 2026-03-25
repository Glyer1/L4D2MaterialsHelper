//组件编辑

#ifndef COMPONENTSETTING_H
#define COMPONENTSETTING_H

#include <QDialog>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include "datastructures.h"

namespace Ui {
class Componentsetting;
}

class Componentsetting : public QDialog
{
    Q_OBJECT

public:
    explicit Componentsetting(QWidget *parent = nullptr);
    ~Componentsetting();

    //获取当前编辑组件的材质列表
    QList<MaterialInComponent> getMaterialList();
    //获取组件名字
    QString getComponentName() const {return editName->text();}//新建，编辑组件的时候获取用户输入组件名
    void setComponentData(QString& name, QList<MaterialInComponent>& mats);//编辑已加入的组件信息
private:
    Ui::Componentsetting *ui;
    void updateMaterialList();//刷新材质列表

private slots:
    void onAddMaterial();      // 添加按钮
    void onDeleteMaterial();   // 删除按钮
    void onEditMaterial();   // 差分配置按钮
    void onEditMaterialName(); // 修改中文名按钮
    void onEditMaterialVmt();  // 修改英文名按钮

private:
    QPushButton* btnAdd;
    QPushButton* btnDelete;
    QPushButton* btnEdit;
    QPushButton* btnEditName;  // 修改中文名
    QPushButton* btnEditVmt;   // 修改英文名
    QLineEdit* editName;//材质名字
    QListWidget* listMaterials;//正在编辑的材质列表
    QList<MaterialInComponent> tempMaterials;//临时存储材质配置，编辑时候的材质列表
};

#endif // COMPONENTSETTING_H
