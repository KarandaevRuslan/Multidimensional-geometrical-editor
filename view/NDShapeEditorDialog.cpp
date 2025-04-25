/* NDShapeEditorDialog.cpp */
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

namespace {
    constexpr int   kDimMin = 3;
    constexpr int   kDimMax = 20;
    constexpr const char* kFloatFmt = "%.6g";
} // unnamed

namespace {
    class NoHoverDelegate final : public QStyledItemDelegate {
    public:
        using QStyledItemDelegate::QStyledItemDelegate;

        void paint(QPainter* painter,
                   const QStyleOptionViewItem& option,
                   const QModelIndex& index) const override
        {
            // copy the option and clear hover/focus bits
            QStyleOptionViewItem opt(option);
            opt.state &= ~QStyle::State_MouseOver;
            opt.state &= ~QStyle::State_HasFocus;
            QStyledItemDelegate::paint(painter, opt, index);
        }
    };
}

// ──────────────────────────────────────────────────────────────── ctor ──
NDShapeEditorDialog::NDShapeEditorDialog(const NDShape& startShape,
                                         QWidget* parent)
    : QDialog(parent)
    , shape_(std::make_shared<NDShape>(startShape))
{
    setWindowTitle(tr("Shape editor"));
    resize(720, 600);

    // Main layout with generous margins
    auto* mainBox = new QVBoxLayout(this);
    mainBox->setContentsMargins(10, 6, 10, 10);
    mainBox->setSpacing(8);

    /* ── Dimension ─────────────────────────────────────────────── */
    auto* dimHBox = new QHBoxLayout;
    dimHBox->setContentsMargins(3, 4, 3, 0);
    // ensure spacing/padding between label and spinbox
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

    // Vertices page
    auto* vertPage = new QWidget(this);
    auto* vertVBox = new QVBoxLayout(vertPage);
    vertVBox->setContentsMargins(6, 5, 6, 5);
    vertVBox->setSpacing(8);
    vertTable_ = new QTableWidget(vertPage);
    vertTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    vertTable_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    vertTable_->horizontalHeader()
        ->setSectionResizeMode(QHeaderView::ResizeToContents);
    vertTable_->resizeColumnsToContents();
    vertTable_->resizeRowsToContents();
    vertTable_->horizontalHeader()
        ->setSectionResizeMode(QHeaderView::Interactive);
    vertTable_->verticalHeader()->setVisible(true);
    vertTable_->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    vertTable_->verticalHeader()
        ->setSectionResizeMode(QHeaderView::Interactive);

    vertTable_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(vertTable_, &QWidget::customContextMenuRequested,
            this,       &NDShapeEditorDialog::showVertContextMenu);

    auto makeAction = [this](const QString& name,
                             const QString& text,
                             const QKeySequence& shortcut,
                             auto slotPtr)
    {
        QAction* act = new QAction(text, this);
        act->setShortcut(shortcut);
        act->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        connect(act, &QAction::triggered, this, slotPtr);
        vertTable_->addAction(act);            // register both context-menu and shortcut
        vertActions_[name] = act;
    };

    makeAction("add",    tr("Add"),        QKeySequence::New,           &NDShapeEditorDialog::addVertex);
    makeAction("copy",   tr("Copy"),       QKeySequence::Copy,          &NDShapeEditorDialog::copyVertices);
    makeAction("cut",    tr("Cut"),        QKeySequence::Cut,           &NDShapeEditorDialog::cutVertices);
    makeAction("paste",  tr("Paste"),      QKeySequence::Paste,         &NDShapeEditorDialog::pasteVertices);
    makeAction("delete", tr("Delete"),     QKeySequence::Delete,        &NDShapeEditorDialog::removeVertices);

    vertVBox->addWidget(vertTable_);

    // Adjacency page
    auto* adjPage = new QWidget(this);
    auto* adjVBox = new QVBoxLayout(adjPage);
    adjVBox->setContentsMargins(8, 6, 8, 6);
    adjVBox->setSpacing(6);
    adjTable_ = new QTableWidget(adjPage);
    adjTable_->setSelectionMode(QAbstractItemView::NoSelection);
    adjTable_->horizontalHeader()
        ->setSectionResizeMode(QHeaderView::ResizeToContents);
    adjTable_->verticalHeader()
        ->setSectionResizeMode(QHeaderView::ResizeToContents);
    adjTable_->horizontalHeader()->setDefaultSectionSize(24);
    adjTable_->verticalHeader()->setDefaultSectionSize(20);
    adjTable_->horizontalHeader()
        ->setSectionResizeMode(QHeaderView::Interactive);
    adjTable_->verticalHeader()
        ->setSectionResizeMode(QHeaderView::Interactive);

    adjTable_->setItemDelegate(new NoHoverDelegate(adjTable_));

    adjVBox->addWidget(adjTable_);

    tableTabs->addTab(vertPage, tr("Vertices"));
    tableTabs->addTab(adjPage,  tr("Edges"));

    /* ── OK / Cancel ──────────────────────────────────────────────── */
    auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                            QDialogButtonBox::Cancel,
                                        this);
    mainBox->addWidget(btnBox);

    /* ── signals ─────────────────────────────────────────────────── */
    connect(dimSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this,       &NDShapeEditorDialog::onDimensionChanged);
    connect(vertTable_, &QTableWidget::itemChanged,
            this,       &NDShapeEditorDialog::onVertexTableItemChanged);
    connect(adjTable_, &QTableWidget::cellClicked,
            this,       &NDShapeEditorDialog::onAdjCellClicked);
    connect(btnBox,  &QDialogButtonBox::accepted,
            this,    &NDShapeEditorDialog::accept);
    connect(btnBox,  &QDialogButtonBox::rejected,
            this,    &NDShapeEditorDialog::reject);

    /* ── initial fill ─────────────────────────────────────────────── */
    rebuildVertexTable();
    rebuildAdjacencyTable();
}

// ─────────────────────────────────── dimension changed ──
void NDShapeEditorDialog::onDimensionChanged(int d)
{
    if (std::size_t(d) == shape_->getDimension())
        return;

    // Clone to new dimension, keep IDs/edges
    *shape_ = shape_->clone(std::size_t(d));

    rebuildVertexTable();
    rebuildAdjacencyTable();
}

// ─────────────────────────────────── vertices ──
void NDShapeEditorDialog::refreshVertexVerticalHeader()
{
    QStringList hdr;
    for (std::size_t id : rowToId_)
        hdr << QString::number(id);
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
        hdr << tr("x%1").arg(c+1);
    vertTable_->setHorizontalHeaderLabels(hdr);

    rowToId_.resize(rows);

    for (int r = 0; r < rows; ++r)
    {
        const auto id     = std::size_t(verts[r].first);
        const auto& coord = verts[r].second;
        rowToId_[r] = id;

        for (int c = 0; c < cols; ++c)
        {
            auto* it = new QTableWidgetItem(QString::asprintf(kFloatFmt, coord[c]));
            vertTable_->setItem(r, c, it);
        }
    }

    refreshVertexVerticalHeader();

    vertTable_->blockSignals(false);
}

void NDShapeEditorDialog::addVertex()
{
    const std::size_t newId = shape_->addVertex(
        std::vector<double>(shape_->getDimension(), 0.0));

    rebuildVertexTable();
    rebuildAdjacencyTable();
}

void NDShapeEditorDialog::removeVertices()
{
    auto sel = vertTable_->selectionModel()->selectedRows();
    if (sel.isEmpty()) return;

    std::vector<std::size_t> ids;
    for (const QModelIndex& idx : sel)
        ids.push_back(rowToId_[idx.row()]);

    std::sort(ids.begin(), ids.end(), std::greater<>());

    // remove each
    for (auto id : ids)
        shape_->removeVertex(id);

    // rebuild tables
    rebuildVertexTable();
    rebuildAdjacencyTable();
}

void NDShapeEditorDialog::onVertexTableItemChanged(QTableWidgetItem* it)
{
    if (!it)
        return;

    const int row = it->row();
    const int col = it->column();
    const std::size_t id = rowToId_[row];

    auto coords = shape_->getVertex(id);
    coords[col] = it->text().toDouble();
    shape_->setVertexCoords(id, coords);
}

// ─────────────────────────────────── adjacency ──
void NDShapeEditorDialog::refreshAdjHeaders()
{
    QStringList hdr;
    for (std::size_t id : rowToId_)
        hdr << QString::number(id);
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
        it->setText("");
        if (r == c) {
            it->setFlags(it->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsSelectable);
            it->setBackground(QBrush(colorUndefined_));
        } else {
            it->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            bool connected = (mat[r + 1][c + 1] != 0);
            const QColor fill = connected ? colorTrue_ : colorFalse_;
            it->setBackground(QBrush(fill));
        }
        adjTable_->setItem(r, c, it);
    }}
    adjTable_->blockSignals(false);
}

void NDShapeEditorDialog::onAdjCellClicked(int row, int col) {
    // ignore diagonal
    if (row == col) return;

    // get both items
    auto* it     = adjTable_->item(row, col);
    auto* itSym  = adjTable_->item(col, row);
    if (!it || !itSym) return;

    // determine new state
    const bool wasGreen = (it->background().color() == colorTrue_);
    const bool newState  = !wasGreen;
    const QColor fill    = newState ? colorTrue_ : colorFalse_;
    QBrush brush(fill);

    // apply to both
    it->setBackground(brush);
    itSym->setBackground(brush);

    std::size_t id1 = rowToId_[row];
    std::size_t id2 = rowToId_[col];
    if (newState) {
        shape_->addEdge(id1, id2);
    } else {
        shape_->removeEdge(id1, id2);
    }
}

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

void NDShapeEditorDialog::copyVertices()
{
    vertClipboard_.clear();
    auto sel = vertTable_->selectionModel()->selectedRows();
    for (const QModelIndex& idx : sel) {
        std::size_t id = rowToId_[idx.row()];
        vertClipboard_.append(shape_->getVertex(id));
    }
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

    const std::size_t dim = shape_->getDimension();
    for (const auto& oldCoords : vertClipboard_) {
        std::vector<double> newCoords(dim, 0.0);
        const std::size_t copyCount = std::min(dim, oldCoords.size());
        for (std::size_t i = 0; i < copyCount; ++i)
            newCoords[i] = oldCoords[i];
        shape_->addVertex(newCoords);
    }

    rebuildVertexTable();
    rebuildAdjacencyTable();
}
