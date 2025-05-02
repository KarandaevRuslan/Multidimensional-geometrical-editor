#ifndef SCENE_OBJECT_LIST_VIEW_H
#define SCENE_OBJECT_LIST_VIEW_H

#include <QListView>
#include <QKeyEvent>

class SceneObjectListView : public QListView {
    Q_OBJECT
public:
    explicit SceneObjectListView(QWidget* parent = nullptr)
        : QListView(parent)
    {}

signals:
    void exportRequested();
    void importRequested();

protected:

    bool event(QEvent* ev) override;

    void keyPressEvent(QKeyEvent* ev) override;
};

#endif // SCENE_OBJECT_LIST_VIEW_H
