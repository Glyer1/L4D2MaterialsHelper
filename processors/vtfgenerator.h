#ifndef VTFGENERATOR_H
#define VTFGENERATOR_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QMap>
#include "../datastructures.h"

class VTFGenerator : public QObject
{
    Q_OBJECT
public:
    explicit VTFGenerator(QObject *parent = nullptr);

    // 设置工具路径
    void setMareTFPath(const QString &path) { MareTFPath = path; }
    void setTempDir(const QString &dir) { tempDir = dir; }

    // 设置材质列表（需要原始材质信息来判断动图）
    void setMaterials(const QList<Material> &mats) { materials = mats; }

    // 设置生成的TGA和PNG文件列表（来自预处理）
    void setTgaFiles(const QStringList &files) { tgaFiles = files; }
    void setPngFiles(const QStringList &files) { pngFiles = files; }

    // 执行转换
    bool convertAll();

signals:
    void progressUpdated(int current, int total);
    void logMessage(const QString &msg);
    void processOutput(const QString &output);

private:
    QString MareTFPath;
    QString tempDir;
    QList<Material> materials;
    QStringList tgaFiles;
    QStringList pngFiles;

    // 动图帧分组
    QMap<QString, QStringList> animatedGroups;  // key: 基础名, value: 帧文件列表

    // 解析动图材质
    void parseAnimatedMaterials();

    // 转换单张图片
    bool convertSingleFile(const QString &inputFile, const QString &outputFile);

    // 转换动图
    bool convertAnimatedTexture(const QString &baseName, const QStringList &frameFiles);

    // 运行MareTF
    bool runMareTF(const QStringList &args, const QString &workDir = "");
};

#endif // VTFGENERATOR_H
