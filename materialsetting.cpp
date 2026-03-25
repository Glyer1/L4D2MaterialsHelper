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
MaterialSetting::MaterialSetting(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MaterialSetting)
{
    ui->setupUi(this);

    setWindowTitle("材质设置");
    setFixedSize(500, 900);

    //主界面竖直布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    //基础信息组，创建组别，然后里头加竖直分布
    QGroupBox *groupBasic = new QGroupBox("基础信息", this);
    QVBoxLayout *basicLayout = new QVBoxLayout(groupBasic);

    // 材质名
    QHBoxLayout *nameLayout = new QHBoxLayout();
    nameLayout->addWidget(new QLabel("材质名:"));
    comboMatName = new QComboBox(this);
    comboMatName->setEnabled(false);  // 先选组件才能选材质
    nameLayout->addWidget(comboMatName);
    basicLayout->addLayout(nameLayout);

    // 所属组件
    QHBoxLayout *compLayout = new QHBoxLayout();
    compLayout->addWidget(new QLabel("所属组件:"));
    comboComponent = new QComboBox(this);
    compLayout->addWidget(comboComponent);
    basicLayout->addLayout(compLayout);

    // 是否有差分
    chkHasDiff = new QCheckBox("启用差分（从组件中选择）", this);
    basicLayout->addWidget(chkHasDiff);

    // 差分下拉选择（单选）
    basicLayout->addWidget(new QLabel("选择差分："));
    comboDiff = new QComboBox(this);
    comboDiff->setEnabled(false);
    basicLayout->addWidget(comboDiff);

    mainLayout->addWidget(groupBasic);  // 加到主布局！

    //贴图路径组
    QGroupBox *groupTexture = new QGroupBox("贴图设置", this);
    QVBoxLayout *texLayout = new QVBoxLayout(groupTexture);

    // 基础贴图（只有路径，没有来源选择）
    QHBoxLayout *fileLayout = new QHBoxLayout();
    fileLayout->addWidget(new QLabel("基础贴图名:"));
    editFilename = new QLineEdit(this);
    fileLayout->addWidget(editFilename);
    texLayout->addLayout(fileLayout);

    QLabel *animatedHint = new QLabel(
        "💡动图贴图：用 @ 分隔多个帧，例如：tex_sora_0@tex_sora_1@tex_sora_2\n"
        "程序会自动识别将其合并成一张vtf,并用第一个'@'前的差分名命名"
        "*注意*！Lightwarp(vtf) Bumpmap(png) Envmap(vtf)都填文件名，不要后缀"
        "以上是选择'用户文件夹'时你们自己提供的，直接丢到SourceTextures里即可",
        this);
    animatedHint->setWordWrap(true);
    animatedHint->setStyleSheet("color: #666; font-size: 9pt; padding: 4px;");
    texLayout->addWidget(animatedHint);

    // Lightwarp（不填、用户文件夹、工具自带）
    QHBoxLayout *lightwarpLayout = new QHBoxLayout();
    lightwarpLayout->addWidget(new QLabel("Lightwarp:"));
    editLightwarp = new QLineEdit(this);
    comboLightwarpSource = new QComboBox(this);
    comboLightwarpSource->addItem("不填", SOURCE_NONE);
    comboLightwarpSource->addItem("用户文件夹", SOURCE_USER);
    comboLightwarpSource->addItem("工具自带", SOURCE_TOOL);
    comboLightwarpSource->setCurrentIndex(2);  // 默认选"工具自带"
    lightwarpLayout->addWidget(editLightwarp);
    lightwarpLayout->addWidget(comboLightwarpSource);
    texLayout->addLayout(lightwarpLayout);

    // Bumpmap（不填、用户文件夹、代码）
    QHBoxLayout *bumpmapLayout = new QHBoxLayout();
    bumpmapLayout->addWidget(new QLabel("Bumpmap:"));
    editBumpmap = new QLineEdit(this);
    editBumpmap->setEnabled(false);  // 默认禁用
    comboBumpmapSource = new QComboBox(this);
    comboBumpmapSource->addItem("不填", SOURCE_NONE);
    comboBumpmapSource->addItem("用户文件夹", SOURCE_USER);
    comboBumpmapSource->addItem("代码(dev/dev_normal)", SOURCE_CODE);
    comboBumpmapSource->setCurrentIndex(0);  // 默认选"不填"
    bumpmapLayout->addWidget(editBumpmap);
    bumpmapLayout->addWidget(comboBumpmapSource);
    texLayout->addLayout(bumpmapLayout);

    // Envmap（不填、用户文件夹、代码）
    QHBoxLayout *envmapLayout = new QHBoxLayout();
    envmapLayout->addWidget(new QLabel("Envmap:"));
    editEnvmap = new QLineEdit(this);
    editEnvmap->setEnabled(false);  // 默认禁用
    comboEnvmapSource = new QComboBox(this);
    comboEnvmapSource->addItem("不填", SOURCE_NONE);
    comboEnvmapSource->addItem("用户文件夹", SOURCE_USER);
    comboEnvmapSource->addItem("代码(env_cubemap)", SOURCE_CODE);
    comboEnvmapSource->setCurrentIndex(0);  // 默认选"不填"
    envmapLayout->addWidget(editEnvmap);
    envmapLayout->addWidget(comboEnvmapSource);
    texLayout->addLayout(envmapLayout);

    mainLayout->addWidget(groupTexture);  // 加到主布局

    //渲染参数组
    QGroupBox *groupAlpha = new QGroupBox("Alpha设置", this);
    QVBoxLayout *alphaLayout = new QVBoxLayout(groupAlpha);

    chkAlpha = new QCheckBox("加alpha通道(有(无)差分→差分名(材质名)包含'bright'或'亮'会更亮，如眼睛高亮)", this);
    alphaLayout->addWidget(chkAlpha);

    // 新增：独立Alpha贴图选项
    QHBoxLayout *alphaTexLayout = new QHBoxLayout();
    chkAlphaTexture = new QCheckBox("使用独立Alpha贴图（用于透明，将贴图当做Alpha通道）", this);
    editAlphaTexture = new QLineEdit(this);
    editAlphaTexture->setPlaceholderText("输入贴图名（不含后缀），如: tex_alpha");
    editAlphaTexture->setEnabled(false);  // 默认禁用

    alphaTexLayout->addWidget(chkAlphaTexture);
    alphaTexLayout->addWidget(editAlphaTexture);
    alphaLayout->addLayout(alphaTexLayout);

    // 提示标签
    QLabel *alphaTexHint = new QLabel(
        "💡 独立Alpha贴图：从 SourcePath 加载指定贴图作为本贴图Alpha通道用于透明效果\n"
        "与上面的高亮Alpha互斥，勾选此项会自动取消高亮Alpha",
        this);
    alphaTexHint->setWordWrap(true);
    alphaTexHint->setStyleSheet("color: #666; font-size: 9pt;");
    alphaLayout->addWidget(alphaTexHint);

    mainLayout->addWidget(groupAlpha);

    // ========== 其他复选框全部创建但隐藏（为了保持数据兼容）==========
    chkNocull = new QCheckBox(this);
    chkNodecal = new QCheckBox(this);
    chkSelfillum = new QCheckBox(this);
    chkHalflambert = new QCheckBox(this);
    chkPhong = new QCheckBox(this);
    chkTranslucent = new QCheckBox(this);
    chkAlphatest = new QCheckBox(this);

    // 隐藏
    chkNocull->hide();
    chkNodecal->hide();
    chkSelfillum->hide();
    chkHalflambert->hide();
    chkPhong->hide();
    chkTranslucent->hide();
    chkAlphatest->hide();

    //自定义
    QGroupBox *groupCustom = new QGroupBox("自定义VMT内容", this);
    QVBoxLayout *customLayout = new QVBoxLayout(groupCustom);
    customLayout->setSpacing(8);  // 设置控件间距

    // ===== 新增：只读提示框 =====
    QTextEdit *hintEdit = new QTextEdit(this);
    hintEdit->setReadOnly(true);  // 设置为只读
    hintEdit->setMaximumHeight(200);  // 限制高度
    hintEdit->setStyleSheet(
        "QTextEdit {"
        "   background-color: #f8f9fa;"
        "   color: #2c3e50;"
        "   font-size: 10pt;"
        "   border: 1px solid #dee2e6;"
        "   border-radius: 4px;"
        "   padding: 8px;"
        "}"
        "QTextEdit:focus {"
        "   border-color: #86b7fe;"
        "   outline: 0;"
        "}"
        );
    hintEdit->setText(
        "【可用变量占位符】\n"
        "{{base}}   - 基础贴图路径（自动添加）\n"
        "{{light}}  - Lightwarp路径\n"
        "{{bump}}   - Bumpmap路径\n"
        "{{env}}    - Envmap路径\n"
        "\n"
        "【示例模板】\n"
        "\"VertexLitGeneric\"\n"
        "{\n"
        "    $basetexture \"{{base}}\"\n"
        "    $halflambert 1\n"
        "    $lightwarp \"{{light}}\"\n"
        "    Proxies\n"
        "    {\n"
        "        \"AnimatedTexture\"\n"
        "        {\n"
        "            \"animatedTextureVar\" \"$basetexture\"\n"
        "            \"animatedTextureFrameNum\"\n"
        "            \"animatedTextureFrameRate\" 30\n"
        "        }\n"
        "    }\n"
        "}"
        );

    customLayout->addWidget(hintEdit);  // 先加提示框

    vmtCustomEdit = new QTextEdit(this);
    vmtCustomEdit->setPlaceholderText(
        "在此输入VMT内容，变量占位符会在生成时自动替换为实际路径"
        );
    vmtCustomEdit->setMinimumHeight(250);

    vmtCustomEdit->setStyleSheet(
        "QTextEdit {"
        "   font-family: 'Consolas', 'Courier New', monospace;"
        "   font-size: 10pt;"
        "   border: 1px solid #ced4da;"
        "   border-radius: 4px;"
        "   padding: 8px;"
        "}"
        "QTextEdit:focus {"
        "   border-color: #86b7fe;"
        "   outline: 0;"
        "}"
        );
    customLayout->addWidget(vmtCustomEdit);

    customLayout->setStretchFactor(hintEdit, 0);    // 提示框不拉伸
    customLayout->setStretchFactor(vmtCustomEdit, 1); // 编辑框拉伸

    mainLayout->addWidget(groupCustom);
    // 让groupCustom获得所有额外空间
    mainLayout->setStretchFactor(groupCustom, 2);

    //确定取消
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        // 检查：如果勾选了启用差分，但下拉框没有可用选项或没选中
        if (chkHasDiff->isChecked() &&
            (comboDiff->count() == 0 || comboDiff->currentIndex() < 0)) {
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

    // 两个alpha互斥逻辑连接
    connect(chkAlphaTexture, &QCheckBox::toggled, this, &MaterialSetting::onAlphaTextureToggled);
    connect(chkAlpha, &QCheckBox::toggled, this, &MaterialSetting::onAlphaHighlightToggled);

    // 初始化Lightwarp状态（默认选工具自带）
    onLightwarpSourceChanged(2);    //工具自带
    onBumpmapSourceChanged(0);    // 不填
    onEnvmapSourceChanged(0);     // 不填
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
    comboDiff->setEnabled(checked);
}

//设置材质
void MaterialSetting::setMaterialData(Material &mat, const QList<Component> &comps)
{
    tempComponents = comps;  // 保存组件列表
    currentMaterialId = mat.id;  // 保存当前材质ID

    // 填充组件下拉框
    comboComponent->clear();
    for (const auto &c : comps) {
        comboComponent->addItem(c.name);
    }
    comboComponent->setCurrentText(mat.Mcomponent);

    onComponentChanged(comboComponent->currentIndex());  // 刷新材质下拉框

    // 设置下拉框里头的当前材质（用vmtName匹配）
    if (!mat.vmtName.isEmpty()) {
        for (int i = 0; i < comboMatName->count(); i++) {
            if (comboMatName->itemData(i).toString() == mat.vmtName) {
                comboMatName->setCurrentIndex(i);
                break;
            }
        }
    }

    // 先获取当前选择的材质是否有差分
    QString currentVmtName = comboMatName->currentData().toString();
    int compIndex = comboComponent->currentIndex();
    bool materialHasDiff = false;

    if (compIndex >= 0 && compIndex < tempComponents.size()) {
        const Component &comp = tempComponents[compIndex];
        for (const auto &m : comp.materials) {
            if (m.vmtName == currentVmtName) {
                materialHasDiff = m.hasDiff;
                break;
            }
        }
    }

    // 如果材质本身有差分，强制勾选启用差分
    if (materialHasDiff) {
        chkHasDiff->setChecked(true);
        chkHasDiff->setEnabled(false);  // 禁用，不让用户取消
    } else {
        chkHasDiff->setEnabled(true);
    }

    // 设置差分
    if (mat.hasDiff && !mat.diffNames.isEmpty()) {
        QString target = mat.diffNames.first();
        for (int i = 0; i < comboDiff->count(); i++) {
            QString text = comboDiff->itemText(i);
            QString name = text.split(" (").first();
            if (name == target) {
                comboDiff->setCurrentIndex(i);
                break;
            }
        }
    }
    else if (materialHasDiff && comboDiff->count() > 0)
    {
        // 如果有差分但没设置，默认选第一个
        comboDiff->setCurrentIndex(0);
    }
    // 贴图路径
    editFilename->setText(mat.MFilename);
    // Lightwarp
    editLightwarp->setText(mat.Mlightwarp);
    int lightwarpIndex = comboLightwarpSource->findData(mat.MlightwarpSource);
    if (lightwarpIndex >= 0) {
        comboLightwarpSource->setCurrentIndex(lightwarpIndex);
    } else {
        comboLightwarpSource->setCurrentIndex(2); // 默认工具自带
    }
    onLightwarpSourceChanged(comboLightwarpSource->currentIndex());

    // Bumpmap
    editBumpmap->setText(mat.Mbumpmap);
    int bumpmapIndex = comboBumpmapSource->findData(mat.MbumpmapSource);
    if (bumpmapIndex >= 0) {
        comboBumpmapSource->setCurrentIndex(bumpmapIndex);
    } else {
        comboBumpmapSource->setCurrentIndex(0); // 默认不填
    }
    onBumpmapSourceChanged(comboBumpmapSource->currentIndex());

    // Envmap
    editEnvmap->setText(mat.Menvmap);
    int envmapIndex = comboEnvmapSource->findData(mat.MenvmapSource);
    if (envmapIndex >= 0) {
        comboEnvmapSource->setCurrentIndex(envmapIndex);
    } else {
        comboEnvmapSource->setCurrentIndex(0); // 默认不填
    }
    onEnvmapSourceChanged(comboEnvmapSource->currentIndex());

    vmtCustomEdit->setText(mat.customVmtContent);

    chkAlphaTexture->setChecked(mat.useAlphaTexture);
    editAlphaTexture->setText(mat.alphaTextureName);

    // 根据状态更新控件可用性（互斥逻辑）
    if (mat.useAlphaTexture) {
        // 如果使用了独立Alpha贴图，禁用高亮Alpha
        chkAlpha->setChecked(false);
        chkAlpha->setEnabled(false);
        editAlphaTexture->setEnabled(true);
    } else if (mat.MAlpha) {
        // 如果使用了高亮Alpha，禁用独立Alpha贴图
        chkAlphaTexture->setChecked(false);
        chkAlphaTexture->setEnabled(false);
        editAlphaTexture->setEnabled(false);
    } else {
        // 都没使用，都启用
        chkAlpha->setEnabled(true);
        chkAlphaTexture->setEnabled(true);
        editAlphaTexture->setEnabled(false);  // 没勾选时输入框禁用
    }

    // 渲染参数
    chkAlpha->setChecked(mat.MAlpha);
    chkNocull->setChecked(mat.Mnocull);
    chkNodecal->setChecked(mat.Mnodecal);
    chkSelfillum->setChecked(mat.Mselfillum);
    chkHalflambert->setChecked(mat.Mhalflambert);
    chkPhong->setChecked(mat.Mphong);
    chkTranslucent->setChecked(mat.Mtranslucent);
    chkAlphatest->setChecked(mat.Malphatest);
}

//mainwindow材质列表关闭本弹窗后获取材质信息
void MaterialSetting::getMaterialData(Material &mat)
{
    // 基础信息
    mat.Mname = comboMatName->currentText().split(" (").first();
    mat.vmtName = comboMatName->currentData().toString();
    mat.Mcomponent = comboComponent->currentText();

    // ===== 修改：hasDiff 由材质本身决定，不依赖复选框 =====
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

    mat.hasDiff = materialHasDiff;  // 直接使用材质的实际差分属性

    // 差分处理
    if (mat.hasDiff) {
        if (comboDiff->count() > 0 && comboDiff->currentIndex() >= 0) {
            mat.diffNames.clear();
            mat.diffNames << comboDiff->currentText();
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
    mat.MAlpha = chkAlpha->isChecked() ? 1 : 0;
    mat.Mnocull = chkNocull->isChecked() ? 1 : 0;
    mat.Mnodecal = chkNodecal->isChecked() ? 1 : 0;
    mat.Mselfillum = chkSelfillum->isChecked() ? 1 : 0;
    mat.Mhalflambert = chkHalflambert->isChecked() ? 1 : 0;
    mat.Mphong = chkPhong->isChecked() ? 1 : 0;
    mat.Mtranslucent = chkTranslucent->isChecked() ? 1 : 0;
    mat.Malphatest = chkAlphatest->isChecked() ? 1 : 0;
}

//调整所属组件，刷新材质下拉框
void MaterialSetting::onComponentChanged(int index)
{
    comboMatName->clear();
    comboDiff->clear();
    chkHasDiff->setChecked(false);
    chkHasDiff->setEnabled(true);  // 先恢复启用
    comboDiff->setEnabled(false);

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
    comboDiff->clear();
    chkHasDiff->setChecked(false);
    chkHasDiff->setEnabled(true);  // 先恢复启用
    comboDiff->setEnabled(false);

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
            comboDiff->setEnabled(true);

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
                    comboDiff->addItem(diff);
                }
            }

            // 如果所有差分都被使用了，就禁用差分选择
            if (comboDiff->count() == 0) {
                chkHasDiff->setChecked(false);
                comboDiff->setEnabled(false);
                // 可以提示用户所有差分都已使用
            }
            else {
                // 无差分的材质，保持复选框可用
                chkHasDiff->setEnabled(true);
            }
        }
        break;
    }
}


void MaterialSetting::setEditingMode(bool isEdit)
{
    // 如果是编辑模式，可能需要锁定某些字段不让修改
    // 比如组件和材质名称在编辑时不应该再改变
    if (isEdit) {
        comboComponent->setEnabled(false);
        comboMatName->setEnabled(false);
    } else {
        // 新建模式，可以自由选择
        comboComponent->setEnabled(true);
        comboMatName->setEnabled(true);
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
