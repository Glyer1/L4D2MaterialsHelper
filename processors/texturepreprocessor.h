#ifndef TEXTUREPREPROCESSOR_H
#define TEXTUREPREPROCESSOR_H

#include <QObject>
#include <QString>
#include <QStringList>
#include "../datastructures.h"

class TexturePreprocessor : public QObject
{
    Q_OBJECT
public:
    explicit TexturePreprocessor(QObject *parent = nullptr);
    ~TexturePreprocessor();  // 添加析构函数
    // 设置输入路径
    void setSourcePath(const QString &path) { sourcePath = path; }
    
    // 设置材质列表
    void setMaterials(const QList<Material> &mats) { materials = mats; }
    
    // 执行预处理
    bool process();
    
    // 获取生成的TGA文件列表
    QStringList getGeneratedTGA() const { return tgaFiles; }
    
    // 获取复制的PNG文件列表
    QStringList getGeneratedPNG() const { return pngFiles; }

signals:
    // 进度信号
    void progressUpdated(int current, int total);
    
    // 日志信号
    void logMessage(const QString &msg);

private:
    QString sourcePath;              // 源文件路径
    QList<Material> materials;        // 材质列表
    QStringList tgaFiles;             // 生成的TGA文件
    QStringList pngFiles;             // 复制的PNG文件
    
    // 处理单个材质
    bool processMaterial(const Material &mat, const QString &tempDir);
    
    // 处理需要Alpha的材质（PNG转TGA）
    bool processAlphaMaterial(const Material &mat, const QString &srcFile, const QString &dstFile);
    
    // 复制普通PNG
    bool copyPNGMaterial(const Material &mat, const QString &srcFile, const QString &dstFile);

    bool processAlphaTextureMaterial(const Material &mat, const QString &srcFile, const QString &alphaSrcFile, const QString &dstFile);

    // 使用预加载的Alpha贴图处理（用于动图）
    bool processAlphaTextureMaterialWithPreloaded(
        const Material &mat,
        const QString &srcFile,
        const QImage &alphaImage,
        const QString &dstFile);
};

#endif // TEXTUREPREPROCESSOR_H
