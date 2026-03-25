#ifndef MATERIALSETTING_H
#define MATERIALSETTING_H

#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QListWidget>
#include <QDialog>
#include <QTextEdit>
#include "datastructures.h"
namespace Ui {
class MaterialSetting;
}


class MaterialSetting : public QDialog
{
    Q_OBJECT

public:
    explicit MaterialSetting(QWidget *parent = nullptr);
    ~MaterialSetting();

    // 加载材质数据到弹窗
    void setMaterialData(Material &mat, const QList<Component> &componentNames);
    // 获取编辑后的材质数据
    void getMaterialData(Material &mat);
    void setAllMaterials(const QList<Material> &mats) { allMaterials = mats; }
    void setEditingMode(bool isEdit);
private:
    Ui::MaterialSetting *ui;

    // 来源类型常量
    const QString SOURCE_NONE = "none";
    const QString SOURCE_USER = "user";
    const QString SOURCE_TOOL = "tool";
    const QString SOURCE_CODE = "code";

    // 新增：独立Alpha贴图控件
    QCheckBox *chkAlphaTexture;        // 使用独立Alpha贴图
    QLineEdit *editAlphaTexture;       // Alpha贴图名称输入

    QComboBox *comboMatName;  // 下拉选择组件材质
    QComboBox *comboComponent;
    QCheckBox *chkHasDiff;
    QComboBox *comboDiff;

    QLineEdit *editFilename;

    QLineEdit *editLightwarp;
    QComboBox *comboLightwarpSource;

    QLineEdit *editBumpmap;
    QComboBox *comboBumpmapSource;

    QLineEdit *editEnvmap;
    QComboBox *comboEnvmapSource;

    QCheckBox *chkAlpha;
    QCheckBox *chkNocull;
    QCheckBox *chkNodecal;
    QCheckBox *chkSelfillum;
    QCheckBox *chkHalflambert;
    QCheckBox *chkPhong;
    QCheckBox *chkTranslucent;
    QCheckBox *chkAlphatest;

    QTextEdit *vmtCustomEdit;
    QCheckBox *chkUseCustomVmt;

    QList<Component> tempComponents;  //暂时存储组件列表用于查询差分
    QList<Material> allMaterials;//存储所有材质
    QString currentMaterialId;  // 当前正在编辑的材质ID

private slots:
    void onComponentChanged(int index);  // 组件选择变化
    void onHasDiffChanged(bool checked);  // 是否有差分改变
    void onMatNameChanged(int index);

    void onLightwarpSourceChanged(int index);
    void onBumpmapSourceChanged(int index);
    void onEnvmapSourceChanged(int index);

    // 新增：Alpha贴图相关槽函数
    void onAlphaTextureToggled(bool checked);
    void onAlphaHighlightToggled(bool checked);  // 需要连接chkAlpha的信号
};

#endif // MATERIALSETTING_H
