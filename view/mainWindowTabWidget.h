#ifndef MAIN_WINDOW_TAB_WIDGET_H
#define MAIN_WINDOW_TAB_WIDGET_H

#include "sceneRenderer.h"
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

    std::weak_ptr<SceneRenderer> sceneRenderer() const;

    void setScene(std::shared_ptr<Scene> scene);
    void setSceneColorificator(std::shared_ptr<SceneColorificator> colorificator);

    // Setter for the shared delegate.
    void setDelegate(const std::shared_ptr<class SceneObjectDelegate>& delegate);

    // Setter for the sub-presenter.
    void setPresenterMainTab(const std::shared_ptr<PresenterMainTab>& presenter);

private slots:
    void copySelected();
    void pasteObject();
    void deleteSelected();
    void onListContextMenu(const QPoint &pos);

private:
    SceneRenderer* sceneRenderer_;
    QListView* listView_;
    std::unique_ptr<QUndoStack> undoStack_;

    // Shared sub-presenter for this tab.
    std::shared_ptr<PresenterMainTab> presenterMainTab_;

    std::shared_ptr<Scene> scene_;
    std::shared_ptr<SceneColorificator> sceneColorificator_;

    std::shared_ptr<SceneObjectModel> model_;

    // Shared delegate injected from PresenterMain.
    std::shared_ptr<class SceneObjectDelegate> delegate_;
};

#endif // MAIN_WINDOW_TAB_WIDGET_H
