#ifndef PROCESSWORKER_H
#define PROCESSWORKER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include "../processors/texturepreprocessor.h"
#include "../processors/vtfgenerator.h"
#include "../processors/vmtgenerator.h"
#include "../processors/vpkgenerator.h"
#include "../datastructures.h"
#include "../utils/fileutils.h"

class ProcessWorker : public QObject
{
    Q_OBJECT
public:
    explicit ProcessWorker(QObject *parent = nullptr);

    void setParams(bool skipPre, bool skipV, bool skipM, bool skipP,
                   const QString &sourcePath, const QString &outputPath,
                   const QString &matPath, const QString &vtfPath, const QString &vpkPath,
                   const QList<Component> &comps, const QList<Material> &mats,
                   const QList<ComponentVariants> &variants,
                   const Addoninfo &globalInfo);

public slots:
    void doWork();

signals:
    void logMessage(const QString &msg);
    void progressUpdated(int current, int total);
    void workFinished(bool success);

private:
    bool skipPreprocess;
    bool skipVtf;
    bool skipVmt;
    bool skipVpk;
    QString sourceBasePath;
    QString outputPath;
    QString matPath;
    QString vtfToolPath;
    QString vpkToolPath;
    QList<Component> components;
    QList<Material> materials;
    QList<ComponentVariants> projectVariants;
    Addoninfo globalAddonInfo;
};

#endif // PROCESSWORKER_H
