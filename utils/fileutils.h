#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <QString>
#include <QDir>
#include <QCoreApplication>
#include <QSettings>
#include <QDebug>

class FileUtils
{
public:
    // 获取程序所在目录（exe所在目录）
    static QString getAppDir() {
        return QCoreApplication::applicationDirPath();
    }

    // 获取临时目录（exe所在目录下的Resources/Temp文件夹）
    static QString getTempDir() {
        QString tempDir = getAppDir() + "/Resources/Temp";
        ensureDirExists(tempDir);
        return tempDir;
    }

    // 获取输出目录（exe所在目录下的/Resources/Output文件夹）
    static QString getOutputDir() {
        QString outputDir = getAppDir() + "/Resources/Output";
        ensureDirExists(outputDir);
        return outputDir;
    }

    // 确保目录存在
    static bool ensureDirExists(const QString &path) {
        QDir dir;
        if (!dir.exists(path)) {
            if (!dir.mkpath(path)) {
                qDebug() << "创建目录失败:" << path;
                return false;
            }
        }
        return true;
    }

    // 清理临时目录,只清理多帧
    static bool cleanTempDir() {
        QString tempDir = getTempDir();
        QDir dir(tempDir);
        if (dir.exists()) {
            return dir.removeRecursively();
        }
        return true;
    }

    // 从QSettings获取路径（带默认值）
    static QString getPathFromSettings(const QString &key, const QString &defaultValue) {
        QSettings settings("Local", "Path");
        return settings.value(key, defaultValue).toString();
    }

    // 保存路径到QSettings
    static void savePathToSettings(const QString &key, const QString &value) {
        QSettings settings("Local", "Path");
        settings.setValue(key, value);
    }
};

#endif // FILEUTILS_H
