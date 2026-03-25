#include "texturepreprocessor.h"
#include "../utils/fileutils.h"
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QDir>
#include <FreeImage.h>
#include <QApplication>

TexturePreprocessor::TexturePreprocessor(QObject *parent) : QObject(parent)
{
    // 从QSettings加载源路径
    sourcePath = FileUtils::getPathFromSettings("SourcePath", QApplication::applicationDirPath());
}

//主要处理函数
bool TexturePreprocessor::process()
{
    // 获取临时目录
    QString tempDir = FileUtils::getTempDir();

    // 创建输出目录
    QString tgaDir = tempDir + "/tgaGenerated";
    QString pngDir = tempDir + "/pngGenerated";

    if (!FileUtils::ensureDirExists(tgaDir) || !FileUtils::ensureDirExists(pngDir)) {
        emit logMessage("无法创建输出目录");
        return false;
    }

    // 清空之前的文件列表
    tgaFiles.clear();
    pngFiles.clear();

    int total = materials.size();
    int current = 0;

    emit logMessage(QString("开始预处理纹理，共 %1 个材质").arg(total));

    //逐个设置好的处理材质
    for (const Material &mat : materials) {
        if (mat.MFilename.isEmpty()) {
            current++;
            continue;
        }

        emit progressUpdated(current, total);
        emit logMessage(QString("处理: %1").arg(mat.MFilename));

        //调用开始处理失败处置
        if (!processMaterial(mat, tempDir)) {
            emit logMessage(QString("处理失败: %1").arg(mat.MFilename));
        }

        current++;
    }

    emit progressUpdated(total, total);
    emit logMessage(QString("预处理完成。生成TGA: %1个, 复制PNG: %2个")
                        .arg(tgaFiles.size()).arg(pngFiles.size()));

    return true;
}


//材质分开处理-分tga还是png，还有帧
bool TexturePreprocessor::processMaterial(const Material &mat, const QString &tempDir)
{
    // 检查是否是多帧动图
    if (mat.MFilename.contains('@')) {
        // 动图处理
        QStringList frames = mat.MFilename.split('@', Qt::SkipEmptyParts);
        emit logMessage(QString("检测到动图材质: %1，共 %2 帧").arg(mat.vmtName).arg(frames.size()));

        bool allSuccess = true;
        //统一判断是否需要TGA
        bool needTga = (mat.MAlpha || mat.useAlphaTexture);
        //有独立Alpha贴图加载一次先
        QImage alphaImage;
        //判断是否使用独立alpha，检查独立alpha在不在
        if (mat.useAlphaTexture && !mat.alphaTextureName.isEmpty()) {
            QString alphaSrcFile = sourcePath + "/" + mat.alphaTextureName + ".png";
            if (!alphaImage.load(alphaSrcFile)) {
                emit logMessage(QString("警告：Alpha贴图加载失败: %1，将使用默认Alpha").arg(alphaSrcFile));
            }
        }

        //多帧逐帧处理
        for (int i = 0; i < frames.size(); i++) {
            QString frameName = frames[i].trimmed();
            QString srcFile = sourcePath + "/" + frameName + ".png";


            if (!QFile::exists(srcFile)) {
                emit logMessage(QString("动图帧文件不存在: %1").arg(srcFile));
                allSuccess = false;
                continue;
            }

            // 动图帧的命名规则：vmtName + (差分名) + _帧序号,防止覆盖
            QString frameVmtName;
            if (mat.hasDiff && !mat.diffNames.isEmpty()) {
                frameVmtName = mat.vmtName + "_" + mat.diffNames.first() + "_frame" + QString::number(i);
            } else {
                frameVmtName = mat.vmtName + "_frame" + QString::number(i);
            }

            // 创建临时Material对象用于处理
            Material frameMat = mat;
            frameMat.MFilename = frameName;
            frameMat.vmtName = frameVmtName;

            // 根据Alpha标志决定处理方式
            if (needTga) {
                QString dstFile = tempDir + "/tgaGenerated/" + frameVmtName + ".tga";

                //使用预加载的Alpha贴图处理,alpha通道用独立贴图
                if (mat.useAlphaTexture && !alphaImage.isNull()) {
                    if (processAlphaTextureMaterialWithPreloaded(frameMat, srcFile, alphaImage, dstFile)) {
                        tgaFiles << frameVmtName + ".tga";
                    } else {
                        allSuccess = false;
                    }
                }
                //普通alpha通道处理
                else {
                    // 普通高亮Alpha
                    if (processAlphaMaterial(frameMat, srcFile, dstFile)) {
                        tgaFiles << frameVmtName + ".tga";
                    } else {
                        allSuccess = false;
                    }
                }
            }
            //普通png
            else {
                QString dstFile = tempDir + "/pngGenerated/" + frameVmtName + ".png";
                if (copyPNGMaterial(frameMat, srcFile, dstFile)) {
                    pngFiles << frameVmtName + ".png";
                } else {
                    allSuccess = false;
                }
            }
        }
        return allSuccess;
    }

    QString srcFile = sourcePath + "/" + mat.MFilename + ".png";

    // 检查源文件是否存在
    if (!QFile::exists(srcFile)) {
        emit logMessage(QString("文件不存在: %1").arg(srcFile));
        return false;
    }

    //处理独立alpha贴图
    if (mat.useAlphaTexture && !mat.alphaTextureName.isEmpty()) {
        QString alphaSrcFile = sourcePath + "/" + mat.alphaTextureName + ".png";
        if (!QFile::exists(alphaSrcFile)) {
            emit logMessage(QString("Alpha贴图不存在: %1").arg(alphaSrcFile));
            return false;
        }

        QString dstFile = tempDir + "/tgaGenerated/" + mat.MFilename + ".tga";
        if (processAlphaTextureMaterial(mat, srcFile, alphaSrcFile, dstFile)) {
            tgaFiles << mat.MFilename + ".tga";
            return true;
        }
        return false;
    }

    // 根据Alpha标志决定处理方式tga还是png
    if (mat.MAlpha) {
        QString dstFile = tempDir + "/tgaGenerated/" + mat.MFilename + ".tga";
        if (processAlphaMaterial(mat, srcFile, dstFile)) {
            tgaFiles << mat.MFilename + ".tga";
            return true;
        }
    } else {
        QString dstFile = tempDir + "/pngGenerated/" + mat.MFilename + ".png";
        if (copyPNGMaterial(mat, srcFile, dstFile)) {
            pngFiles << mat.MFilename + ".png";
            return true;
        }
    }

    return false;
}

//tga处理方式，使用FreeImage加一张alpha通道(比黑亮一点)
bool TexturePreprocessor::processAlphaMaterial(const Material &mat, const QString &srcFile, const QString &dstFile)
{
    // 测试FreeImage是否正常
    static bool freeImageInitialized = false;
    if (!freeImageInitialized) {
        FreeImage_Initialise();
        freeImageInitialized = true;
        emit logMessage("FreeImage initialized");
    }

    qDebug() << "FreeImage version:" << FreeImage_GetVersion();

    QImage image(srcFile);

    if (image.isNull()) {
        emit logMessage(QString("无法加载图片: %1").arg(srcFile));
        return false;
    }

    // 转换为RGBA8888格式（8位每通道）
    image = image.convertToFormat(QImage::Format_RGBA8888);

    // 垂直翻转图片（解决倒转问题）
    image = image.mirrored();

    // 要让PS显示16，需要设置线性值约为 16^(1/2.2) ≈ 5.5
    int alphaValue = 6;  // 设置6PS显示RGBS0 11 11 11

    // 检查是否有差分,如果有差分，差分名字包含"bright"或"亮"，就用更高的值
    bool shouldBeBright = false;

    if (mat.hasDiff && !mat.diffNames.isEmpty()) {
        // 有差分的材质：根据选中的差分名字判断
        if (mat.diffNames.first().contains("bright") ||
            mat.diffNames.first().contains("亮")) {
            shouldBeBright = true;
        }
    } else {
        // 无差分的材质：直接根据材质名字判断
        if (mat.Mname.contains("bright") ||
            mat.Mname.contains("亮") ||
            mat.vmtName.contains("bright") ||
            mat.vmtName.contains("亮")) {
            shouldBeBright = true;
        }
    }
    if (shouldBeBright) {
        alphaValue = 14;  // 比正常亮一点，眼睛亮度
    }

    //返回指针，然后对Alpha通道设置统一alpha值
    unsigned char *bits = image.bits();
    for (int i = 3; i < image.sizeInBytes(); i += 4) {
        bits[i] = alphaValue;
    }

    QFileInfo fileInfo(dstFile);
    QDir().mkpath(fileInfo.path());

    if (QFile::exists(dstFile)) {
        QFile::remove(dstFile);
    }

    // 创建8位每通道的FreeImage位图（空）
    FIBITMAP *bitmap = FreeImage_AllocateT(FIT_BITMAP, image.width(), image.height(), 32);
    if (!bitmap) {
        emit logMessage("无法分配FreeImage位图");
        return false;
    }

    //通道交换,ps里头[R,G,B],FreeImage[B,G,R],防止通道颜色值设置错误
    for (int i = 0; i < image.sizeInBytes(); i += 4) {
        // 交换 R 和 B 通道
        unsigned char r = bits[i];
        bits[i] = bits[i+2];
        bits[i+2] = r;
    }

    // 复制像素数据
    memcpy(FreeImage_GetBits(bitmap), image.bits(), image.sizeInBytes());

    // 保存为TGA（8位）
    QByteArray fileNameBytes = dstFile.toLocal8Bit();
    BOOL success = FreeImage_Save(FIF_TARGA, bitmap, fileNameBytes.data());

    // 清理
    FreeImage_Unload(bitmap);

    if (!success) {
        emit logMessage(QString("FreeImage保存TGA失败: %1").arg(dstFile));
        return false;
    }

    // 验证生成文件是否存在
    if (QFile::exists(dstFile)) {
        QFileInfo info(dstFile);
        emit logMessage(QString("✅ 生成8位TGA: %1 (大小: %2 字节)")
                            .arg(dstFile).arg(info.size()));
    }

    return true;
}

//png处理方式：直接移动到Temp/pngGenerated
bool TexturePreprocessor::copyPNGMaterial(const Material &mat, const QString &srcFile, const QString &dstFile)
{
    // 如果目标文件已存在，先删除
    if (QFile::exists(dstFile)) {
        if (!QFile::remove(dstFile)) {
            emit logMessage(QString("无法删除已存在的文件: %1").arg(dstFile));
            return false;
        }
        emit logMessage(QString("删除已存在的文件: %1").arg(dstFile));
    }

    if (QFile::copy(srcFile, dstFile)) {
        emit logMessage(QString("复制PNG: %1 -> %2").arg(srcFile).arg(dstFile));
        return true;
    } else {
        emit logMessage(QString("复制PNG失败: %1").arg(srcFile));
        return false;
    }
}

TexturePreprocessor::~TexturePreprocessor()
{
    // 目前没有特别需要清理的
}


//透明的那个alpha，拿别的贴图做alpha通道
bool TexturePreprocessor::processAlphaTextureMaterial(const Material &mat, const QString &srcFile, const QString &alphaSrcFile, const QString &dstFile)
{
    // 加载主贴图
    QImage mainImage(srcFile);
    if (mainImage.isNull()) {
        emit logMessage(QString("无法加载主贴图: %1").arg(srcFile));
        return false;
    }

    // 加载Alpha贴图（当成灰度图用）
    QImage alphaImage(alphaSrcFile);
    if (alphaImage.isNull()) {
        emit logMessage(QString("无法加载Alpha贴图: %1").arg(alphaSrcFile));
        return false;
    }

    // 确保尺寸一致
    if (alphaImage.size() != mainImage.size()) {
        alphaImage = alphaImage.scaled(mainImage.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        emit logMessage(QString("Alpha贴图已缩放至 %1x%2").arg(mainImage.width()).arg(mainImage.height()));
    }

    // 主贴图转RGBA
    mainImage = mainImage.convertToFormat(QImage::Format_RGBA8888);

    // Alpha贴图转灰度（8位）
    QImage grayAlpha = alphaImage.convertToFormat(QImage::Format_Grayscale8);

    // 垂直翻转
    mainImage = mainImage.mirrored();
    grayAlpha = grayAlpha.mirrored();

    // 应用Alpha通道
    unsigned char *mainBits = mainImage.bits();
    unsigned char *grayBits = grayAlpha.bits();

    for (int y = 0; y < mainImage.height(); y++) {
        for (int x = 0; x < mainImage.width(); x++) {
            int mainIdx = (y * mainImage.width() + x) * 4;  // RGBA = 4字节
            int grayIdx = y * grayAlpha.width() + x;        // 灰度 = 1字节,灰度对象

            // 把灰度值当成Alpha通道（0-255）
            unsigned char alphaValue = grayBits[grayIdx];
            mainBits[mainIdx + 3] = alphaValue;  // 设置Alpha通道
        }
    }

    // 交换R和B通道,换成PS的对应[R,G,B]通道
    for (int i = 0; i < mainImage.sizeInBytes(); i += 4) {
        unsigned char r = mainBits[i];
        mainBits[i] = mainBits[i + 2];
        mainBits[i + 2] = r;
    }

    // 保存为TGA（使用FreeImage）
    FIBITMAP *bitmap = FreeImage_AllocateT(FIT_BITMAP, mainImage.width(), mainImage.height(), 32);
    if (!bitmap) {
        emit logMessage("无法分配FreeImage位图");
        return false;
    }

    memcpy(FreeImage_GetBits(bitmap), mainImage.bits(), mainImage.sizeInBytes());

    QByteArray fileNameBytes = dstFile.toLocal8Bit();
    BOOL success = FreeImage_Save(FIF_TARGA, bitmap, fileNameBytes.data());
    FreeImage_Unload(bitmap);

    if (!success) {
        emit logMessage(QString("保存TGA失败: %1").arg(dstFile));
        return false;
    }

    emit logMessage(QString("✅ 生成透明Alpha TGA: %1 (Alpha来自: %2)")
                        .arg(dstFile).arg(mat.alphaTextureName));
    return true;
}

// 使用预加载的Alpha贴图处理（用于动图）
bool TexturePreprocessor::processAlphaTextureMaterialWithPreloaded(
    const Material &mat,
    const QString &srcFile,
    const QImage &alphaImage,
    const QString &dstFile)
{
    // 加载主贴图
    QImage mainImage(srcFile);
    if (mainImage.isNull()) {
        emit logMessage(QString("无法加载主贴图: %1").arg(srcFile));
        return false;
    }

    // 确保尺寸一致
    QImage processedAlpha = alphaImage;
    if (processedAlpha.size() != mainImage.size()) {
        processedAlpha = processedAlpha.scaled(mainImage.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    // 主贴图转RGBA
    mainImage = mainImage.convertToFormat(QImage::Format_RGBA8888);

    // Alpha贴图转灰度
    QImage grayAlpha = processedAlpha.convertToFormat(QImage::Format_Grayscale8);

    // 垂直翻转
    mainImage = mainImage.mirrored();
    grayAlpha = grayAlpha.mirrored();

    // 应用Alpha通道
    unsigned char *mainBits = mainImage.bits();
    unsigned char *grayBits = grayAlpha.bits();

    for (int y = 0; y < mainImage.height(); y++) {
        for (int x = 0; x < mainImage.width(); x++) {
            int mainIdx = (y * mainImage.width() + x) * 4;
            int grayIdx = y * grayAlpha.width() + x;
            mainBits[mainIdx + 3] = grayBits[grayIdx];
        }
    }

    // 交换R和B通道
    for (int i = 0; i < mainImage.sizeInBytes(); i += 4) {
        unsigned char r = mainBits[i];
        mainBits[i] = mainBits[i + 2];
        mainBits[i + 2] = r;
    }

    // 保存为TGA
    FIBITMAP *bitmap = FreeImage_AllocateT(FIT_BITMAP, mainImage.width(), mainImage.height(), 32);
    if (!bitmap) {
        emit logMessage("无法分配FreeImage位图");
        return false;
    }

    memcpy(FreeImage_GetBits(bitmap), mainImage.bits(), mainImage.sizeInBytes());

    QByteArray fileNameBytes = dstFile.toLocal8Bit();
    BOOL success = FreeImage_Save(FIF_TARGA, bitmap, fileNameBytes.data());
    FreeImage_Unload(bitmap);

    if (!success) {
        emit logMessage(QString("保存TGA失败: %1").arg(dstFile));
        return false;
    }

    emit logMessage(QString("✅ 生成动图帧TGA (Alpha共用): %1").arg(dstFile));
    return true;
}
