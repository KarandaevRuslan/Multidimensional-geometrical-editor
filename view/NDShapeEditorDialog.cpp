#include "NDShapeEditorDialog.h"

#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QMenu>
#include <stdexcept>
#include <QStyledItemDelegate>
#include <QUndoStack>
#include <QUndoCommand>

// ───────────────────────────────────────────────────────── helper ──
namespace {
constexpr int   kDimMin   = 3;
constexpr int   kDimMax   = 20;
constexpr char  kFloatFmt[] = "%.6g";
}

// ──────────────────────────────────────────────────────── delegates ──
namespace {
class NoHoverDelegate final : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const override
    {
        QStyleOptionViewItem opt(option);
        opt.state &= ~QStyle::State_MouseOver;
        opt.state &= ~QStyle::State_HasFocus;
        QStyledItemDelegate::paint(painter, opt, index);
    }
};
} // unnamed

// ────────────────────────────────────────────── undo command ──
namespace {
class ShapeCommand final : public QUndoCommand {
public:
    ShapeCommand(std::shared_ptr<NDShape>   shape,
                 NDShape                    before,
                 NDShape                    after,
                 const QString&             text,
                 std::function<void()>      reloadDlg)
        : shape_(std::move(shape))
        , before_(std::move(before))
        , after_(std::move(after))
        , reloadDlg_(reloadDlg)
    {
        setText(text);
    }

    void undo() override {
        *shape_ = before_;
        reloadDlg_();
    }

    void redo() override {
        *shape_ = after_;
        reloadDlg_();
    }

private:
    std::shared_ptr<NDShape> shape_;
    NDShape                  before_;
    NDShape                  after_;
    std::function<void()>    reloadDlg_;
};
} // unnamed

// ───────────────────────────────────────────────────────── ctor ──
NDShapeEditorDialog::NDShapeEditorDialog(const NDShape& startShape,
                                         QWidget* parent)
    : QDialog(parent)
    , shape_(std::make_shared<NDShape>(startShape))
    , undoStack_(new QUndoStack(this))
{
    setWindowTitle(tr("Shape editor"));
    resize(650, 600);

    // allow Ctrl+Z / Ctrl+Y shortcuts anywhere in the dialog
    auto* undoAct = undoStack_->createUndoAction(this, tr("Undo"));
    undoAct->setShortcut(QKeySequence::Undo);
    addAction(undoAct);

    auto* redoAct = undoStack_->createRedoAction(this, tr("Redo"));
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
    dimSpin_->setRange(kDimMin, kDimMax);
    dimSpin_->setValue(int(shape_->getDimension()));
    dimSpin_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
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

    vertTable_ = new QTableWidget(vertPage);
    vertTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    vertTable_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    vertTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    vertTable_->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    vertTable_->horizontalHeader()
        ->setSectionResizeMode(QHeaderView::Interactive);
    vertTable_->verticalHeader()
        ->setSectionResizeMode(QHeaderView::Interactive);
    vertTable_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(vertTable_, &QTableWidget::itemChanged,
            this,       &NDShapeEditorDialog::onVertexTableItemChanged);
    connect(vertTable_, &QWidget::customContextMenuRequested,
            this,       &NDShapeEditorDialog::showVertContextMenu);

    auto makeAction = [this](const QString& name,
                             const QString& text,
                             const QKeySequence& shortcut,
                             auto slotPtr) {
        QAction* act = new QAction(text, this);
        act->setShortcut(shortcut);
        act->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        connect(act, &QAction::triggered, this, slotPtr);
        vertTable_->addAction(act);
        vertActions_[name] = act;
    };

    makeAction("add",    tr("Add"),    QKeySequence::New,    &NDShapeEditorDialog::addVertex);
    makeAction("copy",   tr("Copy"),   QKeySequence::Copy,   &NDShapeEditorDialog::copyVertices);
    makeAction("cut",    tr("Cut"),    QKeySequence::Cut,    &NDShapeEditorDialog::cutVertices);
    makeAction("paste",  tr("Paste"),  QKeySequence::Paste,  &NDShapeEditorDialog::pasteVertices);
    makeAction("delete", tr("Delete"), QKeySequence::Delete, &NDShapeEditorDialog::removeVertices);

    vertVBox->addWidget(vertTable_);

    /* --- Adjacency page ---------------------------------------- */
    auto* adjPage = new QWidget(this);
    auto* adjVBox = new QVBoxLayout(adjPage);
    adjVBox->setContentsMargins(8, 6, 8, 6);
    adjVBox->setSpacing(6);

    adjTable_ = new QTableWidget(adjPage);
    adjTable_->setSelectionMode(QAbstractItemView::NoSelection);
    adjTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    adjTable_->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    adjTable_->horizontalHeader()->setDefaultSectionSize(20);
    adjTable_->verticalHeader()->setDefaultSectionSize(20);
    adjTable_->setItemDelegate(new NoHoverDelegate(adjTable_));

    connect(adjTable_, &QTableWidget::cellClicked,
            this,       &NDShapeEditorDialog::onAdjCellClicked);

    adjVBox->addWidget(adjTable_);

    tableTabs->addTab(vertPage, tr("Vertices"));
    tableTabs->addTab(adjPage,  tr("Edges"));

    /* ── OK / Cancel -------------------------------------------- */
    auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(btnBox,  &QDialogButtonBox::accepted, this, &NDShapeEditorDialog::accept);
    connect(btnBox,  &QDialogButtonBox::rejected, this, &NDShapeEditorDialog::reject);
    mainBox->addWidget(btnBox);

    /* ── Dimension change --------------------------------------- */
    connect(dimSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &NDShapeEditorDialog::onDimensionChanged);

    /* ── Initial fill ------------------------------------------- */
    internalReload();
}

// ───────────────────────────────────────── public helpers ──
void NDShapeEditorDialog::internalReload()
{
    blockSignals(true);
    rebuildVertexTable();
    rebuildAdjacencyTable();
    dimSpin_->setValue(int(shape_->getDimension()));
    blockSignals(false);
}

// ─────────────────────────────────── dimension changed ──
void NDShapeEditorDialog::onDimensionChanged(int d)
{
    if (std::size_t(d) == shape_->getDimension())
        return;

    NDShape before = *shape_;
    *shape_ = shape_->clone(std::size_t(d));
    NDShape after  = *shape_;

    undoStack_->push(new ShapeCommand(
        shape_, before, after, tr("Change dimension"),
        [this](){internalReload();}));
}

// ───────────────────────────── vertices (table rebuild/helpers) ──
void NDShapeEditorDialog::refreshVertexVerticalHeader()
{
    QStringList hdr;
    for (int i = 0; i < static_cast<int>(rowToId_.size()); ++i)
        hdr << QString::number(i + 1);
    vertTable_->setVerticalHeaderLabels(hdr);
}

void NDShapeEditorDialog::rebuildVertexTable()
{
    vertTable_->blockSignals(true);
    vertTable_->clear();

    const auto verts = shape_->getAllVertices();
    const int  rows  = int(verts.size());
    const int  cols  = int(shape_->getDimension());

    vertTable_->setRowCount(rows);
    vertTable_->setColumnCount(cols);

    QStringList hdr;
    for (int c = 0; c < cols; ++c)
        hdr << tr("x%1").arg(c + 1);
    vertTable_->setHorizontalHeaderLabels(hdr);

    rowToId_.resize(rows);

    for (int r = 0; r < rows; ++r) {
        std::size_t id       = verts[r].first;
        const auto& coord    = verts[r].second;
        rowToId_[r] = id;
        for (int c = 0; c < cols; ++c) {
            auto* it = new QTableWidgetItem(QString::asprintf(kFloatFmt, coord[c]));
            vertTable_->setItem(r, c, it);
        }
    }

    refreshVertexVerticalHeader();
    vertTable_->blockSignals(false);
}

// ───────────────────────────── vertices (operations) ──
void NDShapeEditorDialog::addVertex()
{
    NDShape before = *shape_;
    shape_->addVertex(std::vector<double>(shape_->getDimension(), 0.0));
    NDShape after  = *shape_;

    undoStack_->push(new ShapeCommand(
        shape_, before, after, tr("Add vertex"),
        [this](){internalReload();}));
}

void NDShapeEditorDialog::removeVertices()
{
    auto sel = vertTable_->selectionModel()->selectedRows();
    if (sel.isEmpty()) return;

    NDShape before = *shape_;

    std::vector<std::size_t> ids;
    for (const QModelIndex& idx : sel)
        ids.push_back(rowToId_[idx.row()]);

    std::sort(ids.begin(), ids.end(), std::greater<>());
    for (auto id : ids)
        shape_->removeVertex(id);

    NDShape after = *shape_;
    undoStack_->push(new ShapeCommand(
        shape_, before, after, tr("Remove vertices"),
        [this](){internalReload();}));
}

void NDShapeEditorDialog::onVertexTableItemChanged(QTableWidgetItem* it)
{
    if (!it) return;

    const int row = it->row();
    const int col = it->column();

    std::size_t id = rowToId_[row];
    auto coords = shape_->getVertex(id);

    bool ok = false;
    double newValue = it->text().toDouble(&ok);
    if (!ok) return;

    constexpr double epsilon = 1e-8;
    if (std::abs(coords[col] - newValue) < epsilon)
        return;

    NDShape before = *shape_;
    coords[col] = newValue;
    shape_->setVertexCoords(id, coords);
    NDShape after = *shape_;

    undoStack_->push(new ShapeCommand(
        shape_, before, after, tr("Edit vertex"),
        [this](){ internalReload(); }));
}


void NDShapeEditorDialog::copyVertices()
{
    vertClipboard_.clear();
    auto sel = vertTable_->selectionModel()->selectedRows();
    for (const QModelIndex& idx : sel)
        vertClipboard_.append(shape_->getVertex(rowToId_[idx.row()]));
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
    undoStack_->push(new ShapeCommand(
        shape_, before, after, tr("Paste vertices"),
        [this](){internalReload();}));
}

// ───────────────────────────── adjacency matrix ──
void NDShapeEditorDialog::refreshAdjHeaders()
{
    QStringList hdr;
    for (int i = 0; i < static_cast<int>(rowToId_.size()); ++i)
        hdr << QString::number(i + 1);
    adjTable_->setHorizontalHeaderLabels(hdr);
    adjTable_->setVerticalHeaderLabels(hdr);
}

void NDShapeEditorDialog::rebuildAdjacencyTable()
{
    adjTable_->blockSignals(true);
    adjTable_->clear();

    const int n = rowToId_.size();
    adjTable_->setRowCount(n);
    adjTable_->setColumnCount(n);
    refreshAdjHeaders();

    const auto mat = shape_->getAdjacencyMatrix();

    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < n; ++c) {
            auto* it = new QTableWidgetItem;
            if (r == c) {
                it->setFlags(it->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsSelectable);
                it->setBackground(QBrush(colorUndefined_));
            } else {
                bool connected = (mat[r + 1][c + 1] != 0);
                it->setBackground(QBrush(connected ? colorTrue_ : colorFalse_));
            }
            adjTable_->setItem(r, c, it);
        }
    }

    adjTable_->blockSignals(false);
}

void NDShapeEditorDialog::onAdjCellClicked(int row, int col)
{
    if (row == col) return;              // ignore diagonal

    auto* it    = adjTable_->item(row, col);
    auto* itSym = adjTable_->item(col, row);
    if (!it || !itSym) return;

    NDShape before = *shape_;

    const bool wasOn = (it->background().color() == colorTrue_);
    const bool nowOn = !wasOn;
    const QColor fill = nowOn ? colorTrue_ : colorFalse_;
    it->setBackground(fill);
    itSym->setBackground(fill);

    std::size_t id1 = rowToId_[row];
    std::size_t id2 = rowToId_[col];
    if (nowOn)
        shape_->addEdge(id1, id2);
    else
        shape_->removeEdge(id1, id2);

    NDShape after = *shape_;
    undoStack_->push(new ShapeCommand(
        shape_, before, after, tr("Toggle edge"),
        [this](){internalReload();}));
}

// ───────────────────────────── context menu helpers ──
void NDShapeEditorDialog::showVertContextMenu(const QPoint& pos)
{
    QMenu menu(this);
    menu.addAction(vertActions_["add"]);
    menu.addSeparator();
    menu.addAction(vertActions_["copy"]);
    menu.addAction(vertActions_["cut"]);
    menu.addAction(vertActions_["paste"]);
    menu.addAction(vertActions_["delete"]);
    menu.exec(vertTable_->viewport()->mapToGlobal(pos));
}
