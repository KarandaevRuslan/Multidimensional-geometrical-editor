#include "mainWindow.h"
#include "mainWindowTabWidget.h"
#include "../view/sceneRenderer.h"
#include <QFile>
#include <QUiLoader>
#include <QDebug>
#include <QLayout>
#include <QLabel>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDate>
#include "../model/opengl/input/sceneInputHandler.h"
#include "../model/opengl/objectController/cameraController.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
    ui_(new Ui::MainWindow),
    presenterMain_(nullptr)
{
    ui_->setupUi(this);

    tabWidget_ = new QTabWidget;
    tabWidget_->setTabsClosable(true);
    tabWidget_->setMovable(true);
    connect(tabWidget_, &QTabWidget::tabCloseRequested,
            this, [this](int index) {
                if (presenterMain_) presenterMain_->removeTab(index);
            });
    connect(tabWidget_, &QTabWidget::currentChanged,
            this, &MainWindow::refreshMenu);

    connect(ui_->actionNew, &QAction::triggered, this, [this]{ if(presenterMain_) presenterMain_->createNewTab(false);});
    connect(ui_->actionExampleScene, &QAction::triggered, this, [this]{ if(presenterMain_) presenterMain_->createNewTab(true);});
    connect(ui_->actionSave,   &QAction::triggered, this, [this]{ if(presenterMain_) presenterMain_->saveCurrentTab(false);});
    connect(ui_->actionSaveAs, &QAction::triggered, this, [this]{ if(presenterMain_) presenterMain_->saveCurrentTab(true);});
    connect(ui_->actionOpen,   &QAction::triggered, this, [this]{ if(presenterMain_) presenterMain_->openSceneInNewTab();});
    connect(ui_->actionExit, &QAction::triggered, this, &QMainWindow::close);

    connect(ui_->actionAbout, &QAction::triggered,
            this, &MainWindow::showAboutDialog);

    auto *container = new QWidget;
    auto *lay = new QVBoxLayout(container);
    lay->setContentsMargins(5, 2, 5, 2);
    lay->setSpacing(4);
    lay->addWidget(tabWidget_);
    setCentralWidget(container);


    hintLabel_   = new QLabel(this);
    hintLabel_->setContentsMargins(8, 0, 0, 0);
    coordsLabel_ = new QLabel(this);
    coordsLabel_->setContentsMargins(0, 0, 8, 0);
    coordsLabel_->setMinimumWidth(170);
    coordsLabel_->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    statusBar()->setSizeGripEnabled(false);

    statusBar()->addWidget(hintLabel_, /*stretch=*/1);
    statusBar()->addWidget(coordsLabel_);
    connect(tabWidget_, &QTabWidget::currentChanged,
            this, &MainWindow::updateStatusBar);
    updateStatusBar();
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

void MainWindow::closeEvent(QCloseEvent* event)
{
    // if any tab has unsaved changes, ask to discard or cancel
    if (presenterMain_ && presenterMain_->hasDirtyTabs()) {
        auto btn = QMessageBox::warning(
            this,
            tr("Unsaved Changes"),
            tr("You have unsaved changes in one or more tabs.\n"
               "Discard changes and exit?"),
            QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Cancel
            );
        if (btn == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
    }
    // otherwise proceed with close
    QMainWindow::closeEvent(event);
}

void MainWindow::refreshMenu(int idx)
{
    ui_->menuEdit->clear();
    ui_->menuView->clear();
    if (idx < 0) return;

    if (auto *tw = qobject_cast<MainWindowTabWidget*>(tabWidget_->widget(idx)))
    {
        const auto editActs = tw->editActions();
        for (QAction* act : editActs) {
            if (act)
                ui_->menuEdit->addAction(act);
            else
                ui_->menuEdit->addSeparator();
        }

        const auto viewActs = tw->viewActions();
        for (QAction* act : viewActs) {
            if (act)
                ui_->menuView->addAction(act);
            else
                ui_->menuView->addSeparator();
        }
    }
}

void MainWindow::updateStatusBar()
{
    int idx = tabWidget_->currentIndex();
    if (idx < 0) {
        // no open tabs
        hintLabel_->setText(tr("Add new scene (File→New scene)"));
        coordsLabel_->clear();
        return;
    }

    auto *tw = qobject_cast<MainWindowTabWidget*>(tabWidget_->widget(idx));
    // if we have a tab but its scene is empty:
    if (tw->sceneObjectCount() == 0) {
        hintLabel_->setText(tr("Add new scene object (Edit→Add)"));
    }
    else {
        // freelook hint
        auto input = tw->inputHandler();
        if (input->freeLookEnabled()) {
            hintLabel_->setText(tr("Press Shift+F to leave free look"));
        } else {
            hintLabel_->setText(tr("Press Shift+F to enter free look"));
        }
    }

    // camera coords on right:
    {
        auto cam = tw->cameraController();

        const QVector3D &p = cam->position();
        coordsLabel_->setText(
            tr("X:%1  Y:%2  Z:%3")
                .arg(p.x(), 0, 'f', 1)
                .arg(p.y(), 0, 'f', 1)
                .arg(p.z(), 0, 'f', 1)
            );
    }
}

void MainWindow::showAboutDialog()
{
    const int year = QDate::currentDate().year();
    const QString html =
        tr("<h3>%1</h3>"
           "<p>Author: <b>%2</b></p>"
           "<p>GitHub:&nbsp;"
           "<a href=\"https://github.com/KarandaevRuslan/Multidimensional-geometrical-editor\">"
           "github.com/KarandaevRuslan/Multidimensional-geometrical-editor</a></p>"
           "<p>&copy; %3&nbsp;%2</p>")
            .arg(QCoreApplication::applicationName())
            .arg("Karandaev Ruslan")
            .arg(year);

    QMessageBox about(this);
    about.setIcon(QMessageBox::Information);
    about.setWindowTitle(tr("About %1").arg(QCoreApplication::applicationName()));
    about.setTextFormat(Qt::RichText);
    about.setText(html);
    about.setStandardButtons(QMessageBox::Ok);
    about.exec();
}
