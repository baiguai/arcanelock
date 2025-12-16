#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set a dark stylesheet for the entire application
    app.setStyleSheet(
        "QMainWindow { background-color: #2b2b2b; color: #f2f2f2; }"
        "QTreeView { background-color: #2b2b2b; color: #f2f2f2; alternate-background-color: #3c3c3c; }"
        "QTreeView::item:selected { background-color: #555555; color: #ffffff; }"
        "QTextEdit { background-color: #2b2b2b; color: #f2f2f2; border: 1px solid #555555; }"
        "QLabel { color: #f2f2f2; }"
        "QSplitter::handle { background-color: #444444; }"
        "QSplitter::handle:hover { background-color: #666666; }"
        "QStatusBar { background-color: #222222; color: #f2f2f2; border-top: 1px solid #555555; }"
    );

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}