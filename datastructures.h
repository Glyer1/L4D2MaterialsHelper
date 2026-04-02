#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#include <QString>
#include <QStringList>
#include <QList>
//每个组件里的存材质信息的结构体（2-3层）
struct MaterialInComponent
{
    QString matId;
    QString name;
    QString vmtName;
    bool hasDiff = false;
    QStringList diffNames;
};

//材质列表的材质结构体(差分)
struct Material
{
    QString id;
    QString materialDefId;
    QString MFilename;         //basetexture名字
    QString vmtName;
    QString Mname;
    QString Mcomponent;
    QString Memissive;
    QString MemissiveSource;

    QString Mlightwarp;      // lightwarp路径
    QString Mbumpmap;        // bumpmap路径
    QString Menvmap;         // envmap路径

    //来源
    QString MlightwarpSource;
    QString MbumpmapSource;
    QString MenvmapSource;


    int MAlpha = 0;
    int Mselect = 0;
    int Mnocull = 1;
    int Mnodecal = 1;
    int Mselfillum = 0;
    int Mhalflambert = 1;
    int Mphong = 1;
    int Mtranslucent = 0;
    int Malphatest = 0;
    bool hasDiff = false;//有无差分
    bool isinit = false;//差分是否被处理过？初始为false
    QStringList diffNames;//材质的差分列表，自己是first()

    // 新增：自定义VMT内容
    bool useCustomVmt = false;        // 是否使用自定义VMT
    QString customVmtContent;          // 自定义VMT模板

    bool useAlphaTexture = false;      // 是否使用独立Alpha贴图
    QString alphaTextureName;          // Alpha贴图名称（不含后缀）

};

//材质列表的材质结构体(理解为组)
struct MaterialGroup{

    QString vmtName;
    QList<Material*> diffList;

    QString displayName;//中文名
    QString component;
};

//mod信息
struct Addoninfo{
    QString addonSteamAppID	= "550";
    QString addontitle;
    QString addonversion = "1.0";
    QString addontagline = "";
    QString addonauthor;
    QString addonDescription = "本材质组件由l4mh产生";
};

struct Addonimage{
    QString imagePath;
};

//组件结构体
struct Component
{
    bool select;
    QString name;
    QList<MaterialInComponent> materials;

    // 新增：组件相关的info信息
    Addoninfo compInfo;
};

// 材质在变体中的配置
struct VariantMaterialConfig {
    QString materialVmtName;    // 材质标识（如 "skin"）
    QString selectedDiff;       // 选中的差分名（为空表示无差分）
    QString baseTexture;        // 对应的中文名（变体）（用于显示）
};

// 单个变体
struct Variant {
    QString variantName;        // 用户自定义的变体名，如 "整体颜色黑"
    QList<VariantMaterialConfig> materials;  // 该变体下所有材质的配置
};

// 组件的所有变体
struct ComponentVariants {
    QString componentName;      // 组件名，如 "整体颜色"
    QList<Variant> variants;
};

// 材质来源常量
const QString SOURCE_NONE = "none";
const QString SOURCE_USER = "user";
const QString SOURCE_TOOL = "tool";
const QString SOURCE_CODE = "code";

#endif // DATASTRUCTURES_H
