#include "mainwindow.h"
#include "componentsetting.h"
#include "materialsetting.h"
#include "processors/vtfgenerator.h"
#include "processors/vpkgenerator.h"
#include "processors/texturepreprocessor.h"
#include "processors/vmtgenerator.h"
#include "variantconfigdialog.h"
#include "utils/fileutils.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QSettings>
#include <QWidget>
#include <QCloseEvent>
#include <QTreeWidget>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , workerThread(nullptr)
    , worker(nullptr)
{
    ui->setupUi(this);


    this->setWindowTitle("L4D2MaterialsHelper(v1.0)");

    // 初始化工作流变量
    skipPreprocess = false;
    skipVtf = false;
    skipVmt = false;
    skipVpk = false;

    QSettings settings("Local","Path");
    QString appPath = QCoreApplication::applicationDirPath();
    //获取QSettings存储，如果有
    //路径1
    QString src = settings.value("SourcePath").toString();
    QString dst = settings.value("OutputPath").toString();
    QString vtfPath = settings.value("VtfPath").toString();
    QString vpkPath = settings.value("VpkPath").toString();
    QString matPath = settings.value("MatPath").toString();
    QString roleName = settings.value("RoleName").toString();  // 读取角色名称
    //路径2
    ui->editSource->setText(!src.isEmpty()?src:(appPath+"/Resources/SourceTextures"));
    ui->editOutput->setText(!dst.isEmpty()?dst:(appPath+"/Resources/OutputComponents"));
    ui->editVtf->setText(!vtfPath.isEmpty()?vtfPath:(appPath+"/Resources/Tools/MareTF/maretf.exe"));
    ui->editVpk->setText(!vpkPath.isEmpty()?vpkPath:(appPath+"/Resources/Tools/Vpk/vpk.exe"));
    ui->editVpkMat->setText(!matPath.isEmpty()?matPath:"shiratsume/HonmeiKnit");
    ui->editName->setText(!roleName.isEmpty()?roleName:"shiratsume");  // 设置角色名称

    //组件表格样式
    ui->twComponents->setColumnCount(1);
    ui->twComponents->setHeaderLabels({"组件结构"});
    //子项缩进像素10
    ui->twComponents->setIndentation(10);

    QHeaderView* cpheader = ui->twComponents->header();
    ui->twComponents->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->twComponents->setColumnWidth(0,40);
    cpheader->setSectionResizeMode(0,QHeaderView::Fixed);
    for(int j = 0;j < ui->twComponents->columnCount(); j++)
    {
        //对第1列开始全部进行拉伸
        cpheader->setSectionResizeMode(j,QHeaderView::Stretch);
    }
    cpheader->setMinimumSectionSize(10);
    cpheader->setStretchLastSection(false);//防止进行二次拉伸，QTree会做一次拉伸，和前边重复了
    ui->twComponents->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);//以父窗口给的大小判定宽度,不以水平竖直布局拉伸


    //材质表格样式
    ui->tbMaterials->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//去水平滚动条
    //ui->tbMaterials->setColumnWidth(0,40);
    for(int i = 0; i<ui->tbMaterials->columnCount();i++)
        ui->tbMaterials->horizontalHeader()->setSectionResizeMode(i,QHeaderView::Stretch);//表格自动调整列宽，这里的代码是拉伸
    ui->tbMaterials->horizontalHeader()->setMinimumSectionSize(10);//最小列宽,可以压缩到最小是这样
    ui->tbMaterials->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);//以父窗口给的大小判定宽度

    ui->tbMaterials->setColumnCount(4);
    ui->tbMaterials->setHorizontalHeaderLabels({"材质名","所属组件","配置数量","操作"});

    updateMaterialTable();  // 初始化表格

    //进度条
    ui->progressBar->setValue(0);


    connect(ui->editName, &QLineEdit::editingFinished, this, &MainWindow::saveAllSettings);
    connect(ui->editSource, &QLineEdit::editingFinished, this, &MainWindow::saveAllSettings);
    connect(ui->editOutput, &QLineEdit::editingFinished, this, &MainWindow::saveAllSettings);
    connect(ui->editVtf, &QLineEdit::editingFinished, this, &MainWindow::saveAllSettings);
    connect(ui->editVpk, &QLineEdit::editingFinished, this, &MainWindow::saveAllSettings);
    connect(ui->editVpkMat, &QLineEdit::editingFinished, this, &MainWindow::saveAllSettings);
}

MainWindow::~MainWindow()
{
    delete ui;
}
//路径区
//源文件夹设置按钮
void MainWindow::on_btnSource_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this,"选择文件夹",ui->editSource->text());

    //设置源文件夹路径
    if(!path.isEmpty())
    {
        ui->editSource->setText(path);
    }
}

void MainWindow::saveAllSettings()
{
    QSettings settings("Local", "Path");
    settings.setValue("VtfPath", ui->editVtf->text());
    settings.setValue("VpkPath", ui->editVpk->text());
    settings.setValue("SourcePath", ui->editSource->text());
    settings.setValue("OutputPath", ui->editOutput->text());
    settings.setValue("MatPath", ui->editVpkMat->text());
    settings.setValue("RoleName", ui->editName->text());

    qDebug() << "设置已保存，角色名称:" << ui->editName->text();
}

//输出目录设置按钮
void MainWindow::on_btnOutput_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this,"选择文件夹",ui->editOutput->text());

    //设置输出文件夹路径
    if(!path.isEmpty())
    {
        ui->editOutput->setText(path);
    }
}

//vtf路径设置按钮
void MainWindow::on_btnVtf_clicked()
{
    QString path = QFileDialog::getOpenFileName(this,"选择vtf转换器",ui->editOutput->text(),"可执行文件(*.exe)");

    //设置输出文件夹路径
    if(!path.isEmpty())
    {
        ui->editVtf->setText(path);
    }
}

//vpk路径设置按钮
void MainWindow::on_btnVpk_clicked()
{
    QString path = QFileDialog::getOpenFileName(this,"选择vtf转换器",ui->editOutput->text(),"可执行文件(*.exe)");

    //设置输出文件夹路径
    if(!path.isEmpty())
    {
        ui->editVpk->setText(path);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // 如果任务还在运行，等待结束
    if (workerThread && workerThread->isRunning()) {
        workerThread->quit();
        workerThread->wait(5000);
        if (workerThread->isRunning()) {
            workerThread->terminate();
            workerThread->wait(1000);
        }
    }

    // 清理（如果 onWorkFinished 已经被调用，指针已经是 nullptr）
    if (worker) {
        delete worker;
        worker = nullptr;
    }
    if (workerThread) {
        delete workerThread;
        workerThread = nullptr;
    }

    // 保存设置...
    QSettings settings("Local", "Path");
    settings.setValue("VtfPath", ui->editVtf->text());
    settings.setValue("VpkPath", ui->editVpk->text());
    settings.setValue("SourcePath", ui->editSource->text());
    settings.setValue("OutputPath", ui->editOutput->text());
    settings.setValue("MatPath", ui->editVpkMat->text());
    settings.setValue("RoleName", ui->editName->text());

    event->accept();
}

//打开文件夹
void MainWindow::on_btnSourceOpen_clicked()
{
    QSettings settings("Local", "Path");

    //存在打开
    QString sourcepath = ui->editSource->text();

    if(sourcepath.isEmpty())
    {
        QMessageBox::warning(this,"提示","文件夹空或不存在!",QMessageBox::Ok,QMessageBox::Ok);
        return ;
    }

    if(QDir(sourcepath).exists())
        QDesktopServices::openUrl(QUrl::fromLocalFile(sourcepath));
    else
    {
        QMessageBox::warning(this,"提示","文件夹空或不存在!",QMessageBox::Ok,QMessageBox::Ok);
        settings.setValue("SourcePath", "");
    }
}

void MainWindow::on_btnOutputOpen_clicked()
{

    QSettings settings("Local", "Path");

    //存在打开
    QString outputpath = ui->editOutput->text();

    if(outputpath.isEmpty())
    {
        QMessageBox::warning(this,"提示","文件夹空或不存在!",QMessageBox::Ok,QMessageBox::Ok);
        return ;
    }
    if(QDir(outputpath).exists())
        QDesktopServices::openUrl(QUrl::fromLocalFile(outputpath));
    else
    {
        QMessageBox::warning(this,"提示","文件夹空或不存在!",QMessageBox::Ok,QMessageBox::Ok);
        settings.setValue("OutputPath", "");
    }
}


void MainWindow::on_btnVtfOpen_clicked()
{
    QSettings settings("Local", "Path");

    //存在打开
    QString vtfpath = ui->editVtf->text();

    if(vtfpath.isEmpty())
    {
        QMessageBox::warning(this,"提示","路径空!",QMessageBox::Ok,QMessageBox::Ok);
        return ;
    }

    QFileInfo vtf(vtfpath);
    if(vtf.exists() && vtf.isFile())
    {
        QString path = vtf.absolutePath();
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    }
    else
    {
        QMessageBox::warning(this,"提示","文件夹空或不存在!",QMessageBox::Ok,QMessageBox::Ok);
        settings.setValue("VtfPath", "");
        ui->editVtf->clear();
    }
}


void MainWindow::on_btnVpkOpen_clicked()
{
    QSettings settings("Local", "Path");

    //存在打开
    QString vpkpath = ui->editVpk->text();

    if(vpkpath.isEmpty())
    {
        QMessageBox::warning(this,"提示","路径空!",QMessageBox::Ok,QMessageBox::Ok);
        return ;
    }

    QFileInfo vpk(vpkpath);
    if(vpk.exists() && vpk.isFile())
    {
        QString path = vpk.absolutePath();
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    }
    else
    {
        QMessageBox::warning(this,"提示","文件夹空或不存在!",QMessageBox::Ok,QMessageBox::Ok);
        settings.setValue("VpkPath", "");
        ui->editVpk->clear();
    }
}


//组件区
//增加组件按钮
void MainWindow::on_btnNewComponent_clicked()
{
    Componentsetting dlg(this);

    if (dlg.exec() == QDialog::Accepted) {
        QString compName = dlg.getComponentName();

        // 检查组件名是否为空
        if (compName.isEmpty()) {
            QMessageBox::warning(this, "错误", "组件名不能为空");
            return;
        }
        // 检查组件名是否重复
        for (const Component &comp : components) {
            if (comp.name == compName) {
                QMessageBox::warning(this, "错误", "组件名已存在，请使用其他名称");
                return;
            }
        }

        Component comp;
        comp.select = false;
        comp.name = compName;
        comp.materials = dlg.getMaterialList();

        //调用组件Info设置对话框
        QStringList compNames;
        for (const auto &c : components) {
            compNames << c.name;
        }
        compNames << compName;  // 加上新组件

        //创建新组件的info对象,然后展示
        Addoninfo info;
        info.addontitle = compName + "材质包";  // 默认标题
        info.addonauthor = globalAddonInfo.addonauthor;
        info.addonversion = globalAddonInfo.addonversion;
        info.addontagline = globalAddonInfo.addontagline;
        info.addonDescription = globalAddonInfo.addonDescription;

        if (showComponentInfoDialog(info, compNames, compName)) {
            // 保存到新组件的compInfo中
            comp.compInfo.addontitle = info.addontitle;
            comp.compInfo.addonDescription = info.addonDescription;

            // 更新全局设置
            globalAddonInfo.addonversion = info.addonversion;
            globalAddonInfo.addonauthor = info.addonauthor;
            globalAddonInfo.addontagline = info.addontagline;
        }

        components.append(comp);
        updateTreeWidget();
    }
}

// 删除选中组件的按钮
void MainWindow::on_btnDelComponent_clicked()
{
    QList<QString> deletedComponents;  // 记录被删除的组件名

    // 先记录要删除的组件名
    for (int i = 0; i < components.size(); i++) {
        QTreeWidgetItem *item = ui->twComponents->topLevelItem(i);
        if (item && item->checkState(0) == Qt::Checked) {
            deletedComponents.append(components[i].name);
        }
    }

    if (deletedComponents.isEmpty()) {
        QMessageBox::warning(this, "提示", "请选中要删除的组件！");
        return;
    }

    // 确认删除
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认删除",
        QString("确定要删除选中的 %1 个组件吗？\n对应的材质配置也会被删除。").arg(deletedComponents.size()),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply != QMessageBox::Yes) return;

    // 第一步：删除组件
    for (int i = components.size() - 1; i >= 0; i--) {
        QTreeWidgetItem *item = ui->twComponents->topLevelItem(i);
        if (item && item->checkState(0) == Qt::Checked) {
            components.removeAt(i);
        }
    }

    // 第二步：清理材质表中属于被删除组件的材质
    QList<Material> validMaterials;
    int removedCount = 0;

    for (const Material &mat : materials) {
        bool componentExists = false;

        // 检查这个材质所属的组件是否还存在
        for (const Component &comp : components) {
            if (comp.name == mat.Mcomponent) {
                componentExists = true;
                break;
            }
        }

        if (componentExists) {
            validMaterials.append(mat);  // 组件还在，保留材质
        } else {
            removedCount++;  // 组件已删除，材质也要删除
            qDebug() << "删除组件关联的材质:" << mat.Mname << mat.vmtName << mat.diffNames;
        }
    }

    // 更新材质表
    if (removedCount > 0) {
        //valid不包含不合法的，直接赋值给全局变量来删除不合法的。
        materials = validMaterials;
        updateMaterialTable();
        QMessageBox::information(this, "提示",
                                 QString("已删除 %1 个组件，并清理了 %2 个相关的材质配置")
                                     .arg(deletedComponents.size()).arg(removedCount));
    }

    updateTreeWidget();  // 刷新显示
}

//编辑组件按钮
void MainWindow::on_btnEditComponent_clicked()
{
    int selected = -1;
    for(int i = 0; i < components.size(); i++) {
        QTreeWidgetItem* item = ui->twComponents->topLevelItem(i);
        if (item && item->checkState(0) == Qt::Checked) {
            selected = i;
            break;
        }
    }

    if(selected == -1) {
        QMessageBox::warning(this,"提示","请选中一个组件！");
        return;
    }

    Component &comp = components[selected];

    Componentsetting dlg(this);
    dlg.setComponentData(comp.name, comp.materials);

    if(dlg.exec() == QDialog::Accepted) {
        QString newName = dlg.getComponentName();
        QString oldName = comp.name;  // 保存旧名字

        if (newName != comp.name) {
            if (newName.isEmpty()) {
                QMessageBox::warning(this, "错误", "组件名不能为空");
                return;
            }
            for (int i = 0; i < components.size(); i++) {
                if (i != selected && components[i].name == newName) {
                    QMessageBox::warning(this, "错误", "组件名已存在");
                    return;
                }
            }
        }

        // 保存旧的材质列表和新的组件列表材质
        QList<MaterialInComponent> oldMaterials = comp.materials;
        QList<MaterialInComponent> newMaterials = dlg.getMaterialList();

        QMap<QString,QString> vmtNameChangeMap;
        QMap<QString,QString> displayNameChangeMap;


        //记录新旧vmtName display名字不同的材质
        for(MaterialInComponent& micold : oldMaterials)
        {
            for(MaterialInComponent& micnew: newMaterials)
            {
                if(micold.matId == micnew.matId)
                {
                    if(micnew.vmtName!=micold.vmtName)
                    {
                        vmtNameChangeMap[micold.matId] = micnew.vmtName;
                    }
                    if(micnew.name!=micold.name)
                    {
                        displayNameChangeMap[micold.matId] = micnew.name;
                    }
                }
            }
        }

        bool nameChanged = false;
        if (newName != oldName) {
            for (Material &mat : materials) {
                if (mat.Mcomponent == oldName) {
                    mat.Mcomponent = newName;
                    if(mat.diffNames.isEmpty())
                        qDebug()<<mat.vmtName<<mat.Mcomponent<<mat.vmtName<<"无差分"<<mat.materialDefId;
                    else
                        qDebug()<<mat.vmtName<<mat.Mcomponent<<mat.vmtName<<mat.diffNames.first()<<mat.materialDefId;
                    nameChanged = true;
                }
            }

            qDebug()<<"1";
            for(MaterialInComponent &mic: comp.materials)
            {
                if(!mic.diffNames.isEmpty())
                    qDebug()<<mic.diffNames.first()<<mic.matId;
                else
                    qDebug()<<mic.vmtName<<"无差分"<<mic.matId;
            }
            qDebug()<<"1";

            if(nameChanged)
            {
                comp.name = newName;
                qDebug()<<comp.name;
                qDebug()<<"2";
                for(MaterialInComponent &mic: comp.materials)
                {
                    if(!mic.diffNames.isEmpty())
                        qDebug()<<mic.diffNames.first()<<mic.matId;
                    else
                        qDebug()<<mic.vmtName<<"无差分"<<mic.matId;
                }
                qDebug()<<"2";
            }

            // 同步变体配置
            for (ComponentVariants &cv : projectVariants) {
                if (cv.componentName == oldName) {
                    cv.componentName = newName;
                }
            }
        }
        comp.materials = newMaterials;

        //清理材质表：检查组件、材质、差分
        QList<Material> validMaterials;
        int removedCount = 0;
        int modifiedCount = 0;

        for (Material &mat : materials) {
            // 只处理属于当前编辑组件的材质
            if (mat.Mcomponent != comp.name) {
                validMaterials.append(mat);
                continue;
            }

            if(vmtNameChangeMap.contains(mat.materialDefId))
            {
                mat.vmtName = vmtNameChangeMap[mat.materialDefId];
                modifiedCount++;
            }
            if(displayNameChangeMap.contains(mat.materialDefId))
            {
                mat.Mname = displayNameChangeMap[mat.materialDefId];
                modifiedCount++;
            }

            //在新列表，在旧列表（保留/编辑） 不在新列表，在旧列表（删） 在新列表，不在旧列表(增)

            // 在当前组件的旧材质列表中查找对应的材质定义，找旧的材质列表里头有没有这个在组件里的材质，有就用指针oldMic记录
            const MaterialInComponent *oldMic = nullptr;
            for (const MaterialInComponent &mic : oldMaterials) {

                if (mic.matId == mat.materialDefId) {
                    oldMic = &mic;
                    break;
                }
            }

            // 在当前组件的新材质列表中查找对应的材质定义，找旧的材质列表里头有没有这个组件里新的材质,有就用指针newMic记录
            const MaterialInComponent *newMic = nullptr;
            for (const MaterialInComponent &mic : newMaterials) {

                qDebug()<<"mic.matId"<<mic.matId<<"mat.materialDefId"<<mat.materialDefId;

                if (mic.matId == mat.materialDefId) {
                    newMic = &mic;
                    break;
                }
            }

            // 如果这个旧材质在新组件中已经不存在了，删除它
            if (!newMic) {
                removedCount++;
                qDebug() << "删除材质(组件中已移除):" << mat.Mname << mat.vmtName<<mat.materialDefId<<mat.id;
                continue;
            }

            //处理某个具体材质从无差分变成有差分
            if (!oldMic->hasDiff && newMic->hasDiff) {
                // 这个材质从无差分变成了有差分
                // 把该材质的所有实例变成有差分，并选择第一个差分名
                mat.hasDiff = true;
                if (!newMic->diffNames.isEmpty()) {
                    mat.diffNames = QStringList() << newMic->diffNames.first();
                    modifiedCount++;
                    qDebug() << "材质从无差分变成有差分:" << mat.Mname << mat.vmtName
                             << "选择差分:" << mat.diffNames.first();
                }
                validMaterials.append(mat);
                continue;
            }

            //处理有差分被删除的情况（检查差分是否还在）
            if (mat.hasDiff) {
                // 检查当前使用的差分是否还在新组件的差分列表中
                if (!mat.diffNames.isEmpty()) {
                    QString currentDiff = mat.diffNames.first();
                    if (newMic->diffNames.contains(currentDiff)) {
                        // 如果当前差分还在，保留
                        validMaterials.append(mat);
                    } else {
                        // 如果当前差分已被删除，删除这个材质实例
                        removedCount++;
                        qDebug() << "删除材质(差分被删除):" << mat.Mname
                                 << mat.vmtName << currentDiff;
                    }
                } else {
                    // 有差分但没有选具体差分，这种情况不应该发生
                    removedCount++;
                }
                continue;
            }

            // 无差分的材质 直接保留
            validMaterials.append(mat);
        }

        //处理新增的材质和差分
        int addedCount = 0;

        // 遍历新材质列表进行同步
        for (const MaterialInComponent &newMic : newMaterials) {
            // 检查这个材质在 validMaterials 中是否已存在（通过 component + vmtName）
            bool materialExists = false;
            for (const Material &mat : validMaterials) {
                if (mat.Mcomponent == comp.name && mat.materialDefId == newMic.matId) {
                    materialExists = true;
                    break;
                }
            }

            if (!materialExists) {
                // 材质不存在 → 添加该材质的所有差分
                if (newMic.hasDiff && !newMic.diffNames.isEmpty()) {
                    for (const QString &diff : newMic.diffNames) {
                        Material newMat;
                        newMat.id = QUuid::createUuid().toString(QUuid::WithoutBraces);  // 新生成实例ID
                        newMat.materialDefId = newMic.matId;  // 保存材质定义ID
                        newMat.Mcomponent = comp.name;
                        newMat.vmtName = newMic.vmtName;
                        newMat.Mname = newMic.name;
                        newMat.hasDiff = true;
                        newMat.diffNames = QStringList() << diff;
                        newMat.isinit = false;
                        validMaterials.append(newMat);
                        addedCount++;
                    }
                } else {
                    // 无差分材质
                    Material newMat;
                    newMat.id = QUuid::createUuid().toString(QUuid::WithoutBraces);  // 新生成实例ID
                    newMat.materialDefId = newMic.matId;  // 保存材质定义ID
                    newMat.Mcomponent = comp.name;
                    newMat.vmtName = newMic.vmtName;
                    newMat.Mname = newMic.name;
                    newMat.hasDiff = false;
                    newMat.isinit = false;
                    validMaterials.append(newMat);
                    addedCount++;
                }
            } else {
                // 材质已存在，检查是否有新增的差分
                // 收集这个材质在 validMaterials 中已有的差分名
                QStringList existingDiffs;
                for (const Material &mat : validMaterials) {
                    if (mat.Mcomponent == comp.name && mat.vmtName == newMic.vmtName) {
                        if (mat.hasDiff && !mat.diffNames.isEmpty()) {
                            existingDiffs.append(mat.diffNames.first());
                        }
                    }
                }

                // 遍历新材质的所有差分，找出新增的
                for (const QString &newDiff : newMic.diffNames) {
                    if (!existingDiffs.contains(newDiff)) {
                        // 新增差分，创建配置实例
                        Material newMat;
                        newMat.id = QUuid::createUuid().toString(QUuid::WithoutBraces);  // 新生成实例ID
                        newMat.materialDefId = newMic.matId;  // 保存材质定义ID
                        newMat.Mcomponent = comp.name;
                        newMat.vmtName = newMic.vmtName;
                        newMat.Mname = newMic.name;
                        newMat.hasDiff = true;
                        newMat.diffNames = QStringList() << newDiff;
                        newMat.isinit = false;
                        validMaterials.append(newMat);
                        addedCount++;
                    }
                }
            }
        }

        // 更新材质表
        if (removedCount > 0 || modifiedCount > 0 || addedCount > 0) {
            materials = validMaterials;
            updateMaterialTable();
            QString msg;
            if (removedCount > 0) msg += QString("删除 %1 个材质; ").arg(removedCount);
            if (modifiedCount > 0) msg += QString("更新 %1 个材质; ").arg(modifiedCount);
            if (addedCount > 0) msg += QString("新增 %1 个差分配置").arg(addedCount);
            QMessageBox::information(this, "提示", msg);
        }

        updateTreeWidget();
    }
}

// 更新树形控件，单纯就是显示，数据早已存储。
void MainWindow::updateTreeWidget()
{
    ui->twComponents->clear();
    ui->twComponents->setColumnCount(1);
    ui->twComponents->setHeaderLabels({"组件结构"});
    //循环，第一层-组件
    for (int i = 0; i < components.size(); i++) {
        Component &comp = components[i];

        QTreeWidgetItem *item = new QTreeWidgetItem(ui->twComponents);
        item->setText(0, comp.name + QString(" (%1个材质)").arg(comp.materials.size()));
        //设置不检查
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(0, comp.select ? Qt::Checked : Qt::Unchecked);

        for (const MaterialInComponent &mat : comp.materials) {
            QTreeWidgetItem *matItem = new QTreeWidgetItem(item);
            QString matText = "  └─ " + mat.name+ " (" + mat.vmtName + ")";
            if (mat.hasDiff) {
                //循环2，第二层-材质
                matText += QString(" (%1个差分)").arg(mat.diffNames.size());
            }
            matItem->setText(0, matText);

            if (mat.hasDiff) {
                for (const QString &diffName : mat.diffNames) {
                    //循环3，第三层-差分
                    QTreeWidgetItem *diffItem = new QTreeWidgetItem(matItem);
                    diffItem->setText(0, "      └─ " + diffName);
                }
            }
        }
    }
    ui->twComponents->expandAll();
}

// 组件信息设置按钮
void MainWindow::on_btnInfo_clicked()
{
    // 找到第一个勾选的组件
    int selectedIndex = -1;
    QString selectedName;

    for(int i = 0; i < components.size(); i++) {
        QTreeWidgetItem* item = ui->twComponents->topLevelItem(i);
        if (item && item->checkState(0) == Qt::Checked) {
            selectedIndex = i;
            selectedName = components[i].name;
            break;  // 只取第一个
        }
    }

    if(selectedIndex == -1) {
        QMessageBox::warning(this, "提示", "请至少勾选一个组件！");
        return;
    }

    // 获取选中的组件
    Component &comp = components[selectedIndex];

    // 准备组件列表（用于显示）
    QStringList compNames;
    for (const auto &c : components) {
        compNames << c.name;
    }

    // 准备info数据
    Addoninfo info;
    info.addontitle = comp.compInfo.addontitle;
    if (info.addontitle.isEmpty()) {
        info.addontitle = comp.name + "材质包";  // 默认标题
    }
    info.addonDescription = comp.compInfo.addonDescription;
    info.addonauthor = globalAddonInfo.addonauthor;
    info.addonversion = globalAddonInfo.addonversion;
    info.addontagline = globalAddonInfo.addontagline;

    // 显示对话框
    if (showComponentInfoDialog(info, compNames, comp.name)) {
        // 保存到组件的compInfo中
        comp.compInfo.addontitle = info.addontitle;
        comp.compInfo.addonDescription = info.addonDescription;

        // 更新全局设置
        globalAddonInfo.addonversion = info.addonversion;
        globalAddonInfo.addonauthor = info.addonauthor;
        globalAddonInfo.addontagline = info.addontagline;

        QMessageBox::information(this, "提示",
                                 QString("组件 '%1' 的信息已保存").arg(comp.name));
    }
}

// 显示组件Info设置对话框
bool MainWindow::showComponentInfoDialog(Addoninfo &info, const QStringList &componentNames, const QString &currentComponent)
{
    // 保存传入的组件描述，用于判断是否使用全局
    QString originalDesc = info.addonDescription;

    // 创建对话框
    QDialog dlg(this);
    dlg.setWindowTitle("组件信息设置");
    dlg.setFixedSize(450, 450);

    // 创建布局
    QVBoxLayout *mainLayout = new QVBoxLayout(&dlg);
    QLabel *tipLabel = new QLabel("注意：当前正在设置组件：" + currentComponent);
    tipLabel->setStyleSheet("color: blue;");
    mainLayout->insertWidget(0, tipLabel);

    // 组件选择（只读显示当前组件）
    QHBoxLayout *compLayout = new QHBoxLayout();
    compLayout->addWidget(new QLabel("当前组件:"));
    QLineEdit *editCurrentComp = new QLineEdit(currentComponent);
    editCurrentComp->setReadOnly(true);
    compLayout->addWidget(editCurrentComp);
    mainLayout->addLayout(compLayout);

    // 添加分隔线
    QFrame *line1 = new QFrame();
    line1->setFrameShape(QFrame::HLine);
    line1->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(line1);

    // ==== 全局设置区域 ====
    QGroupBox *groupGlobal = new QGroupBox("全局设置（修改后会影响所有新组件）");
    QVBoxLayout *globalLayout = new QVBoxLayout(groupGlobal);

    // addonversion（全局）
    QHBoxLayout *versionLayout = new QHBoxLayout();
    versionLayout->addWidget(new QLabel("版本号:"));
    QLineEdit *editVersion = new QLineEdit(globalAddonInfo.addonversion);
    versionLayout->addWidget(editVersion);
    globalLayout->addLayout(versionLayout);

    // addonauthor（全局）
    QHBoxLayout *authorLayout = new QHBoxLayout();
    authorLayout->addWidget(new QLabel("作者:"));
    QLineEdit *editAuthor = new QLineEdit(globalAddonInfo.addonauthor);
    authorLayout->addWidget(editAuthor);
    globalLayout->addLayout(authorLayout);

    // addontagline（全局）
    QHBoxLayout *taglineLayout = new QHBoxLayout();
    taglineLayout->addWidget(new QLabel("标签行:"));
    QLineEdit *editTagline = new QLineEdit(globalAddonInfo.addontagline);
    taglineLayout->addWidget(editTagline);
    globalLayout->addLayout(taglineLayout);

    // 全局描述
    QHBoxLayout *globalDescLayout = new QHBoxLayout();
    globalDescLayout->addWidget(new QLabel("全局描述:"));
    QLineEdit *editGlobalDesc = new QLineEdit(globalAddonInfo.addonDescription);
    editGlobalDesc->setPlaceholderText("所有组件的默认描述");
    globalDescLayout->addWidget(editGlobalDesc);
    globalLayout->addLayout(globalDescLayout);

    mainLayout->addWidget(groupGlobal);

    // 添加分隔线
    QFrame *line2 = new QFrame();
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(line2);

    // ==== 当前组件独有设置 ====
    QGroupBox *groupCurrent = new QGroupBox("当前组件设置");
    QVBoxLayout *currentLayout = new QVBoxLayout(groupCurrent);

    // addontitle（组件标题）
    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->addWidget(new QLabel("组件标题:"));
    QLineEdit *editTitle = new QLineEdit(info.addontitle);
    editTitle->setPlaceholderText("输入此组件的标题");
    titleLayout->addWidget(editTitle);
    currentLayout->addLayout(titleLayout);

    // 组件描述
    QHBoxLayout *descLayout = new QHBoxLayout();
    descLayout->addWidget(new QLabel("组件描述:"));
    QLineEdit *editDesc = new QLineEdit(info.addonDescription);
    editDesc->setPlaceholderText("输入此组件的描述");
    descLayout->addWidget(editDesc);

    QCheckBox *chkUseGlobalDesc = new QCheckBox("使用全局描述");
    descLayout->addWidget(chkUseGlobalDesc);
    currentLayout->addLayout(descLayout);

    mainLayout->addWidget(groupCurrent);

    // SteamAppID（只读显示）
    QHBoxLayout *appidLayout = new QHBoxLayout();
    appidLayout->addWidget(new QLabel("Steam AppID:"));
    QLineEdit *editAppID = new QLineEdit(info.addonSteamAppID);
    editAppID->setReadOnly(true);
    appidLayout->addWidget(editAppID);
    mainLayout->addLayout(appidLayout);

    // 确定取消按钮
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttonBox);

    // ==== 初始化状态 ====
    // 判断是否使用全局描述：如果组件描述等于全局描述，就勾选
    bool isUsingGlobal = (info.addonDescription == globalAddonInfo.addonDescription);
    chkUseGlobalDesc->setChecked(isUsingGlobal);
    editDesc->setEnabled(!isUsingGlobal);

    // 如果不使用全局描述，确保编辑框显示的是组件的原始描述
    if (!isUsingGlobal) {
        editDesc->setText(info.addonDescription);
    }

    // 连接勾选框信号
    connect(chkUseGlobalDesc, &QCheckBox::toggled, [editDesc, editGlobalDesc](bool checked) {
        editDesc->setEnabled(!checked);
        if (checked) {
            // 勾选时，从全局描述框取值
            editDesc->setText(editGlobalDesc->text());
        }
        // 取消勾选时，不清空editDesc，保留当前文本让用户编辑
    });

    // 注意：不要连接 editGlobalDesc 的 textChanged 信号自动修改 editDesc
    // 否则会干扰用户的自定义输入

    // 连接信号
    connect(buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    // 显示对话框
    if (dlg.exec() == QDialog::Accepted) {
        // 1. 先保存全局设置（独立保存，不影响当前组件）
        globalAddonInfo.addonversion = editVersion->text().trimmed();
        globalAddonInfo.addonauthor = editAuthor->text().trimmed();
        globalAddonInfo.addontagline = editTagline->text().trimmed();
        globalAddonInfo.addonDescription = editGlobalDesc->text().trimmed();

        // 2. 保存当前组件的设置
        info.addontitle = editTitle->text().trimmed();

        // 根据复选框状态决定如何保存描述
        if (chkUseGlobalDesc->isChecked()) {
            // 使用全局描述：保存的是全局描述的值
            info.addonDescription = globalAddonInfo.addonDescription;
        } else {
            // 使用自定义描述：保存编辑框的内容
            info.addonDescription = editDesc->text().trimmed();
        }

        // 3. 全局字段也保存到info中（用于返回）
        info.addonversion = globalAddonInfo.addonversion;
        info.addonauthor = globalAddonInfo.addonauthor;
        info.addontagline = globalAddonInfo.addontagline;

        return true;
    }

    return false;
}


//材质区
//增加材质按钮,准备改成自动生成材质
void MainWindow::on_btnNewMaterial_clicked()
{
    int added = 0;
    int skipped = 0;

    //找materials全局变量是否存在这个
    for(Component comp : components)
    {
        for(MaterialInComponent mic : comp.materials)
        {
            bool exists = false;
            for(Material m: materials)
            {
                if(m.Mcomponent == comp.name && m.vmtName == mic.vmtName)
                {
                    exists = true;
                    break;
                }
            }

            if(exists)
            {
                skipped++;
                continue;
            }

            //没有重复,有差分情况
            if(mic.hasDiff && !mic.diffNames.isEmpty())
            {
                //对每一个差分加入material
                for(QString &diff : mic.diffNames)
                {
                    qDebug() << "=== 创建材质实例 ===";
                    qDebug() << "组件名:" << comp.name;
                    qDebug() << "材质名:" << mic.name;
                    qDebug() << "vmtName:" << mic.vmtName;
                    qDebug() << "差分名:" << diff;

                    qDebug() << "创建材质实例，差分名:" << diff;

                    Material newmat;
                    newmat.Mcomponent = comp.name;
                    newmat.vmtName = mic.vmtName;
                    newmat.Mname = mic.name;
                    newmat.id = QUuid::createUuid().toString(QUuid::WithoutBraces);  // 新生成实例ID
                    newmat.materialDefId = mic.matId;  // 保存材质定义ID
                    newmat.hasDiff = true;
                    newmat.diffNames = QStringList()<<diff;
                    newmat.isinit = false;
                    newmat.MlightwarpSource = SOURCE_TOOL;
                    newmat.Mlightwarp = QString("%1/Resources/Resources/Vtf/toon_light.vtf").arg(QApplication::applicationDirPath());
                    materials.append(newmat);
                    added++;
                }
            }
            //没重复无差分情况
            else
            {
                Material newmat;
                newmat.Mcomponent = comp.name;
                newmat.vmtName = mic.vmtName;
                newmat.Mname = mic.name;
                newmat.id = QUuid::createUuid().toString(QUuid::WithoutBraces);  // 新生成实例ID
                newmat.materialDefId = mic.matId;  // 保存材质定义ID
                newmat.hasDiff = false;
                newmat.diffNames = QStringList();
                newmat.isinit = false;
                newmat.MlightwarpSource = SOURCE_TOOL;
                newmat.Mlightwarp = QString("%1/Resources/Resources/Vtf/toon_light.vtf").arg(QApplication::applicationDirPath());
                materials.append(newmat);
                added++;
            }
        }
    }

    if(added > 0 || skipped > 0)
    {
        updateMaterialTable();
        updateTreeWidget();
        QMessageBox::information(this,"完成生成",QString("新增%1个差分，跳过%2个差分").arg(added).arg(skipped));
    }
    else
    {
        QMessageBox::information(this,"提示","所有材质已经生成,无需添加");
    }


}

//删除材质按钮
void MainWindow::on_btnDelMaterial_clicked()
{
    // 获取当前选中的行
    int currentRow = ui->tbMaterials->currentRow();

    if (currentRow < 0) {
        QMessageBox::warning(this, "提示", "请先选中要删除的材质");
        return;
    }

    //获取选中材质名，组件名
    QString component = ui->tbMaterials->item(currentRow,1)->text();
    QString matName = ui->tbMaterials->item(currentRow,0)->text();

    QString vmtName;
    for(Material& m : materials)
    {
        if(m.Mcomponent == component && m.Mname == matName)
        {
            vmtName = m.vmtName;
            break;
        }
    }

    //确认删除
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认删除",
        QString("确定要删除材质 \"%1\" 吗？").arg(matName),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        // 从列表中遍历组件名一样，然后vmtName又一样的差分进行移除
        for(int i = materials.size()-1 ; i >= 0; i-- )
        {
            if(materials[i].Mcomponent == component && materials[i].vmtName == vmtName)
            {
                materials.removeAt(i);
            }
        }

        // 刷新表格
        updateMaterialTable();
    }
}

//刷新材质表格
void MainWindow::updateMaterialTable()
{
    ui->tbMaterials->setAutoScroll(false);

    //由materials当场生成当场存储
    //QMap groupMap<组件名+材质名(qstring),材质组结构体(struct)>
    QMap<QString,MaterialGroup> groupMap;

    for(Material &m:materials)
    {
        //QMap groupMap<组件名+材质名(qstring),材质组结构体(struct)>
        QString key = m.Mcomponent+"|"+m.vmtName;

        //没加过这个组件
        if(!groupMap.contains(key))
        {
            MaterialGroup group;
            group.component = m.Mcomponent;
            group.vmtName = m.vmtName;
            group.displayName = m.Mname;
            group.diffList.append(&m);

            groupMap[key] = group;
        }
        else
        {
            //加过这个组件直接加mat进去
            groupMap[key].diffList.append(&m);
        }
    }

    ui->tbMaterials->setRowCount(groupMap.size());
    int row = 0;
    for(auto it = groupMap.begin();it !=groupMap.end();it++)
    {
        MaterialGroup &group = it.value();
        //一二列直接显示信息
        ui->tbMaterials->setItem(row,0,new QTableWidgetItem(group.displayName));
        ui->tbMaterials->setItem(row,1,new QTableWidgetItem(group.component));

        //三列该材质已配置多少个差分,不用检查实际差分，只是检查是不是初始化
        int inited = 0;
        for(Material* diff : group.diffList)
        {
            if(diff->isinit)
            {
                inited++;
                if(!diff->diffNames.isEmpty())
                    qDebug()<<"当前的差分名字:"<<diff->diffNames.first()<<"diff->id"<<diff->id;
                else
                    qDebug()<<"当前的差分名字:无差分的"<<diff->vmtName<<"diff->id"<<diff->id;
            }
        }

        QString status = QString("已配置:%1/%2").arg(inited).arg(group.diffList.size());
        qDebug()<<"group.diffList.size():"<<group.diffList.size();

        ui->tbMaterials->setItem(row,2,new QTableWidgetItem(status));

        //四列按钮
        QPushButton *btnSetting = new QPushButton("设置", ui->tbMaterials);

        connect(btnSetting, &QPushButton::clicked,
                this, &MainWindow::onMaterialSettingClicked,
                Qt::UniqueConnection);

        btnSetting->setProperty("component",group.component);
        btnSetting->setProperty("vmtName",group.vmtName);
        ui->tbMaterials->setCellWidget(row, 3, btnSetting);
        ++row;
    }
}

//新增：材质设置按钮点击,单个设置
void MainWindow::onMaterialSettingClicked()
{
    //获取发送者的btn对象
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    QString vmtName = btn->property("vmtName").toString();
    QString component = btn->property("component").toString();


    QList<Material*> diffList;
    for(Material &mat:materials)
    {
        if(mat.vmtName == vmtName && mat.Mcomponent == component)
        {
            diffList.append(&mat);
        }
    }
    if (diffList.isEmpty()) return;

    MaterialSetting dlg(this);

    dlg.setAllMaterials(materials);  //继续传防止重命名

    //直接传 components 列表
    //dlg.setMaterialData(materials[row], components);//传组件可以选差分旧逻辑
    qDebug() << "当前点击设置的材质所属组件：" << component << ", vmtName:" << vmtName;
    for(Material* m1 : diffList)
    {
        qDebug() << "材质" << vmtName << "hasDiff:" << m1->hasDiff
                 << "包含差分:" << (m1->diffNames.isEmpty() ? "无" : m1->diffNames.first());
    }

    dlg.setMaterialGroup(component,vmtName,diffList,components);

    if (dlg.exec() == QDialog::Accepted) {
        //dlg.getMaterialData(materials[row]);//赋值到这个引用体直接修改材质
        updateMaterialTable();
        updateTreeWidget();
    }
}


//日志等进行操作处理
// 复选框槽函数
void MainWindow::on_cbSkipPreprocess_toggled(bool checked)
{
    skipPreprocess = checked;
}

void MainWindow::on_cbSkipVtf_toggled(bool checked)
{
    skipVtf = checked;
}

void MainWindow::on_cbSkipVmt_toggled(bool checked)
{
    skipVmt = checked;
}

void MainWindow::on_cbSkipVpk_toggled(bool checked)
{
    skipVpk = checked;
}

// 执行按钮
#if 0
void MainWindow::on_btnExecute_clicked()
{
    // 每次执行前清空日志
    ui->l4mhRiZhi->clear();
    ui->l4mhRiZhi->append("开始执行...");

    // 初始化进度条
    ui->progressBar->setValue(0);
    ui->progressBar->setVisible(true);

    QStringList tgaFiles, pngFiles; // 在函数开头定义，让所有步骤都能访问

    //总步数
    int totalSteps = 0;
    if (!skipPreprocess) totalSteps++;
    if (!skipVtf) totalSteps++;
    if (!skipVmt) totalSteps++;
    if (!skipVpk) totalSteps++;

    int currentStep=0;

    if (!skipPreprocess) {
        ui->l4mhRiZhi->append("=== 执行纹理预处理 ===");

        if (ui->editSource->text().isEmpty()) {
            ui->l4mhRiZhi->append("错误：源文件路径为空");
            return;
        }

        if (materials.isEmpty()) {
            ui->l4mhRiZhi->append("错误：没有材质需要处理");
            return;
        }

        TexturePreprocessor preprocessor;

        connect(&preprocessor, &TexturePreprocessor::logMessage,
                this, [this](const QString &msg) {
                    ui->l4mhRiZhi->append(msg);
                });

        // 连接进度信号
        connect(&preprocessor, &TexturePreprocessor::progressUpdated,
                this, [this, totalSteps, currentStep](int current, int total) {
                    // 当前步骤占总进度的比例
                    int stepProgress = (current * 100) / total / totalSteps;
                    int baseProgress = (currentStep * 100) / totalSteps;
                    ui->progressBar->setValue(baseProgress + stepProgress);
                });

        preprocessor.setSourcePath(ui->editSource->text());
        preprocessor.setMaterials(materials);

        if (!preprocessor.process()) {
            ui->l4mhRiZhi->append("错误：纹理预处理失败");
            return;
        }

        tgaFiles = preprocessor.getGeneratedTGA();
        pngFiles = preprocessor.getGeneratedPNG();

        ui->l4mhRiZhi->append(QString("纹理预处理完成，生成TGA: %1个, PNG: %2个")
                                  .arg(tgaFiles.size()).arg(pngFiles.size()));

        currentStep++;
        ui->progressBar->setValue((currentStep * 100) / totalSteps);
    }

    if (!skipVtf) {
        ui->l4mhRiZhi->append("=== 执行VTF生成 ===");
        // 这里可以使用 tgaFiles 和 pngFiles
        // VTFCmd 路径（放在 Tools/VTF/ 下）
        // 使用用户设置的路径（从 editVtf 获取）
        QString maretfPath = ui->editVtf->text();  // 用户浏览选择的路径

        // 如果用户没设置，给个提示
        if (maretfPath.isEmpty() || !QFile::exists(maretfPath)) {
            ui->l4mhRiZhi->append("错误：请先设置正确的 VTF 转换器路径");
            return;
        }

        VTFGenerator vtfGen;
        connect(&vtfGen, &VTFGenerator::logMessage,
                this, [this](const QString &msg) {
                    ui->l4mhRiZhi->append(msg);
                });
        // 连接进度信号
        connect(&vtfGen, &VTFGenerator::progressUpdated,
                this, [this, totalSteps, currentStep](int current, int total) {
                    int stepProgress = (current * 100) / total / totalSteps;
                    int baseProgress = (currentStep * 100) / totalSteps;
                    ui->progressBar->setValue(baseProgress + stepProgress);
                });

        vtfGen.setMareTFPath(maretfPath);
        vtfGen.setTempDir(FileUtils::getTempDir());
        vtfGen.setMaterials(materials);  // 传入材质列表，用于识别动图
        vtfGen.setTgaFiles(tgaFiles);
        vtfGen.setPngFiles(pngFiles);

        if (!vtfGen.convertAll()) {
            ui->l4mhRiZhi->append("错误：VTF 生成失败");
            return;
        }

        ui->l4mhRiZhi->append("VTF 生成完成");

        currentStep++;
        ui->progressBar->setValue((currentStep * 100) / totalSteps);
    }

    if (!skipVmt) {
        ui->l4mhRiZhi->append("=== 执行VMT生成 ===");

        QStringList invalidRefs;
        for (const ComponentVariants &cv : projectVariants) {
            const Component *comp = nullptr;
            for (const Component &c : components) {
                if (c.name == cv.componentName) {
                    comp = &c;
                    break;
                }
            }
            if (!comp) continue;

            for (const Variant &v : cv.variants) {
                for (const VariantMaterialConfig &vmc : v.materials) {
                    bool matExists = false;
                    for (const MaterialInComponent &mic : comp->materials) {
                        if (mic.vmtName == vmc.materialVmtName) {
                            matExists = true;
                            break;
                        }
                    }
                    if (!matExists) {
                        invalidRefs << QString("组件[%1] 变体[%2] 材质[%3]")
                                           .arg(cv.componentName).arg(v.variantName).arg(vmc.materialVmtName);
                    }
                }
            }
        }

        if (!invalidRefs.isEmpty()) {
            QString msg = "以下变体配置引用了不存在的材质，请重新配置变体或恢复材质：\n\n";
            msg += invalidRefs.join("\n");
            msg += "\n\n是否继续执行？（这些变体将不会生成对应材质）";

            QMessageBox::StandardButton reply = QMessageBox::warning(this,
                                                                     "变体配置错误", msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

            if (reply == QMessageBox::No) {
                ui->l4mhRiZhi->append("已停止执行，请修复变体配置");
                return;
            }
        }

        if (projectVariants.isEmpty()) {
            ui->l4mhRiZhi->append("错误：请先配置变体");
            return;
        }

        if (ui->editOutput->text().isEmpty()) {
            ui->l4mhRiZhi->append("错误：请设置输出路径");
            return;
        }

        VMTGenerator vmtGen;
        connect(&vmtGen, &VMTGenerator::logMessage,
                this, [this](const QString &msg) {
                    ui->l4mhRiZhi->append(msg);
                });
        // 连接进度信号
        connect(&vmtGen, &VMTGenerator::progressUpdated,
                this, [this, totalSteps, currentStep](int current, int total) {
                    int stepProgress = (current * 100) / total / totalSteps;
                    int baseProgress = (currentStep * 100) / totalSteps;
                    ui->progressBar->setValue(baseProgress + stepProgress);
                });

        vmtGen.setGlobalValues(
            globalAddonInfo.addonversion,
            globalAddonInfo.addonauthor,
            globalAddonInfo.addontagline,
            globalAddonInfo.addonDescription
            );

        vmtGen.setOutputPath(ui->editOutput->text());
        vmtGen.setMatPath(ui->editVpkMat->text());
        vmtGen.setTempDir(FileUtils::getTempDir());
        vmtGen.setMareTFPath(ui->editVtf->text());
        vmtGen.setSourceBasePath(ui->editSource->text());
        vmtGen.setComponents(components);
        vmtGen.setMaterials(materials);
        vmtGen.setProjectVariants(projectVariants);

        if (!vmtGen.generateAll()) {
            ui->l4mhRiZhi->append("错误：VMT生成失败");
            return;
        }

        currentStep++;
        ui->progressBar->setValue((currentStep * 100) / totalSteps);
    }

    if (!skipVpk) {
        ui->l4mhRiZhi->append("=== 执行VPK打包 (4/4) ===");

        QString vpkPath = ui->editVpk->text();

        if (vpkPath.isEmpty() || !QFile::exists(vpkPath)) {
            ui->l4mhRiZhi->append("错误：请先设置正确的 VPK 工具路径");
            saveLogToFile();
            return;
        }

        VPKGenerator vpkGen;

        // 连接日志信号
        connect(&vpkGen, &VPKGenerator::logMessage,
                this, [this](const QString &msg) {
                    ui->l4mhRiZhi->append(msg);
                });

        // 连接进度信号
        connect(&vpkGen, &VPKGenerator::progressUpdated,
                this, [this, totalSteps, currentStep](int current, int total) {
                    int stepProgress = (current * 100) / total / totalSteps;
                    int baseProgress = (currentStep * 100) / totalSteps;
                    ui->progressBar->setValue(baseProgress + stepProgress);
                });

        vpkGen.setVpkPath(vpkPath);
        vpkGen.setOutputPath(ui->editOutput->text());

        if (!vpkGen.packAll()) {
            ui->l4mhRiZhi->append("错误：VPK打包失败");
            saveLogToFile();
            return;
        }

        currentStep++;
        ui->progressBar->setValue((currentStep * 100) / totalSteps);
    }

    ui->l4mhRiZhi->append("执行完成！");

    ui->progressBar->setValue(100);
    ui->l4mhRiZhi->append("执行完成！");

    // 保存日志到文件
    saveLogToFile();
}
#endif

void MainWindow::on_btnExecute_clicked()
{
    if (workerThread && worker) {
        QMessageBox::warning(this, "提示", "已有任务正在执行，请等待完成");
        return;
    }

    ui->l4mhRiZhi->clear();
    ui->l4mhRiZhi->append("开始执行...");
    ui->progressBar->setValue(0);
    ui->progressBar->setVisible(true);
    ui->btnExecute->setEnabled(false);

    // 无父对象
    workerThread = new QThread(nullptr);
    worker = new ProcessWorker();
    worker->moveToThread(workerThread);

    worker->setParams(skipPreprocess, skipVtf, skipVmt, skipVpk,
                      ui->editSource->text(), ui->editOutput->text(),
                      ui->editVpkMat->text(), ui->editVtf->text(), ui->editVpk->text(),
                      components, materials, projectVariants, globalAddonInfo);

    connect(workerThread, &QThread::started, worker, &ProcessWorker::doWork);
    connect(worker, &ProcessWorker::logMessage, this, &MainWindow::onWorkerLog);
    connect(worker, &ProcessWorker::progressUpdated, this, &MainWindow::onWorkerProgress);
    connect(worker, &ProcessWorker::workFinished, this, &MainWindow::onWorkFinished);

    // 不要这两个连接！
    // m_deleteWorkerConn = connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);
    // m_deleteThreadConn = connect(workerThread, &QThread::finished, workerThread, &QObject::deleteLater);

    workerThread->start();
}

// 保存日志到文件的函数
void MainWindow::saveLogToFile()
{
    // 创建Log目录
    QString appDir = QCoreApplication::applicationDirPath();
    QString logDir = appDir + "/Resources/Log";
    QDir dir;
    if (!dir.exists(logDir)) {
        dir.mkpath(logDir);
    }

    // 生成带时间的文件名
    QDateTime now = QDateTime::currentDateTime();
    QString timeStr = now.toString("yyyy-MM-dd_hh-mm-ss");
    QString fileName = QString("log_%1.txt").arg(timeStr);

    lastLogFilePath = logDir + "/" + fileName;

    // 写入日志
    QFile file(lastLogFilePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << ui->l4mhRiZhi->toPlainText();
        file.close();
        ui->l4mhRiZhi->append(QString("日志已保存至: %1").arg(lastLogFilePath));
    }
}

// 结束按钮
void MainWindow::on_btnEnd_clicked()
{
    close();  // 关闭窗口
}

// 打开日志按钮
void MainWindow::on_btnOpenRiZhi_clicked()
{
    if (lastLogFilePath.isEmpty() || !QFile::exists(lastLogFilePath)) {
        // 如果没有最新日志，尝试打开Log目录下的最新文件
        QString appDir = QCoreApplication::applicationDirPath();
        QString logDir = appDir + "/Log";

        QDir dir(logDir);
        if (!dir.exists()) {
            QMessageBox::information(this, "提示", "没有找到日志文件");
            return;
        }

        // 获取所有日志文件并按时间排序
        QStringList logFiles = dir.entryList(QStringList() << "log_*.txt", QDir::Files, QDir::Time);
        if (logFiles.isEmpty()) {
            QMessageBox::information(this, "提示", "没有找到日志文件");
            return;
        }

        lastLogFilePath = logDir + "/" + logFiles.first();
    }

    // 用系统默认程序打开日志文件
    QDesktopServices::openUrl(QUrl::fromLocalFile(lastLogFilePath));
}

// 保存测试用例
void MainWindow::on_btnSaveTestCase_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(this, "保存测试用例",
                                                    QApplication::applicationDirPath()+ "/Resources/Saved", "测试用例 (*.tcase)");

    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "错误", "无法保存文件");
        return;
    }

    QTextStream out(&file);

    // 用简单的格式保存，每行一个数据，用特殊符号分隔
    out << "[GLOBAL]\n";
    out << "version=" << globalAddonInfo.addonversion << "\n";
    out << "author=" << globalAddonInfo.addonauthor << "\n";
    out << "tagline=" << globalAddonInfo.addontagline << "\n";
    out << "desc=" << globalAddonInfo.addonDescription << "\n";

    out << "[COMPONENTS]\n";
    out << "count=" << components.size() << "\n";

    for (int i = 0; i < components.size(); i++) {
        const Component &comp = components[i];
        out << "comp_start\n";
        out << "name=" << comp.name << "\n";
        out << "title=" << comp.compInfo.addontitle << "\n";
        out << "desc=" << comp.compInfo.addonDescription << "\n";
        out << "material_count=" << comp.materials.size() << "\n";

        for (const MaterialInComponent &mat : comp.materials) {
            out << "mat=" << mat.name << "|" << mat.vmtName << "|" << mat.hasDiff;
            if (mat.hasDiff) {
                out << "|" << mat.diffNames.join(",");
            }
            out << "|" << mat.matId << "\n";  // ← 加在这里
            out << "\n";
        }
        out << "comp_end\n";
    }

    out << "[MATERIALS]\n";
    out << "count=" << materials.size() << "\n";

    for (int i = 0; i < materials.size(); i++) {
        const Material &mat = materials[i];
        out << "mat_start\n";
        out << "id=" << mat.id << "\n";
        out << "Mname=" << mat.Mname << "\n";
        out << "vmtName=" << mat.vmtName << "\n";
        out << "materialDefId=" << mat.materialDefId << "\n";
        out << "Mcomponent=" << mat.Mcomponent << "\n";
        out << "MFilename=" << mat.MFilename << "\n";
        out << "Mlightwarp=" << mat.Mlightwarp << "|" << mat.MlightwarpSource << "\n";
        out << "Mbumpmap=" << mat.Mbumpmap << "|" << mat.MbumpmapSource << "\n";
        out << "Menvmap=" << mat.Menvmap << "|" << mat.MenvmapSource << "\n";
        out << "Memissive=" << mat.Memissive << "|" << mat.MemissiveSource << "\n";
        out << "hasDiff=" << mat.hasDiff << "\n";
        out << "diffNames=" << mat.diffNames.join(",") << "\n";
        out << "isInit=" << mat.isinit << "\n";
        out << "useAlphaTexture=" << mat.useAlphaTexture << "\n";
        out << "alphaTextureName=" << mat.alphaTextureName << "\n";
        out << "customVmt=" << QString(mat.customVmtContent).replace("\n", "\\n") << "\n";  // 把换行符转义
        out << "params=" << mat.MAlpha << "," << mat.Mnocull << "," << mat.Mnodecal << ","
            << mat.Mselfillum << "," << mat.Mhalflambert << "," << mat.Mphong << ","
            << mat.Mtranslucent << "," << mat.Malphatest << "\n";
        out << "mat_end\n";
    }

    out << "[VARIANTS]\n";
    out << "count=" << projectVariants.size() << "\n";

    for (const ComponentVariants &cv : projectVariants) {
        out << "cv_start\n";
        out << "compName=" << cv.componentName << "\n";
        out << "variant_count=" << cv.variants.size() << "\n";

        for (const Variant &v : cv.variants) {
            out << "var_start\n";
            out << "varName=" << v.variantName << "\n";
            out << "mat_config_count=" << v.materials.size() << "\n";

            for (const VariantMaterialConfig &vmc : v.materials) {
                out << "vmc=" << vmc.materialVmtName << "|"
                    << vmc.selectedDiff << "|" << vmc.baseTexture << "\n";
            }
            out << "var_end\n";
        }
        out << "cv_end\n";
    }

    file.close();
    QMessageBox::information(this, "成功", "测试用例已保存");
}

// 加载测试用例
void MainWindow::on_btnLoadTestCase_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "加载测试用例",
                                                    QApplication::applicationDirPath()+ "/Resources/Saved", "测试用例 (*.tcase)");

    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法加载文件");
        return;
    }

    // 先清空现有数据
    components.clear();
    materials.clear();
    projectVariants.clear();

    QTextStream in(&file);
    QString line;
    QString section;

    Component currentComp;
    Material currentMat;
    bool inComp = false;
    bool inMat = false;

    ComponentVariants currentCV;
    Variant currentVariant;
    bool inCV = false;
    bool inVariant = false;

    while (!in.atEnd()) {
        line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        // 调试输出
        qDebug() << "读取行:" << line;

        if (line.startsWith('[') && line.endsWith(']')) {
            section = line.mid(1, line.length() - 2);
            qDebug() << "切换到段落:" << section;
            continue;
        }

        if (section == "GLOBAL") {
            if (line.startsWith("version="))
                globalAddonInfo.addonversion = line.mid(8);
            else if (line.startsWith("author="))
                globalAddonInfo.addonauthor = line.mid(7);
            else if (line.startsWith("tagline="))
                globalAddonInfo.addontagline = line.mid(8);
            else if (line.startsWith("desc="))
                globalAddonInfo.addonDescription = line.mid(5);
        }
        else if (section == "COMPONENTS") {
            if (line == "comp_start") {
                currentComp = Component();
                inComp = true;
                qDebug() << "开始读取组件";
            }
            else if (line == "comp_end") {
                if (inComp) {
                    components.append(currentComp);
                    inComp = false;
                    qDebug() << "组件读取完成，当前组件数:" << components.size();
                }
            }
            else if (inComp) {
                if (line.startsWith("name=")) {
                    currentComp.name = line.mid(5);
                }
                else if (line.startsWith("title=")) {
                    currentComp.compInfo.addontitle = line.mid(6);
                }
                else if (line.startsWith("desc=")) {
                    currentComp.compInfo.addonDescription = line.mid(5);
                }
                else if (line.startsWith("mat=")) {
                    QStringList parts = line.mid(4).split('|');
                    if (parts.size() >= 3) {
                        MaterialInComponent mat;
                        mat.name = parts[0];
                        mat.vmtName = parts[1];
                        mat.hasDiff = (parts[2] == "1");
                        int idx = 3;
                        if (mat.hasDiff) {
                            mat.diffNames = parts[3].split(',', Qt::SkipEmptyParts);
                            idx = 4;
                        }
                        if (parts.size() > idx) {
                            mat.matId = parts[idx];  // 读取保存的 matId
                        } else {
                            mat.matId = QUuid::createUuid().toString(QUuid::WithoutBraces);  // 兼容旧文件
                        }
                        currentComp.materials.append(mat);
                        qDebug() << "添加材质到组件:" << mat.name;
                    }
                }
            }
        }
        else if (section == "MATERIALS") {
            if (line == "mat_start") {
                currentMat = Material();
                currentMat.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
                inMat = true;
                qDebug() << "开始读取材质";
            }
            else if (line == "mat_end") {
                if (inMat) {
                    materials.append(currentMat);
                    inMat = false;
                    qDebug() << "材质读取完成，当前材质数:" << materials.size();
                }
            }
            else if (inMat) {
                if (line.startsWith("Mname="))
                    currentMat.Mname = line.mid(6);
                else if (line.startsWith("vmtName="))
                    currentMat.vmtName = line.mid(8);
                else if (line.startsWith("Mcomponent="))
                    currentMat.Mcomponent = line.mid(11);
                else if (line.startsWith("MFilename="))
                    currentMat.MFilename = line.mid(10);
                else if (line.startsWith("materialDefId="))
                    currentMat.materialDefId = line.mid(14);
                else if (line.startsWith("Mlightwarp=")) {
                    QStringList parts = line.mid(11).split('|');
                    if (parts.size() >= 2) {
                        currentMat.Mlightwarp = parts[0];
                        currentMat.MlightwarpSource = parts[1];
                    }
                }
                else if (line.startsWith("Mbumpmap=")) {
                    QStringList parts = line.mid(9).split('|');
                    if (parts.size() >= 2) {
                        currentMat.Mbumpmap = parts[0];
                        currentMat.MbumpmapSource = parts[1];
                    }
                }
                else if (line.startsWith("Menvmap=")) {
                    QStringList parts = line.mid(8).split('|');
                    if (parts.size() >= 2) {
                        currentMat.Menvmap = parts[0];
                        currentMat.MenvmapSource = parts[1];
                    }
                }
                else if (line.startsWith("hasDiff="))
                    currentMat.hasDiff = (line.mid(8) == "1");
                else if (line.startsWith("diffNames=")) {
                    QString diffs = line.mid(10);
                    if (!diffs.isEmpty()) {
                        currentMat.diffNames = diffs.split(',', Qt::SkipEmptyParts);
                    }
                }
                else if (line.startsWith("Memissive=")) {  // 新增
                    QStringList parts = line.mid(10).split('|');
                    if (parts.size() >= 2) {
                        currentMat.Memissive = parts[0];
                        currentMat.MemissiveSource = parts[1];
                    }
                }
                else if (line.startsWith("isInit="))
                    currentMat.isinit = (line.mid(7) == "1");
                else if (line.startsWith("useAlphaTexture="))
                    currentMat.useAlphaTexture = (line.mid(16) == "1");
                else if (line.startsWith("alphaTextureName="))
                    currentMat.alphaTextureName = line.mid(17);
                else if (line.startsWith("customVmt=")) {
                    QString content = line.mid(10);
                    content.replace("\\n", "\n");  // 把转义的换行符恢复
                    currentMat.customVmtContent = content;
                }
                else if (line.startsWith("params=")) {
                    QStringList nums = line.mid(7).split(',');
                    if (nums.size() >= 8) {
                        currentMat.MAlpha = nums[0].toInt();
                        currentMat.Mnocull = nums[1].toInt();
                        currentMat.Mnodecal = nums[2].toInt();
                        currentMat.Mselfillum = nums[3].toInt();
                        currentMat.Mhalflambert = nums[4].toInt();
                        currentMat.Mphong = nums[5].toInt();
                        currentMat.Mtranslucent = nums[6].toInt();
                        currentMat.Malphatest = nums[7].toInt();
                    }
                }
            }
        }
        else if (section == "VARIANTS") {
            if (line == "cv_start") {
                currentCV = ComponentVariants();
                inCV = true;
            }
            else if (line == "cv_end") {
                if (inCV) {
                    projectVariants.append(currentCV);
                    inCV = false;
                }
            }
            else if (inCV) {
                if (line.startsWith("compName=")) {
                    currentCV.componentName = line.mid(9);
                }
                else if (line == "var_start") {
                    currentVariant = Variant();
                    inVariant = true;
                }
                else if (line == "var_end") {
                    if (inVariant) {
                        currentCV.variants.append(currentVariant);
                        inVariant = false;
                    }
                }
                else if (inVariant) {
                    if (line.startsWith("varName=")) {
                        currentVariant.variantName = line.mid(8);
                    }
                    else if (line.startsWith("vmc=")) {
                        QStringList parts = line.mid(4).split('|');
                        if (parts.size() >= 3) {
                            VariantMaterialConfig vmc;
                            vmc.materialVmtName = parts[0];
                            vmc.selectedDiff = parts[1];
                            vmc.baseTexture = parts[2];
                            currentVariant.materials.append(vmc);
                        }
                    }
                }
            }
        }

    }

    file.close();

    //兼容旧文件的materialDefId
    for (Material &mat : materials) {
        bool found = false;
        for (const Component &comp : components) {
            if (comp.name == mat.Mcomponent) {
                for (const MaterialInComponent &mic : comp.materials) {
                    if (mic.vmtName == mat.vmtName) {
                        // 如果 materialDefId 已经是正确的，就不改；否则更新
                        if (mat.materialDefId != mic.matId) {
                            qDebug() << "修复 materialDefId:" << mat.vmtName
                                     << "旧:" << mat.materialDefId
                                     << "新:" << mic.matId;
                            mat.materialDefId = mic.matId;
                        }
                        found = true;
                        break;
                    }
                }
                break;
            }
        }
        if (!found) {
            qDebug() << "警告：材质" << mat.Mname << "找不到对应的材质定义";
        }
    }

    // 刷新界面
    updateTreeWidget();
    updateMaterialTable();

    qDebug() << "加载完成，组件数:" << components.size() << "材质数:" << materials.size();

    QMessageBox::information(this, "成功",
                             QString("已加载 %1 个组件，%2 个贴图")
                                 .arg(components.size())
                                 .arg(materials.size()));
}


//变体处理区
//变体按钮
void MainWindow::on_btnVariant_clicked()
{
    QList<ComponentVariants> currentVariants;

    // 如果已经有保存的变体配置，使用它
    if (!projectVariants.isEmpty()) {
        currentVariants = projectVariants;

        // 检查是否有新增的组件，如果有则添加对应的空配置
        for (const Component &comp : components) {
            bool found = false;
            for (const ComponentVariants &cv : currentVariants) {
                if (cv.componentName == comp.name) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                // 新增组件，添加空的变体配置
                ComponentVariants newCv;
                newCv.componentName = comp.name;
                currentVariants.append(newCv);
            }
        }

        // 清理已删除组件的变体配置
        for (int i = currentVariants.size() - 1; i >= 0; --i) {
            bool found = false;
            for (const Component &comp : components) {
                if (comp.name == currentVariants[i].componentName) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                currentVariants.removeAt(i);
            }
        }
    } else {
        // 第一次打开，创建空的变体配置
        for (const Component &comp : components) {
            ComponentVariants cv;
            cv.componentName = comp.name;
            currentVariants.append(cv);
        }
    }

    VariantConfigDialog dlg(components, currentVariants, this);

    if (dlg.exec() == QDialog::Accepted) {
        projectVariants = dlg.getVariants();
        QMessageBox::information(this, "提示", "变体配置已保存");
    }
}

void MainWindow::onWorkFinished(bool success)
{
    ui->btnExecute->setEnabled(true);

    if (success) {
        ui->l4mhRiZhi->append("执行完成！");
        saveLogToFile();
    } else {
        ui->l4mhRiZhi->append("执行失败！");
    }

    // 直接 delete，不用 deleteLater
    if (worker) {
        delete worker;
        worker = nullptr;
    }
    if (workerThread) {
        workerThread->quit();
        workerThread->wait(1000);
        delete workerThread;
        workerThread = nullptr;
    }
}

void MainWindow::onWorkerLog(const QString &msg)
{
    ui->l4mhRiZhi->append(msg);
}

void MainWindow::onWorkerProgress(int current, int total)
{
    if (total > 0) {
        ui->progressBar->setValue(current);
    }
}


