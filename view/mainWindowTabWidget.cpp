#include "mainWindowTabWidget.h"
#include "commands/changeSceneObjectCommand.h"
#include "addSceneObjectDialog.h"
#include "sceneObjectEditorWidget.h"
#include "commands/addSceneObjectCommand.h"
#include "commands/removeSceneObjectCommand.h"
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QShortcut>
#include <QContextMenuEvent>
#include <QMessageBox>
#include "../tools/SceneSerialization.h"
#include <QFileDialog>
#include "../model/opengl/input/sceneInputHandler.h"

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
    editor_ = new SceneObjectEditorWidget(rightWidget);
    // ——— editor → undo stack + render refresh ———
    connect(editor_, &SceneObjectEditorWidget::objectEdited,
            this,[this](const SceneObject &upd,const QColor &col,bool geomChanged){
                const QModelIndex idx = listView_->currentIndex();
                if(!idx.isValid()) return;

                undoStack_->push(new ChangeSceneObjectCommand(
                    model_, idx.row(), upd, col,
                    [this, geomChanged]{
                        if(geomChanged) sceneRenderer_->updateAll();
                        markDirty();
                    },
                    [this]{
                        editor_->rebuildUiFromCurrent();
                    }));
            });

    rightLayout->addWidget(editor_);
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

    // --------- Create actions and shortcuts ----------
    auto makeAction = [this](const QString& name, const QString& text, const QKeySequence& shortcut, auto slot, auto target) {
        QAction* act = new QAction(text, this);
        act->setShortcut(shortcut);
        act->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        connect(act, &QAction::triggered, this, slot);\
        target->addAction(act); // Registers shortcut
        actions_[name] = act;
    };

    makeAction("undo",   tr("Undo"),    QKeySequence::Undo,    [this]{ undoStack_->undo(); }, this);
    makeAction("redo",   tr("Redo"),    QKeySequence::Redo,    [this]{ undoStack_->redo(); }, this);
    makeAction("copy",   tr("Copy"),    QKeySequence::Copy,    &MainWindowTabWidget::copySelected, listView_);
    makeAction("cut",    tr("Cut"),     QKeySequence::Cut,     &MainWindowTabWidget::cutSelected, listView_);
    makeAction("paste",  tr("Paste"),   QKeySequence::Paste,   &MainWindowTabWidget::pasteObject, listView_);
    makeAction("delete", tr("Delete"),  QKeySequence::Delete,  &MainWindowTabWidget::deleteSelected, listView_);
    makeAction("add", tr("Add"), QKeySequence::New,
               [this]{
                   AddSceneObjectDialog dlg(this);
                   if (dlg.exec() != QDialog::Accepted) return;

                   static int nextId = 1;
                   SceneObject obj = dlg.makeSceneObject(nextId++);
                   QColor      col = dlg.color();

                   undoStack_->push(new AddSceneObjectCommand(
                       model_, obj, col,
                       [this]{
                           sceneRenderer_->updateAll();
                           markDirty();
                       }));

                   selectLastObject();
               },
               listView_);
    makeAction("exportObj", tr("Export Object"),
               QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S),
               &MainWindowTabWidget::exportSelectedObject, listView_);

    makeAction("importObj", tr("Import Object"),
               QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_L),
               &MainWindowTabWidget::importObject, listView_);


    // Model is created but will be assigned scene/colorificator later:
    model_ = std::make_shared<SceneObjectModel>(scene_, sceneColorificator_, this);
    listView_->setModel(model_.get());
    connect(listView_->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &MainWindowTabWidget::onCurrentRowChanged);
}

MainWindowTabWidget::~MainWindowTabWidget(){
    qDebug() << "mainWindowTabWidget is dead";
}

// Provide a simple context menu (right-click) for the list view:
void MainWindowTabWidget::onListContextMenu(const QPoint &pos)
{
    QMenu contextMenu(this);

    contextMenu.addAction(actions_["add"]);
    contextMenu.addSeparator();
    contextMenu.addAction(actions_["cut"]);
    contextMenu.addAction(actions_["copy"]);
    contextMenu.addAction(actions_["paste"]);
    contextMenu.addAction(actions_["delete"]);
    contextMenu.addSeparator();
    contextMenu.addAction(actions_["exportObj"]);
    contextMenu.addAction(actions_["importObj"]);

    contextMenu.exec(listView_->viewport()->mapToGlobal(pos));
}

void MainWindowTabWidget::setScene(std::shared_ptr<Scene> scene) {
    scene_ = scene;
    if (sceneRenderer_) {
        sceneRenderer_->setScene(scene_);
    }
    if (model_) {
        model_->setScene(scene_);
    }

    auto input = sceneRenderer_->inputHandler();
    if (input) {
        connect(input.get(), &SceneInputHandler::freeLookModeToggled,
                presenterMainTab_->getParentPresenter()->getMainWindow(),
                &MainWindow::updateStatusBar,
                Qt::QueuedConnection);
    }

    connect(input.get(), &SceneInputHandler::cameraMoved,
            presenterMainTab_->getParentPresenter()->getMainWindow(),
            &MainWindow::updateStatusBar,
            Qt::QueuedConnection);
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
    qDebug() << "Shortcut triggered. Focus is on:" << QApplication::focusWidget();

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
            *(parent->copyBuffer_) = objPtr->clone();
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
            QString baseName = parent->copyBuffer_->name;
            static QRegularExpression reCopyNum(R"( - Copy(?: - (\d+))?$)");
            QRegularExpressionMatch match = reCopyNum.match(baseName);

            if (match.hasMatch()) {
                if (match.captured(1).isEmpty()) {
                    baseName += " - 2";
                } else {
                    int num = match.captured(1).toInt();
                    baseName = baseName.left(baseName.lastIndexOf(" - ")) + QString(" - %1").arg(num + 1);
                }
            } else {
                baseName += " - Copy";
            }

            SceneObject newObj = parent->copyBuffer_->clone();
            newObj.name = baseName;

            auto cmd = new AddSceneObjectCommand(
                model_,
                newObj,
                *(parent->copyColor_),
                [this]() {
                    sceneRenderer_->updateAll();
                    markDirty();
                }
            );
            undoStack_->push(cmd);

            selectLastObject();
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
        [this]() {
            sceneRenderer_->updateAll();
            markDirty();
        }
        );
    undoStack_->push(cmd);
}

void MainWindowTabWidget::onCurrentRowChanged(const QModelIndex &current,
                                              const QModelIndex &previous)
{
    qDebug() << current;
    if (!current.isValid()) { editor_->clear(); return; }

    std::shared_ptr<SceneObject> obj = model_->getObjectByRow(current.row());

    editor_->setObject(obj,
    [this, obj]() {
        QColor col = sceneColorificator_->getColorForObject(obj->uid);
        return col;
    });
}

void MainWindowTabWidget::selectLastObject()
{
    int row = model_->rowCount() - 1;
    if (row >= 0) {
        QModelIndex idx = model_->index(row, 0);
        listView_->setCurrentIndex(idx);
    }
}

// ---------- Export -------------------------------------------------
void MainWindowTabWidget::exportSelectedObject()
{
    if (!model_) return;

    try {
        const QModelIndex idx = listView_->currentIndex();
        if (!idx.isValid()) {
            QMessageBox::information(this, tr("Export object"),
                                     tr("Select an object first."));
            return;
        }
        auto obj = model_->getObjectByRow(idx.row());
        if (!obj) {
            QMessageBox::critical(this, tr("Export object"),
                                  tr("Internal error – cannot fetch object."));
            return;
        }

        const QString fileName =
            QFileDialog::getSaveFileName(this, tr("Save object as"),
                                         obj->name + u".json"_qs,
                                         tr("JSON files (*.json)"));
        if (fileName.isEmpty()) return;

        QFile f(fileName);
        if (!f.open(QIODevice::WriteOnly)) {
            QMessageBox::critical(
                this,
                tr("Export object"),
                tr("Cannot open “%1” for writing:\n%2")
                    .arg(fileName)
                    .arg(f.errorString())
                );
            return;
        }

        QColor col = sceneColorificator_->getColorForObject(obj->uid);
        QJsonDocument doc(SceneSerializer::objectToJson(*obj, col));
        f.write(doc.toJson(QJsonDocument::Indented));
        f.close();
    }
    catch (const std::exception &ex) {
        QMessageBox::critical(
            this,
            tr("Export object"),
            tr("An error occurred during export:\n%1").arg(ex.what())
            );
    }
    catch (...) {
        QMessageBox::critical(
            this,
            tr("Export object"),
            tr("An unknown error occurred during export.")
            );
    }
}


// ---------- Import -------------------------------------------------
void MainWindowTabWidget::importObject()
{
    if (!model_) return;

    try {
        const QString fileName =
            QFileDialog::getOpenFileName(this, tr("Load object"),
                                         QString(), tr("JSON files (*.json)"));
        if (fileName.isEmpty()) return;

        QFile f(fileName);
        if (!f.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(
                this,
                tr("Import object"),
                tr("Cannot open “%1”:\n%2")
                    .arg(fileName)
                    .arg(f.errorString())
                );
            return;
        }

        QJsonParseError perr;
        QJsonDocument   doc = QJsonDocument::fromJson(f.readAll(), &perr);
        f.close();

        if (perr.error != QJsonParseError::NoError || !doc.isObject()) {
            QMessageBox::critical(
                this,
                tr("Import object"),
                tr("Invalid JSON:\n%1 at offset %2")
                    .arg(perr.errorString())
                    .arg(perr.offset)
                );
            return;
        }

        QColor col;
        auto   obj = SceneSerializer::objectFromJson(doc.object(), &col);
        if (!obj) {
            QMessageBox::critical(this, tr("Import object"),
                                  tr("Failed to interpret object data."));
            return;
        }

        obj->uid = QUuid::createUuid();

        undoStack_->push(new AddSceneObjectCommand(
            model_, obj->clone(), col, [this]{
                sceneRenderer_->updateAll();
                markDirty();
            }));
        selectLastObject();
    }
    catch (const std::exception &ex) {
        QMessageBox::critical(
            this,
            tr("Import object"),
            tr("An error occurred during import:\n%1").arg(ex.what())
            );
    }
    catch (...) {
        QMessageBox::critical(
            this,
            tr("Import object"),
            tr("An unknown error occurred during import.")
            );
    }
}

void MainWindowTabWidget::markDirty(){
    presenterMainTab_->markDirty();

    PresenterMain* pm = presenterMainTab_->getParentPresenter();
    if (pm && pm->getMainWindow()) {
        pm->getMainWindow()->updateStatusBar();
    }
}

QList<QAction*> MainWindowTabWidget::editActions() const
{
    return {
        actions_.value("undo"),
        actions_.value("redo"),
        nullptr,                     // -------------
        actions_.value("cut"),
        actions_.value("copy"),
        actions_.value("paste"),
        actions_.value("delete"),
        nullptr,                     // -------------
        actions_.value("add"),
        actions_.value("exportObj"),
        actions_.value("importObj")
    };
}

int MainWindowTabWidget::sceneObjectCount() const {
    return static_cast<int>(scene_->objectCount());
}

CameraController* MainWindowTabWidget::cameraController() const {
    return sceneRenderer_->cameraController().get();
}

SceneInputHandler* MainWindowTabWidget::inputHandler() const {
    return sceneRenderer_->inputHandler().get();
}
