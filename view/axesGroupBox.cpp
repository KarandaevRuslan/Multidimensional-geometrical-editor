#include "axesGroupBox.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>

AxesGroupBox::AxesGroupBox(const QString& t,
                           const std::vector<double>& start,
                           QWidget* parent)
    : QGroupBox(t, parent)
{
    auto *outer = new QVBoxLayout;
    setLayout(outer);

    for (double d : start)
    {
        auto *row  = new QWidget(this);
        auto *hbox = new QHBoxLayout(row); hbox->setContentsMargins(0,0,0,0);

        auto *sl = new QSlider(Qt::Horizontal,row);
        sl->setTracking(true);              // emit while dragging
        auto *sp = new QDoubleSpinBox(row);
        sp->setDecimals(3);
        sp->setValue(d);

        sliders_.push_back(sl);
        spins_.push_back(sp);

        hbox->addWidget(sl);
        hbox->addWidget(sp);
        outer->addWidget(row);

        /* -------- two-way binding -------- */
        connect(sl, &QSlider::valueChanged, sp,
                [sp](int v){ sp->setValue(v/100.0); });
        connect(sp, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                sl, [sl](double v){ sl->setValue(int(v*100)); });

        /* -------- propagate change -------- */
        connect(sl, &QSlider::valueChanged,
                this, &AxesGroupBox::valueChanged);
        connect(sp, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &AxesGroupBox::valueChanged);
    }
}

void AxesGroupBox::setRange(double mn, double mx)
{
    for (auto *sl : sliders_) {
        sl->setRange(int(mn*100), int(mx*100));
    }
    for (auto *sp : spins_) {
        sp->setRange(mn,mx);
    }
}

std::vector<double> AxesGroupBox::value() const
{
    std::vector<double> v;
    for (auto *sp : spins_) v.push_back(sp->value());
    return v;
}
void AxesGroupBox::setValue(const std::vector<double>& v)
{
    if (v.empty()) {
        for (auto* sp : spins_)
            sp->setValue(0.0);
        return;
    }

    for (size_t i=0;i<v.size() && i<spins_.size(); ++i)
        spins_[i]->setValue(v[i]);
}
