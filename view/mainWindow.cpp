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

    // 1) Create the tab widget as before
    tabWidget_ = new QTabWidget;
    tabWidget_->setTabsClosable(true);
    tabWidget_->setMovable(true);
    connect(tabWidget_, &QTabWidget::tabCloseRequested,
            this, [this](int index) {
                if (presenterMain_) presenterMain_->removeTab(index);
            });
    connect(ui_->actionNew, &QAction::triggered,
            this, &MainWindow::addNewSceneTab);

    // 2) Wrap it in a container widget + layout
    auto *container = new QWidget;
    auto *lay = new QVBoxLayout(container);
    // tweak these numbers to taste
    lay->setContentsMargins(5, 2, 5, 2);
    lay->setSpacing(4);

    lay->addWidget(tabWidget_);

    // 3) And make *that* the central widget
    setCentralWidget(container);
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
