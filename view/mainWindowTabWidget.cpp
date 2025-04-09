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
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);

    // ------------------ Left side ------------------
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
    QWidget* centerWidget = new QWidget(splitter);
    QVBoxLayout* centerLayout = new QVBoxLayout(centerWidget);

    sceneRenderer_ = new SceneRendererWidget(centerWidget);

    centerLayout->addWidget(sceneRenderer_);
    centerWidget->setLayout(centerLayout);

    // ------------------ Right side ------------------
    QWidget* rightWidget = new QWidget(splitter);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightWidget);
    rightWidget->setLayout(rightLayout);

    // Put our three child widgets in the splitter:
    splitter->addWidget(leftWidget);
    splitter->addWidget(centerWidget);
    splitter->addWidget(rightWidget);

    // Set size policies to allow proper stretching
    leftWidget->setMinimumWidth(150);
    leftWidget->setMaximumWidth(250);
    leftWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    centerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    rightWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    // Explicitly set initial splitter sizes
    splitter->setSizes({150, 600, 150});

    // Now create the main layout for this tab widget, and add the splitter into it:
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(splitter);
    setLayout(mainLayout);

    // -------------- Setup standard shortcuts --------------
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

    auto cutShortcut = new QShortcut(QKeySequence::Cut, this);
    connect(cutShortcut, &QShortcut::activated, this, &MainWindowTabWidget::cutSelected);

    auto pasteShortcut = new QShortcut(QKeySequence::Paste, this);
    connect(pasteShortcut, &QShortcut::activated, this, &MainWindowTabWidget::pasteObject);

    auto deleteShortcut = new QShortcut(QKeySequence::Delete, this);
    connect(deleteShortcut, &QShortcut::activated, this, &MainWindowTabWidget::deleteSelected);

    // Model is created but will be assigned scene/colorificator later:
    model_ = std::make_shared<SceneObjectModel>(scene_, sceneColorificator_, this);
    listView_->setModel(model_.get());
}

MainWindowTabWidget::~MainWindowTabWidget(){
    qDebug() << "mainWindowTabWidget is dead";
}

// Provide a simple context menu (right-click) for the list view:
void MainWindowTabWidget::onListContextMenu(const QPoint &pos)
{
    QMenu contextMenu;
    QAction* cutAct = contextMenu.addAction(tr("Cut"));
    QAction* copyAct = contextMenu.addAction(tr("Copy"));
    QAction* pasteAct = contextMenu.addAction(tr("Paste"));
    QAction* deleteAct = contextMenu.addAction(tr("Delete"));

    QAction* selectedAct = contextMenu.exec(listView_->viewport()->mapToGlobal(pos));
    if (!selectedAct) return;

    if (selectedAct == cutAct) {
        cutSelected();
    }
    else if (selectedAct == copyAct) {
        copySelected();
    }
    else if (selectedAct == pasteAct) {
        pasteObject();
    }
    else if (selectedAct == deleteAct) {
        deleteSelected();
    }
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

// -------------- Copy / Cut / Paste / Delete logic --------------
void MainWindowTabWidget::copySelected()
{
    if (!model_) return;
    auto index = listView_->currentIndex();
    if (!index.isValid()) return;

    auto objPtr = model_->getObjectByRow(index.row());
    if (!objPtr) return;

    QVariant colorVar = model_->data(model_->index(index.row(), 0), SceneObjectModel::ColorRole);

    // Use the shared clipboard from the parent presenter.
    if (presenterMainTab_) {
        auto parent = presenterMainTab_->getParentPresenter();
        if (parent) {
            *(parent->copyBuffer_) = *objPtr;
            *(parent->copyColor_) = colorVar.isValid() ? colorVar.value<QColor>() : QColor(SceneColorificator::defaultColor);
        }
    }
}

void MainWindowTabWidget::cutSelected()
{
    if (!model_) return;
    auto index = listView_->currentIndex();
    if (!index.isValid()) return;

    copySelected();
    deleteSelected();
}

void MainWindowTabWidget::pasteObject()
{
    if (!model_) return;

    if (presenterMainTab_) {
        auto parent = presenterMainTab_->getParentPresenter();
        if (parent && !parent->copyBuffer_->name.isEmpty()) {
            // Create a new object based on the shared clipboard.
            SceneObject newObj = *(parent->copyBuffer_);
            newObj.name += " - " + tr("Copy");

            auto cmd = new AddSceneObjectCommand(
                model_,
                newObj,
                *(parent->copyColor_),
                [this]() { sceneRenderer_->updateAll(); }
            );
            undoStack_->push(cmd);
        }
    }
}

void MainWindowTabWidget::deleteSelected()
{
    if (!model_) return;
    auto index = listView_->currentIndex();
    if (!index.isValid()) return;

    auto cmd = new RemoveSceneObjectCommand(
        model_,
        index.row(),
        [this]() { sceneRenderer_->updateAll(); }
        );
    undoStack_->push(cmd);
}
