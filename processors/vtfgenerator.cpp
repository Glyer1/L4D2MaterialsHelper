#include "vtfgenerator.h"
#include "../utils/fileutils.h"
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QDebug>

VTFGenerator::VTFGenerator(QObject *parent) : QObject(parent) {}

//扫描全部处理动帧材质（单个处理）
void VTFGenerator::parseAnimatedMaterials()
{
    animatedGroups.clear();

    for (const Material &mat : materials) {
        // 检查是否是动图（MFilename 包含 @）
        if (mat.MFilename.contains('@')) {

            //获取帧
            QStringList frames = mat.MFilename.split('@', Qt::SkipEmptyParts);
            if (frames.isEmpty()) continue;
            QString baseName = mat.vmtName;  // 用 vmtName 作为基础名，用于存animatedGroups做key，value是帧0 帧1...
            if(mat.hasDiff && !mat.diffNames.isEmpty())
                baseName = mat.vmtName+"_"+mat.diffNames.first();

            QStringList frameFiles;

            emit logMessage(QString("解析动图: %1, 基础名: %2, %3 帧")
                                .arg(mat.MFilename).arg(baseName).arg(frames.size()));

            for (int i = 0; i < frames.size(); i++) {
                QString frameName = frames[i].trimmed();

                bool needTga = (mat.MAlpha || mat.useAlphaTexture);

                if (needTga) {
                    // 有 Alpha 的找到已经生成得 TGA
                    QString tgaFile = QString("tgaGenerated/%1_frame%2.tga")
                                          .arg(baseName).arg(i);
                    QString fullPath = tempDir + "/" + tgaFile;

                    if (QFile::exists(fullPath)) {
                        frameFiles.append(fullPath);
                        emit logMessage(QString("  找到帧 %1: %2").arg(i).arg(tgaFile));
                    } else {
                        emit logMessage(QString("  警告：找不到帧文件 %1").arg(tgaFile));
                    }
                } else {
                    // 无 Alpha 的复制 PNG
                    QString pngFile = QString("pngGenerated/%1_frame%2.png")
                                          .arg(baseName).arg(i);
                    QString fullPath = tempDir + "/" + pngFile;

                    if (QFile::exists(fullPath)) {
                        frameFiles.append(fullPath);
                        emit logMessage(QString("  找到帧 %1: %2").arg(i).arg(pngFile));
                    } else {
                        emit logMessage(QString("  警告：找不到帧文件 %1").arg(pngFile));
                    }
                }
            }

            if (!frameFiles.isEmpty()) {
                //往key是基础名(对应动图各个差分)放动帧列表，frameFiles这个列表存的是一堆path
                animatedGroups[baseName] = frameFiles;
                emit logMessage(QString("动图解析成功: %1, %2 帧").arg(baseName).arg(frameFiles.size()));
            } else {
                emit logMessage(QString("警告：动图 %1 没有找到任何帧文件").arg(baseName));
            }
        }
    }
}


//转化单个文件的处理函数
bool VTFGenerator::convertSingleFile(const QString &inputFile, const QString &outputFile)
{
    QFileInfo info(inputFile);

    // 删除已存在的文件
    if (QFile::exists(outputFile)) {
        QFile::remove(outputFile);
    }

    // 注意：这里不再处理文件名，outputFile 已经是正确的路径
    // 文件名已经在 convertAll 中根据是否有差分决定好了

    // MareTF 的单帧转换参数
    QStringList args;
    args << "create" << inputFile;
    args << "--output" << outputFile;
    args << "--version" << "7.2";

    // 根据文件扩展名决定格式
    if (info.suffix() == "tga") {
        args << "--format" << "DXT5";  // 带Alpha用DXT5
    } else {
        args << "--format" << "DXT5";  // 无Alpha也DXT5，防止出问题，原本是DXT1
    }

    args << "--filter" << "KAISER";
    args << "--srgb";                    // sRGB颜色空间
    args << "--resize-method" << "BIGGER";

    emit logMessage(QString("转换: %1 -> %2").arg(info.fileName()).arg(QFileInfo(outputFile).fileName()));

    //启动maretf处理
    return runMareTF(args);
}

//转化动画材质
bool VTFGenerator::convertAnimatedTexture(const QString &outputBaseName, const QStringList &frameFiles)
{
    if (frameFiles.isEmpty()) {
        emit logMessage(QString("错误：动图 %1 没有帧文件").arg(outputBaseName));
        return false;
    }

    //最后放到/Temp/vtfGenerated
    QString outputDir = tempDir + "/vtfGenerated";
    FileUtils::ensureDirExists(outputDir);
    QString outputPath = outputDir + "/" + outputBaseName + ".vtf";

    // 删除已存在的文件
    if (QFile::exists(outputPath)) {
        QFile::remove(outputPath);
    }

    // 创建临时文件夹存放所有帧
    QString tempFrameDir = tempDir + "/animated_frames/" + outputBaseName;
    FileUtils::ensureDirExists(tempFrameDir);

    // 清空临时文件夹
    QDir dir(tempFrameDir);
    dir.removeRecursively();
    FileUtils::ensureDirExists(tempFrameDir);

    // 复制所有帧到临时文件夹，并按 MareTF 要求的格式命名
    // 格式：基础名 + 四位数字序号 + 扩展名，例如：anim0000.tga, anim0001.tga
    QString baseFrameName = "anim";  // 基础名，随便起，但要一致

    //对每一个animatedGroups数据结构的每个地址拷过来（这里不会覆盖）
    for (int i = 0; i < frameFiles.size(); ++i) {
        QString srcFile = frameFiles[i];
        if (!QFile::exists(srcFile)) {
            emit logMessage(QString("错误：帧文件不存在: %1").arg(srcFile));
            return false;
        }

        //后缀变小写
        QString ext = QFileInfo(srcFile).suffix().toLower();
        // 关键：使用统一的基础名 + 序号，确保 MareTF 能识别为序列
        QString dstFile = tempFrameDir + QString("/%1%2.%3")
                                             .arg(baseFrameName)
                                             .arg(i, 4, 10, QChar('0'))
                                             .arg(ext);
        if (!QFile::copy(srcFile, dstFile)) {
            emit logMessage(QString("错误：复制帧文件失败: %1 -> %2").arg(srcFile).arg(dstFile));
            return false;
        }
        emit logMessage(QString("  准备帧 %1: %2").arg(i).arg(QFileInfo(dstFile).fileName()));
    }

    // 构建 MareTF 的参数列表
    QStringList args;
    args << "create";
    //前边已存好文件到animated_frame，提供第一帧的完整路径作为输入，MareTF 会自动找后续帧
    QString firstFrame = tempFrameDir + "/" + baseFrameName + "0000."
                         + QFileInfo(frameFiles.first()).suffix().toLower();
    args << firstFrame;  // 例如：.../anim0000.tga
    args << "--output" << outputPath;
    args << "--version" << "7.2";
    args << "--format" << "DXT5";
    args << "--filter" << "KAISER";
    args << "--srgb";
    args << "--animated-frames";  // 标志参数，不加值！之前落坑。
    args << "--resize-method" << "BIGGER";

    emit logMessage(QString("转换动图: %1, %2 帧 -> %3").arg(outputBaseName).arg(frameFiles.size()).arg(outputPath));
    emit logMessage(QString("第一帧: %1").arg(firstFrame));

    // 工作目录设为帧所在目录（虽然用绝对路径应该也行，但这样更保险）
    bool success = runMareTF(args, tempFrameDir);

    // 清理临时文件夹
    QDir(tempFrameDir).removeRecursively();

    return success;
}

//运行maretf转化
bool VTFGenerator::runMareTF(const QStringList &args, const QString &workDir)
{

    qDebug() << "=== MareTF 调试信息 ===";
    qDebug() << "工具路径:" << MareTFPath;
    qDebug() << "工作目录:" << (workDir.isEmpty() ? "无" : workDir);
    qDebug() << "参数列表:";
    for (int i = 0; i < args.size(); ++i) {
        qDebug() << "  args[" << i << "] =" << args[i];
    }

    // 检查第一个帧文件是否存在
    for (const QString &arg : args) {
        if (arg != "create" && !arg.startsWith("--") && arg.contains(".")) {
            QFileInfo info(arg);
            qDebug() << "检查输入文件:" << arg;
            qDebug() << "  绝对路径:" << info.absoluteFilePath();
            qDebug() << "  是否存在:" << (info.exists() ? "是" : "否");
            qDebug() << "  可读:" << (info.isReadable() ? "是" : "否");
            break;
        }
    }
    qDebug() << "===========检查结束===========";

    if (!QFile::exists(MareTFPath)) {
        emit logMessage(QString("工具不存在: %1").arg(MareTFPath));
        return false;
    }

    QProcess process;
    if (!workDir.isEmpty()) {
        // 如果传入了工作目录，就用它
        process.setWorkingDirectory(workDir);
        emit logMessage(QString("使用传入的工作目录: %1").arg(workDir));
    } else {
        // 否则从 args 里找第一个文件所在的目录
        for (const QString &arg : args) {
            // 跳过非文件参数（如 "create"、"--output"等）
            if (arg == "create" || arg.startsWith("--")) continue;

            QFileInfo info(arg);
            if (info.exists()) {
                process.setWorkingDirectory(info.path());
                emit logMessage(QString("自动设置工作目录: %1").arg(info.path()));
                break;
            }
        }
    }

    // 直接使用传入的 args，不需要额外添加
    emit logMessage(QString("执行: %1 %2").arg(MareTFPath).arg(args.join(" ")));

    process.start(MareTFPath, args);

    if (!process.waitForStarted()) {
        emit logMessage("无法启动工具");
        return false;
    }

    // 60秒超时
    if (!process.waitForFinished(60000)) {
        emit logMessage("工具执行超时（60秒）");
        process.kill();
        return false;
    }

    QString output = process.readAllStandardOutput();
    QString error = process.readAllStandardError();

    if (!output.isEmpty()) {
        emit processOutput(output);
    }

    if (!error.isEmpty()) {
        emit logMessage("错误: " + error);
    }

    if (process.exitCode() != 0) {
        emit logMessage(QString("工具执行失败，退出代码: %1").arg(process.exitCode()));
        return false;
    }

    return true;
}

bool VTFGenerator::convertAll()
{
    // 如果工具路径为空，从设置读取
    if (MareTFPath.isEmpty()) {
        // 从 QSettings 读取用户之前设置的路径
        QSettings settings("Local", "Path");
        MareTFPath = settings.value("VtfPath").toString();

        // 如果还是为空，再用默认路径
        if (MareTFPath.isEmpty()) {
            QString appDir = QCoreApplication::applicationDirPath();
            MareTFPath = appDir + "/Resources/Tools/MareTF/maretf.exe";
            emit logMessage(QString("使用默认工具路径: %1").arg(MareTFPath));
        } else {
            emit logMessage(QString("使用用户设置的工具路径: %1").arg(MareTFPath));
        }
    }
    QString outputDir = tempDir + "/vtfGenerated";
    FileUtils::ensureDirExists(outputDir);

    // 先扫描所有材质解析动图材质
    parseAnimatedMaterials();

    // 调试：输出解析结果
    emit logMessage(QString("解析到 %1 个动图").arg(animatedGroups.size()));
    for (auto it = animatedGroups.begin(); it != animatedGroups.end(); ++it) {
        emit logMessage(QString("  动图: %1, %2 帧").arg(it.key()).arg(it.value().size()));
    }

    int total = materials.size();
    int current = 0;
    bool allSuccess = true;

    emit logMessage(QString("开始生成 VTF，共 %1 个材质").arg(total));

    //转化材质
    for (const Material &mat : materials) {
        emit progressUpdated(current, total);

        QString baseName = mat.vmtName;
        if(mat.hasDiff && !mat.diffNames.isEmpty())
        {
            baseName = mat.vmtName+"_"+mat.diffNames.first();
        }

        //先检查单个差分是否被包含在动图QMap里
        if (animatedGroups.contains(baseName))
        {
            // 动图材质：用之前解析好的帧文件
            emit logMessage(QString("处理动图: %1").arg(baseName));

            // 动图输出文件名：有差分用差分名，无差分用材质名
            QString animatedOutputName;
            if (mat.hasDiff && !mat.diffNames.isEmpty()) {
                animatedOutputName = mat.vmtName+"_"+mat.diffNames.first();  // 有差分用差分名
            } else {
                animatedOutputName = baseName;  // 无差分用材质名
            }

            //调用转化函数
            if (!convertAnimatedTexture(animatedOutputName, animatedGroups[baseName])) {
                emit logMessage(QString("动图转换失败: %1").arg(baseName));
                allSuccess = false;
            }
        }
        //检查 MFilename 是否包含 @，避免进入单帧处理
        else if (mat.MFilename.contains('@'))
        {
            // 这种情况不应该发生，说明 parseAnimatedMaterials 没处理好
            emit logMessage(QString("警告：动图材质 %1 未被正确解析").arg(baseName));
            allSuccess = false;
        }
        else
        {
            // 单帧材质
            QString inputFile;

            //判断是否需要TGA（高亮Alpha 或 独立Alpha贴图）
            bool needTga = (mat.MAlpha || mat.useAlphaTexture);

            //有alpha在tgaGenerated里面找
            if (needTga) {
                QString tgaName = QString("%1.tga").arg(mat.MFilename);
                QString tgaFilePath = tempDir + "/tgaGenerated/" + tgaName;
                QFileInfo tgaFile(tgaFilePath);
                if (tgaFile.exists()) {
                    inputFile = tempDir + "/tgaGenerated/" + tgaName;
                } else {
                    emit logMessage(QString("找不到TGA文件: %1").arg(tgaName));
                    continue;
                }
            }
            //没有alpha在pngGenerated里面找
            else {
                QString pngName = QString("%1.png").arg(mat.MFilename);
                QString pngFilePath = tempDir + "/pngGenerated/" + pngName;
                QFileInfo pngFile(pngFilePath);
                if (pngFile.exists()) {
                    inputFile = tempDir + "/pngGenerated/" + pngName;
                } else {
                    emit logMessage(QString("找不到PNG文件: %1").arg(pngName));
                    continue;
                }
            }

            if (inputFile.isEmpty()) {
                emit logMessage(QString("找不到输入文件: %1").arg(mat.MFilename));
                allSuccess = false;
            } else {
                // 根据是否有差分决定输出文件名
                QString outputFileName;
                if (mat.hasDiff && !mat.diffNames.isEmpty()) {
                    // 有差分的材质：用差分名
                    outputFileName = mat.vmtName+"_"+mat.diffNames.first() + ".vtf";
                } else {
                    // 无差分的材质：用材质名
                    outputFileName = baseName + ".vtf";
                }
                QString outputFile = outputDir + "/" + outputFileName;
                if (!convertSingleFile(inputFile, outputFile)) {
                    emit logMessage(QString("转换失败: %1").arg(mat.MFilename));
                    allSuccess = false;
                }
            }
        }

        current++;

    }

    emit progressUpdated(total, total);
    emit logMessage("VTF 生成完成");

    return allSuccess;
}
