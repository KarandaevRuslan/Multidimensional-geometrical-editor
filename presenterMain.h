#ifndef PRESENTERMAIN_H
#define PRESENTERMAIN_H

#include "view/mainwindow.h"
#include "model/scene.h"
#include "model/sceneColorificator.h"

/**
 * @class PresenterMain
 * @brief Acts as the main coordinator between the UI, scene model, and scene colorification logic.
 *
 * This class serves as the central presenter in the application, connecting the user interface
 * (MainWindow) with the underlying data model (Scene) and additional visual logic (SceneColorificator).
 * It enables separation of concerns between the presentation layer and business logic.
 */
class PresenterMain {
public:
    /**
     * @brief Constructs the main presenter with references to the view and model components.
     *
     * @param mainWindow Shared pointer to the main window instance (UI).
     * @param scene Shared pointer to the scene model containing geometric data.
     * @param sceneColorificator Shared pointer to the scene colorificator for visual modifications.
     */
    PresenterMain(std::shared_ptr<MainWindow> mainWindow,
                  std::shared_ptr<Scene> scene,
                  std::shared_ptr<SceneColorificator> sceneColorificator);

private:
    /// Reference to the main UI window.
    std::shared_ptr<MainWindow> mainWindow_;

    /// Reference to the scene data model managing geometric entities.
    std::shared_ptr<Scene> scene_;

    /// Reference to the component responsible for applying color schemes to the scene.
    std::shared_ptr<SceneColorificator> sceneColorificator_;
};

#endif // PRESENTERMAIN_H
