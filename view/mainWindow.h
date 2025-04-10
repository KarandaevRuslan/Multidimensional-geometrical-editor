#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QListView>
#include <QMainWindow>
#include <QUndoStack>
#include <QTabWidget>
#include "../forms/ui_mainWindow.h"
#include "../view/sceneRenderer.h"
#include "dataModels/sceneObjectModel.h"
#include "delegates/sceneObjectDelegate.h"

class PresenterMain;

/**
 * @brief The MainWindow class for the Geometric Editor.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a MainWindow.
     * @param parent The parent widget.
     */
    MainWindow(QWidget* parent = nullptr);

    /**
     * @brief Destructor for MainWindow.
     */
    ~MainWindow();

    void setPresenterMain(PresenterMain *presenterMain);

    // Expose the QTabWidget so that PresenterMain can add new tabs.
    QTabWidget* getTabWidget() const { return tabWidget_; }

private slots:
    void addNewSceneTab();

private:
    Ui::MainWindow* ui_;
    QTabWidget* tabWidget_;

    // The main presenter is not owned by MainWindow.
    PresenterMain* presenterMain_;
};

#endif // MAIN_WINDOW_H
