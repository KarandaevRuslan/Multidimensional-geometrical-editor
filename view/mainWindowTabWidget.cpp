#include "mainWindowTabWidget.h"
#include "commands/addSceneObjectCommand.h"
#include "commands/removeSceneObjectCommand.h"
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QShortcut>
#include <QContextMenuEvent>

MainWindowTabWidget::MainWindowTabWidget(QWidget *parent)
    : QWidget(parent)
{
    // We'll use a horizontal QSplitter to get three columns,
    // each of which can be resized by the user.
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);

    // ------------------ Left side ------------------
    // A simple QWidget containing the QListView, plus maybe
    // an optional label or toolbar above it.
    QWidget* leftWidget = new QWidget(splitter);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftWidget);

    // Construct the list view and undo stack.
    listView_ = new QListView(leftWidget);
    undoStack_ = std::make_unique<QUndoStack>(this);

    // Enable context menu on the list:
    listView_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(listView_, &QListView::customContextMenuRequested,
            this, &MainWindowTabWidget::onListContextMenu);

    leftLayout->addWidget(listView_);
    leftWidget->setLayout(leftLayout);

    // ------------------ Center side ------------------
    // This will hold the scene renderer.
    QWidget* centerWidget = new QWidget(splitter);
    QVBoxLayout* centerLayout = new QVBoxLayout(centerWidget);

    sceneRenderer_ = new SceneRenderer(centerWidget);
    // The scene and colorificator are set later via setScene(...) etc.

    centerLayout->addWidget(sceneRenderer_);
    centerWidget->setLayout(centerLayout);

    // ------------------ Right side ------------------
    // Just an empty VBox layout that you can populate or leave empty.
    QWidget* rightWidget = new QWidget(splitter);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightWidget);
    // Put placeholders or leave it empty
    // e.g. rightLayout->addWidget(new QLabel("Right Side Placeholder"));
    rightWidget->setLayout(rightLayout);

    // Put our three child widgets in the splitter:
    splitter->addWidget(leftWidget);
    splitter->addWidget(centerWidget);
    splitter->addWidget(rightWidget);

    // Now create the main layout for this tab widget,
    // and add the splitter into it:
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(splitter);
    setLayout(mainLayout);

    // -------------- Setup standard shortcuts --------------
    // We want Undo, Redo, Copy, Paste to work in this tab:
    auto undoShortcut = new QShortcut(QKeySequence::Undo, this);
    connect(undoShortcut, &QShortcut::activated, this, [this]() {
        undoStack_->undo();
    });

    auto redoShortcut = new QShortcut(QKeySequence::Redo, this);
    connect(redoShortcut, &QShortcut::activated, this, [this]() {
        undoStack_->redo();
    });

    auto copyShortcut = new QShortcut(QKeySequence::Copy, this);
    connect(copyShortcut, &QShortcut::activated, this, &MainWindowTabWidget::copySelected);

    auto pasteShortcut = new QShortcut(QKeySequence::Paste, this);
    connect(pasteShortcut, &QShortcut::activated, this, &MainWindowTabWidget::pasteObject);

    // Model is created but will be assigned scene/colorificator later:
    model_ = std::make_shared<SceneObjectModel>(scene_, sceneColorificator_, this);
    listView_->setModel(model_.get());

    // If you want "Undo"/"Redo" menu actions somewhere, you can create them:
    // auto undoAction = undoStack_->createUndoAction(this, tr("Undo"));
    // auto redoAction = undoStack_->createRedoAction(this, tr("Redo"));
    // But that is optional for the main menu or toolbars, etc.
}

// Provide a simple context menu (right-click) for the list view:
void MainWindowTabWidget::onListContextMenu(const QPoint &pos)
{
    QMenu contextMenu;
    QAction* copyAct = contextMenu.addAction(tr("Copy"));
    QAction* pasteAct = contextMenu.addAction(tr("Paste"));
    QAction* deleteAct = contextMenu.addAction(tr("Delete"));

    QAction* selectedAct = contextMenu.exec(listView_->viewport()->mapToGlobal(pos));
    if (!selectedAct) return;

    if (selectedAct == copyAct) {
        copySelected();
    }
    else if (selectedAct == pasteAct) {
        pasteObject();
    }
    else if (selectedAct == deleteAct) {
        deleteSelected();
    }
}

std::weak_ptr<SceneRenderer> MainWindowTabWidget::sceneRenderer() const {
    // Return our sceneRenderer_, but as a weak_ptr if needed:
    // If you truly want to store a std::shared_ptr, you'd
    // have to create it that way. Right now it's unique_ptr.
    return std::weak_ptr<SceneRenderer>();
}

void MainWindowTabWidget::setScene(std::shared_ptr<Scene> scene) {
    scene_ = scene;
    if (sceneRenderer_) {
        sceneRenderer_->setScene(scene_);
    }
    if (model_) {
        model_->setScene(scene_);
    }
}

void MainWindowTabWidget::setSceneColorificator(std::shared_ptr<SceneColorificator> colorificator) {
    sceneColorificator_ = colorificator;
    if (sceneRenderer_) {
        sceneRenderer_->setSceneColorificator(sceneColorificator_);
    }
    if (model_) {
        model_->setSceneColorificator(sceneColorificator_);
    }
}

void MainWindowTabWidget::setDelegate(const std::shared_ptr<SceneObjectDelegate>& delegate) {
    delegate_ = delegate;
    if (delegate_ && listView_) {
        listView_->setItemDelegate(delegate_.get());
    }
}

void MainWindowTabWidget::setPresenterMainTab(const std::shared_ptr<PresenterMainTab>& presenter) {
    presenterMainTab_ = presenter;
}

// -------------- Copy / Paste / Delete logic --------------
void MainWindowTabWidget::copySelected()
{
    if (!model_) return;
    auto index = listView_->currentIndex();
    if (!index.isValid()) return;

    auto objPtr = model_->getObjectByRow(index.row());
    if (!objPtr) return;

    // Use the shared clipboard from the parent presenter.
    if (presenterMainTab_) {
        auto parent = presenterMainTab_->getParentPresenter();
        if (parent) {
            *(parent->copyBuffer_) = *objPtr;
        }
    }
}

void MainWindowTabWidget::pasteObject()
{
    if (!model_) return;

    if (presenterMainTab_) {
        auto parent = presenterMainTab_->getParentPresenter();
        if (parent && !parent->copyBuffer_->name.isEmpty()) {
            // Create a new object based on the shared clipboard.
            static int syntheticId = 10000;
            SceneObject newObj = *(parent->copyBuffer_);
            newObj.id = syntheticId++;
            newObj.name += "_Copy";

            auto cmd = new AddSceneObjectCommand(model_, newObj, *(parent->copyColor_));
            undoStack_->push(cmd);
        }
    }
}

void MainWindowTabWidget::deleteSelected()
{
    if (!model_) return;
    auto index = listView_->currentIndex();
    if (!index.isValid()) return;

    auto cmd = new RemoveSceneObjectCommand(model_, index.row());
    undoStack_->push(cmd);
}
