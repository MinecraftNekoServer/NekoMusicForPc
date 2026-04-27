#include "mainwindow.h"

#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("NekoMusic"));
    resize(1000, 680);

    // Menu bar
    auto *menuBar = this->menuBar();
    auto *fileMenu = menuBar->addMenu(tr("&File"));
    fileMenu->addAction(tr("&Open"), QKeySequence::Open, this, [this]() {
        // TODO: open file dialog
    });
    fileMenu->addSeparator();
    fileMenu->addAction(tr("&Quit"), QKeySequence::Quit, this, &QWidget::close);

    auto *helpMenu = menuBar->addMenu(tr("&Help"));
    helpMenu->addAction(tr("&About"), this, [this]() {
        // TODO: about dialog
    });

    // Toolbar
    auto *toolbar = addToolBar(tr("Playback"));
    toolbar->addAction(tr("Play"), [this]() {
        // TODO: play
    });
    toolbar->addAction(tr("Pause"), [this]() {
        // TODO: pause
    });

    // Status bar
    statusBar()->showMessage(tr("Ready"));
}

MainWindow::~MainWindow() = default;
