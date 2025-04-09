#include "mainWindow.h"
#include "mainWindowTabWidget.h"
#include "../view/sceneRenderer.h"
#include "commands/addSceneObjectCommand.h"
#include "commands/removeSceneObjectCommand.h"
#include <QFile>
#include <QUiLoader>
#include <QDebug>
#include <QLayout>
#include <QLabel>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
    ui_(new Ui::MainWindow),
    presenterMain_(nullptr)
{
    ui_->setupUi(this);
    tabWidget_ = new QTabWidget(this);

    // Enable the Close button on each tab:
    tabWidget_->setTabsClosable(true);

    // Allow reordering tabs by mouse drag:
    tabWidget_->setMovable(true);

    // Connect to handle the user clicking the "X" on a tab:
    connect(tabWidget_, &QTabWidget::tabCloseRequested,
            this, [this](int index) {
                if (presenterMain_) {
                    presenterMain_->removeTab(index);
                }
            });

    connect(ui_->actionNew, &QAction::triggered, this, &MainWindow::addNewSceneTab);
    setCentralWidget(tabWidget_);
}

MainWindow::~MainWindow()
{
    delete ui_;
    qDebug() << "Main window destroyed";
}

void MainWindow::setPresenterMain(PresenterMain *presenterMain)
{
    presenterMain_ = presenterMain;
}

void MainWindow::addNewSceneTab()
{
    if (presenterMain_) {
        // Delegate the creation of a new tab (and its sub-presenter) to PresenterMain.
        presenterMain_->createNewTab();
    }
}
