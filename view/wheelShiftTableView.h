#ifndef WHEEL_SHIFT_TABLE_VIEW_H
#define WHEEL_SHIFT_TABLE_VIEW_H

#include <QTableView>
#include <QWheelEvent>
#include <QScrollBar>

class WheelShiftTableView : public QTableView
{
    Q_OBJECT
public:
    using QTableView::QTableView;

protected:
    void wheelEvent(QWheelEvent *ev) override;
private:
    void ensurePerPixel();
    int rest_ = 0;
};


#endif // WHEEL_SHIFT_TABLE_VIEW_H
