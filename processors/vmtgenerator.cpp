#include "vmtgenerator.h"
#include "../utils/fileutils.h"
#include "processors/vtfgenerator.h"
#include <QFile>
#include <QTextStream>
#include <QSettings>  // 添加这个
#include "../datastructures.h"
#include <QMessageBox>

VMTGenerator::VMTGenerator(QObject *parent) : QObject(parent) {}

//生成vmt文件内容
QString VMTGenerator::generateVMTContent(const Material &mat, const QString &selectedDiff)
{
    qDebug() << "=== generateVMTContent ===";
    qDebug() << "mat.vmtName:" << mat.vmtName << "selectedDiff:" << selectedDiff;

    // 计算基础贴图路径,matpath是用户定义的materials下的路径
    QString baseTexture;
    if (!selectedDiff.isEmpty()) {
        baseTexture = matPath + "/" + mat.vmtName + "_"  + selectedDiff;
    } else {
        baseTexture = matPath + "/" + mat.vmtName;
    }

    QString vmt;

    //自定义VMT处理（带防御性检查）
    if (!mat.customVmtContent.isEmpty()) {
        vmt = mat.customVmtContent;

        // 1. {{base}} - 基础贴图（必须存在，否则报错）
        if (vmt.contains("{{base}}")) {
            if (baseTexture.isEmpty()) {
                emit logMessage(QString("❌ 错误：材质 %1 使用了{{base}}但基础贴图路径为空，请检查材质路径设置").arg(mat.vmtName));
                vmt.replace("{{base}}", "ERROR_MISSING_BASE_TEXTURE");
            } else {
                vmt.replace("{{base}}", baseTexture);
            }
        }

        // 2. {{light}} - Lightwarp（可选，没设置时替换成空并提示）
        if (vmt.contains("{{light}}")) {
            QString lightPath;
            if (!mat.Mlightwarp.isEmpty()) {
                QString lightName = cleanFileName(mat.Mlightwarp);
                if (mat.MlightwarpSource == SOURCE_TOOL || mat.MlightwarpSource == SOURCE_USER) {
                    lightPath = matPath + "/" + lightName;
                }
            }

            if (lightPath.isEmpty()) {
                if (!mat.Mlightwarp.isEmpty()) {
                    emit logMessage(QString("⚠️ 提示：材质 %1 的Lightwarp路径计算为空（来源=%2）").arg(mat.vmtName).arg(mat.MlightwarpSource));
                } else {
                    emit logMessage(QString("⚠️ 提示：材质 %1 使用了{{light}}但MaterialSetting中未设置Lightwarp").arg(mat.vmtName));
                }
            }
            vmt.replace("{{light}}", lightPath);
        }

        // 3. {{bump}} - Bumpmap（可选，没设置时替换成空并提示）
        if (vmt.contains("{{bump}}")) {
            QString bumpPath;
            if (!mat.Mbumpmap.isEmpty()) {
                QString bumpName = cleanFileName(mat.Mbumpmap);
                if (mat.MbumpmapSource == SOURCE_USER) {
                    bumpPath = matPath + "/" + bumpName;
                } else if (mat.MbumpmapSource == SOURCE_CODE) {
                    bumpPath = "dev/dev_normal";
                }
            }

            if (bumpPath.isEmpty()) {
                if (!mat.Mbumpmap.isEmpty()) {
                    emit logMessage(QString("⚠️ 提示：材质 %1 的Bumpmap路径计算为空（来源=%2）").arg(mat.vmtName).arg(mat.MbumpmapSource));
                } else {
                    emit logMessage(QString("⚠️ 提示：材质 %1 使用了{{bump}}但MaterialSetting中未设置Bumpmap").arg(mat.vmtName));
                }
            }
            vmt.replace("{{bump}}", bumpPath);
        }

        // 4. {{env}} - Envmap（可选，没设置时替换成空并提示）
        if (vmt.contains("{{env}}")) {
            QString envPath;
            if (!mat.Menvmap.isEmpty()) {
                QString envName = cleanFileName(mat.Menvmap);
                if (mat.MenvmapSource == SOURCE_USER) {
                    envPath = matPath + "/" + envName;
                } else if (mat.MenvmapSource == SOURCE_CODE) {
                    envPath = "env_cubemap";
                }
            }

            if (envPath.isEmpty()) {
                if (!mat.Menvmap.isEmpty()) {
                    emit logMessage(QString("⚠️ 提示：材质 %1 的Envmap路径计算为空（来源=%2）").arg(mat.vmtName).arg(mat.MenvmapSource));
                } else {
                    emit logMessage(QString("⚠️ 提示：材质 %1 使用了{{env}}但MaterialSetting中未设置Envmap").arg(mat.vmtName));
                }
            }
            vmt.replace("{{env}}", envPath);
        }

        // 5. {{aemissive}} - 自发光贴图
        if (vmt.contains("{{aemissive}}")) {
            QString emissivePath;
            if (!mat.Memissive.isEmpty() && mat.MemissiveSource == SOURCE_USER) {
                QString emissiveName = cleanFileName(mat.Memissive);
                emissivePath = matPath + "/" + emissiveName;
            }
            vmt.replace("{{aemissive}}", emissivePath);
        }

        // 最终检查：是否还有未识别的变量标记
        if (vmt.contains("{{") && vmt.contains("}}")) {
            // 提取未替换的变量名用于提示
            int start = vmt.indexOf("{{");
            int end = vmt.indexOf("}}", start);
            if (end > start) {
                QString unknownVar = vmt.mid(start, end - start + 2);
                emit logMessage(QString("⚠️ 警告：材质 %1 的VMT中有未识别的变量 %2，请检查拼写（可用：{{base}} {{light}} {{bump}} {{env}}）").arg(mat.vmtName).arg(unknownVar));
            }
        }

        qDebug() << "自定义VMT生成完成，长度:" << vmt.length();
        return vmt;  // 直接返回，不再追加默认参数
    }

    //默认生成逻辑（用户没填自定义VMT时）
    vmt = "\"VertexLitGeneric\"\n{\n";
    vmt += "    $basetexture \"" + baseTexture + "\"\n";

    // Lightwarp 路径处理
    if (!mat.Mlightwarp.isEmpty()) {
        QString lightName = cleanFileName(mat.Mlightwarp);
        if (mat.MlightwarpSource == SOURCE_TOOL || mat.MlightwarpSource == SOURCE_USER) {
            vmt += "    $lightwarp \"" + matPath + "/" + lightName + "\"\n";
        }
    }

    // Bumpmap 路径处理
    if (!mat.Mbumpmap.isEmpty()) {
        QString bumpName = cleanFileName(mat.Mbumpmap);
        if (mat.MbumpmapSource == SOURCE_CODE) {
            vmt += "    $bumpmap \"dev/dev_normal\"\n";
        } else if (mat.MbumpmapSource == SOURCE_USER) {
            vmt += "    $bumpmap \"" + matPath + "/" + bumpName + "\"\n";
        }
    }

    // Envmap 路径处理
    if (!mat.Menvmap.isEmpty()) {
        QString envName = cleanFileName(mat.Menvmap);
        if (mat.MenvmapSource == SOURCE_CODE) {
            vmt += "    $envmap \"env_cubemap\"\n";
        } else if (mat.MenvmapSource == SOURCE_USER) {
            vmt += "    $envmap \"" + matPath + "/" + envName + "\"\n";
        }
    }

    vmt += "}\n";
    return vmt;
}
bool VMTGenerator::copyVTF(const QString &vtfName, const QString &destDir)
{
    // 源文件路径：Temp/vtfGenerated/xxx.vtf
    QString srcFile = tempDir + "/vtfGenerated/" + vtfName;
    QString dstFile = destDir + "/" + vtfName;

    if (!QFile::exists(srcFile)) {
        emit logMessage(QString("警告：VTF文件不存在 %1").arg(srcFile));
        return false;
    }

    // 如果目标文件已存在，先删除
    if (QFile::exists(dstFile)) {
        QFile::remove(dstFile);
    }

    return QFile::copy(srcFile, dstFile);
}

bool VMTGenerator::generateAll()
{
    QSettings settings("Local", "Path");
    QString roleName = settings.value("RoleName").toString();

    QString defaultVersion = globalVersion.isEmpty() ? "1.0" : globalVersion;
    QString defaultTagline = globalTagline;
    QString defaultAuthor = globalAuthor.isEmpty() ? "L4MH" : globalAuthor;
    QString defaultDescription = globalDescription.isEmpty() ? "使用L4MH生成" : globalDescription;

    int totalVariants = 0;
    for (const ComponentVariants &cv : projectVariants) {
        totalVariants += cv.variants.size();
    }

    int current = 0;
    bool allSuccess = true;

    emit logMessage(QString("开始生成VMT，共 %1 个变体").arg(totalVariants));

    //先创建基础材质组件目录
    QString baseComponentName = "基础材质";
    QString baseVariantName = "基础材质";
    QString baseDir = outputPath + "/" + roleName + baseComponentName + "/"
                      + roleName + baseVariantName + "/materials/" + matPath;

    if (!FileUtils::ensureDirExists(baseDir)) {
        emit logMessage(QString("警告：无法创建基础材质目录 %1").arg(baseDir));
        // 继续执行，不返回失败
    } else {
        emit logMessage(QString("创建基础材质目录: %1").arg(baseDir));

        QString baseAddonInfoPath = outputPath + "/" + roleName + baseComponentName + "/"
                                    + roleName + baseVariantName + "/addoninfo.txt";
        QString baseTitle = roleName + baseVariantName;

        //只有一个，所以直接创建addoninfo了
        generateAddonInfo(baseAddonInfoPath, baseTitle,
                          defaultAuthor, defaultDescription,
                          defaultVersion, defaultTagline);
    }

    // 用于记录已经复制过的文件，避免重复
    QSet<QString> copiedFiles;

    //先收集所有需要复制到基础组件的文件
    // 遍历所有材质，收集lightwarp和envmap
    for (const Material &mat : materials) {
        copyToBaseComponent(mat, baseDir, sourceBasePath, copiedFiles);
    }

    emit logMessage(QString("基础材质目录中的文件:"));
    QDir baseDirInfo(baseDir);
    QStringList files = baseDirInfo.entryList(QDir::Files);
    for (const QString &file : files) {
        emit logMessage(QString("  - %1").arg(file));
    }

    //找各个变体的所在组件分类（一个分类底下是多个变体）
    for (const ComponentVariants &cv : projectVariants) {

        const Component *foundComp = nullptr;
        //在组件找一样的
        for (const Component &comp : components) {
            if (comp.name == cv.componentName) {
                foundComp = &comp;
                break;
            }
        }

        //找变体
        for (const Variant &var : cv.variants) {
            // 创建目标目录：OutputPath/组件名/变体名/materials/MatPath/
            QString targetDir = outputPath + "/" + roleName + cv.componentName + "/"
                                + roleName + var.variantName + "/materials/" + matPath;

            QString variantRootDir = outputPath + "/" + roleName + cv.componentName + "/"
                                     + roleName + var.variantName;

            emit logMessage(QString("目标目录: %1").arg(targetDir));

            if (!FileUtils::ensureDirExists(targetDir)) {
                emit logMessage(QString("错误：无法创建目录 %1").arg(targetDir));
                allSuccess = false;
                continue;
            }

            QString addonInfoPath = variantRootDir + "/addoninfo.txt";
            QString title = roleName + var.variantName;

            if (foundComp) {

                // 使用组件中用户设置的info，如果为空则用全局的
                QString author = foundComp->compInfo.addonauthor;
                if (author.isEmpty()) {
                    author = defaultAuthor;  // 使用全局作者
                }

                QString desc = foundComp->compInfo.addonDescription;
                if (desc.isEmpty()) {
                    desc = defaultDescription;  // 使用全局描述
                }

                // 使用处理后的 author 和 desc，而不是直接从 compInfo 取
                generateAddonInfo(addonInfoPath, title,
                                  author,      // ← 改这里！
                                  desc,        // ← 改这里！
                                  defaultVersion, defaultTagline);
            } else {
                // 如果没有找到组件，使用默认值
                generateAddonInfo(addonInfoPath, title,
                                  defaultAuthor, defaultDescription,
                                  defaultVersion, defaultTagline);
            }

            emit logMessage(QString("处理变体: %1/%2").arg(cv.componentName).arg(var.variantName));

            // 为变体中的每个材质生成VMT，逐一复制VTF过去
            for (const VariantMaterialConfig &matConfig : var.materials) {
                // 找到对应的材质配置，应该找差分，有点问题
                const Material *mat = nullptr;
                for (const Material &m : materials) {
                    if(m.hasDiff && !m.diffNames.first().isEmpty())
                    {
                        if (m.vmtName == matConfig.materialVmtName &&
                            matConfig.selectedDiff == m.diffNames.first()
                            )
                        {
                            mat = &m;
                            break;
                        }
                    }
                    else
                    {
                        if(m.vmtName==matConfig.materialVmtName)
                        {
                            mat = &m;
                            break;
                        }
                    }
                }

                if (!mat) {
                    emit logMessage(QString("警告：找不到材质 %1").arg(matConfig.materialVmtName));
                    continue;
                }

                // 生成VMT文件名
                QString vmtFileName = mat->vmtName + ".vmt";

                // 生成VMT内容（这里传入selectedDiff用于确定贴图路径）
                QString vmtContent = generateVMTContent(*mat, matConfig.selectedDiff);

                // 写入文件
                QString vmtPath = targetDir + "/" + vmtFileName;
                QFile file(vmtPath);
                if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QTextStream out(&file);
                    out << vmtContent;
                    file.close();
                    emit logMessage(QString("  生成VMT: %1 (使用贴图: %2)")
                                        .arg(vmtFileName)
                                        .arg(matConfig.selectedDiff.isEmpty() ? mat->vmtName : matConfig.selectedDiff));
                } else {
                    emit logMessage(QString("  错误：无法写入 %1").arg(vmtPath));
                    allSuccess = false;
                }

                // 复制对应的VTF文件
                QString vtfFileName;
                if (mat->hasDiff && !matConfig.selectedDiff.isEmpty())
                {
                    vtfFileName = mat->vmtName + "_" +matConfig.selectedDiff + ".vtf";
                }
                else
                {
                    vtfFileName = mat->vmtName + ".vtf";
                }

                if (copyVTF(vtfFileName, targetDir)) {
                    emit logMessage(QString("  复制VTF: %1").arg(vtfFileName));
                } else {
                    emit logMessage(QString("  警告：VTF文件 %1 复制失败").arg(vtfFileName));
                }

                //处理bumpmap（复制到当前变体）
                if (!copyBumpmapToVariant(*mat, targetDir, sourceBasePath)) {
                    emit logMessage(QString("  警告：bumpmap处理失败"));
                }

                // 处理 emissive
                if (!copyEmissiveToVariant(*mat, targetDir, sourceBasePath)) {
                    emit logMessage(QString("  警告：emissive处理失败"));
                }
            }

            current++;
            emit progressUpdated(current, totalVariants);
        }
    }

    emit logMessage("VMT生成完成");
    return allSuccess;
}

void VMTGenerator::generateAddonInfo(const QString &filePath, const QString &title,
                                     const QString &author, const QString &description,
                                     const QString &version, const QString &tagline)
{
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "\"AddonInfo\"\n";
        out << "{\n";
        out << "     addonSteamAppID\t\t\"550\"\n";
        out << "     addontitle\t\t\t\"" << title << "\"\n";
        out << "     addonversion\t\t\t\"" << version << "\"\n";  // 使用传入的版本
        out << "     addontagline\t\t\t\"" << tagline << "\"\n";   // 使用传入的标签行
        out << "     addonauthor\t\t\t\"" << author << "\"\n";     // 使用传入的作者
        out << "     addonDescription\t\t\"" << description << "\"\n";
        out << "}\n";
        file.close();
        emit logMessage(QString("  ✅ 生成addoninfo: %1").arg(filePath));
    } else {
        emit logMessage(QString("  ❌ 错误：无法写入addoninfo %1").arg(filePath));
    }
}

// 统一处理文件名：去掉后缀和路径，只保留纯文件名
QString VMTGenerator::cleanFileName(const QString &fileName)
{
    if (fileName.isEmpty()) return fileName;

    QFileInfo info(fileName);
    return info.completeBaseName();  // 返回不带后缀的文件名
}

// 获取源文件路径（传入不带后缀）
QString VMTGenerator::getSourceFilePath(const QString &baseName, const QString &sourcePath, const QString &suffix)
{
    if (baseName.isEmpty()) return QString();
    return sourcePath + "/" + baseName + "." + suffix;
}

// 获取目标VTF路径（传入不带后缀）
QString VMTGenerator::getTargetVtfPath(const QString &baseName, const QString &targetDir)
{
    return targetDir + "/" + baseName + ".vtf";
}

// 处理需要复制到基础材质组件的文件（lightwarp, envmap）
bool VMTGenerator::copyToBaseComponent(const Material &mat, const QString &baseDir,
                                       const QString &sourceBasePath, QSet<QString> &copiedFiles)
{
    bool allSuccess = true;

    emit logMessage(QString("检查材质: lightwarp=%1(%2), envmap=%3(%4)")
                        .arg(mat.Mlightwarp).arg(mat.MlightwarpSource)
                        .arg(mat.Menvmap).arg(mat.MenvmapSource));

    //if处理 Lightwarp
    if (!mat.Mlightwarp.isEmpty() &&
        (mat.MlightwarpSource == SOURCE_TOOL || mat.MlightwarpSource == SOURCE_USER)) {

        QString fileName = cleanFileName(mat.Mlightwarp);
        QString key = "lightwarp:" + fileName;  // 加前缀避免冲突

        //在复制过的表格找有没有复制过这个
        if (!copiedFiles.contains(key)) {
            QString srcFile;
            if (mat.MlightwarpSource == SOURCE_TOOL) {
                QString exeDir = QCoreApplication::applicationDirPath();
                srcFile = exeDir + "/Resources/Resources/Vtf/" + fileName + ".vtf";
            } else {
                srcFile = sourceBasePath + "/" + fileName + ".vtf";
            }

            if (!QFile::exists(srcFile)) {
                emit logMessage(QString("警告：lightwarp 文件不存在 %1").arg(srcFile));
                allSuccess = false;
            } else {
                QString dstFile = baseDir + "/" + fileName + ".vtf";
                if (QFile::exists(dstFile)) QFile::remove(dstFile);

                if (QFile::copy(srcFile, dstFile)) {
                    copiedFiles.insert(key);
                    emit logMessage(QString("  复制 lightwarp: %1.vtf (来自%2)")
                                        .arg(fileName)
                                        .arg(mat.MlightwarpSource == SOURCE_TOOL ? "工具" : "用户"));
                } else {
                    emit logMessage(QString("  错误：复制 lightwarp 失败"));
                    allSuccess = false;
                }
            }
        }
    }

    //if处理 Envmap（独立执行，不是 else if！）
    if (!mat.Menvmap.isEmpty() && mat.MenvmapSource == SOURCE_USER) {

        QString fileName = cleanFileName(mat.Menvmap);
        QString key = "envmap:" + fileName;  // 加前缀避免冲突

        if (!copiedFiles.contains(key)) {
            QString srcFile = sourceBasePath + "/" + fileName + ".vtf";

            if (!QFile::exists(srcFile)) {
                emit logMessage(QString("警告：envmap 文件不存在 %1").arg(srcFile));
                allSuccess = false;
            } else {
                QString dstFile = baseDir + "/" + fileName + ".vtf";
                if (QFile::exists(dstFile)) QFile::remove(dstFile);

                if (QFile::copy(srcFile, dstFile)) {
                    copiedFiles.insert(key);
                    emit logMessage(QString("  复制 envmap: %1.vtf").arg(fileName));
                } else {
                    emit logMessage(QString("  错误：复制 envmap 失败"));
                    allSuccess = false;
                }
            }
        }
    }

    return allSuccess;
}

// 处理 bumpmap（复制到当前变体）
bool VMTGenerator::copyBumpmapToVariant(const Material &mat, const QString &targetDir,
                                        const QString &sourceBasePath)
{
    if (mat.MbumpmapSource != SOURCE_USER || mat.Mbumpmap.isEmpty()) {
        return true;
    }

    QString bumpName = cleanFileName(mat.Mbumpmap);

    //如果是vtf就直接转移算了
    QString srcIsVtf = sourceBasePath + "/" + bumpName + ".vtf";
    //复用拷贝
    if(QFile::exists(srcIsVtf)){

        emit logMessage(QString("存在bumpmap的vtf文件，不进行转换").arg(bumpName));
        //复制转换后的 VTF 到目标目录
        QString srcVtf = srcIsVtf;
        QString dstVtf = targetDir + "/" + bumpName + ".vtf";

        if (!QFile::exists(srcVtf)) {
            emit logMessage(QString("警告：bumpmap VTF 不存在 %1").arg(srcVtf));
            return false;
        }

        if (QFile::exists(dstVtf)) {
            QMessageBox::warning(nullptr, "警告", "该BumpMapVTF与其他VTF重名，请进行检查");
            QFile::remove(dstVtf);
        }

        bool success = QFile::copy(srcVtf, dstVtf);
        if (success) {
            emit logMessage(QString("  ✅ bumpmap 复制成功: %1.vtf").arg(bumpName));
        } else {
            emit logMessage(QString("  ❌ 复制 VTF 失败"));
        }

        return success;
    }


    QString srcFile = sourceBasePath + "/" + bumpName + ".png";

    emit logMessage(QString("=== bumpmap 调试 ==="));
    emit logMessage(QString("bumpName: %1").arg(bumpName));
    emit logMessage(QString("sourceBasePath: %1").arg(sourceBasePath));
    emit logMessage(QString("完整源路径: %1").arg(srcFile));
    emit logMessage(QString("文件是否存在: %1").arg(QFile::exists(srcFile) ? "是" : "否"));

    if (!QFile::exists(srcFile)) {
        emit logMessage(QString("警告：bumpmap 源文件不存在 %1").arg(srcFile));
        return false;
    }

    //确保 bumpmap 被转换
    // 1. 先复制到临时 PNG 目录
    QString tempPngDir = tempDir + "/pngGenerated/";
    FileUtils::ensureDirExists(tempPngDir);

    QString tempPngFile = tempPngDir + bumpName + ".png";
    if (QFile::exists(tempPngFile)) {
        QFile::remove(tempPngFile);
    }

    if (!QFile::copy(srcFile, tempPngFile)) {
        emit logMessage(QString("警告：无法复制 bumpmap 到临时目录 %1").arg(tempPngFile));
        return false;
    }

    // 2. 调用 VTFGenerator 转换
    VTFGenerator vtfGen;
    vtfGen.setMareTFPath(maretfPath);

    Material tempMat;
    tempMat.MFilename = bumpName;
    tempMat.vmtName = bumpName;
    tempMat.MAlpha = false;
    tempMat.useAlphaTexture = false;
    tempMat.hasDiff = false;
    tempMat.Mcomponent = "";

    //装载临时参数进singleMatList然后放进vtfGen
    QList<Material> singleMatList;
    singleMatList.append(tempMat);
    vtfGen.setMaterials(singleMatList);//传入材质列表(单个材质)，做准备
    vtfGen.setTempDir(tempDir);//设置临时目录地址

    QStringList pngFiles;
    pngFiles.append(bumpName + ".png");
    vtfGen.setPngFiles(pngFiles);
    vtfGen.setTgaFiles(QStringList());

    emit logMessage(QString("  开始转换 bumpmap..."));

    //开始转化
    if (!vtfGen.convertAll()) {
        emit logMessage(QString("警告：bumpmap 转换失败 %1").arg(bumpName));
        return false;
    }

    // 3. 复制转换后的 VTF 到目标目录
    QString srcVtf = tempDir + "/vtfGenerated/" + bumpName + ".vtf";
    QString dstVtf = targetDir + "/" + bumpName + ".vtf";

    if (!QFile::exists(srcVtf)) {
        emit logMessage(QString("警告：转换后的 bumpmap VTF 不存在 %1").arg(srcVtf));
        return false;
    }

    if (QFile::exists(dstVtf)) {
        QFile::remove(dstVtf);
    }

    bool success = QFile::copy(srcVtf, dstVtf);
    if (success) {
        emit logMessage(QString("  ✅ bumpmap 转换成功: %1.vtf").arg(bumpName));
    } else {
        emit logMessage(QString("  ❌ 复制 VTF 失败"));
    }

    return success;
}

//复制emissive贴图到变体
bool VMTGenerator::copyEmissiveToVariant(const Material &mat, const QString &targetDir, const QString &sourceBasePath)
{
    // 只有用户文件夹模式才需要复制
    if (mat.MemissiveSource != SOURCE_USER || mat.Memissive.isEmpty()) {
        return true;
    }

    QString emissiveName = cleanFileName(mat.Memissive);

    // 1. 优先检查 VTF 文件
    QString srcVtf = sourceBasePath + "/" + emissiveName + ".vtf";
    if (QFile::exists(srcVtf)) {
        QString dstVtf = targetDir + "/" + emissiveName + ".vtf";
        if (QFile::exists(dstVtf)) {
            QFile::remove(dstVtf);
        }
        if (QFile::copy(srcVtf, dstVtf)) {
            emit logMessage(QString("  ✅ Emissive 复制成功: %1.vtf").arg(emissiveName));
            return true;
        } else {
            emit logMessage(QString("  ❌ Emissive 复制失败: %1.vtf").arg(emissiveName));
            return false;
        }
    }

    // 2. 检查 PNG 文件
    QString srcPng = sourceBasePath + "/" + emissiveName + ".png";
    if (!QFile::exists(srcPng)) {
        emit logMessage(QString("  ⚠️ Emissive 文件不存在: %1 (尝试了 .vtf 和 .png)").arg(emissiveName));
        return false;
    }

    // 3. PNG 需要转换成 VTF
    emit logMessage(QString("  转换 Emissive PNG: %1.png").arg(emissiveName));

    // 创建临时目录
    QString tempPngDir = tempDir + "/pngGenerated/";
    FileUtils::ensureDirExists(tempPngDir);

    QString tempPngFile = tempPngDir + emissiveName + ".png";
    if (QFile::exists(tempPngFile)) {
        QFile::remove(tempPngFile);
    }

    if (!QFile::copy(srcPng, tempPngFile)) {
        emit logMessage(QString("  ❌ 复制 Emissive PNG 到临时目录失败"));
        return false;
    }

    // 调用 VTFGenerator 转换
    VTFGenerator vtfGen;
    vtfGen.setMareTFPath(maretfPath);

    Material tempMat;
    tempMat.MFilename = emissiveName;
    tempMat.vmtName = emissiveName;
    tempMat.MAlpha = false;
    tempMat.useAlphaTexture = false;
    tempMat.hasDiff = false;

    QList<Material> singleMatList;
    singleMatList.append(tempMat);
    vtfGen.setMaterials(singleMatList);
    vtfGen.setTempDir(tempDir);

    QStringList pngFiles;
    pngFiles.append(emissiveName + ".png");
    vtfGen.setPngFiles(pngFiles);
    vtfGen.setTgaFiles(QStringList());

    if (!vtfGen.convertAll()) {
        emit logMessage(QString("  ❌ Emissive PNG 转换失败: %1").arg(emissiveName));
        return false;
    }

    // 复制转换后的 VTF 到目标目录
    QString srcConvertedVtf = tempDir + "/vtfGenerated/" + emissiveName + ".vtf";
    QString dstVtf = targetDir + "/" + emissiveName + ".vtf";

    if (!QFile::exists(srcConvertedVtf)) {
        emit logMessage(QString("  ❌ 转换后的 Emissive VTF 不存在: %1").arg(emissiveName));
        return false;
    }

    if (QFile::exists(dstVtf)) {
        QFile::remove(dstVtf);
    }

    if (QFile::copy(srcConvertedVtf, dstVtf)) {
        emit logMessage(QString("  ✅ Emissive 转换成功: %1.vtf").arg(emissiveName));
        return true;
    } else {
        emit logMessage(QString("  ❌ 复制 Emissive VTF 失败"));
        return false;
    }
}
