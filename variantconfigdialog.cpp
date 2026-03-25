#include "variantconfigdialog.h"
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>

VariantConfigDialog::VariantConfigDialog(const QList<Component> &comps,
                                         const QList<ComponentVariants> &existingVariants,
                                         QWidget *parent)
    : QDialog(parent)
    , components(comps)
    , componentVariants(existingVariants)
    , currentVariantItem(nullptr)
{
    setupUI();
    loadData();
}

void VariantConfigDialog::setupUI()
{
    setWindowTitle("变体配置");
    setMinimumSize(800, 600);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 树形控件
    treeWidget = new QTreeWidget(this);
    treeWidget->setHeaderLabels({"组件/变体/材质", "差分选择"});
    treeWidget->setColumnWidth(0, 300);
    treeWidget->setEditTriggers(QTreeWidget::DoubleClicked); // 允许编辑
    connect(treeWidget, &QTreeWidget::itemClicked, this, &VariantConfigDialog::onItemClicked);
    connect(treeWidget, &QTreeWidget::itemChanged, this, &VariantConfigDialog::onVariantRenamed);

    mainLayout->addWidget(treeWidget);

    // 按钮区域
    QHBoxLayout *btnLayout = new QHBoxLayout();

    btnAddVariant = new QPushButton("新建变体", this);
    btnRemoveVariant = new QPushButton("删除变体", this);
    btnAutoGenerate = new QPushButton("自动生成所有组合", this);

    connect(btnAddVariant, &QPushButton::clicked, this, &VariantConfigDialog::onAddVariant);
    connect(btnRemoveVariant, &QPushButton::clicked, this, &VariantConfigDialog::onRemoveVariant);
    connect(btnAutoGenerate, &QPushButton::clicked, this, &VariantConfigDialog::onAutoGenerate);

    btnLayout->addWidget(btnAddVariant);
    btnLayout->addWidget(btnRemoveVariant);
    btnLayout->addWidget(btnAutoGenerate);
    btnLayout->addStretch();

    mainLayout->addLayout(btnLayout);

    // 确定取消按钮
    QHBoxLayout *dialogBtnLayout = new QHBoxLayout();
    btnOk = new QPushButton("确定", this);
    btnCancel = new QPushButton("取消", this);

    connect(btnOk, &QPushButton::clicked, this, &QDialog::accept);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);

    dialogBtnLayout->addStretch();
    dialogBtnLayout->addWidget(btnOk);
    dialogBtnLayout->addWidget(btnCancel);

    mainLayout->addLayout(dialogBtnLayout);
}

//把变体配置数据加载到树形控件
#if 0
四层循环每次遍历的单个变量对应：
    第一层：变体面板里的组件（内包含很多变体）
        第二层：变体面板里的组件里的某一个变体

                第三层：变体面板里的组件对应的组件列表的组件，然后它有材质列表，这个材质列表的某一项（最新的，如果相比之前有变动会处理第四层的材质，就是因为这个和第四层联动）
                    第四层：变体面板里的组件的某一个变体的某一个材质（下拉框的）

                            第一第二层的作用就是对每一个变体面板里的每一个组件进行处理
#endif
//重新加载面板数据，同步信息
void VariantConfigDialog::loadData()
{
    treeWidget->blockSignals(true);
    // ===== 同步组件和材质变化 =====
    // 1. 删除已经不存在的组件配置
    for (int i = componentVariants.size() - 1; i >= 0; --i) {
        bool found = false;
        for (const Component &comp : components) {
            if (comp.name == componentVariants[i].componentName) {
                found = true;
                break;
            }
        }
        if (!found) {
            componentVariants.removeAt(i);  // 组件已被删除
        }
    }

    // 2. 为每个现有组件的变体同步材质变化
    for (ComponentVariants &cv : componentVariants) {
        // 找到对应的组件
        const Component *comp = nullptr;
        for (const Component &c : components) {
            if (c.name == cv.componentName) {
                comp = &c;
                break;
            }
        }
        if (!comp) continue;

        // 为每个变体同步材质
        for (Variant &var : cv.variants) {
            QList<VariantMaterialConfig> newMaterials;

            // 按照组件当前的材质顺序重新构建
            qDebug() << "=== loadData ===";

            for (const MaterialInComponent &mat : comp->materials) {
                qDebug() << "mat.name:" << mat.name << "mat.vmtName:" << mat.vmtName << "mat.hasDiff:" << mat.hasDiff;
                bool found = false;

                for (const VariantMaterialConfig &oldMat : var.materials) {
                    if (oldMat.materialVmtName == mat.vmtName) {
                        // 材质还在，保留原配置（包括选中的差分）
                        // 但需要检查选中的差分是否还在
                        if (mat.hasDiff && mat.diffNames.contains(oldMat.selectedDiff)) {
                            // 差分还在，保留原选中
                            newMaterials.append(oldMat);
                        } else if (mat.hasDiff && !mat.diffNames.isEmpty()) {
                            // 原选中差分已不存在，选第一个
                            VariantMaterialConfig updatedMat = oldMat;
                            updatedMat.selectedDiff = mat.diffNames.first();
                            newMaterials.append(updatedMat);
                        } else {
                            // 无差分的材质，用材质名
                            VariantMaterialConfig updatedMat = oldMat;
                            updatedMat.selectedDiff = "";
                            newMaterials.append(updatedMat);
                        }
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    // 新增的材质，创建默认配置
                    VariantMaterialConfig newMat;
                    newMat.materialVmtName = mat.vmtName;
                    newMat.selectedDiff = mat.hasDiff ? mat.diffNames.first() : "";
                    newMat.baseTexture = mat.name;
                    newMaterials.append(newMat);
                }
            }

            // 替换为新的材质列表（自动删除了已经不存在的材质）
            var.materials = newMaterials;
        }
    }

    // 如果 componentVariants 为空，为每个组件创建一个默认配置
    if (componentVariants.isEmpty()) {
        //对每一个组件哦
        for (const Component &comp : components) {
            ComponentVariants cv;
            cv.componentName = comp.name;

            // 创建一个默认变体
            Variant defaultVar;
            defaultVar.variantName = comp.name + "默认";

            // 为每个组件的各个材质创建默认配置
            for (const MaterialInComponent &mat : comp.materials) {
                VariantMaterialConfig matConfig;
                matConfig.materialVmtName = mat.vmtName;

                matConfig.selectedDiff = mat.hasDiff ? mat.diffNames.first() : "";
                matConfig.baseTexture = mat.name;
                defaultVar.materials.append(matConfig);
            }

            cv.variants.append(defaultVar);
            componentVariants.append(cv);
        }
    }

    treeWidget->clear();

    for (const ComponentVariants &cv : componentVariants) {
        // 第一层：组件
        QTreeWidgetItem *compItem = new QTreeWidgetItem(treeWidget);
        //这里设置了变体名字
        compItem->setText(0, cv.componentName);
        compItem->setFlags(compItem->flags() & ~Qt::ItemIsEditable);

        for (const Variant &var : cv.variants) {
            // 第二层：变体
            QTreeWidgetItem *varItem = new QTreeWidgetItem(compItem);
            varItem->setText(0, var.variantName);
            varItem->setFlags(varItem->flags() | Qt::ItemIsEditable);
            varItem->setData(0, Qt::UserRole, "variant");

            //新增存住variantName
            varItem->setData(0, Qt::UserRole + 1, var.variantName);
            qDebug() << "存变体名:" << var.variantName;  // 看输出

            for (const VariantMaterialConfig &mat : var.materials) {
                // 第三层：材质（单独一个，第一个setdata告诉他我是材质，第二个告诉他我的z材质名）
                QTreeWidgetItem *matItem = new QTreeWidgetItem(varItem);
                matItem->setText(0, mat.baseTexture + " (" + mat.materialVmtName + ")");
                matItem->setFlags(matItem->flags() & ~Qt::ItemIsEditable);
                matItem->setData(0, Qt::UserRole, "material");
                matItem->setData(0, Qt::UserRole + 1, mat.materialVmtName);
            }
        }
    }

    treeWidget->expandAll();

    //为所有变体创建下拉框
    for (int i = 0; i < treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem *compItem = treeWidget->topLevelItem(i);//获取顶层结点
        for (int j = 0; j < compItem->childCount(); ++j) {
            QTreeWidgetItem *varItem = compItem->child(j);//获取子节点
            updateMaterialCombos(varItem,true);//这里才第三层
        }
    }

    treeWidget->blockSignals(false);
}

//刷新变体列表
//增删改变体后，重建树形控件，并让选中状态不丢失。
void VariantConfigDialog::refreshVariantList()
{
    // 保存当前选中的变体信息
    QString selectedComp;
    QString selectedVar;
    QTreeWidgetItem *currentItem = treeWidget->currentItem();

    if (currentItem) {
        //找第二层-变体
        QTreeWidgetItem *varItem = currentItem;
        while (varItem && varItem->parent() && varItem->parent()->parent()) {
            varItem = varItem->parent();
        }
        if (varItem && varItem->parent()) {
            //获取组件第一列的文本
            selectedComp = varItem->parent()->text(0);
            selectedVar = varItem->text(0);
        }
    }

    // 重新加载数据
    loadData();

    // 恢复之前选中的变体
    if (!selectedComp.isEmpty() && !selectedVar.isEmpty()) {
        //每一个组件
        for (int i = 0; i < treeWidget->topLevelItemCount(); ++i) {
            QTreeWidgetItem *compItem = treeWidget->topLevelItem(i);
            //找到选择的组件
            if (compItem->text(0) == selectedComp) {
                //对组件下每一个变体进行遍历
                for (int j = 0; j < compItem->childCount(); ++j) {
                    QTreeWidgetItem *varItem = compItem->child(j);
                    //找到变体
                    if (varItem->text(0) == selectedVar) {
                        treeWidget->setCurrentItem(varItem);
                        // 确保选中后更新下拉框显示（虽然已经都有了，但保持选中状态）
                        updateMaterialCombos(varItem,true);
                        break;
                    }
                }
                break;
            }
        }
    }
}

//增加变体
void VariantConfigDialog::onAddVariant()
{
    QTreeWidgetItem *current = treeWidget->currentItem();
    if (!current) {
        QMessageBox::warning(this, "提示", "请先选中一个组件");
        return;
    }

    // 找到组件项
    while (current->parent() != nullptr) {
        current = current->parent();
    }

    QString compName = current->text(0);

    // 找到对应的组件变体配置
    for (ComponentVariants &cv : componentVariants) {
        if (cv.componentName == compName) {
            // 创建新变体
            Variant newVar;
            newVar.variantName = compName + "新变体" + QString::number(cv.variants.size() + 1);

            // 找到对应的组件，为每个材质创建默认配置
            for (const Component &comp : components) {
                if (comp.name == compName) {
                    for (const MaterialInComponent &mat : comp.materials) {
                        VariantMaterialConfig matConfig;
                        matConfig.materialVmtName = mat.vmtName;
                        matConfig.selectedDiff = mat.hasDiff ? mat.diffNames.first() :"";
                        qDebug() << "新建变体 - mat.vmtName:" << mat.vmtName << "selectedDiff:" << matConfig.selectedDiff;
                        matConfig.baseTexture = mat.name;//暂时先传材质中文名（皮肤skin，中文名就是皮肤）
                        newVar.materials.append(matConfig);
                    }
                    break;
                }
            }

            cv.variants.append(newVar);
            break;
        }
    }

    refreshVariantList();

    // 自动选中新创建的变体并显示下拉框
    //找更新的对应顶层组件
    QTreeWidgetItem *newCompItem = nullptr;
    for (int i = 0; i < treeWidget->topLevelItemCount(); ++i) {
        if (treeWidget->topLevelItem(i)->text(0) == compName) {
            newCompItem = treeWidget->topLevelItem(i);
            break;
        }
    }

    //顶层变体
    if (newCompItem && newCompItem->childCount() > 0) {
        //找最后一个，最后一个是新增加的变体
        QTreeWidgetItem *newVarItem = newCompItem->child(newCompItem->childCount() - 1);
        treeWidget->setCurrentItem(newVarItem);
        //立即显示下拉框
        updateMaterialCombos(newVarItem,false);
    }
}

//删除变体
void VariantConfigDialog::onRemoveVariant()
{
    QTreeWidgetItem *current = treeWidget->currentItem();
    if (!current || current->parent() == nullptr) {
        QMessageBox::warning(this, "提示", "请选中一个变体");
        return;
    }

    // 确保选中的是变体（第二层）
    if (current->parent()->parent() != nullptr) {
        QMessageBox::warning(this, "提示", "请选中一个变体");
        return;
    }

    QString compName = current->parent()->text(0);
    QString varName = current->text(0);

    // 确认删除
    if (QMessageBox::question(this, "确认删除",
                              QString("确定要删除变体 \"%1\" 吗？").arg(varName)) != QMessageBox::Yes) {
        return;
    }

    // 从数据中删除
    //从总变体表中找最外层组件
    for (ComponentVariants &cv : componentVariants) {
        if (cv.componentName == compName) {
            //找这个这个外层组件里头的变体
            for (int i = 0; i < cv.variants.size(); ++i) {
                if (cv.variants[i].variantName == varName) {
                    cv.variants.removeAt(i);
                    break;
                }
            }
            break;
        }
    }

    refreshVariantList();
}

//变体重命名
void VariantConfigDialog::onVariantRenamed(QTreeWidgetItem *item, int column)
{
    qDebug() << "onVariantRenameddebug1 进入, item:" << item->text(0) << "type:" << item->data(0, Qt::UserRole).toString();

    qDebug() << "onVariantRenameddebug2 被调用, item:" << item->text(0) << "column:" << column;

    qDebug() << "onVariantRenameddebug3 called, item text:" << item->text(0)
             << "type:" << item->data(0, Qt::UserRole).toString();

    if (!item || column != 0) return;

    // 检查是否是变体项
    if (item->data(0, Qt::UserRole).toString() != "variant") return;

    QString newName = item->text(0);
    if (newName.isEmpty()) {
        // 如果为空，恢复原名称（通过刷新）,直接从存所有变体的地方获取。
        refreshVariantList();
        return;
    }

    QString compName = item->parent()->text(0);

    //关键：获取旧的变体名
    // 从 item 的 data 中存储旧的变体名
    QString oldName = item->data(0, Qt::UserRole + 1).toString();

    //还没有存储旧名字直接警告，一般不会发生
    if (oldName.isEmpty()) {
#if 0
        oldName=item->text(0);
#endif

#if 1
        QMessageBox::information(this,"警告","没有存储旧名字");
        //refreshVariantList();
        return ;
#endif
    }

    // 更新数据
    //用oldname找到对应组件去改名
    for (ComponentVariants &cv : componentVariants) {
        if (cv.componentName == compName) {
            //找和oldname对应变体，准备放名字
            for (Variant &var : cv.variants) {
                if (var.variantName == oldName) {  // 用 oldName 查找
                    // 检查是否与其他变体重名
                    bool nameExists = false;
                    for (const Variant &v : cv.variants) {
                        if (v.variantName == newName && &v != &var) {
                            nameExists = true;
                            break;
                        }
                    }

                    //没重复就修改
                    if (!nameExists) {
                        var.variantName = newName;
                        // 存储新名字到 data 中，方便下次编辑
                        item->setData(0, Qt::UserRole + 1, newName);
                    } else {
                        QMessageBox::warning(this, "提示", "变体名称已存在");
                        // 恢复原显示
                        item->setText(0, oldName);
                    }
                    break;
                }
            }
            break;
        }
    }
}

//差分改变，换选项触发，updatematerialscombo绑定
void VariantConfigDialog::onDiffChanged(int index)
{
    //获取发信控件
    QComboBox *combo = qobject_cast<QComboBox*>(sender());
    if (!combo) return;

    // 获取实际存储的值
    QString diffName;
    //看当前下拉框选项
    if (combo->itemData(index).isValid()) {
        diffName = combo->itemData(index).toString();  // 是无差分，这里就是空字符串 addItem("无差分") + setItemData(0, "")
    } else {
        diffName = combo->currentText();  // 正常差分，用显示的文本
        if (diffName == "无差分") diffName = "";  // 保险起见，防止无差分没设置
    }

    QString materialVmt = combo->property("materialVmt").toString();
    QString variantName = combo->property("variantName").toString();
    QString compName = combo->property("componentName").toString();
\
    // 找到对应的变体并更新
    for (ComponentVariants &cv : componentVariants) {
        if (cv.componentName == compName) {
            for (Variant &var : cv.variants) {
                if (var.variantName == variantName) {
                    for (VariantMaterialConfig &mat : var.materials) {
                        if (mat.materialVmtName == materialVmt) {
                            //修改选择的差分
                            mat.selectedDiff = diffName;
                            break;
                        }
                    }
                    break;
                }
            }
            break;
        }
    }
}

//变体点击
void VariantConfigDialog::onItemClicked(QTreeWidgetItem *item, int column)
{
    if (!item || column != 0) return;

    // 如果是变体项，显示该变体的材质差分选择
    if (item->data(0, Qt::UserRole).toString() == "variant") {
        currentVariantItem = item;
        updateMaterialCombos(item,false);
    }
}

//updateMaterialCombos只负责改下拉框，数据早存好了
void VariantConfigDialog::updateMaterialCombos(QTreeWidgetItem *variantItem, bool forceRefresh)
{
    disconnect(treeWidget, &QTreeWidget::itemChanged, this, &VariantConfigDialog::onVariantRenamed);

    if (!variantItem) return;

    QString compName = variantItem->parent()->text(0);
    QString varName = variantItem->text(0);

    // 找到当前变体的配置
    Variant *currentVariant = nullptr;
    for (ComponentVariants &cv : componentVariants) {
        if (cv.componentName == compName) {
            for (Variant &var : cv.variants) {
                if (var.variantName == varName) {
                    currentVariant = &var;
                    break;
                }
            }
            break;
        }
    }//找到或没找到currentVariant

    if (!currentVariant) return;

    // 为每个材质创建或更新下拉框，子节点在loaddata创好了，固定的
    for (int i = 0; i < variantItem->childCount(); ++i) {
        QTreeWidgetItem *matItem = variantItem->child(i);//第一次为nullptr
        QString materialVmt = matItem->data(0, Qt::UserRole + 1).toString();

        // 获取或创建下拉框，itemWidget(matItem, 1)); 树形组件第matitem(3)层，然后第2列
        QComboBox *combo = qobject_cast<QComboBox*>(treeWidget->itemWidget(matItem, 1));

        if (!combo) {
            // 第一次创建
            combo = new QComboBox(this);
            treeWidget->setItemWidget(matItem, 1, combo);

            //创建不可见属性，但是可读，对应上组件，vmt名字，材质名，不是具体差分
            combo->setProperty("materialVmt", materialVmt);
            combo->setProperty("variantName", varName);
            combo->setProperty("componentName", compName);

            // 填充选项，因为直接用component更优，不用看变体因为组件-材质有多少个，固定对应（除非更改），不需要浏览变体
            for (const Component &comp : components) {
                if (comp.name == compName) {
                    for (const MaterialInComponent &mat : comp.materials) {
                        //找到循环变量的对应材质
                        if (mat.vmtName == materialVmt) {
                            if (mat.hasDiff) {
                                combo->addItems(mat.diffNames);
                            } else {
                                combo->addItem("无差分");
                                // 存储实际值（空字符串）在 item data 中
                                combo->setItemData(0, "");
                            }
                            break;
                        }
                    }
                    break;
                }
            }

            //连接差分改变函数，下拉换选项就会触发，让diffchanged存好数据
            connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                    this, &VariantConfigDialog::onDiffChanged);
        }

        // 遍历变体中的材质，找到当前的材质，只更新选中值，不清空重建
        for (const VariantMaterialConfig &matConfig : currentVariant->materials) {
            if (matConfig.materialVmtName == materialVmt) {
                //ondiffschanged已经改好了
                int idx = -1;
                if(matConfig.selectedDiff.isEmpty())
                    idx = combo->findText("无差分");
                else
                    idx = combo->findText(matConfig.selectedDiff);
                if (idx >= 0) {
                    combo->setCurrentIndex(idx);//下拉框设置
                }
                break;
            }
        }
    }

    connect(treeWidget, &QTreeWidget::itemChanged, this, &VariantConfigDialog::onVariantRenamed);
}

//递归生成所有组合
void VariantConfigDialog::generateAllCombinations(const Component &comp, int materialIndex,
                                                  Variant &currentVariant,
                                                  QList<Variant> &results)
{
    //材质个数刚好相等的时候就成了一个变体
    if (materialIndex >= comp.materials.size()) {
        // 生成变体名
        QStringList parts;
        //这里不是单纯材质列表，每个mat都存了对应的材质差分(skin差分skin_sora)信息
        for (const VariantMaterialConfig &mat : currentVariant.materials) {
            if (!mat.selectedDiff.isEmpty()) {
                parts << mat.selectedDiff;
            }
        }

        //将这些不同材质对应差分进行join用_隔开
        currentVariant.variantName = comp.name + parts.join("_");
        results.append(currentVariant);
        return;
    }

    const MaterialInComponent &mat = comp.materials[materialIndex];

    if (!mat.hasDiff || mat.diffNames.isEmpty()) {
        // 无差分的材质，只有一个选择
        VariantMaterialConfig config;
        config.materialVmtName = mat.vmtName;
        config.selectedDiff = "";
        config.baseTexture = mat.name;
        currentVariant.materials.append(config);

        generateAllCombinations(comp, materialIndex + 1, currentVariant, results);

        currentVariant.materials.removeLast();
    } else {
        // 有差分的材质，每个差分一个分支，因为是循环，前面定死了，所后面以会轮着生后面一层，每一层都这样
        for (const QString &diff : mat.diffNames) {
            VariantMaterialConfig config;
            config.materialVmtName = mat.vmtName;
            config.selectedDiff = diff;
            config.baseTexture = mat.name;
            currentVariant.materials.append(config);

            generateAllCombinations(comp, materialIndex + 1, currentVariant, results);

            currentVariant.materials.removeLast();
        }
    }
}

//自动生成变体
void VariantConfigDialog::onAutoGenerate()
{
    // 获取当前选中的组件
    QTreeWidgetItem *current = treeWidget->currentItem();
    if (!current) {
        QMessageBox::warning(this, "提示", "请先选中一个组件");
        return;
    }

    // 找到组件项
    while (current->parent() != nullptr) {
        current = current->parent();
    }

    QString compName = current->text(0);

    // 找到对应的组件
    Component targetComp;
    for (const Component &comp : components) {
        if (comp.name == compName) {
            targetComp = comp;
            break;
        }
    }

    if (targetComp.name.isEmpty()) return;

    // 生成所有组合
    QList<Variant> newVariants;
    Variant currentVariant;
    generateAllCombinations(targetComp, 0, currentVariant, newVariants);

    // 添加到数据中
    for (ComponentVariants &cv : componentVariants) {
        if (cv.componentName == compName) {
            cv.variants.append(newVariants);
            break;
        }
    }

    refreshVariantList();

    QMessageBox::information(this, "完成",
                             QString("已生成 %1 个变体").arg(newVariants.size()));
}
