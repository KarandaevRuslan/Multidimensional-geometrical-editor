#include "mainwindow.h"
#include <QApplication>
#include <QKeySequence>
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>

MainWindow::MainWindow()
{
    QMenu *menuWindow = menuBar()->addMenu(tr("&Window"));
    menuWindow->addAction(tr("Quit"), QKeySequence(Qt::CTRL | Qt::Key_Q),
                          qApp, QApplication::closeAllWindows);

}

void MainWindow::onAddNew()
{

}
