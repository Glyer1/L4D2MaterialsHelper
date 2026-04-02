#ifndef MATERIALSETTING_H
#define MATERIALSETTING_H

#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QListWidget>
#include <QDialog>
#include <QTextEdit>
#include "datastructures.h"
#include <QLabel>
namespace Ui {
class MaterialSetting;
}


class MaterialSetting : public QDialog
{
    Q_OBJECT

public:
    explicit MaterialSetting(QWidget *parent = nullptr);
    ~MaterialSetting();

    // 获取编辑后的材质数据,没用了
    //void getMaterialData(Material &mat);
    void setAllMaterials(const QList<Material> &mats) { allMaterials = mats; }

    //设置材质设置的信息，代替setMaterialData方法
    void setMaterialGroup(const QString &component,
                      const QString &vmtName,
                      const QList<Material*> &diffList,
                          const QList<Component> &comps);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

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
    QLabel *labApplyToAll;
    QPushButton *btnApplyToAll;//这个是强制应用预设到差分的

    QLineEdit *editFilename;

    QLineEdit *editLightwarp;
    QComboBox *comboLightwarpSource;

    QLineEdit *editBumpmap;
    QComboBox *comboBumpmapSource;

    QLineEdit *editEnvmap;
    QComboBox *comboEnvmapSource;

    QComboBox *comboBaseSource;

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

    QList<Material*> currentDiffList;//当前材质组所有差分，指针，可以改值
    Material* currentMaterial;//当前正在编辑的差分，指针，可以改值
    QString currentComponent;
    QString currentVmtName;

    QComboBox* comboDiffSelect;//差分下拉框
    QComboBox* comboVmtTemplate;//vmt内容

    QPushButton *btnBrowseFilename;
    QPushButton *btnBrowseLightwarp;
    QPushButton *btnBrowseBumpmap;
    QPushButton *btnBrowseEnvmap;
    QPushButton *btnBrowseAlphaTexture;  // 独立Alpha贴图浏览按钮

    QLineEdit *editEmissive;
    QComboBox *comboEmissiveSource;
    QPushButton *btnBrowseEmissive;


    void loadMaterialToUI(const Material &mat);//展示到面板上

    //拆分getmaterial，旧代码因为和保存同步的代码混在一起，直接去掉换差分的代码
    void saveMaterialParams(Material &mat);   // 只保存贴图参数

    // 辅助函数：处理拖拽/浏览的文件
    void copyToSourcePath(const QString &filePath);
    void handleFileDrop(const QStringList &files, QLineEdit *targetEdit,
                        const QString &expectedSuffix, bool multiFile,QComboBox *sourceCombo);

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

    void onDiffSelectChanged(int index);//切换差分切换整个面板的数据
    void onAccepted();//新增

    void onApplyToAll();//将组件差分预设强制应用到当前组件

    void onVmtTemplateChanged(int index);

    void onEmissiveSourceChanged(int index);
};

#endif // MATERIALSETTING_H
