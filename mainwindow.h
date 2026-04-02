#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "datastructures.h"
#include <QMainWindow>
#include <QList>
#include <QUuid>
#include <QThread>
#include "Workers/processworker.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    //显示组件Info设置对话框
    bool showComponentInfoDialog(Addoninfo &info, const QStringList &componentNames, const QString &currentComponent);

private:
    Ui::MainWindow *ui;
    QList<Component> components;
    QList<Material> materials;//已有材质
    Addoninfo globalAddonInfo;  // 存储全局的info设置
    QList<ComponentVariants> projectVariants;//项目所有变体
    QString lastLogFilePath;  // 记录最新生成的日志文件路径
    void saveAllSettings();

    //工作流控制变量
    bool skipPreprocess;
    bool skipVtf;
    bool skipVmt;
    bool skipVpk;

private:
    QThread *workerThread=nullptr;
    ProcessWorker *worker=nullptr;

private:
    QMetaObject::Connection m_workerFinishedConn;
    QMetaObject::Connection m_threadFinishedConn;
    QMetaObject::Connection m_deleteWorkerConn;
    QMetaObject::Connection m_deleteThreadConn;
private slots:
    void on_btnSource_clicked();
    void on_btnOutput_clicked();
    void saveLogToFile();
    void on_btnVtf_clicked();
    void on_btnVpk_clicked();
    void on_btnNewComponent_clicked();
    void on_btnNewMaterial_clicked();
    void updateTreeWidget();  // 添加更新函数
    void on_btnDelComponent_clicked();
    void on_btnEditComponent_clicked();
    void onMaterialSettingClicked();       // 材质设置按钮点击
    void updateMaterialTable();            // 刷新材质表格
    void on_btnDelMaterial_clicked();
    void on_btnInfo_clicked();  // 新增：组件信息设置按钮

    void on_btnExecute_clicked();        // 执行按钮
    void on_btnEnd_clicked();            // 结束按钮
    void on_btnOpenRiZhi_clicked();      // 打开日志按钮
    void on_btnVariant_clicked();        //变体编辑

    //复选框槽函数
    void on_cbSkipPreprocess_toggled(bool checked);
    void on_cbSkipVtf_toggled(bool checked);
    void on_cbSkipVmt_toggled(bool checked);
    void on_cbSkipVpk_toggled(bool checked);
    void on_btnSaveTestCase_clicked();  // 保存测试用例
    void on_btnLoadTestCase_clicked();  // 加载测试用例



    void on_btnSourceOpen_clicked();

    void on_btnOutputOpen_clicked();

    void on_btnVtfOpen_clicked();

    void on_btnVpkOpen_clicked();

private slots:
    void onWorkFinished(bool success);
    void onWorkerLog(const QString &msg);
    void onWorkerProgress(int current, int total);

protected:
    void closeEvent(QCloseEvent *event) override;
};



#endif // MAINWINDOW_H
