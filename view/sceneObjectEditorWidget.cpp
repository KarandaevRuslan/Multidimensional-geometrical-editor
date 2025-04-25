#include "sceneObjectEditorWidget.h"

#include "../model/projection.h"
#include "../model/scene.h"
#include "NDShapeEditorDialog.h"
#include "axesGroupBox.h"

#include <QFormLayout>
#include <QColorDialog>
#include <QTableWidget>
#include <QHeaderView>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QMessageBox>
#include <QShortcut>
#include <Qmenu>

/* ---------- helpers ---------- */
static QString css(const QColor& c){ return QString("background:%1").arg(c.name()); }

/* ---------- ctor ---------- */
SceneObjectEditorWidget::SceneObjectEditorWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *lay = new QFormLayout(this);

    idLabel_   = new QLabel(this);
    uidLabel_   = new QLabel(this);
    uidTextLabel_ = new QLabel(tr("UID:"), this);
    nameEdit_  = new QLineEdit(this);
    connect(nameEdit_,&QLineEdit::editingFinished,
            this,&SceneObjectEditorWidget::nameEditingFinished);

    colorBtn_  = new QPushButton(tr("Pick…"),this);
    connect(colorBtn_,&QPushButton::clicked,
            this,&SceneObjectEditorWidget::chooseColor);

    projCombo_ = new QComboBox(this);
    projCombo_->addItems({tr("None"), tr("Perspective"), tr("Orthographic"), tr("Stereographic")});
    connect(projCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,&SceneObjectEditorWidget::projectionChanged);

    perspDist_ = new QDoubleSpinBox(this);
    perspDist_->setRange(-1e6,1e6);
    perspDist_->setDecimals(3);
    perspDist_->setVisible(false);
    distanceLabel_ = new QLabel(tr("Distance:"), this);
    distanceLabel_->setVisible(false);
    connect(perspDist_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,&SceneObjectEditorWidget::projectionChanged);

    rotTable_ = new QTableWidget(this);
    rotTable_->setColumnCount(3);
    rotTable_->setHorizontalHeaderLabels({tr("Axis i"), tr("Axis j"), tr("Angle")});
    rotTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    connect(rotTable_, &QTableWidget::cellChanged,
            this, &SceneObjectEditorWidget::rotCellChanged);

    /* --- context-menu support --- */
    rotTable_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(rotTable_, &QWidget::customContextMenuRequested,
            this, &SceneObjectEditorWidget::showRotContextMenu);

    // --------- Create actions and shortcuts ----------
    auto makeAction = [this](const QString& name, const QString& text, const QKeySequence& shortcut, auto slot) {
        QAction* act = new QAction(text, this);
        act->setShortcut(shortcut);
        act->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        connect(act, &QAction::triggered, this, slot);
        rotTable_->addAction(act); // Registers shortcut
        rotatorActions_[name] = act;
    };
    makeAction("add", tr("Add"), QKeySequence::New, &SceneObjectEditorWidget::addRotator);
    makeAction("copy", tr("Copy"), QKeySequence::Copy, &SceneObjectEditorWidget::copyRotator);
    makeAction("cut", tr("Cut"), QKeySequence::Cut, &SceneObjectEditorWidget::cutRotator);
    makeAction("paste", tr("Paste"), QKeySequence::Paste, &SceneObjectEditorWidget::pasteRotator);
    makeAction("delete", tr("Delete"), QKeySequence::Delete, &SceneObjectEditorWidget::deleteRotator);
    makeAction("up", tr("Move Up"), QKeySequence(Qt::CTRL | Qt::Key_Up), &SceneObjectEditorWidget::moveRotatorUp);
    makeAction("down", tr("Move Down"), QKeySequence(Qt::CTRL | Qt::Key_Down), &SceneObjectEditorWidget::moveRotatorDown);

    rotGroupBox_ = new QGroupBox(tr("Rotators"), this);
    QVBoxLayout* rotBoxLayout = new QVBoxLayout(rotGroupBox_);
    rotBoxLayout->setContentsMargins(5,5,5,5);
    rotBoxLayout->addWidget(rotTable_);

    scaleBox_  = new AxesGroupBox(tr("Scale"),  {1,1,1}, this);
    scaleBox_->setRange(0.01, 15);
    offsetBox_ = new AxesGroupBox(tr("Offset"), {0,0,0}, this);
    offsetBox_->setRange(-25, 25);
    connect(scaleBox_,  &AxesGroupBox::valueChanged,
            this, &SceneObjectEditorWidget::scaleChanged);
    connect(offsetBox_, &AxesGroupBox::valueChanged,
            this, &SceneObjectEditorWidget::offsetChanged);

    shapeBtn_ = new QPushButton(tr("Edit shape…"),this);
    connect(shapeBtn_,&QPushButton::clicked,
            this,&SceneObjectEditorWidget::openShapeDialog);


    lay->addRow(tr("ID:"),           idLabel_);
    lay->addRow(uidTextLabel_, uidLabel_);
    lay->addRow(tr("Name:"),         nameEdit_);
    lay->addRow(tr("Color:"),        colorBtn_);
    lay->addRow(rotGroupBox_);
    lay->addRow(tr("Projection:"),   projCombo_);
    lay->addRow(distanceLabel_,      perspDist_);
    lay->addRow(scaleBox_);
    lay->addRow(offsetBox_);
    lay->addRow(shapeBtn_);

    setEnabled(false);
}

/* ---------- public ---------- */
void SceneObjectEditorWidget::setObject(std::shared_ptr<SceneObject>        obj,
                                        std::function<QColor()>             colorGetter)
{
    cur_                  = obj;
    curColorGetter_       = colorGetter;

    rebuildUiFromCurrent();
    setEnabled(bool(cur_));
}

void SceneObjectEditorWidget::clear()
{
    cur_.reset();
    setEnabled(false);
}

/* ---------- populate UI ---------- */
void SceneObjectEditorWidget::rebuildUiFromCurrent()
{
    if(!cur_) return;

    /* trivial fields */
    idLabel_->setText(QString::number(cur_->id));
    uidLabel_->setText(cur_->uid.toString(QUuid::WithoutBraces));
    nameEdit_->setText(cur_->name);
    colorBtn_->setStyleSheet(css(curColorGetter_()));

    /* projection combobox / spin */
    perspDist_->blockSignals(true);
    projCombo_->blockSignals(true);

    if(!cur_->projection){
        projCombo_->setCurrentIndex(0);
        perspDist_->setVisible(false);
        distanceLabel_->setVisible(false);
    }else if(dynamic_cast<PerspectiveProjection*>(cur_->projection.get())){
        projCombo_->setCurrentIndex(1);
        perspDist_->setVisible(true);
        distanceLabel_->setVisible(true);
        auto *pp = static_cast<PerspectiveProjection*>(cur_->projection.get());
        perspDist_->setValue(pp->getDistance());
    }else if(dynamic_cast<OrthographicProjection*>(cur_->projection.get())){
        projCombo_->setCurrentIndex(2);
        perspDist_->setVisible(false);
        distanceLabel_->setVisible(false);
    }else{
        projCombo_->setCurrentIndex(3);
        perspDist_->setVisible(false);
        distanceLabel_->setVisible(false);
    }
    perspDist_->blockSignals(false);
    projCombo_->blockSignals(false);

    /* rotators */
    rotTable_->blockSignals(true);
    rotTable_->clearContents();
    rotTable_->setRowCount(static_cast<int>(cur_->rotators.size()));
    for (int r = 0; r < rotTable_->rowCount(); ++r)
    {
        const Rotator& rot = cur_->rotators[r];
        rotTable_->setItem(r, 0, new QTableWidgetItem(QString::number(rot.axis1())));
        rotTable_->setItem(r, 1, new QTableWidgetItem(QString::number(rot.axis2())));
        rotTable_->setItem(r, 2, new QTableWidgetItem(QString::number(rot.angle())));
    }
    rotTable_->blockSignals(false);

    scaleBox_->blockSignals(true);
    scaleBox_->setValue(cur_->scale);
    scaleBox_->blockSignals(false);

    offsetBox_->blockSignals(true);
    offsetBox_->setValue(cur_->offset);
    offsetBox_->blockSignals(false);
}

/* ---------- commit helper ---------- */
void SceneObjectEditorWidget::commit(bool geometryChanged,
                                     SceneObject& obj, QColor& color)
{
    if(!cur_) return;

    obj.name = nameEdit_->text();

    /* projection */
    switch(projCombo_->currentIndex()){
    case 0: obj.projection.reset(); break;
    case 1: obj.projection = std::make_shared<PerspectiveProjection>(perspDist_->value()); break;
    case 2: obj.projection = std::make_shared<OrthographicProjection>(); break;
    case 3: obj.projection = std::make_shared<StereographicProjection>(); break;
    }

    /* rotators from table */
    obj.rotators.clear();
    for(int r=0;r<rotTable_->rowCount();++r){
        obj.rotators.emplace_back(
            rotTable_->item(r,0)->text().toUInt(),
            rotTable_->item(r,1)->text().toUInt(),
            rotTable_->item(r,2)->text().toDouble());
    }

    obj.scale  = scaleBox_->value();
    obj.offset = offsetBox_->value();

    try{
        Scene::convertObject(obj, 3);
    } catch (const std::exception &ex){
        QString error = QString::fromUtf8(ex.what());
        QMessageBox::warning(this, tr("Invalid scene object"), error);
        rebuildUiFromCurrent();
        return;
    }
    emit objectEdited(obj, color, geometryChanged);
}

/* ---------- slots ---------- */
void SceneObjectEditorWidget::chooseColor()
{
    QColor c = QColorDialog::getColor(curColorGetter_(), this);
    if(!c.isValid() || c == curColorGetter_()) return;
    colorBtn_->setStyleSheet(css(c));

    SceneObject upd = cur_->clone();
    commit(true, upd, c);                       ///< colour change → repaint
}

void SceneObjectEditorWidget::projectionChanged()
{
    int newIndex    = projCombo_->currentIndex();
    double newDist  = perspDist_->value();
    bool sameProj = false;
    if (!cur_->projection && newIndex == 0) {
        sameProj = true;
    } else if (auto *pp = dynamic_cast<PerspectiveProjection*>(cur_->projection.get())) {
        sameProj = (newIndex == 1 && pp->getDistance() == newDist);
    } else if (dynamic_cast<OrthographicProjection*>(cur_->projection.get())) {
        sameProj = (newIndex == 2);
    } else if (dynamic_cast<StereographicProjection*>(cur_->projection.get())) {
        sameProj = (newIndex == 3);
    }
    if (sameProj)
        return;

    SceneObject upd = cur_->clone();
    QColor c = curColorGetter_();
    commit(true, upd, c);                      ///< geometry changed
}

void SceneObjectEditorWidget::nameEditingFinished()
{
    QString newName = nameEdit_->text();
    if (newName == cur_->name)
        return;

    SceneObject upd = cur_->clone();
    QColor c = curColorGetter_();
    commit(false, upd, c);                     ///< no renderer update needed
}

void SceneObjectEditorWidget::rotCellChanged(int row, int col)
{
    if (row < 0 || row >= (int)cur_->rotators.size())
        return;

    const Rotator &oldR = cur_->rotators[row];
    const QTableWidgetItem *it = rotTable_->item(row, col);
    if (!it) return;
    bool changed = false;
    switch (col) {
        case 0:
            changed = (it->text().toUInt() != oldR.axis1());
            break;
        case 1:
            changed = (it->text().toUInt() != oldR.axis2());
            break;
        case 2:
            changed = (it->text().toDouble() != oldR.angle());
            break;
    }
    if (!changed)
        return;

    SceneObject upd = cur_->clone();
    QColor c = curColorGetter_();
    commit(true, upd, c);
}

void SceneObjectEditorWidget::scaleChanged()
{
    if (scaleBox_->value() == cur_->scale)
        return;

    SceneObject upd = cur_->clone();
    QColor c = curColorGetter_();
    commit(true, upd, c);
}

void SceneObjectEditorWidget::offsetChanged()
{
    if (offsetBox_->value() == cur_->offset)
        return;

    SceneObject upd = cur_->clone();
    QColor c = curColorGetter_();
    commit(true, upd, c);
}

void SceneObjectEditorWidget::openShapeDialog()
{
    if(!cur_) return;
    NDShapeEditorDialog dlg(*cur_->shape,this);
    if(dlg.exec() == QDialog::Accepted){
        SceneObject upd = cur_->clone();
        upd.shape = dlg.takeShape();
        QColor c = curColorGetter_();
        commit(true, upd, c);
    }
}

// Rotator
void SceneObjectEditorWidget::addRotator() {
    rotTable_->blockSignals(true);
    int r = rotTable_->rowCount();
    rotTable_->insertRow(r);
    rotTable_->setItem(r, 0, new QTableWidgetItem("0"));
    rotTable_->setItem(r, 1, new QTableWidgetItem("1"));
    rotTable_->setItem(r, 2, new QTableWidgetItem("0.0"));
    rotTable_->blockSignals(false);
    rotTable_->setCurrentCell(r, 0);
    // commit changes
    SceneObject upd = cur_->clone(); QColor c = curColorGetter_();
    commit(true, upd, c);
}

void SceneObjectEditorWidget::copyRotator() {
    rotClipboard_.clear();
    auto sel = rotTable_->selectionModel()->selectedRows();
    for (const QModelIndex &idx : sel) {
        int row = idx.row();
        auto a = rotTable_->item(row, 0)->text().toUInt();
        auto b = rotTable_->item(row, 1)->text().toUInt();
        auto ang = rotTable_->item(row, 2)->text().toDouble();
        rotClipboard_.append(Rotator(a,b,ang));
    }
}

void SceneObjectEditorWidget::cutRotator() {
    copyRotator();
    deleteRotator();
}

void SceneObjectEditorWidget::pasteRotator() {
    if (rotClipboard_.isEmpty())
        return;

    rotTable_->blockSignals(true);
    int r = rotTable_->rowCount();
    for (const Rotator &rot : rotClipboard_) {
        rotTable_->insertRow(r);
        rotTable_->setItem(r, 0, new QTableWidgetItem(QString::number(rot.axis1())));
        rotTable_->setItem(r, 1, new QTableWidgetItem(QString::number(rot.axis2())));
        rotTable_->setItem(r, 2, new QTableWidgetItem(QString::number(rot.angle())));
        r++;
    }
    rotTable_->blockSignals(false);
    if (!rotClipboard_.isEmpty()) rotTable_->setCurrentCell(rotTable_->rowCount()-1, 0);
    SceneObject upd = cur_->clone(); QColor c = curColorGetter_();
    commit(true, upd, c);
}

void SceneObjectEditorWidget::deleteRotator() {
    rotTable_->blockSignals(true);
    auto sel = rotTable_->selectionModel()->selectedRows();
    // remove from bottom to top
    QList<int> rows;
    for (const QModelIndex &idx : sel) rows.append(idx.row());
    if (rows.isEmpty()) {
        rotTable_->blockSignals(false);
        return;
    }
    std::sort(rows.begin(), rows.end(), std::greater<int>());
    for (int row : rows) rotTable_->removeRow(row);

    SceneObject upd = cur_->clone();
    rotTable_->blockSignals(false);

    QColor c = curColorGetter_();
    commit(true, upd, c);
}

void SceneObjectEditorWidget::moveRotatorUp() {
    int row = rotTable_->currentRow();
    if (row <= 0)
        return;

    rotTable_->blockSignals(true);
    for (int col = 0; col < rotTable_->columnCount(); ++col) {
        auto *itemA = rotTable_->item(row,   col);
        auto *itemB = rotTable_->item(row-1, col);

        // 3-step swap of the text
        QString tmp = itemA->text();
        itemA->setText(itemB->text());
        itemB->setText(tmp);
    }
    rotTable_->blockSignals(false);
    rotTable_->setCurrentCell(row-1, rotTable_->currentColumn());

    // commit the change
    SceneObject upd = cur_->clone();
    QColor c = curColorGetter_();
    commit(true, upd, c);
}

void SceneObjectEditorWidget::moveRotatorDown() {
    int row = rotTable_->currentRow();
    if (row < 0 || row >= rotTable_->rowCount() - 1)
        return;

    rotTable_->blockSignals(true);
    for (int col = 0; col < rotTable_->columnCount(); ++col) {
        auto *itemA = rotTable_->item(row,   col);
        auto *itemB = rotTable_->item(row+1, col);

        // 3-step swap of the text
        QString tmp = itemA->text();
        itemA->setText(itemB->text());
        itemB->setText(tmp);
    }
    rotTable_->blockSignals(false);
    rotTable_->setCurrentCell(row+1, rotTable_->currentColumn());

    SceneObject upd = cur_->clone();
    QColor c = curColorGetter_();
    commit(true, upd, c);
}

void SceneObjectEditorWidget::showRotContextMenu(const QPoint &pos)
{
    QMenu menu(this);

    // Use the same actions — they'll show shortcut hints automatically
    menu.addAction(rotatorActions_["add"]);
    menu.addSeparator();
    menu.addAction(rotatorActions_["copy"]);
    menu.addAction(rotatorActions_["cut"]);
    menu.addAction(rotatorActions_["paste"]);
    menu.addAction(rotatorActions_["delete"]);
    menu.addSeparator();
    menu.addAction(rotatorActions_["up"]);
    menu.addAction(rotatorActions_["down"]);

    menu.exec(rotTable_->viewport()->mapToGlobal(pos));
}



