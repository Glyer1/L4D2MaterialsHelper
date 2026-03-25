#include "vpkgenerator.h"
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QDebug>

VPKGenerator::VPKGenerator(QObject *parent) : QObject(parent) {}

//打包全部
bool VPKGenerator::packAll()
{
    //查看工具路径是否有效
    if (vpkPath.isEmpty() || !QFile::exists(vpkPath)) {
        emit logMessage("错误：VPK工具路径无效");
        return false;
    }

    if (outputPath.isEmpty()) {
        emit logMessage("错误：输出路径为空");
        return false;
    }

    QDir outputDir(outputPath);
    if (!outputDir.exists()) {
        emit logMessage("错误：输出目录不存在");
        return false;
    }

    // 获取OutputPath下一层的所有组件文件夹
    QStringList componentDirs = outputDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    if (componentDirs.isEmpty()) {
        emit logMessage("警告：没有找到任何组件文件夹");
        return true;
    }

    // 收集所有需要打包的变体文件夹
    QList<QPair<QString, QString>> variantsToPack; // <组件名, 变体文件夹路径>（空）

    for (const QString &componentDir : componentDirs) {
        QString componentPath = outputPath + "/" + componentDir;
        QDir componentDirObj(componentPath);

        // 获取组件下的所有变体文件夹
        QStringList variantDirs = componentDirObj.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

        for (const QString &variantDir : variantDirs) {
            QString variantPath = componentPath + "/" + variantDir;
            variantsToPack.append(qMakePair(componentDir, variantPath));//<组件名, 变体文件夹路径>（放置）
        }
    }

    if (variantsToPack.isEmpty()) {
        emit logMessage("警告：没有找到任何变体文件夹");
        return true;
    }

    emit logMessage(QString("找到 %1 个变体文件夹，开始打包...").arg(variantsToPack.size()));

    int successCount = 0;
    int current = 0;

    for (const auto &variant : variantsToPack) {
        current++;
        emit progressUpdated(current, variantsToPack.size());

        QString componentName = variant.first;
        QString variantPath = variant.second;
        QFileInfo variantInfo(variantPath);
        QString variantName = variantInfo.fileName();  // 变体文件夹名

        emit logMessage(QString("正在打包: %1/%2").arg(componentName).arg(variantName));

        // 打包这个变体文件夹，调用packDirectory
        if (packDirectory(variantPath)) {
            successCount++;
            emit logMessage(QString("  ✅ 打包成功: %1.vpk").arg(variantName));
        } else {
            emit logMessage(QString("  ❌ 打包失败: %1/%2").arg(componentName).arg(variantName));
        }
    }

    emit logMessage(QString("打包完成，成功: %1/%2").arg(successCount).arg(variantsToPack.size()));
    return successCount > 0;
}
//调用工具打包，单个变体进行处理
bool VPKGenerator::packDirectory(const QString &dirPath)
{
    // 检查目录是否存在
    QDir dir(dirPath);
    if (!dir.exists()) {
        emit logMessage("  目录不存在: " + dirPath);
        return false;
    }

    // 获取变体文件夹的父目录（组件文件夹路径）
    QFileInfo dirInfo(dirPath);
    QString componentPath = dirInfo.path();  // 组件文件夹路径
    QString variantName = dirInfo.fileName(); // 变体文件夹名

    // vpk.exe 会在当前工作目录生成 变体文件夹名.vpk
    QString vpkPath = componentPath + "/" + variantName + ".vpk";

    emit logMessage("  目标路径: " + vpkPath);

    // 如果已存在同名vpk，先删除
    if (QFile::exists(vpkPath)) {
        QFile::remove(vpkPath);
    }

    // 构建命令行参数
    QStringList args;
    args << dirPath;

    emit logMessage("  执行: " + this->vpkPath + " " + dirPath);

    // 执行vpk命令，工作目录设置为组件文件夹
    QProcess process;
    process.setWorkingDirectory(componentPath);
    //这里不是成员变量哦，是vpk.exe
    process.start(this->vpkPath, args);

    if (!process.waitForStarted()) {
        emit logMessage("  无法启动vpk工具");
        return false;
    }

    if (!process.waitForFinished(30000)) {
        emit logMessage("  vpk执行超时");
        process.kill();
        return false;
    }

    // 检查文件是否生成
    if (QFile::exists(vpkPath)) {
        emit logMessage("  ✅ VPK生成成功: " + vpkPath);
        return true;
    }

    emit logMessage("  ❌ VPK文件未生成: " + vpkPath);
    return false;
}
