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
#include <stdexcept>

namespace {
    constexpr int   kDimMin = 3;
    constexpr int   kDimMax = 20;
    constexpr const char* kFloatFmt = "%.6g";
} // unnamed

// ──────────────────────────────────────────────────────────────── ctor ──
NDShapeEditorDialog::NDShapeEditorDialog(const NDShape& startShape,
                                         QWidget* parent)
    : QDialog(parent)
    , shape_(std::make_shared<NDShape>(startShape))
{
    setWindowTitle(tr("Shape editor"));
    resize(640, 480);

    auto* mainBox = new QVBoxLayout(this);

    /* ── Dimension group ───────────────────────────────────────────── */
    auto* dimGrp   = new QGroupBox(tr("Dimension"), this);
    auto* dimHBox  = new QHBoxLayout(dimGrp);
    dimHBox->addWidget(new QLabel(tr("Number of coordinates:"), dimGrp));

    dimSpin_ = new QSpinBox(dimGrp);
    dimSpin_->setRange(kDimMin, kDimMax);
    dimSpin_->setValue(int(shape_->getDimension()));
    dimHBox->addWidget(dimSpin_);
    mainBox->addWidget(dimGrp);

    /* ── Vertex group ─────────────────────────────────────────────── */
    auto* vertGrp  = new QGroupBox(tr("Vertices"), this);
    auto* vertVBox = new QVBoxLayout(vertGrp);

    vertTable_  = new QTableWidget(vertGrp);
    vertTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    vertTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    vertTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    vertTable_->verticalHeader()->setVisible(false);
    vertVBox->addWidget(vertTable_);

    auto* btnHBox = new QHBoxLayout;
    addVertexBtn_ = new QToolButton(vertGrp);
    addVertexBtn_->setText(tr("Add vertex"));
    delVertexBtn_ = new QToolButton(vertGrp);
    delVertexBtn_->setText(tr("Remove selected"));
    btnHBox->addWidget(addVertexBtn_);
    btnHBox->addWidget(delVertexBtn_);
    btnHBox->addStretch();
    vertVBox->addLayout(btnHBox);
    mainBox->addWidget(vertGrp);

    /* ── Adjacency group ──────────────────────────────────────────── */
    auto* adjGrp  = new QGroupBox(tr("Adjacency matrix"), this);
    auto* adjVBox = new QVBoxLayout(adjGrp);

    adjTable_ = new QTableWidget(adjGrp);
    adjTable_->setSelectionMode(QAbstractItemView::NoSelection);
    adjTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    adjVBox->addWidget(adjTable_);
    mainBox->addWidget(adjGrp);

    /* ── OK / Cancel ──────────────────────────────────────────────── */
    auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                            QDialogButtonBox::Cancel, this);
    mainBox->addWidget(btnBox);

    /* ── signals ─────────────────────────────────────────────────── */
    connect(dimSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this,       &NDShapeEditorDialog::onDimensionChanged);
    connect(addVertexBtn_, &QToolButton::clicked,
            this,          &NDShapeEditorDialog::onAddVertex);
    connect(delVertexBtn_, &QToolButton::clicked,
            this,          &NDShapeEditorDialog::onRemoveSelectedVertex);
    connect(vertTable_, &QTableWidget::itemChanged,
            this,       &NDShapeEditorDialog::onVertexTableItemChanged);
    connect(adjTable_,  &QTableWidget::itemChanged,
            this,       &NDShapeEditorDialog::onAdjItemChanged);
    connect(btnBox,  &QDialogButtonBox::accepted,
            this,    [this](){ writeBackIntoShape(); accept(); });
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
void NDShapeEditorDialog::rebuildVertexTable()
{
    vertTable_->blockSignals(true);
    vertTable_->clear();

    const auto verts = shape_->getAllVertices();
    const int  rows  = int(verts.size());
    const int  cols  = int(shape_->getDimension());

    vertTable_->setRowCount(rows);
    vertTable_->setColumnCount(cols + 1);           // +1 for ID column

    // header labels
    QStringList hdr;
    hdr << tr("ID");
    for (int c = 0; c < cols; ++c)
        hdr << tr("x%1").arg(c + 1);
    vertTable_->setHorizontalHeaderLabels(hdr);

    rowToId_.resize(rows);

    for (int r = 0; r < rows; ++r)
    {
        const auto id     = std::size_t(verts[r].first);
        const auto& coord = verts[r].second;
        rowToId_[r] = id;

        auto* idItem = new QTableWidgetItem(QString::number(id));
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);
        vertTable_->setItem(r, 0, idItem);

        for (int c = 0; c < cols; ++c)
        {
            auto* it = new QTableWidgetItem(QString::asprintf(kFloatFmt, coord[c]));
            vertTable_->setItem(r, c + 1, it);
        }
    }

    vertTable_->blockSignals(false);
}

void NDShapeEditorDialog::onAddVertex()
{
    const std::size_t newId = shape_->addVertex(
        std::vector<double>(shape_->getDimension(), 0.0));

    rowToId_.append(newId);
    rebuildVertexTable();
    rebuildAdjacencyTable();
}

void NDShapeEditorDialog::onRemoveSelectedVertex()
{
    const int row = vertTable_->currentRow();
    if (row < 0 || row >= vertTable_->rowCount())
        return;

    const std::size_t id = rowToId_[row];
    shape_->removeVertex(id);

    rowToId_.removeAt(row);
    rebuildVertexTable();
    rebuildAdjacencyTable();
}

void NDShapeEditorDialog::onVertexTableItemChanged(QTableWidgetItem* it)
{
    if (!it || it->column() == 0)   // ID column uneditable
        return;

    const int row = it->row();
    const int col = it->column() - 1;  // skip ID column
    const std::size_t id = rowToId_.value(row);

    auto coords = shape_->getAllVertices()[row].second; // copy old
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

    for (int r = 0; r <= n; ++r)
    {
        for (int c = 0; c <= n; ++c)
        {
            auto* it = new QTableWidgetItem;
            if (r == c)
            {
                it->setText("--");
                it->setFlags(it->flags() & ~Qt::ItemIsEditable);
            }
            else
            {
                const auto id1 = rowToId_[r];
                const auto id2 = rowToId_[c];
                const int  val = (id1 < mat.size() && id2 < mat.size())
                                    ? mat[id1][id2]
                                    : 0;
                it->setText(QString::number(val));
            }
            adjTable_->setItem(r, c, it);
        }
    }
    adjTable_->blockSignals(false);
}

void NDShapeEditorDialog::onAdjItemChanged(QTableWidgetItem* it)
{
    if (!it) return;
    const QString t = it->text().trimmed();
    if (t == "0" || t == "1") return;          // ok

    it->setText("0");                         // fallback to 0
}

// ─────────────────────────────────── write-back ──
void NDShapeEditorDialog::writeBackIntoShape()
{
    // vertices already synchronised on each edit

    // adjacency – build raw matrix (without headers) row-major by ID
    const int n = rowToId_.size();
    std::vector<std::vector<int>> mat(n, std::vector<int>(n, 0));

    for (int r = 0; r < n; ++r)
    {
        for (int c = r + 1; c < n; ++c)
        {
            mat[rowToId_[r]][rowToId_[c]] = adjTable_->item(r, c)->text().toInt();
        }
    }
    shape_->updateFromAdjacencyMatrix(mat);
}
