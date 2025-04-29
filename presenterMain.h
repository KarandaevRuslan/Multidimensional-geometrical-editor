#ifndef PRESENTER_MAIN_H
#define PRESENTER_MAIN_H

#include "view/mainwindow.h"
#include "model/scene.h"
#include "model/sceneColorificator.h"
#include <memory>
#include <vector>
#include <QDebug>
#include <QObject>

/**
 * @class PresenterMain
 * @brief Acts as the main coordinator between the UI, scene model, and scene colorification logic.
 *
 * Also owns the shared clipboard data and the shared delegate that is used across all tabs.
 */
class PresenterMain
{
public:
    /**
     * @brief Constructs the main presenter with a pointer to the main window.
     *
     * @param mainWindow A pointer to the main window instance (UI).
     */
    PresenterMain(MainWindow* mainWindow);
    ~PresenterMain();

    /// Called when a new tab is to be created.
    void createNewTab();

    // Shared clipboard variables across tabs.
    std::shared_ptr<SceneObject> copyBuffer_;
    std::shared_ptr<QColor> copyColor_;

    // Shared delegate for the list views in all tabs.
    std::shared_ptr<SceneObjectDelegate> sharedDelegate_;

    void removeTab(int index);

    void openSceneInNewTab();
    void saveCurrentTab(bool saveAs);

    bool hasDirtyTabs() const;

    MainWindow* getMainWindow() const { return mainWindow_; }
    std::shared_ptr<class PresenterMainTab> presenterFor(QWidget *w) const;
private:
    /// Reference to the main UI window.
    MainWindow* mainWindow_;

    /// Container for sub-presenters (one for each tab).
    std::vector<std::shared_ptr<class PresenterMainTab>> tabPresenters_;
    bool loadFromFile(const QString& fn,
                      std::shared_ptr<Scene> scene,
                      std::shared_ptr<SceneColorificator> sceneColorificator);
};


/**
 * @class PresenterMainTab
 * @brief Sub-presenter for managing the presentation logic for an individual tab.
 *
 * It owns a pointer to the tab widget and has access to the shared scene, scene colorificator, and the parent presenter.
 */
class PresenterMainTab
{
public:
    PresenterMainTab(class MainWindowTabWidget* tabWidget,
                     std::shared_ptr<Scene> scene,
                     std::shared_ptr<SceneColorificator> sceneColorificator,
                     PresenterMain* parent);
    ~PresenterMainTab();

    // Returns the parent presenter (which holds the shared clipboard and delegate).
    PresenterMain* getParentPresenter() const;

    QWidget* getTabWidget() const;

    bool isDirty() const;

    /* ---------- persistence ---------- */
    bool save(bool saveAs, QWidget* parentWindow);

    /* ---------- bookkeeping ---------- */
    void markDirty();
    void markSaved(QString filePath);

private:
    void updateLabel();

    class MainWindowTabWidget* tabWidget_;
    std::shared_ptr<Scene> scene_;
    std::shared_ptr<SceneColorificator> sceneColorificator_;
    PresenterMain* parent_;

    QString filePath_;
    bool isDirty_ = false;
    QString baseName_ = QObject::tr("Untitled");

    bool load(QWidget* parentWindow);
};

#endif // PRESENTER_MAIN_H
