#include "sceneObjectListView.h"

#include <QListView>
#include <QKeyEvent>

bool SceneObjectListView::event(QEvent* ev) {
    if (ev->type() == QEvent::ShortcutOverride) {
        auto* ke = static_cast<QKeyEvent*>(ev);
        if ((ke->modifiers() & Qt::ControlModifier) &&
            (ke->modifiers() & Qt::ShiftModifier))
        {
#ifdef Q_OS_WIN
            quint32 vk = ke->nativeVirtualKey();
            if (vk == 0x53 || vk == 0x4C) { // S or L
                ke->accept();             // Block QShortcuts
                return true;              // and do nothing more
            }
#else
            quint32 sc = ke->nativeScanCode();
            if (sc == 31  || sc == 38) {  // ScanCode S or L
                ke->accept();
                return true;
            }
#endif
        }
    }
    return QListView::event(ev);
}

void SceneObjectListView::keyPressEvent(QKeyEvent* ev) {
    if ((ev->modifiers() & Qt::ControlModifier) &&
        (ev->modifiers() & Qt::ShiftModifier))
    {
#ifdef Q_OS_WIN
        quint32 vk = ev->nativeVirtualKey();
        if (vk == 0x53) {             // Ctrl+Shift+S
            emit exportRequested();
            return;
        }
        if (vk == 0x4C) {             // Ctrl+Shift+L
            emit importRequested();
            return;
        }
#else
        quint32 sc = ev->nativeScanCode();
        if (sc == 31) {               // ScanCode S
            emit exportRequested();
            return;
        }
        if (sc == 38) {               // ScanCode L
            emit importRequested();
            return;
        }
#endif
    }
    QListView::keyPressEvent(ev);
}
