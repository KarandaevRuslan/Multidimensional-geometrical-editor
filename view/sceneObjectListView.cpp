#include "sceneObjectListView.h"

#include <QListView>
#include <QKeyEvent>
#include <QProcessEnvironment>
#include <QLatin1String>

bool SceneObjectListView::event(QEvent* ev) {
    if (ev->type() == QEvent::ShortcutOverride) {
        auto* ke = static_cast<QKeyEvent*>(ev);
        if ((ke->modifiers() & Qt::ControlModifier) &&
            (ke->modifiers() & Qt::ShiftModifier))
        {
#ifdef Q_OS_WIN
            quint32 vk = ke->nativeVirtualKey();
            if (vk == 0x53 || vk == 0x4C) { // 'S' or 'L'
                ke->accept();
                return true;
            }
#elif defined(Q_OS_LINUX)
            static const bool isWayland = (
                QProcessEnvironment::systemEnvironment()
                    .value("XDG_SESSION_TYPE", QString())
                    .compare("wayland", Qt::CaseInsensitive) == 0
                );
            quint32 sc = ke->nativeScanCode();
            if (isWayland) {
                // evdev: S=31, L=38
                if (sc == 31 || sc == 38) {
                    ke->accept();
                    return true;
                }
            } else {
                // X11: kernel+8 → S=39, L=46
                if (sc == 39 || sc == 46) {
                    ke->accept();
                    return true;
                }
            }
#elif defined(Q_OS_MAC)
            quint32 sc = ke->nativeScanCode();
            // macOS virtual keycodes: S=0x01, L=0x25
            if (sc == 0x01 || sc == 0x25) {
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
#elif defined(Q_OS_LINUX)
        static const bool isWayland = (
            QProcessEnvironment::systemEnvironment()
                .value("XDG_SESSION_TYPE", QString())
                .compare("wayland", Qt::CaseInsensitive) == 0
            );
        quint32 sc = ev->nativeScanCode();
        if (isWayland) {
            // evdev
            if (sc == 31) {           // S
                emit exportRequested();
                return;
            }
            if (sc == 38) {           // L
                emit importRequested();
                return;
            }
        } else {
            // X11 (kernel+8)
            if (sc == 39) {           // S
                emit exportRequested();
                return;
            }
            if (sc == 46) {           // L
                emit importRequested();
                return;
            }
        }
#elif defined(Q_OS_MAC)
        quint32 sc = ev->nativeScanCode();
        if (sc == 0x01) {             // macOS S
            emit exportRequested();
            return;
        }
        if (sc == 0x25) {             // macOS L
            emit importRequested();
            return;
        }
#endif
    }

    QListView::keyPressEvent(ev);
}
