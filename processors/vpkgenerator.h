#ifndef VPKGENERATOR_H
#define VPKGENERATOR_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QProcess>

class VPKGenerator : public QObject
{
    Q_OBJECT
public:
    explicit VPKGenerator(QObject *parent = nullptr);

    // 设置工具路径
    void setVpkPath(const QString &path) { vpkPath = path; }

    // 设置输出路径
    void setOutputPath(const QString &path) { outputPath = path; }

    // 执行打包
    bool packAll();

signals:
    void logMessage(const QString &msg);
    void progressUpdated(int current, int total);

private:
    QString vpkPath;
    QString outputPath;

    bool packDirectory(const QString &dirPath);
};

#endif // VPKGENERATOR_H
