#include "addSceneObjectDialog.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QColorDialog>
#include <QLabel>
#include <QDialogButtonBox>
#include <QUuid>
#include <QMessageBox>

/* ---------- ctor & UI ---------- */
AddSceneObjectDialog::AddSceneObjectDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Add New Scene Object"));
    buildUi();
}

void AddSceneObjectDialog::buildUi()
{
    auto *mainLay = new QVBoxLayout(this);
    auto *form    = new QFormLayout();
    mainLay->addLayout(form);

    /* -- name -- */
    nameEdit_ = new QLineEdit(this);
    form->addRow(tr("Name:"), nameEdit_);

    /* -- kind + dimension -- */
    QWidget *kindRow = new QWidget(this);
    auto *kindLay = new QHBoxLayout(kindRow);
    kindLay->setContentsMargins(0,0,0,0);
    kindCombo_ = new QComboBox(kindRow);
    kindCombo_->addItems({tr("Empty"), tr("N-Hypercube"), tr("N-Simplex")});
    connect(kindCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AddSceneObjectDialog::kindChanged);
    dimSpin_ = new QSpinBox(kindRow);
    dimSpin_->setRange(3, 20);
    dimSpin_->setValue(4);
    kindLay->addWidget(kindCombo_);
    kindLay->addWidget(dimSpin_);
    form->addRow(tr("Kind:"), kindRow);

    /* -- color -- */
    colorBtn_ = new QPushButton(tr("Pick…"), this);
    colorBtn_->setStyleSheet(css(chosenColor_));
    connect(colorBtn_, &QPushButton::clicked,
            this, &AddSceneObjectDialog::chooseColor);
    form->addRow(tr("Color:"), colorBtn_);

    /* -- projection -- */
    projCombo_ = new QComboBox(this);
    projCombo_->addItems({tr("None"), tr("Perspective"), tr("Orthographic"), tr("Stereographic")});
    projCombo_->setCurrentIndex(1);
    connect(projCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AddSceneObjectDialog::projectionChanged);
    perspDistSpin_ = new QDoubleSpinBox(this);
    perspDistSpin_->setRange(-1e6, 1e6);
    perspDistSpin_->setDecimals(3);
    distanceLbl_ = new QLabel(tr("Distance:"), this);
    distanceLbl_->setVisible(false);
    perspDistSpin_->setVisible(false);

    perspDistSpin_->setValue(6.0);
    projectionChanged(1);
    form->addRow(tr("Projection:"), projCombo_);
    form->addRow(distanceLbl_, perspDistSpin_);

    /* -- OK / Cancel -- */
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                    Qt::Horizontal, this);
    connect(bb, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLay->addWidget(bb);

    kindChanged(kindCombo_->currentIndex());
}

/* ---------- slots ---------- */
void AddSceneObjectDialog::chooseColor()
{
    QColor c = QColorDialog::getColor(chosenColor_, this);
    if (!c.isValid()) return;
    chosenColor_ = c;
    colorBtn_->setStyleSheet(css(chosenColor_));
}

void AddSceneObjectDialog::projectionChanged(int idx)
{
    bool persp = (idx == 1);
    distanceLbl_->setVisible(persp);
    perspDistSpin_->setVisible(persp);
}

void AddSceneObjectDialog::kindChanged(int idx)
{
    bool needsDim = (idx != 0);
    dimSpin_->setEnabled(needsDim);
}

/* ---------- simple getters ---------- */
QString AddSceneObjectDialog::name()        const { return nameEdit_->text(); }
AddSceneObjectDialog::Kind
AddSceneObjectDialog::kind()        const { return Kind(kindCombo_->currentIndex()); }
int     AddSceneObjectDialog::dimension()   const { return dimSpin_->value(); }
QColor  AddSceneObjectDialog::color()       const { return chosenColor_; }

std::shared_ptr<Projection> AddSceneObjectDialog::projection() const
{
    switch (projCombo_->currentIndex()) {
    case 1:  return std::make_shared<PerspectiveProjection>(perspDistSpin_->value());
    case 2:  return std::make_shared<OrthographicProjection>();
    case 3:  return std::make_shared<StereographicProjection>();
    default: return {};
    }
}

/* ---------- shape builders ---------- */
std::shared_ptr<NDShape> AddSceneObjectDialog::buildHypercube(int n) const
{
    auto s = std::make_shared<NDShape>(n);
    std::vector<std::size_t> verts;
    const std::size_t total = 1u << n;
    for (std::size_t i = 0; i < total; ++i) {
        std::vector<double> coord(n);
        for (int bit = 0; bit < n; ++bit)
            coord[bit] = (i & (1u << bit)) ? 1.0 : -1.0;
        verts.push_back(s->addVertex(coord));
    }
    for (std::size_t i = 0; i < total; ++i)
        for (int bit = 0; bit < n; ++bit)
            if (i < (i ^ (1u << bit)))
                s->addEdge(verts[i], verts[i ^ (1u << bit)]);
    return s;
}

std::shared_ptr<NDShape> AddSceneObjectDialog::buildSimplex(int n) const
{
    auto s = std::make_shared<NDShape>(n);
    std::vector<std::size_t> verts;

    for (int i = 0; i <= n; ++i) {
        std::vector<double> v(n, 0.0);
        if (i < n) v[i] = 1.0;
        else       std::fill(v.begin(), v.end(), -1.0);
        verts.push_back(s->addVertex(v));
    }
    for (std::size_t i = 0; i < verts.size(); ++i)
        for (std::size_t j = i+1; j < verts.size(); ++j)
            s->addEdge(verts[i], verts[j]);
    return s;
}

/* ---------- factory for complete SceneObject ---------- */
SceneObject AddSceneObjectDialog::makeSceneObject(int visualId) const
{
    SceneObject obj;
    obj.uid       = QUuid::createUuid();
    obj.id        = visualId;
    obj.name      = this->name();
    obj.shape     = nullptr;

    switch (kind()) {
    case Kind::Hypercube: obj.shape = buildHypercube(dimension()); break;
    case Kind::Simplex:   obj.shape = buildSimplex  (dimension()); break;
    case Kind::Empty:     obj.shape = std::make_shared<NDShape>(dimension()); break;
    }

    obj.projection = this->projection();
    obj.rotators   = {};
    obj.scale      = {3,3,3};
    obj.offset     = {0,0,0};

    return obj;
}

/* ------- accept() override ------- */
void AddSceneObjectDialog::accept()
{
    if (nameEdit_->text().trimmed().isEmpty())
        nameEdit_->setText(defaultName());

    SceneObject probe = makeSceneObject(0);
    try {
        Scene::convertObject(probe, 3);
    } catch (const std::exception& ex) {
        QMessageBox::warning(this, tr("Cannot add object"),
                             tr("This object cannot be projected to 3-D:\n%1")
                                 .arg(QString::fromUtf8(ex.what())));
        return;
    }

    QDialog::accept();
}
