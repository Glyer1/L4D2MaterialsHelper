#ifndef VARIANTCONFIGDIALOG_H
#define VARIANTCONFIGDIALOG_H

#include <QDialog>
#include <QTreeWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "datastructures.h"

class VariantConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit VariantConfigDialog(const QList<Component> &components,
                                 const QList<ComponentVariants> &existingVariants,
                                 QWidget *parent = nullptr);

    // 获取配置结果
    QList<ComponentVariants> getVariants() const { return componentVariants; }

private slots:
    void onAddVariant();           // 添加变体
    void onRemoveVariant();        // 删除变体
    void onVariantRenamed(QTreeWidgetItem *item, int column);  // 变体重命名
    void onDiffChanged(int index); // 差分选择变化
    void onAutoGenerate();         // 自动生成所有组合
    void onItemClicked(QTreeWidgetItem *item, int column); // 点击项

private:
    void setupUI();
    void loadData();
    void refreshVariantList();
    void updateMaterialCombos(QTreeWidgetItem *variantItem, bool forceRefresh);
    void generateAllCombinations(const Component &comp, int materialIndex,
                                 Variant &currentVariant,
                                 QList<Variant> &results);

    QTreeWidget *treeWidget;
    QPushButton *btnAddVariant;
    QPushButton *btnRemoveVariant;
    QPushButton *btnAutoGenerate;
    QPushButton *btnOk;
    QPushButton *btnCancel;

    QList<Component> components;
    QList<ComponentVariants> componentVariants;
    QTreeWidgetItem *currentVariantItem;
};

#endif // VARIANTCONFIGDIALOG_H
