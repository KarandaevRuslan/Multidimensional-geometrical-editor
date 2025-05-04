#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QListView>
#include <QMainWindow>
#include <QUndoStack>
#include <QTabWidget>
#include <QLabel>
#include "../forms/ui_mainWindow.h"

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

    void updateStatusBar();

private slots:
    void refreshMenu(int tabIndex);
    void showAboutDialog();

private:
    Ui::MainWindow* ui_;
    QTabWidget* tabWidget_;
    QLabel* hintLabel_;
    QLabel* coordsLabel_;

    // The main presenter is not owned by MainWindow.
    PresenterMain* presenterMain_;
protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // MAIN_WINDOW_H
