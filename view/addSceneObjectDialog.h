#ifndef ADD_SCENE_OBJECT_DIALOG_H
#define ADD_SCENE_OBJECT_DIALOG_H

#include <QDialog>
#include <QColor>
#include <QComboBox>
#include <memory>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>

#include "../model/scene.h"
#include "../model/sceneColorificator.h"
#include "../model/NDShape.h"
#include "../model/projection.h"

class AddSceneObjectDialog : public QDialog
{
    Q_OBJECT
public:
    enum class Kind { Empty, Hypercube, Simplex };

    explicit AddSceneObjectDialog(QWidget *parent = nullptr);

    /* ------------ getters after exec() == Accepted ------------ */
    QString         name()        const;
    Kind            kind()        const;
    int             dimension()   const;     ///< Only meaningful for Hypercube/Simplex
    QColor          color()       const;
    std::shared_ptr<Projection> projection() const;

    /**
     * @brief Convenience helper: build a fully-initialised SceneObject
     *        (rotators = {}, scale = {3,3,3}, offset = {0,0,0}).
     *
     * @param visualId  visual-only integer id (you usually keep a counter in the model)
     */
    SceneObject makeSceneObject(int visualId) const;

private slots:
    void chooseColor();
    void projectionChanged(int idx);

protected:
    void accept() override;

private:
    /* ------------ widgets ------------ */
    QLineEdit   *nameEdit_;
    QComboBox   *kindCombo_;
    QSpinBox    *dimSpin_;
    QPushButton *colorBtn_;
    QComboBox   *projCombo_;
    QDoubleSpinBox *perspDistSpin_;
    QLabel      *distanceLbl_;

    QColor chosenColor_{SceneColorificator::defaultColor};

    /* helpers */
    void buildUi();
    QString css(const QColor &c) const
    { return QStringLiteral("background:%1").arg(c.name()); }
    QString defaultName() const  { return tr("Innominate"); }

    /* internal builders for shapes */
    std::shared_ptr<NDShape> buildHypercube(int n) const;
    std::shared_ptr<NDShape> buildSimplex  (int n) const;
};

#endif // ADD_SCENE_OBJECT_DIALOG_H
