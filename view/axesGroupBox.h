#ifndef AXES_GROUP_BOX_H
#define AXES_GROUP_BOX_H

#include <QHBoxLayout>
#include <QWidget>
#include <vector>

#include <QGroupBox>
#include <vector>

class QSlider;
class QDoubleSpinBox;

/**
 * @brief A vertical stack of ( slider + spin ) rows that edits
 *        a 1-D vector (Scale or Offset).
 *
 *  • `valueChanged()` is emitted *continuously* while the user drags
 *    any slider or edits any spin-box.
 *  • Use `setRange(min,max)` to configure all coordinates.
 */
class AxesGroupBox : public QGroupBox
{
    Q_OBJECT
public:
    AxesGroupBox(const QString& title,
                 const std::vector<double>& start,
                 QWidget* parent = nullptr);

    void setRange(double mn, double mx);          ///< min/max for every axis
    std::vector<double> value() const;            ///< current vector
    void setValue(const std::vector<double>& v);  ///< update from outside

signals:
    void valueChanged();                          ///< fired on every user edit

private:
    std::vector<QSlider*>       sliders_;
    std::vector<QDoubleSpinBox*> spins_;
};

#endif // AXES_GROUP_BOX_H
