#ifndef VMTGENERATOR_H
#define VMTGENERATOR_H

#include <QObject>
#include <QString>
#include <QStringList>
#include "../datastructures.h"

class VMTGenerator : public QObject
{
    Q_OBJECT
public:
    explicit VMTGenerator(QObject *parent = nullptr);

    void setOutputPath(const QString &path) { outputPath = path; }
    void setMatPath(const QString &path) { matPath = path; }
    void setTempDir(const QString &dir) { tempDir = dir; }
    void setComponents(const QList<Component> &comps) { components = comps; }
    void setMaterials(const QList<Material> &mats) { materials = mats; }
    void setProjectVariants(const QList<ComponentVariants> &vars) { projectVariants = vars; }

    bool generateAll();
    void setSourceBasePath(const QString &path) { sourceBasePath = path; }
    void setMareTFPath(const QString &path) { maretfPath = path; }

    void setGlobalValues(const QString &version, const QString &author,
                         const QString &tagline, const QString &desc) {
        globalVersion = version;
        globalAuthor = author;
        globalTagline = tagline;
        globalDescription = desc;
    }

signals:
    void logMessage(const QString &msg);
    void progressUpdated(int current, int total);


private:
    QString outputPath;      // 用户设置的输出目录
    QString matPath;         // 材质路径前缀，如 "shiratsume/a"
    QString tempDir;         // 临时目录，存放生成的VTF
    QList<Component> components;
    QList<Material> materials;
    QList<ComponentVariants> projectVariants;
    QString sourceBasePath;  // 新增：用户设置的源文件路径
    QString maretfPath;  // MareTF 工具路径

    QString globalVersion;
    QString globalAuthor;
    QString globalTagline;
    QString globalDescription;

    QString generateVMTContent(const Material &mat, const QString &selectedDiff);
    bool copyVTF(const QString &vtfName, const QString &destDir);

    // 统一处理文件名：去掉后缀和路径，只保留纯文件名
    QString cleanFileName(const QString &fileName);

    // 获取源文件路径（带后缀）
    QString getSourceFilePath(const QString &baseName, const QString &sourcePath, const QString &suffix);

    // 获取目标VTF路径（不带后缀）
    QString getTargetVtfPath(const QString &baseName, const QString &targetDir);

    bool copyBumpmapToVariant(const Material &mat, const QString &targetDir,
                              const QString &sourceBasePath);

    bool copyToBaseComponent(const Material &mat, const QString &baseDir,
                                           const QString &sourceBasePath, QSet<QString> &copiedFiles);

    void generateAddonInfo(const QString &filePath, const QString &title,
                           const QString &author, const QString &description,
                           const QString &version, const QString &tagline);

    bool copyEmissiveToVariant(const Material &mat, const QString &targetDir, const QString &sourceBasePath);
};

#endif // VMTGENERATOR_H
