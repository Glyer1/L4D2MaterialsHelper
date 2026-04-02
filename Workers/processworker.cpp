#include "processworker.h"
#include <QDebug>

ProcessWorker::ProcessWorker(QObject *parent) : QObject(parent) {}

//设置参数
void ProcessWorker::setParams(bool skipPre, bool skipV, bool skipM, bool skipP,
                              const QString &sourcePath, const QString &outputPath,
                              const QString &matPath, const QString &vtfPath, const QString &vpkPath,
                              const QList<Component> &comps, const QList<Material> &mats,
                              const QList<ComponentVariants> &variants,
                              const Addoninfo &globalInfo)
{
    skipPreprocess = skipPre;
    skipVtf = skipV;
    skipVmt = skipM;
    skipVpk = skipP;
    sourceBasePath = sourcePath;
    this->outputPath = outputPath;
    this->matPath = matPath;
    vtfToolPath = vtfPath;
    vpkToolPath = vpkPath;
    components = comps;
    materials = mats;
    projectVariants = variants;
    globalAddonInfo = globalInfo;
}

//干活函数，就是on_btnExecute()
void ProcessWorker::doWork()
{
    QStringList tgaFiles, pngFiles;
    int totalSteps = 0;
    if (!skipPreprocess) totalSteps++;
    if (!skipVtf) totalSteps++;
    if (!skipVmt) totalSteps++;
    if (!skipVpk) totalSteps++;
    int currentStep = 0;

    // ========== 步骤1：纹理预处理 ==========
    if (!skipPreprocess) {
        emit logMessage("=== 执行纹理预处理 ===");

        if (sourceBasePath.isEmpty()) {
            emit logMessage("错误：源文件路径为空");
            emit workFinished(false);
            return;
        }

        if (materials.isEmpty()) {
            emit logMessage("错误：没有材质需要处理");
            emit workFinished(false);
            return;
        }

        TexturePreprocessor preprocessor;
        connect(&preprocessor, &TexturePreprocessor::logMessage, this, &ProcessWorker::logMessage);
        connect(&preprocessor, &TexturePreprocessor::progressUpdated, this, [this, totalSteps, currentStep](int current, int total) {
            if (totalSteps > 0 && total > 0) {
                int stepProgress = (current * 100) / total / totalSteps;
                int baseProgress = (currentStep * 100) / totalSteps;
                emit progressUpdated(baseProgress + stepProgress, 100);
            }
        });

        preprocessor.setSourcePath(sourceBasePath);
        preprocessor.setMaterials(materials);

        if (!preprocessor.process()) {
            emit logMessage("错误：纹理预处理失败");
            emit workFinished(false);
            return;
        }

        tgaFiles = preprocessor.getGeneratedTGA();
        pngFiles = preprocessor.getGeneratedPNG();

        emit logMessage(QString("纹理预处理完成，生成TGA: %1个, PNG: %2个")
                            .arg(tgaFiles.size()).arg(pngFiles.size()));

        currentStep++;
        if (totalSteps > 0) {
            emit progressUpdated((currentStep * 100) / totalSteps, 100);
        }
    }

    // ========== 步骤2：VTF生成 ==========
    if (!skipVtf) {
        emit logMessage("=== 执行VTF生成 ===");

        if (vtfToolPath.isEmpty() || !QFile::exists(vtfToolPath)) {
            emit logMessage("错误：请先设置正确的 VTF 转换器路径");
            emit workFinished(false);
            return;
        }

        VTFGenerator vtfGen;
        connect(&vtfGen, &VTFGenerator::logMessage, this, &ProcessWorker::logMessage);
        connect(&vtfGen, &VTFGenerator::progressUpdated, this, [this, totalSteps, currentStep](int current, int total) {
            if (totalSteps > 0 && total > 0) {
                int stepProgress = (current * 100) / total / totalSteps;
                int baseProgress = (currentStep * 100) / totalSteps;
                emit progressUpdated(baseProgress + stepProgress, 100);
            }
        });

        vtfGen.setMareTFPath(vtfToolPath);
        vtfGen.setTempDir(FileUtils::getTempDir());
        vtfGen.setMaterials(materials);
        vtfGen.setTgaFiles(tgaFiles);
        vtfGen.setPngFiles(pngFiles);

        if (!vtfGen.convertAll()) {
            emit logMessage("错误：VTF 生成失败");
            emit workFinished(false);
            return;
        }

        emit logMessage("VTF 生成完成");

        currentStep++;
        if (totalSteps > 0) {
            emit progressUpdated((currentStep * 100) / totalSteps, 100);
        }
    }

    // ========== 步骤3：VMT生成 ==========
    if (!skipVmt) {
        emit logMessage("=== 执行VMT生成 ===");

        // 检查变体材质是否存在
        QStringList invalidRefs;
        for (const ComponentVariants &cv : projectVariants) {
            const Component *comp = nullptr;
            for (const Component &c : components) {
                if (c.name == cv.componentName) {
                    comp = &c;
                    break;
                }
            }
            if (!comp) continue;

            for (const Variant &v : cv.variants) {
                for (const VariantMaterialConfig &vmc : v.materials) {
                    bool matExists = false;
                    for (const MaterialInComponent &mic : comp->materials) {
                        if (mic.vmtName == vmc.materialVmtName) {
                            matExists = true;
                            break;
                        }
                    }
                    if (!matExists) {
                        invalidRefs << QString("组件[%1] 变体[%2] 材质[%3]")
                                           .arg(cv.componentName).arg(v.variantName).arg(vmc.materialVmtName);
                    }
                }
            }
        }

        if (!invalidRefs.isEmpty()) {
            emit logMessage("警告：以下变体配置引用了不存在的材质：");
            for (const QString &ref : invalidRefs) {
                emit logMessage("  " + ref);
            }
            emit logMessage("这些变体将不会生成对应材质");
        }

        if (projectVariants.isEmpty()) {
            emit logMessage("错误：请先配置变体");
            emit workFinished(false);
            return;
        }

        if (outputPath.isEmpty()) {
            emit logMessage("错误：请设置输出路径");
            emit workFinished(false);
            return;
        }

        VMTGenerator vmtGen;
        connect(&vmtGen, &VMTGenerator::logMessage, this, &ProcessWorker::logMessage);
        connect(&vmtGen, &VMTGenerator::progressUpdated, this, [this, totalSteps, currentStep](int current, int total) {
            if (totalSteps > 0 && total > 0) {
                int stepProgress = (current * 100) / total / totalSteps;
                int baseProgress = (currentStep * 100) / totalSteps;
                emit progressUpdated(baseProgress + stepProgress, 100);
            }
        });

        vmtGen.setGlobalValues(globalAddonInfo.addonversion, globalAddonInfo.addonauthor,
                               globalAddonInfo.addontagline, globalAddonInfo.addonDescription);
        vmtGen.setOutputPath(outputPath);
        vmtGen.setMatPath(matPath);
        vmtGen.setTempDir(FileUtils::getTempDir());
        vmtGen.setMareTFPath(vtfToolPath);
        vmtGen.setSourceBasePath(sourceBasePath);
        vmtGen.setComponents(components);
        vmtGen.setMaterials(materials);
        vmtGen.setProjectVariants(projectVariants);

        if (!vmtGen.generateAll()) {
            emit logMessage("错误：VMT生成失败");
            emit workFinished(false);
            return;
        }

        currentStep++;
        if (totalSteps > 0) {
            emit progressUpdated((currentStep * 100) / totalSteps, 100);
        }
    }

    // ========== 步骤4：VPK打包 ==========
    if (!skipVpk) {
        emit logMessage("=== 执行VPK打包 ===");

        if (vpkToolPath.isEmpty() || !QFile::exists(vpkToolPath)) {
            emit logMessage("错误：请先设置正确的 VPK 工具路径");
            emit workFinished(false);
            return;
        }

        VPKGenerator vpkGen;
        connect(&vpkGen, &VPKGenerator::logMessage, this, &ProcessWorker::logMessage);
        connect(&vpkGen, &VPKGenerator::progressUpdated, this, [this, totalSteps, currentStep](int current, int total) {
            if (totalSteps > 0 && total > 0) {
                int stepProgress = (current * 100) / total / totalSteps;
                int baseProgress = (currentStep * 100) / totalSteps;
                emit progressUpdated(baseProgress + stepProgress, 100);
            }
        });

        vpkGen.setVpkPath(vpkToolPath);
        vpkGen.setOutputPath(outputPath);

        if (!vpkGen.packAll()) {
            emit logMessage("错误：VPK打包失败");
            emit workFinished(false);
            return;
        }

        currentStep++;
        if (totalSteps > 0) {
            emit progressUpdated((currentStep * 100) / totalSteps, 100);
        }
    }

    emit logMessage("执行完成！");
    emit progressUpdated(100, 100);
    emit workFinished(true);
}
