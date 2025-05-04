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
#include <numeric>
#include <memory>
#include <cmath>

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
    kindCombo_->addItems({
        tr("Empty"),
        tr("N-Hypercube"),
        tr("N-Simplex"),
        tr("N-Cross polytope"),
        tr("Permutohedron")
    });
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

/* ---------- regular simplex -------------- */
std::shared_ptr<NDShape> AddSceneObjectDialog::buildSimplex(int n) const
{
    if (n <= 0)
        throw std::invalid_argument("dimension must be positive");

    using Vec = std::vector<double>;

    /* ========== 1. raw vertices in (n+1)-space, centred at the origin ======
       Take the (n+1) standard basis vectors e₀ … e_n in ℝⁿ⁺¹
       and subtract their centroid 𝟙/(n+1).  The result is a regular
       simplex lying in the hyper-plane Σxᵢ = 0.                                  */
    const int bigN = n + 1;
    const double centroid = 1.0 / static_cast<double>(bigN);

    std::vector<Vec> raw;           // n+1 raw vertices in ℝⁿ⁺¹
    raw.reserve(bigN);

    for (int i = 0; i < bigN; ++i)
    {
        Vec w(bigN, -centroid);     // start with −centroid
        w[i] += 1.0;                // add basis vector eᵢ
        raw.push_back(std::move(w));
    }

    /* ========== 2. build Householder reflection that sends 𝟙  →  e_{n} ===== */
    Vec ones(bigN, 1.0 / std::sqrt(static_cast<double>(bigN)));   // 𝟙/√(n+1)
    Vec ez(bigN, 0.0);  ez[bigN - 1] = 1.0;                       // last axis

    Vec u(bigN);
    for (int i = 0; i < bigN; ++i) u[i] = ones[i] - ez[i];        // 𝟙̄ – e_{n}
    double uNorm2 = 0.0;
    for (double x : u) uNorm2 += x * x;

    if (uNorm2 > 1e-12)
    {
        const double invNorm = 1.0 / std::sqrt(uNorm2);
        for (double& x : u) x *= invNorm;                         // normalise u
    }
    /* (For n==1, u becomes zero and the reflection degenerates to identity) */

    auto applyHouseholder = [&](const Vec& v) -> Vec
    {
        if (uNorm2 <= 1e-12) return v;                            // n == 1

        double dot = 0.0;
        for (int i = 0; i < bigN; ++i) dot += u[i] * v[i];        // u·v

        Vec res(bigN);
        for (int i = 0; i < bigN; ++i) res[i] = v[i] - 2.0 * dot * u[i];
        return res;
    };

    /* ========== 3. add rotated vertices (first n coords) to NDShape ======= */
    auto shape = std::make_shared<NDShape>(static_cast<std::size_t>(n));
    std::vector<std::size_t> verts;  verts.reserve(bigN);

    for (const Vec& w : raw)
    {
        Vec r = applyHouseholder(w);                  // now lies in x_{n}=0
        r.pop_back();                                 // drop last coord → ℝⁿ

        verts.push_back(shape->addVertex(r));         // store vertex
    }

    /* ========== 4. connect every pair of vertices (complete graph) ========= */
    for (std::size_t i = 0; i < verts.size(); ++i)
        for (std::size_t j = i + 1; j < verts.size(); ++j)
            shape->addEdge(verts[i], verts[j]);

    return shape;
}

/* ---------- cross-polytope ------------- */
std::shared_ptr<NDShape> AddSceneObjectDialog::buildCrossPolytope(int n) const
{
    auto shp = std::make_shared<NDShape>(n);
    std::vector<std::size_t> idx;

    // vertices ±e_i
    for (int i = 0; i < n; ++i) {
        std::vector<double> v(n, 0.0);
        v[i] =  1.0; idx.push_back(shp->addVertex(v));
        v[i] = -1.0; idx.push_back(shp->addVertex(v));
    }

    // edges: vertices on different axes
    for (std::size_t a = 0; a < idx.size(); ++a)
        for (std::size_t b = a + 1; b < idx.size(); ++b)
            if (a / 2 != b / 2)
                shp->addEdge(idx[a], idx[b]);

    return shp;
}

/* ---------- permutohedron -------------- */
/* ---------- permutohedron -------------- */
std::shared_ptr<NDShape> AddSceneObjectDialog::buildPermutohedron(int n) const
{
    if (n <= 0)
        throw std::invalid_argument("dimension must be positive");

    using Perm = std::vector<int>;

    auto encode = [](const Perm& p) -> std::string
    {
        std::string out;
        out.reserve(p.size() * 3);
        for (std::size_t i = 0; i < p.size(); ++i)
        {
            out += std::to_string(p[i]);
            if (i + 1 < p.size()) out.push_back(',');
        }
        return out;
    };

    /* ---------- 1.  enumerate permutations and store raw vertices --------- */
    const double shift = 0.5 * static_cast<double>(n + 1);   // centre at origin

    std::vector<Perm>               permutations;
    std::vector<std::vector<double>> raw;                    // temporary store

    Perm perm(n);
    std::iota(perm.begin(), perm.end(), 1);                  // 1 2 … n
    do
    {
        std::vector<double> v(n);
        for (int i = 0; i < n; ++i)
            v[i] = static_cast<double>(perm[i]) - shift;

        permutations.push_back(perm);
        raw.push_back(std::move(v));

    } while (std::next_permutation(perm.begin(), perm.end()));

    /* ---------- 2.  prepare Householder reflection H = I - 2 u uᵀ -------- */
    // ones̄  = (1,1,…,1)/√n
    const double invSqrtN = 1.0 / std::sqrt(static_cast<double>(n));

    std::vector<double> u(n);
    for (int i = 0; i < n; ++i)
        u[i] = invSqrtN - (i == n - 1 ? 1.0 : 0.0);          // ones̄ - eₙ

    // If u is the zero vector (only when n==1), skip the reflection
    double uNorm2 = 0.0;
    for (double x : u) uNorm2 += x * x;

    if (uNorm2 > 1e-12)
    {
        const double invUNorm = 1.0 / std::sqrt(uNorm2);
        for (double& x : u) x *= invUNorm;                   // normalize u
    }
    // else: keep u ≡ 0, so H = I (n==1 case)

    auto applyHouseholder = [&](const std::vector<double>& v) -> std::vector<double>
    {
        if (uNorm2 <= 1e-12) return v;                       // no-op for n==1

        double dot = 0.0;
        for (int i = 0; i < n; ++i) dot += u[i] * v[i];      // u·v

        std::vector<double> res(n);
        for (int i = 0; i < n; ++i)
            res[i] = v[i] - 2.0 * dot * u[i];                // v - 2(u·v)u

        return res;
    };

    /* ---------- 3.  add rotated vertices to NDShape ----------------------- */
    auto shape = std::make_shared<NDShape>(static_cast<std::size_t>(n));
    std::unordered_map<std::string, std::size_t> vertexIdOf;   // perm → id
    vertexIdOf.reserve(raw.size());

    for (std::size_t k = 0; k < raw.size(); ++k)
    {
        const std::vector<double> coords = applyHouseholder(raw[k]);
        const std::size_t id = shape->addVertex(coords);
        vertexIdOf.emplace(encode(permutations[k]), id);
    }

    /* ---------- 4.  connect permutations differing by one adjacent swap --- */
    for (const Perm& base : permutations)
    {
        const std::size_t idA = vertexIdOf[encode(base)];

        for (int i = 0; i < n - 1; ++i)
            for (int j = i + 1; j < n; ++j)
            {
                if (std::abs(base[i] - base[j]) != 1) continue;

                Perm neigh = base;
                std::swap(neigh[i], neigh[j]);

                if (auto it = vertexIdOf.find(encode(neigh)); it != vertexIdOf.end())
                {
                    const std::size_t idB = it->second;
                    if (idA < idB) shape->addEdge(idA, idB); // keep “add once” rule
                }
            }
    }

    return shape;
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
        case Kind::Hypercube:      obj.shape = buildHypercube     (dimension()); break;
        case Kind::Simplex:        obj.shape = buildSimplex       (dimension()); break;
        case Kind::CrossPolytope:  obj.shape = buildCrossPolytope (dimension()); break;
        case Kind::Permutohedron:  obj.shape = buildPermutohedron (dimension()); break;
        case Kind::Empty:          obj.shape = std::make_shared<NDShape>(dimension()); break;
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
