//材质列表单行设置

#include "materialsetting.h"
#include "ui_materialsetting.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QInputDialog>
#include <QMessageBox>
#include <QStandardItem>
#include <QFileDialog>
#include <QSettings>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
MaterialSetting::MaterialSetting(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MaterialSetting)
{
    ui->setupUi(this);

    setAcceptDrops(true);  // 启用拖拽

    setWindowTitle("材质设置");
    setMinimumSize(700, 700);
    setMaximumSize(700, 700);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    //基础信息组（改用 QGridLayout）
    QGroupBox *groupBasic = new QGroupBox("基础信息", this);
    QGridLayout *basicLayout = new QGridLayout(groupBasic);

    // 行0: 所属组件
    basicLayout->addWidget(new QLabel("所属组件:"), 0, 0);
    comboComponent = new QComboBox(this);
    basicLayout->addWidget(comboComponent, 0, 1);

    // 行1: 材质名
    basicLayout->addWidget(new QLabel("材质名:"), 1, 0);
    comboMatName = new QComboBox(this);
    comboMatName->setEnabled(false);
    basicLayout->addWidget(comboMatName, 1, 1);

    // 行2: 启用差分（直接禁用）
    chkHasDiff = new QCheckBox("启用差分（从组件中选择）", this);
    chkHasDiff->setEnabled(false);  // 直接禁用
    basicLayout->addWidget(chkHasDiff, 2, 0, 1, 2);

    // 行3: 切换差分
    basicLayout->addWidget(new QLabel("切换差分："), 3, 0);
    comboDiffSelect = new QComboBox(this);
    comboDiffSelect->setEnabled(true);
    basicLayout->addWidget(comboDiffSelect, 3, 1);

    // 行4: VMT 模板选择（新增，占位）
    basicLayout->addWidget(new QLabel("VMT模板(txt)："), 4, 0);
    comboVmtTemplate = new QComboBox(this);
    comboVmtTemplate->addItem("自定义");
    QString vmtTemplatesDir = QApplication::applicationDirPath()+"/Resources/Resources/Vmt";
    QDir templateDir(vmtTemplatesDir);

    //获取模版
    if(templateDir.exists())
    {
        QStringList filters;
        filters<<"*.txt";
        QStringList files = templateDir.entryList(filters,QDir::Files);

        for(const QString &file : files)
        {
            QString name = QFileInfo(file).completeBaseName();
            comboVmtTemplate->addItem(name,file);
        }
    }

    connect(comboVmtTemplate, QOverload<int>::of(&QComboBox::currentIndexChanged),this,&MaterialSetting::onVmtTemplateChanged);

    // 从 Resources/Resources/Vmt/ 读取模板文件
    basicLayout->addWidget(comboVmtTemplate, 4, 1);

    //行5，创建应用预设到全部差分按钮
    labApplyToAll = new QLabel("应用到当前组件当前材质按钮", this);
    btnApplyToAll = new QPushButton("应用到所有差分", this);
    connect(btnApplyToAll, &QPushButton::clicked, this, &MaterialSetting::onApplyToAll);
    basicLayout->addWidget(labApplyToAll,5,0);
    basicLayout->addWidget(btnApplyToAll,5,1);

    mainLayout->addWidget(groupBasic);

    //贴图路径组
    QGroupBox *groupTexture = new QGroupBox("贴图设置", this);
    QGridLayout *texLayout = new QGridLayout(groupTexture);  // 改用 QGridLayout

    // 行0: 基础贴图名
    texLayout->addWidget(new QLabel("基础贴图名:"), 0, 0);
    editFilename = new QLineEdit(this);
    btnBrowseFilename = new QPushButton("浏览", this);
    texLayout->addWidget(editFilename, 0, 1);
    comboBaseSource = new QComboBox(this);
    comboBaseSource->addItem("用户文件夹", SOURCE_USER);
    comboBaseSource->setCurrentIndex(0);
    comboBaseSource->setEnabled(false);
    texLayout->addWidget(comboBaseSource,0,2);
    texLayout->addWidget(btnBrowseFilename, 0, 3);

    // 行1: Lightwarp
    texLayout->addWidget(new QLabel("Lightwarp:"), 1, 0);
    editLightwarp = new QLineEdit(this);
    texLayout->addWidget(editLightwarp, 1, 1);
    comboLightwarpSource = new QComboBox(this);
    comboLightwarpSource->addItem("不填", SOURCE_NONE);
    comboLightwarpSource->addItem("用户文件夹", SOURCE_USER);
    comboLightwarpSource->addItem("工具自带", SOURCE_TOOL);
    comboLightwarpSource->setCurrentIndex(2);
    texLayout->addWidget(comboLightwarpSource, 1, 2);
    btnBrowseLightwarp = new QPushButton("浏览", this);
    texLayout->addWidget(btnBrowseLightwarp,1,3);

    // 行2: Bumpmap
    texLayout->addWidget(new QLabel("Bumpmap:"), 2, 0);
    editBumpmap = new QLineEdit(this);
    editBumpmap->setEnabled(false);
    texLayout->addWidget(editBumpmap, 2, 1);
    comboBumpmapSource = new QComboBox(this);
    comboBumpmapSource->addItem("不填", SOURCE_NONE);
    comboBumpmapSource->addItem("用户文件夹", SOURCE_USER);
    comboBumpmapSource->addItem("代码(dev/dev_normal)", SOURCE_CODE);
    comboBumpmapSource->setCurrentIndex(0);
    texLayout->addWidget(comboBumpmapSource, 2, 2);
    btnBrowseBumpmap = new QPushButton("浏览",this);
    texLayout->addWidget(btnBrowseBumpmap,2,3);

    // 行3: Envmap
    texLayout->addWidget(new QLabel("Envmap:"), 3, 0);
    editEnvmap = new QLineEdit(this);
    editEnvmap->setEnabled(false);
    texLayout->addWidget(editEnvmap, 3, 1);
    comboEnvmapSource = new QComboBox(this);
    comboEnvmapSource->addItem("不填", SOURCE_NONE);
    comboEnvmapSource->addItem("用户文件夹", SOURCE_USER);
    comboEnvmapSource->addItem("代码(env_cubemap)", SOURCE_CODE);
    comboEnvmapSource->setCurrentIndex(0);
    texLayout->addWidget(comboEnvmapSource, 3, 2);
    btnBrowseEnvmap = new QPushButton("浏览", this);
    texLayout->addWidget(btnBrowseEnvmap, 3, 3);

    // 行4: Emissive
    texLayout->addWidget(new QLabel("Emissive:"), 4, 0);
    editEmissive = new QLineEdit(this);
    editEmissive->setEnabled(false);
    texLayout->addWidget(editEmissive, 4, 1);
    comboEmissiveSource = new QComboBox(this);
    comboEmissiveSource->addItem("不填", SOURCE_NONE);
    comboEmissiveSource->addItem("用户文件夹", SOURCE_USER);
    comboEmissiveSource->setCurrentIndex(0);
    texLayout->addWidget(comboEmissiveSource, 4, 2);
    btnBrowseEmissive = new QPushButton("浏览", this);
    texLayout->addWidget(btnBrowseEmissive, 4, 3);

    // 动图提示（占5列）
    QLabel *animatedHint = new QLabel(
        "💡动图贴图：用 @ 分隔多个帧，例如：tex_sora_0@tex_sora_1@tex_sora_2\n"
        "程序会自动识别将其合并成一张vtf，并用第一个'@'前的差分名命名\n"
        "基础贴图(png)/Lightwarp(vtf)/Bumpmap(png/vtf)/Envmap(vtf)/独立alpha(png)"
        "只填文件名,将文件放入Resources/SourceTextures，不要后缀\n"
        "或者使用浏览或者拖拽入编辑框来更改图片", this);
    animatedHint->setWordWrap(true);
    animatedHint->setStyleSheet("color: #666; font-size: 9pt;");
    texLayout->addWidget(animatedHint, 5, 0, 1, 3);

    mainLayout->addWidget(groupTexture);

    //Alpha设置组
    QGroupBox *groupAlpha = new QGroupBox("Alpha设置", this);
    QVBoxLayout *alphaLayout = new QVBoxLayout(groupAlpha);

    chkAlpha = new QCheckBox("加alpha通道(差分名包含'bright'或'亮'会更亮，如眼睛高亮)", this);
    alphaLayout->addWidget(chkAlpha);

    QHBoxLayout *alphaTexLayout = new QHBoxLayout();
    chkAlphaTexture = new QCheckBox("使用独立Alpha贴图（用于透明）", this);
    editAlphaTexture = new QLineEdit(this);
    editAlphaTexture->setPlaceholderText("输入贴图名（不含后缀）");
    editAlphaTexture->setEnabled(false);
    btnBrowseAlphaTexture = new QPushButton("浏览", this);
    btnBrowseAlphaTexture->setEnabled(false);
    alphaTexLayout->addWidget(chkAlphaTexture);
    alphaTexLayout->addWidget(editAlphaTexture);
    alphaTexLayout->addWidget(btnBrowseAlphaTexture);
    alphaLayout->addLayout(alphaTexLayout);

    QLabel *alphaTexHint = new QLabel(
        "💡 独立Alpha贴图：从 SourcePath 加载指定贴图作为本贴图Alpha通道\n"
        "与高亮Alpha互斥，勾选此项会自动取消高亮Alpha", this);
    alphaTexHint->setWordWrap(true);
    alphaTexHint->setStyleSheet("color: #666; font-size: 9pt;");
    alphaLayout->addWidget(alphaTexHint);

    mainLayout->addWidget(groupAlpha);

    //自定义VMT内容组（右对齐）
    QGroupBox *groupCustom = new QGroupBox("自定义VMT内容", this);
    QHBoxLayout *customMainLayout = new QHBoxLayout(groupCustom);  // 水平布局

    // 左侧：提示框
    QTextEdit *hintEdit = new QTextEdit(this);
    hintEdit->setReadOnly(true);
    hintEdit->setMaximumWidth(250);
    hintEdit->setMaximumHeight(200);
    hintEdit->setText(
        "【可用变量占位符】\n"
        "{{base}}   - 基础贴图路径\n"
        "{{light}}  - Lightwarp路径\n"
        "{{bump}}   - Bumpmap路径\n"
        "{{env}}    - Envmap路径\n"
        "{{aemissive}}    - emissive路径\n"
        "\n"
        "【示例】\n"
        "\"VertexLitGeneric\"\n"
        "{\n"
        "    $basetexture \"{{base}}\"\n"
        "    $lightwarp \"{{light}}\"\n"
        "}");
    customMainLayout->addWidget(hintEdit);

    // 右侧：编辑框
    vmtCustomEdit = new QTextEdit(this);
    vmtCustomEdit->setPlaceholderText("在此输入VMT内容，占位符会自动替换");
    vmtCustomEdit->setMinimumHeight(200);
    customMainLayout->addWidget(vmtCustomEdit, 1);  // 拉伸

    mainLayout->addWidget(groupCustom);
    mainLayout->setStretchFactor(groupCustom, 1);

    //确定取消按钮
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        // 检查：如果勾选了启用差分，但下拉框没有可用选项或没选中
        if (chkHasDiff->isChecked() &&
            (comboDiffSelect->count() == 0 || comboDiffSelect->currentIndex() < 0)) {
            QMessageBox::warning(this, "错误", "已勾选启用差分，但无可用的差分可选！");
            return;  // 阻止对话框关闭
        }
        accept();
    });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(chkHasDiff, &QCheckBox::toggled, this, &MaterialSetting::onHasDiffChanged);
    connect(comboComponent, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MaterialSetting::onComponentChanged);
    connect(comboMatName, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MaterialSetting::onMatNameChanged);  // 材质从编辑改为选择

    //贴图需要的信号量
    connect(comboLightwarpSource, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MaterialSetting::onLightwarpSourceChanged);
    connect(comboBumpmapSource, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MaterialSetting::onBumpmapSourceChanged);
    connect(comboEnvmapSource, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MaterialSetting::onEnvmapSourceChanged);
    connect(comboEmissiveSource, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MaterialSetting::onEmissiveSourceChanged);
    connect(btnBrowseEmissive, &QPushButton::clicked, this, [this]() {
        QString file = QFileDialog::getOpenFileName(this, "选择Emissive贴图", "", "PNG图片 (*.png);;VTF文件 (*.vtf)");
        if (!file.isEmpty()) {
            handleFileDrop({file}, editEmissive, "png", false, comboEmissiveSource);
        }
    });


    // 两个alpha互斥逻辑连接
    connect(chkAlphaTexture, &QCheckBox::toggled, this, &MaterialSetting::onAlphaTextureToggled);
    connect(chkAlpha, &QCheckBox::toggled, this, &MaterialSetting::onAlphaHighlightToggled);
    connect(buttonBox,&QDialogButtonBox::accepted, this, &MaterialSetting::onAccepted);

    //浏览连接

    // 基础贴图（PNG 或 VTF）
    connect(btnBrowseFilename, &QPushButton::clicked, this, [this]() {
        QStringList files = QFileDialog::getOpenFileNames(this, "选择基础贴图", "",
                                                          "图片文件 (*.png);;VTF文件 (*.vtf)");
        if (!files.isEmpty()) {
            // 检查第一个文件的类型，决定 expectedSuffix
            QFileInfo info(files.first());
            QString suffix = info.suffix().toLower();
            if (suffix == "png" || suffix == "vtf") {
                handleFileDrop(files, editFilename, suffix, true, nullptr);
            } else {
                QMessageBox::warning(this, "格式错误", "请选择 PNG 或 VTF 格式");
            }
        }
    });

    // Lightwarp（VTF）
    connect(btnBrowseLightwarp, &QPushButton::clicked, this, [this]() {
        QString file = QFileDialog::getOpenFileName(this, "选择Lightwarp贴图", "", "VTF文件 (*.vtf)");
        if (!file.isEmpty()) {
            handleFileDrop({file}, editLightwarp, "vtf", false, comboLightwarpSource);
        }
    });

    // Bumpmap（PNG 或 VTF）
    connect(btnBrowseBumpmap, &QPushButton::clicked, this, [this]() {
        QString file = QFileDialog::getOpenFileName(this, "选择Bumpmap贴图", "", "图片文件 (*.png);;VTF文件 (*.vtf)");
        if (!file.isEmpty()) {
            QFileInfo info(file);
            QString suffix = info.suffix().toLower();
            if (suffix == "png" || suffix == "vtf") {
                handleFileDrop({file}, editBumpmap, suffix, false, comboBumpmapSource);
            } else {
                QMessageBox::warning(this, "格式错误", "Bumpmap 请选择 PNG 或 VTF 格式");
            }
        }
    });

    // Envmap（VTF）
    connect(btnBrowseEnvmap, &QPushButton::clicked, this, [this]() {
        QString file = QFileDialog::getOpenFileName(this, "选择Envmap贴图", "", "VTF文件 (*.vtf)");
        if (!file.isEmpty()) {
            handleFileDrop({file}, editEnvmap, "vtf", false, comboEnvmapSource);
        }
    });

    // 独立Alpha（PNG）
    connect(btnBrowseAlphaTexture, &QPushButton::clicked, this, [this]() {
        QString file = QFileDialog::getOpenFileName(this, "选择Alpha贴图", "", "PNG图片 (*.png)");
        if (!file.isEmpty()) {
            handleFileDrop({file}, editAlphaTexture, "png", false, nullptr);
        }
    });

    // 独立Alpha复选框控制浏览按钮
    connect(chkAlphaTexture, &QCheckBox::toggled, this, [this](bool checked) {
        editAlphaTexture->setEnabled(checked);
        btnBrowseAlphaTexture->setEnabled(checked);
        if (!checked) {
            editAlphaTexture->clear();
        }
    });

    // 初始化Lightwarp状态（默认选工具自带）
    onLightwarpSourceChanged(2);    //工具自带
    onBumpmapSourceChanged(0);    // 不填
    onEnvmapSourceChanged(0);     // 不填

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    layout()->setSizeConstraint(QLayout::SetFixedSize);
}

MaterialSetting::~MaterialSetting()
{
    delete ui;
}

void MaterialSetting::onLightwarpSourceChanged(int index)
{
    QString source = comboLightwarpSource->currentData().toString();

    if (source == SOURCE_NONE) {
        // 不填：清空并禁用输入框
        editLightwarp->clear();
        editLightwarp->setEnabled(false);
    }
    else if (source == SOURCE_TOOL) {
        // 工具自带：自动填入默认路径并禁用输入框
        editLightwarp->setText(QString("%1/Resources/Resources/Vtf/toon_light.vtf").arg(QApplication::applicationDirPath()));
        editLightwarp->setEnabled(false);
    }
    else if (source == SOURCE_USER) {
        // 用户文件夹：启用输入框
        editLightwarp->setEnabled(true);
    }
}

void MaterialSetting::onBumpmapSourceChanged(int index)
{
    QString source = comboBumpmapSource->currentData().toString();

    if (source == SOURCE_NONE) {
        // 不填：清空并禁用输入框
        editBumpmap->clear();
        editBumpmap->setEnabled(false);
    }
    else if (source == SOURCE_CODE) {
        // 代码：自动填入dev/dev_normal并禁用输入框
        editBumpmap->setText("dev/dev_normal");
        editBumpmap->setEnabled(false);
    }
    else if (source == SOURCE_USER) {
        // 用户文件夹：启用输入框
        editBumpmap->setEnabled(true);
    }
}

void MaterialSetting::onEnvmapSourceChanged(int index)
{
    QString source = comboEnvmapSource->currentData().toString();

    if (source == SOURCE_NONE) {
        // 不填：清空并禁用输入框
        editEnvmap->clear();
        editEnvmap->setEnabled(false);
    }
    else if (source == SOURCE_CODE) {
        // 代码：自动填入env_cubemap并禁用输入框
        editEnvmap->setText("env_cubemap");
        editEnvmap->setEnabled(false);
    }
    else if (source == SOURCE_USER) {
        // 用户文件夹：启用输入框
        editEnvmap->setEnabled(true);
    }
}

//判定是否开启差分选择，checkbox触发事件
void MaterialSetting::onHasDiffChanged(bool checked)
{
    comboDiffSelect->setEnabled(checked);
}

//材质编辑确定后保存
void MaterialSetting::saveMaterialParams(Material &mat)
{
    // 贴图路径
    mat.MFilename = editFilename->text().trimmed();

    // Lightwarp
    mat.MlightwarpSource = comboLightwarpSource->currentData().toString();
    if (mat.MlightwarpSource == SOURCE_TOOL) {
        mat.Mlightwarp = QString("%1/Resources/Resources/Vtf/toon_light.vtf").arg(QApplication::applicationDirPath());
    } else if (mat.MlightwarpSource == SOURCE_USER) {
        mat.Mlightwarp = editLightwarp->text().trimmed();
    } else {
        mat.Mlightwarp.clear();
    }

    // Bumpmap
    mat.MbumpmapSource = comboBumpmapSource->currentData().toString();
    if (mat.MbumpmapSource == SOURCE_CODE) {
        mat.Mbumpmap = "dev/dev_normal";
    } else if (mat.MbumpmapSource == SOURCE_USER) {
        mat.Mbumpmap = editBumpmap->text().trimmed();
    } else {
        mat.Mbumpmap.clear();
    }

    // Envmap
    mat.MenvmapSource = comboEnvmapSource->currentData().toString();
    if (mat.MenvmapSource == SOURCE_CODE) {
        mat.Menvmap = "env_cubemap";
    } else if (mat.MenvmapSource == SOURCE_USER) {
        mat.Menvmap = editEnvmap->text().trimmed();
    } else {
        mat.Menvmap.clear();
    }

    // Emissive
    mat.MemissiveSource = comboEmissiveSource->currentData().toString();
    if (mat.MemissiveSource == SOURCE_USER) {
        mat.Memissive = editEmissive->text().trimmed();
    } else {
        mat.Memissive.clear();
    }

    // 自定义 VMT
    mat.customVmtContent = vmtCustomEdit->toPlainText();

    // Alpha 相关
    mat.useAlphaTexture = chkAlphaTexture->isChecked();
    mat.alphaTextureName = editAlphaTexture->text().trimmed();

    if (mat.useAlphaTexture) {
        mat.MAlpha = 0;
    } else {
        mat.MAlpha = chkAlpha->isChecked() ? 1 : 0;
    }

    // 标记已配置
    mat.isinit = true;
}

#if 0
//mainwindow材质列表关闭本弹窗后获取材质信息，现在作用为传回一个差分的信息。
void MaterialSetting::getMaterialData(Material &mat)
{
    qDebug() << "=== getMaterialData ===";
    qDebug() << "保存前 mat.Mcomponent:" << mat.Mcomponent;
    qDebug() << "保存前 mat.vmtName:" << mat.vmtName;
    qDebug() << "保存前 mat.diffNames:" << mat.diffNames;

    // 基础信息
    mat.Mname = comboMatName->currentText().split(" (").first();
    mat.vmtName = comboMatName->currentData().toString();
    mat.Mcomponent = comboComponent->currentText();

    //修改：hasDiff 由材质本身决定，不依赖复选框
    //从temp拿hasdiff的值到引用的mat里头,然后temp和那个combo组件选择下拉框的index是对应的
    int compIndex = comboComponent->currentIndex();
    QString vmtName = mat.vmtName;
    bool materialHasDiff = false;

    if (compIndex >= 0 && compIndex < tempComponents.size()) {
        const Component &comp = tempComponents[compIndex];
        for (const auto &m : comp.materials) {
            if (m.vmtName == vmtName) {
                materialHasDiff = m.hasDiff;
                break;
            }
        }
    }
    //有问题，晚点处理
    //mat.hasDiff = materialHasDiff;  // 直接使用材质的实际差分属性

    // 差分处理
    if (mat.hasDiff) {
        if (comboDiffSelect->count() > 0 && comboDiffSelect->currentIndex() >= 0) {
            mat.diffNames.clear();
            mat.diffNames << comboDiffSelect->currentText();
        } else {
            // 如果有差分但没有选中任何项，说明所有差分都被用了，不应该发生
            mat.diffNames.clear();
        }
    } else {
        mat.diffNames.clear();
    }

    // 贴图路径 - 使用正确的字段名
    mat.MFilename = editFilename->text().trimmed();

    // Lightwarp
    mat.MlightwarpSource = comboLightwarpSource->currentData().toString();
    if (mat.MlightwarpSource == SOURCE_TOOL) {
        mat.Mlightwarp = QString("%1/Resources/Resources/Vtf/toon_light.vtf").arg(QApplication::applicationDirPath());
    } else if (mat.MlightwarpSource == SOURCE_USER) {
        mat.Mlightwarp = editLightwarp->text().trimmed();
    } else {
        mat.Mlightwarp.clear();  // SOURCE_NONE
    }

    // Bumpmap
    mat.MbumpmapSource = comboBumpmapSource->currentData().toString();
    if (mat.MbumpmapSource == SOURCE_CODE) {
        mat.Mbumpmap = "dev/dev_normal";
    } else if (mat.MbumpmapSource == SOURCE_USER) {
        mat.Mbumpmap = editBumpmap->text().trimmed();
    } else {
        mat.Mbumpmap.clear();  // SOURCE_NONE
    }

    // Envmap
    mat.MenvmapSource = comboEnvmapSource->currentData().toString();
    if (mat.MenvmapSource == SOURCE_CODE) {
        mat.Menvmap = "env_cubemap";
    } else if (mat.MenvmapSource == SOURCE_USER) {
        mat.Menvmap = editEnvmap->text().trimmed();
    } else {
        mat.Menvmap.clear();  // SOURCE_NONE
    }

    // 新增：保存自定义VMT内容
    mat.customVmtContent = vmtCustomEdit->toPlainText();

    mat.useAlphaTexture = chkAlphaTexture->isChecked();
    mat.alphaTextureName = editAlphaTexture->text().trimmed();

    // 渲染参数，bool转int
    if (mat.useAlphaTexture) {
        mat.MAlpha = 0;  // 独立Alpha时，高亮Alpha强制为0
    } else {
        mat.MAlpha = chkAlpha->isChecked() ? 1 : 0;
    }

    // 渲染参数，bool转int
    mat.Mnocull = chkNocull->isChecked() ? 1 : 0;
    mat.Mnodecal = chkNodecal->isChecked() ? 1 : 0;
    mat.Mselfillum = chkSelfillum->isChecked() ? 1 : 0;
    mat.Mhalflambert = chkHalflambert->isChecked() ? 1 : 0;
    mat.Mphong = chkPhong->isChecked() ? 1 : 0;
    mat.Mtranslucent = chkTranslucent->isChecked() ? 1 : 0;
    mat.Malphatest = chkAlphatest->isChecked() ? 1 : 0;

    qDebug() << "保存后 mat.Mcomponent:" << mat.Mcomponent;
    qDebug() << "保存后 mat.vmtName:" << mat.vmtName;
    qDebug() << "保存后 mat.diffNames:" << mat.diffNames;
}
#endif

//调整所属组件，刷新材质下拉框
void MaterialSetting::onComponentChanged(int index)
{
    comboMatName->clear();
    comboDiffSelect->clear();
    chkHasDiff->setChecked(false);
    //chkHasDiff->setEnabled(false);  // 不启用
    comboDiffSelect->setEnabled(false);

    if (index < 0 || index >= tempComponents.size()) return;

    const Component &comp = tempComponents[index];

    //加入对应组件对应材质
    for (const auto &mat : comp.materials) {
        comboMatName->addItem(mat.name + " (" + mat.vmtName + ")", mat.vmtName);
    }

    //如果材质下拉框项大于0就继续展开差分
    if (comboMatName->count() > 0) {
        onMatNameChanged(0);
    }
}

//填充差分下拉框
void MaterialSetting::onMatNameChanged(int index)
{
    comboDiffSelect->clear();
    chkHasDiff->setChecked(false);
    //chkHasDiff->setEnabled(true);  // 先恢复启用
    comboDiffSelect->setEnabled(false);

    if (index < 0) return;

    QString vmtName = comboMatName->itemData(index).toString();
    int compIndex = comboComponent->currentIndex();
    if (compIndex < 0) return;

    const Component &comp = tempComponents[compIndex];

    for (const auto &mat : comp.materials) {
        if (mat.vmtName != vmtName) continue;

        if (mat.hasDiff) {
            chkHasDiff->setChecked(true);
            chkHasDiff->setEnabled(false);  // 有差分则禁用复选框
            comboDiffSelect->setEnabled(true);

            // 遍历该材质的所有差分，去已添加材质列表找已经使用了的，已使用的不加到列表
            for (const QString &diff : mat.diffNames) {
                bool isUsed = false;

                // 检查这个差分在当前组件+材质下是否已被其他材质使用
                for (const Material &m : allMaterials) {
                    // 用创建差分时分配的id跳过当前正在编辑的材质
                    if (m.id == currentMaterialId) continue;

                    // 检查条件：同一组件 + 同一vmtName + 使用了这个差分
                    if (m.Mcomponent == comp.name &&
                        m.vmtName == vmtName &&
                        m.hasDiff) {

                        // 检查这个差分是否已经被使用
                        for (const QString &usedDiff : m.diffNames) {
                            if (usedDiff == diff) {
                                isUsed = true;
                                break;
                            }
                        }
                    }
                    if (isUsed) break;
                }

                // 只有未被使用的差分才添加到下拉框
                if (!isUsed) {
                    comboDiffSelect->addItem(diff);
                }
            }

            // 如果所有差分都被使用了，就禁用差分选择
            if (comboDiffSelect->count() == 0) {
                chkHasDiff->setChecked(false);
                comboDiffSelect->setEnabled(false);
                // 可以提示用户所有差分都已使用
            }
            else {
                // 无差分的材质，保持复选框可用
                chkHasDiff->setEnabled(false);
            }
        }
        break;
    }
}


void MaterialSetting::onAlphaTextureToggled(bool checked)
{
    editAlphaTexture->setEnabled(checked);

    if (checked) {
        // 勾选独立Alpha时，取消高亮Alpha并禁用
        if (chkAlpha->isChecked()) {
            chkAlpha->setChecked(false);
        }
        chkAlpha->setEnabled(false);
    } else {
        // 取消勾选时，恢复高亮Alpha的可用性
        chkAlpha->setEnabled(true);
        editAlphaTexture->clear();  // 清空输入
    }
}

void MaterialSetting::onAlphaHighlightToggled(bool checked)
{
    if (checked) {
        // 勾选高亮Alpha时，取消独立Alpha并禁用
        if (chkAlphaTexture->isChecked()) {
            chkAlphaTexture->setChecked(false);
        }
        chkAlphaTexture->setEnabled(false);
        editAlphaTexture->setEnabled(false);
    } else {
        // 取消勾选时，恢复独立Alpha的可用性
        chkAlphaTexture->setEnabled(true);
    }
}

//返回一个差分，点确定的时候
void MaterialSetting::onAccepted()
{
    if(currentMaterial){
        //把面板内容存回去
        saveMaterialParams(*currentMaterial);
    }
    accept();
}

//改变差分保存
void MaterialSetting::onDiffSelectChanged(int index)
{
    //选择差分不在列表内
    if(index <0 || index >=currentDiffList.size())return ;

    //当前材质存在就把面板存回去
    if(currentMaterial)
    {
                saveMaterialParams(*currentMaterial);  // 只保存参数，不改差分名
    }

    currentMaterial = currentDiffList[index];

    loadMaterialToUI(*currentMaterial);
}

//设置材质组,初始化当前面板
void MaterialSetting::setMaterialGroup(const QString &component,
                      const QString &vmtName,
                      const QList<Material*> &diffList,
                      const QList<Component> &comps)
{
    qDebug() << "=== setMaterialGroup ===";
    qDebug() << "传入 component:" << component;
    qDebug() << "传入 vmtName:" << vmtName;
    qDebug() << "传入 diffList 大小:" << diffList.size();
    for(Material* m : diffList) {
        qDebug() << "  diff:" << m->diffNames << "Mcomponent:" << m->Mcomponent
                 << "vmtName:" << m->vmtName << "hasDiff:" << m->hasDiff;
    }

    currentComponent = component;
    currentDiffList = diffList;//当前材质组所有差分，指针，可以改值
    currentVmtName = vmtName;
    tempComponents = comps;

    //编辑不给你改组件和材质，嘻嘻
    comboComponent->setEnabled(false);
    comboComponent->clear();
    comboComponent->addItem(component);
    comboComponent->setCurrentIndex(0);

    //填充材质下拉框,只有当前材质
    comboMatName->setEnabled(false);
    //找组件对应
    for(Component comp : tempComponents)
    {

        if(comp.name == component)
        {
            //找材质名vmtName对应
            for(const MaterialInComponent &mic : comp.materials)
            {
                if(mic.vmtName == vmtName)
                {
                    comboMatName->clear();
                    comboMatName->addItem(mic.name+"("+mic.vmtName+")",mic.vmtName);
                    comboMatName->setCurrentIndex(0);

                    qDebug() << "组件定义中材质" << vmtName << "的 hasDiff:" << mic.hasDiff;

                    break;
                }
            }
            break;

        }
    }

    //清空差分下拉款
    comboDiffSelect->clear();
    //遍历差分加入下拉框
    for(Material* diff : currentDiffList)
    {
        QString diffName;
        if(diff->hasDiff && !diff->diffNames.isEmpty())
        {
            diffName = diff->diffNames.first();
            qDebug()<<"下拉框加入差分："<<diff->diffNames.first()<<"\n";
        }
        else
        {
            diffName = "无差分";
            qDebug()<<"下拉框加入无差分："<<"\n";
        }
        comboDiffSelect->addItem(diffName);
    }

    //如果不是空的选第一个
    if(!currentDiffList.isEmpty())
    {
        currentMaterial = currentDiffList.first();
        loadMaterialToUI(*currentMaterial);
        comboDiffSelect->setCurrentIndex(0);
    }

    disconnect(comboDiffSelect, QOverload<int>::of(&QComboBox::currentIndexChanged),
                   this, &MaterialSetting::onDiffSelectChanged);
    connect(comboDiffSelect, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &MaterialSetting::onDiffSelectChanged);



}

//将设置好材质展示材质面板上
void MaterialSetting::loadMaterialToUI(const Material &mat)
{

    qDebug() << "loadMaterialToUI: hasDiff =" << mat.hasDiff;
    // 基础贴图名
    editFilename->setText(mat.MFilename);

    // Lightwarp
    editLightwarp->setText(mat.Mlightwarp);
    int lightwarpIndex = comboLightwarpSource->findData(mat.MlightwarpSource);
    if (lightwarpIndex >= 0) comboLightwarpSource->setCurrentIndex(lightwarpIndex);

    // Bumpmap
    editBumpmap->setText(mat.Mbumpmap);
    int bumpmapIndex = comboBumpmapSource->findData(mat.MbumpmapSource);
    if (bumpmapIndex >= 0) comboBumpmapSource->setCurrentIndex(bumpmapIndex);

    // Envmap
    editEnvmap->setText(mat.Menvmap);
    int envmapIndex = comboEnvmapSource->findData(mat.MenvmapSource);
    if (envmapIndex >= 0) comboEnvmapSource->setCurrentIndex(envmapIndex);

    // Alpha 相关
    chkAlpha->setChecked(mat.MAlpha);
    chkAlphaTexture->setChecked(mat.useAlphaTexture);
    editAlphaTexture->setText(mat.alphaTextureName);

    // Emissive
    editEmissive->setText(mat.Memissive);
    int emissiveIndex = comboEmissiveSource->findData(mat.MemissiveSource);
    if (emissiveIndex >= 0) comboEmissiveSource->setCurrentIndex(emissiveIndex);

    // 自定义 VMT
    vmtCustomEdit->setPlainText(mat.customVmtContent);

    // 差分状态
    chkHasDiff->setChecked(mat.hasDiff);
}

//强制应用预设到组件的材质的差分上
void MaterialSetting::onApplyToAll()
{
    Material currentParams;
    saveMaterialParams(currentParams);

    int appliedCount = 0;
    for(Material* diff : currentDiffList)
    {
        //从当前材质跳过自己
        if(diff == currentMaterial)continue;

        // 复制参数
        diff->Mlightwarp = currentParams.Mlightwarp;
        diff->MlightwarpSource = currentParams.MlightwarpSource;
        diff->Mbumpmap = currentParams.Mbumpmap;
        diff->MbumpmapSource = currentParams.MbumpmapSource;
        diff->Menvmap = currentParams.Menvmap;
        diff->MenvmapSource = currentParams.MenvmapSource;
        diff->MAlpha = currentParams.MAlpha;
        diff->useAlphaTexture = currentParams.useAlphaTexture;
        diff->alphaTextureName = currentParams.alphaTextureName;
        diff->customVmtContent = currentParams.customVmtContent;  // 新增
        diff->isinit = true;
        appliedCount++;
    }

    QMessageBox::information(this, "完成", QString("已应用到 %1 个差分").arg(appliedCount));
}

//拖拽事件
void MaterialSetting::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

//接受拖拽，看鼠标落在哪，然后就传入哪
void MaterialSetting::dropEvent(QDropEvent *event)
{
    //拿url
    const QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty()) return;

    //由url获取文件存如files列表
    QStringList files;
    for (const QUrl &url : urls) {
        files << url.toLocalFile();
    }

    // 获取鼠标下方的控件
    QWidget *target = childAt(event->position().toPoint());

    if (target == editFilename || target == btnBrowseFilename) {
        handleFileDrop(files, editFilename, "png", true, nullptr);
    }
    else if (target == editLightwarp || target == btnBrowseLightwarp) {
        handleFileDrop(files, editLightwarp, "vtf", false, comboLightwarpSource);
    }
    else if (target == editBumpmap || target == btnBrowseBumpmap) {
        // Bumpmap 检查第一个文件的类型
        if (!files.isEmpty()) {
            QFileInfo info(files.first());
            QString suffix = info.suffix().toLower();
            if (suffix == "png" || suffix == "vtf") {
                handleFileDrop(files, editBumpmap, suffix, false, comboBumpmapSource);
            } else {
                QMessageBox::warning(this, "格式错误", "Bumpmap 请拖拽 PNG 或 VTF 文件");
            }
        }
    }
    else if (target == editEnvmap || target == btnBrowseEnvmap) {
        handleFileDrop(files, editEnvmap, "vtf", false, comboEnvmapSource);
    }
    else if (target == editAlphaTexture || target == btnBrowseAlphaTexture) {
        if (chkAlphaTexture->isChecked()) {
            handleFileDrop(files, editAlphaTexture, "png", false, nullptr);
        }
    }
}

//复制文件到 SourcePath
void MaterialSetting::copyToSourcePath(const QString &filePath)
{
    QSettings settings("Local", "Path");
    QString sourcePath = settings.value("SourcePath").toString();
    if (sourcePath.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先在主窗口设置源文件夹路径");
        return;
    }

    QFileInfo info(filePath);
    QString dstPath = sourcePath + "/" + info.fileName();

    if (!QFile::exists(dstPath)) {

        //拷贝
        if (!QFile::copy(filePath, dstPath)) {
            qDebug() << "复制文件失败:" << filePath << "->" << dstPath;
        }
    }
}

//文件拖拽后的处理
void MaterialSetting::handleFileDrop(const QStringList &files, QLineEdit *targetEdit,
                                     const QString &expectedSuffix, bool multiFile, QComboBox *sourceCombo)
{
    if (files.isEmpty()) return;

    QSettings settings("Local", "Path");
    QString sourcePath = settings.value("SourcePath").toString();
    if (sourcePath.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先在主窗口设置源文件夹路径");
        return;
    }

    QStringList names;

    QString existingName;

    // 先检查第一个文件（多文件时只检查第一个）
    QFileInfo firstInfo(files.first());
    QString firstBaseName = firstInfo.completeBaseName();

    // 1. 检查同名的其他格式文件冲突
    QString pngFile = sourcePath + "/" + firstBaseName + ".png";
    QString vtfFile = sourcePath + "/" + firstBaseName + ".vtf";

    if (expectedSuffix == "png" && QFile::exists(vtfFile)) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "警告",
            QString("已存在同名的 VTF 文件: %1.vtf\n"
                    "继续操作会覆盖最终生成的 VTF 文件。\n"
                    "是否继续？").arg(firstBaseName),
            QMessageBox::Yes | QMessageBox::No);
        if (reply != QMessageBox::Yes) return;
    }

    if (expectedSuffix == "vtf" && QFile::exists(pngFile)) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "警告",
            QString("已存在同名的 PNG 文件: %1.png\n"
                    "继续操作会覆盖最终生成的 VTF 文件。\n"
                    "是否继续？").arg(firstBaseName),
            QMessageBox::Yes | QMessageBox::No);
        if (reply != QMessageBox::Yes) return;
    }

    // 2. 检查编辑框内容冲突
    if (!targetEdit->text().isEmpty() && targetEdit->text() != firstBaseName) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "确认覆盖",
            QString("当前已有: %1\n要替换为: %2 吗？")
                .arg(targetEdit->text()).arg(firstBaseName),
            QMessageBox::Yes | QMessageBox::No);
        if (reply != QMessageBox::Yes) return;
    }



    for (const QString &file : files) {
        QFileInfo info(file);
        QString suffix = info.suffix().toLower();

        // 检查文件类型
        if (!expectedSuffix.isEmpty() && suffix != expectedSuffix) {
            QMessageBox::warning(this, "格式错误",
                                 QString("请选择 %1 格式的文件，当前文件: %2").arg(expectedSuffix.toUpper()).arg(info.fileName()));
            return;
        }

        // 复制文件到 SourcePath
        copyToSourcePath(file);

        // 提取文件名（去后缀）
        names << info.completeBaseName();
    }

    if (multiFile && names.size() > 1) {
        targetEdit->setText(names.join("@"));
    } else {
        targetEdit->setText(names.first());
    }

    // 如果有来源下拉框，切换为"用户文件夹"
    if (sourceCombo) {
        int index = sourceCombo->findData(SOURCE_USER);
        if (index >= 0) {
            sourceCombo->setCurrentIndex(index);
        }
    }
}

//改变vmt模版index
void MaterialSetting::onVmtTemplateChanged(int index)
{
    if(index <= 0) return;

    //获取index对应文件名
    QString templateFile = comboVmtTemplate->itemData(index).toString();
    if(templateFile.isEmpty()) return ;

    QString fullPath = QApplication::applicationDirPath()+"/Resources/Resources/Vmt/"+templateFile;

    QFile file(fullPath);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        QString content=in.readAll();
        file.close();

        vmtCustomEdit->setText(content);

    }
    else
    {
        QMessageBox::warning(this, "错误", "无法读取模板文件: " + templateFile);
    }
}

//改变emissive贴图
void MaterialSetting::onEmissiveSourceChanged(int index)
{
    QString source = comboEmissiveSource->currentData().toString();

    if (source == SOURCE_NONE) {
        editEmissive->clear();
        editEmissive->setEnabled(false);
    } else if (source == SOURCE_USER) {
        editEmissive->setEnabled(true);
    }
}
