#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../forms/ui_mainWindow.h"
#include "../view/sceneRenderer.h"

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
     * @param sceneRenderer A pointer to a SceneRenderer widget.
     */
    MainWindow(QWidget* parent = nullptr,
               SceneRenderer* sceneRenderer = nullptr);

    /**
     * @brief Destructor for MainWindow.
     */
    ~MainWindow();

    std::shared_ptr<SceneRenderer> sceneRenderer_;
private:
    Ui::MainWindow* ui;
};

#endif // MAINWINDOW_H
