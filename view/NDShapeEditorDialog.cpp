#include "NDShapeEditorDialog.h"

#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTableView>
#include <QToolButton>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QMenu>
#include <stdexcept>
#include <QStyledItemDelegate>
#include <QUndoStack>
#include <QUndoCommand>
#include "dataModels/vertexTableModel.h"
#include "dataModels/adjacencyMatrixModel.h"
#include "commands/shapeCommand.h"
#include "delegates/noHoverDelegate.h"
#include "adjacencyMatrixView.h"
#include <QScrollBar>

// ───────────────────────────────────────────────────────── ctor ──
NDShapeEditorDialog::NDShapeEditorDialog(const NDShape& startShape,
                                         QWidget* parent)
    : QDialog(parent)
    , shape_(std::make_shared<NDShape>(startShape))
    , undo_(new QUndoStack(this))
    , rowToId_(std::make_shared<std::vector<std::size_t>>())
{
    setWindowTitle(tr("Shape editor"));
    resize(650, 600);

    undo_->setUndoLimit(2500);

    // allow Ctrl+Z / Ctrl+Y shortcuts anywhere in the dialog
    auto* undoAct = undo_->createUndoAction(this, tr("Undo"));
    undoAct->setShortcut(QKeySequence::Undo);
    addAction(undoAct);

    auto* redoAct = undo_->createRedoAction(this, tr("Redo"));
    redoAct->setShortcut(QKeySequence::Redo);
    addAction(redoAct);

    // Main layout with generous margins
    auto* mainBox = new QVBoxLayout(this);
    mainBox->setContentsMargins(10, 6, 10, 10);
    mainBox->setSpacing(8);

    /* ── Dimension ─────────────────────────────────────────────── */
    auto* dimHBox = new QHBoxLayout;
    dimHBox->setContentsMargins(3, 4, 3, 0);
    dimHBox->setSpacing(25);

    auto* dimLabel = new QLabel(tr("Dimension:"), this);
    dimHBox->addWidget(dimLabel);

    dimSpin_ = new QSpinBox(this);
    dimSpin_->setRange(dimMin_, dimMax_);
    dimSpin_->setValue(int(shape_->getDimension()));
    dimSpin_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(dimSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &NDShapeEditorDialog::onDimensionChanged);
    dimHBox->addWidget(dimSpin_);

    mainBox->addLayout(dimHBox);

    /* ── Tables (tabs) ──────────────────────────────────────────── */
    auto* tableTabs = new QTabWidget(this);
    mainBox->addWidget(tableTabs);

    /* --- Vertices page ----------------------------------------- */
    auto* vertPage = new QWidget(this);
    auto* vertVBox = new QVBoxLayout(vertPage);
    vertVBox->setContentsMargins(6, 5, 6, 5);
    vertVBox->setSpacing(8);

    vertModel_ = new VertexTableModel(shape_, undo_, rowToId_, this);
    vertView_  = new QTableView(vertPage);
    vertView_->setModel(vertModel_);
    vertView_->setSelectionBehavior(QAbstractItemView::SelectRows);
    vertView_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    vertView_->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    vertView_->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    vertView_->horizontalHeader()->setDefaultSectionSize(50);
    vertView_->verticalHeader()->setDefaultSectionSize(25);
    vertView_->horizontalHeader()
        ->setSectionResizeMode(QHeaderView::Interactive);
    vertView_->verticalHeader()
        ->setSectionResizeMode(QHeaderView::Interactive);

    vertView_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(vertView_, &QWidget::customContextMenuRequested,
            this,       &NDShapeEditorDialog::showVertContextMenu);

    auto makeAction = [this](const QString& name,
                             const QString& text,
                             const QKeySequence& shortcut,
                             auto slotPtr) {
        QAction* act = new QAction(text, this);
        act->setShortcut(shortcut);
        act->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        connect(act, &QAction::triggered, this, slotPtr);
        vertView_->addAction(act);
        vertActs_[name] = act;
    };

    makeAction("add",    tr("Add"),    QKeySequence::New,    &NDShapeEditorDialog::addVertex);
    makeAction("copy",   tr("Copy"),   QKeySequence::Copy,   &NDShapeEditorDialog::copyVertices);
    makeAction("cut",    tr("Cut"),    QKeySequence::Cut,    &NDShapeEditorDialog::cutVertices);
    makeAction("paste",  tr("Paste"),  QKeySequence::Paste,  &NDShapeEditorDialog::pasteVertices);
    makeAction("delete", tr("Delete"), QKeySequence::Delete, &NDShapeEditorDialog::removeVertices);

    vertVBox->addWidget(vertView_);

    /* --- Adjacency page ---------------------------------------- */
    auto* adjPage = new QWidget(this);
    auto* adjVBox = new QVBoxLayout(adjPage);
    adjVBox->setContentsMargins(8, 6, 8, 6);
    adjVBox->setSpacing(6);

    adjModel_ = new AdjacencyMatrixModel(shape_, undo_, rowToId_, this);
    adjView_  = new AdjacencyMatrixView(adjPage);
    adjView_->setModel(adjModel_);
    adjView_->setSelectionMode(QAbstractItemView::NoSelection);
    adjView_->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    adjView_->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    adjView_->setItemDelegate(new NoHoverDelegate(adjView_));
    adjView_->horizontalHeader()->setDefaultSectionSize(20);
    adjView_->verticalHeader()->setDefaultSectionSize(20);
    connect(adjView_,&QTableView::clicked,
            this,&NDShapeEditorDialog::onAdjCellClicked);

    adjVBox->addWidget(adjView_);

    tableTabs->addTab(vertPage, tr("Vertices"));
    tableTabs->addTab(adjPage,  tr("Edges"));

    /* ── OK / Cancel -------------------------------------------- */
    auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(btnBox,  &QDialogButtonBox::accepted, this, &NDShapeEditorDialog::accept);
    connect(btnBox,  &QDialogButtonBox::rejected, this, &NDShapeEditorDialog::reject);
    mainBox->addWidget(btnBox);
}

void NDShapeEditorDialog::structuralReload()
{
    vertModel_->reload();
    adjModel_->reload();
    dimSpin_->blockSignals(true);
    dimSpin_->setValue(static_cast<int>(shape_->getDimension()));
    dimSpin_->blockSignals(false);
}

// ─────────────────────────────────── dimension changed ──
void NDShapeEditorDialog::onDimensionChanged(int d)
{
    if (std::size_t(d) == shape_->getDimension())
        return;

    NDShape before = *shape_;
    *shape_ = shape_->clone(std::size_t(d));
    NDShape after  = *shape_;

    undo_->push(new ShapeCommand(
        shape_, before, after, tr("Change dimension"),
        [this](){ structuralReload(); }));
}

// ───────────────────────────── vertices (operations) ──
void NDShapeEditorDialog::addVertex()
{
    NDShape before = *shape_;
    shape_->addVertex(std::vector<double>(shape_->getDimension(), 0.0));
    NDShape after  = *shape_;

    undo_->push(new ShapeCommand(
        shape_, before, after, tr("Add vertex"),
        [this](){ structuralReload();}));
}

void NDShapeEditorDialog::removeVertices()
{
    auto sel=vertView_->selectionModel()->selectedRows();
    if(sel.isEmpty()) return;

    NDShape before=*shape_;

    QVector<std::size_t> ids;
    ids.reserve(sel.size());
    for(const QModelIndex& idx : sel)
        ids << rowToId_->at(std::size_t(idx.row()));

    std::sort(ids.begin(), ids.end(), std::greater<>());
    for(auto id : ids) shape_->removeVertex(id);

    NDShape after = *shape_;
    undo_->push(new ShapeCommand(
        shape_, before, after, tr("Remove vertices"),
        [this](){ structuralReload();}));
}

void NDShapeEditorDialog::copyVertices()
{
    vertClipboard_.clear();
    for(const QModelIndex& idx: vertView_->selectionModel()->selectedRows())
        vertClipboard_ << shape_->getVertex(rowToId_->at(std::size_t(idx.row())));
}

void NDShapeEditorDialog::cutVertices()
{
    copyVertices();
    removeVertices();
}

void NDShapeEditorDialog::pasteVertices()
{
    if (vertClipboard_.isEmpty())
        return;

    NDShape before = *shape_;

    const std::size_t dim = shape_->getDimension();
    for (const auto& oldCoords : vertClipboard_) {
        std::vector<double> newCoords(dim, 0.0);
        const std::size_t copyCount = std::min<std::size_t>(dim, oldCoords.size());
        for (std::size_t i = 0; i < copyCount; ++i)
            newCoords[i] = oldCoords[i];
        shape_->addVertex(newCoords);
    }

    NDShape after = *shape_;
    undo_->push(new ShapeCommand(
        shape_, before, after, tr("Paste vertices"),
        [this](){ structuralReload();}));
}

/* edges ----------------------------------------------------------*/
void NDShapeEditorDialog::onAdjCellClicked(const QModelIndex& idx)
{
    adjModel_->toggleEdge(idx.row(), idx.column());
}

// ───────────────────────────── context menu helpers ──
void NDShapeEditorDialog::showVertContextMenu(const QPoint& pos)
{
    QMenu menu(this);
    menu.addAction(vertActs_["add"]);
    menu.addSeparator();
    menu.addAction(vertActs_["copy"]);
    menu.addAction(vertActs_["cut"]);
    menu.addAction(vertActs_["paste"]);
    menu.addAction(vertActs_["delete"]);
    menu.exec(vertView_->viewport()->mapToGlobal(pos));
}
