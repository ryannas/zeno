#ifndef __ZENO_MAINWINDOW_H__
#define __ZENO_MAINWINDOW_H__

#include <unordered_set>
#include <QtWidgets>
#include "dock/ztabdockwidget.h"
#include "panel/zenolights.h"
#include "common.h"
#include "layout/winlayoutrw.h"


class DisplayWidget;
class ZTimeline;

namespace Ui
{
    class MainWindow;
}

class ZenoMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    ZenoMainWindow(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    ~ZenoMainWindow();
    bool inDlgEventLoop() const;
    void setInDlgEventLoop(bool bOn);
    TIMELINE_INFO timelineInfo();
    void setTimelineInfo(TIMELINE_INFO info);
    ZTimeline* timeline() const;
    DisplayWidget *getDisplayWidget();
    void dispatchCommand(QAction* pAction, bool bTriggered);

    QLineEdit* selected = nullptr;
    ZenoLights* lightPanel = nullptr;
    float mouseSen = 0.2;

public slots:
    void openFileDialog();
    void onNewFile();
    bool openFile(QString filePath);
    bool saveFile(QString filePath);
    void saveQuit();
    void save();
    void saveAs();
    void onMaximumTriggered();
    void onMenuActionTriggered(bool bTriggered);
    void onSplitDock(bool);
    void onCloseDock();
    void onToggleDockWidget(DOCK_TYPE, bool);
    void onDockSwitched(DOCK_TYPE);
    void importGraph();
    void exportGraph();
    void onNodesSelected(const QModelIndex& subgIdx, const QModelIndexList& nodes, bool select);
    void onPrimitiveSelected(const std::unordered_set<std::string>& primids);
    void updateViewport(const QString& action = "");
    void onRunFinished();
    void onFeedBack();
    void clearErrorMark();
    void updateLightList();
    void saveDockLayout();
    void loadSavedLayout();
    void onLangChanged(bool bChecked);

protected:
    void resizeEvent(QResizeEvent* event) override;
    bool event(QEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    void init();
    void initMenu();
    void initDocks();
    void initDocksWidget(ZTabDockWidget* pCake, PtrLayoutNode root);
    void _resizeDocks(PtrLayoutNode root);
    void resetDocks(PtrLayoutNode root);
    void initTimelineDock();
    void recordRecentFile(const QString& filePath);
    void saveLayout2();
    void SplitDockWidget(ZTabDockWidget* after, ZTabDockWidget* dockwidget, Qt::Orientation orientation);
    QString getOpenFileByDialog();
    QString uniqueDockObjName(DOCK_TYPE type);

    ZTimeline* m_pTimeline;
    PtrLayoutNode m_layoutRoot;
    bool m_bInDlgEventloop;
    int m_nResizeTimes;
    Ui::MainWindow* m_ui;
};

#endif
