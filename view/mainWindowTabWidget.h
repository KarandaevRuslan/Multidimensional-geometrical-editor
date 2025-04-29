#ifndef MAIN_WINDOW_TAB_WIDGET_H
#define MAIN_WINDOW_TAB_WIDGET_H

#include "sceneObjectEditorWidget.h"
#include "sceneRendererWidget.h"
#include "../presenterMain.h"
#include <QWidget>
#include <QListView>
#include <QUndoStack>
#include <memory>
#include "dataModels/sceneObjectModel.h"

/**
 * @brief The MainWindowTabWidget class for individual tab views.
 */
class MainWindowTabWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindowTabWidget(QWidget *parent = nullptr);
    ~MainWindowTabWidget();

    void setScene(std::shared_ptr<Scene> scene);
    void setSceneColorificator(std::shared_ptr<SceneColorificator> colorificator);

    // Setter for the shared delegate.
    void setDelegate(const std::shared_ptr<class SceneObjectDelegate>& delegate);

    // Setter for the sub-presenter.
    void setPresenterMainTab(const std::shared_ptr<PresenterMainTab>& presenter);

    QList<QAction *> editActions() const;
    int sceneObjectCount() const;
    CameraController *cameraController() const;
    SceneInputHandler *inputHandler() const;
private slots:
    void copySelected();
    void cutSelected();
    void pasteObject();
    void deleteSelected();
    void onListContextMenu(const QPoint &pos);
    void onCurrentRowChanged(const QModelIndex &current, const QModelIndex &previous);
    void exportSelectedObject();
    void importObject();
private:
    SceneRendererWidget* sceneRenderer_;
    QListView* listView_;
    SceneObjectEditorWidget*  editor_;
    std::unique_ptr<QUndoStack> undoStack_;

    std::shared_ptr<PresenterMainTab> presenterMainTab_;

    std::shared_ptr<Scene> scene_;
    std::shared_ptr<SceneColorificator> sceneColorificator_;

    std::shared_ptr<SceneObjectModel> model_;

    // Shared delegate injected from PresenterMain.
    std::shared_ptr<class SceneObjectDelegate> delegate_;

    QMap<QString, QAction*> actions_;

    void selectLastObject();
    void markDirty();
};

#endif // MAIN_WINDOW_TAB_WIDGET_H
