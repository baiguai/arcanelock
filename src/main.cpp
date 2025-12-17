#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set a dark stylesheet for the entire application
    app.setStyleSheet(
        "QMainWindow { background-color: #2b2b2b; color: #f2f2f2; }"
        "QDialog { background-color: #2b2b2b; }"
        "QTreeView { background-color: #2b2b2b; color: #f2f2f2; alternate-background-color: #3c3c3c; border: 1px solid #555555; }"
        "QTreeView::item:selected { background-color: #555555; color: #ffffff; }"
        "QTextEdit { background-color: #3c3c3c; color: #f2f2f2; border: 1px solid #555555; }"
        "QLineEdit { background-color: #3c3c3c; color: #f2f2f2; border: 1px solid #555555; padding: 3px 5px; min-height: 24px; }" // Adjusted height and padding
        "QTreeView QLineEdit { min-height: 24px; }" // Ensure this applies to the in-place editor
        "QLabel { color: #f2f2f2; }"
        "QPushButton { background-color: #4a4a4a; color: #f2f2f2; border: 1px solid #555555; padding: 5px; min-width: 70px; }"
        "QPushButton:hover { background-color: #5a5a5a; }"
        "QPushButton:pressed { background-color: #6a6a6a; }"
        "QListWidget { background-color: #2b2b2b; color: #f2f2f2; border: 1px solid #555555; }"
        "QListWidget::item:selected { background-color: #555555; color: #ffffff; }"
        "QSplitter::handle { background-color: #444444; }"
        "QSplitter::handle:hover { background-color: #666666; }"
        "QStatusBar { background-color: #222222; color: #f2f2f2; border-top: 1px solid #555555; }"
        // New styles for QCompleter and its popup
        "QCompleter { background-color: #3c3c3c; color: #f2f2f2; border: 1px solid #555555; }"
        "QCompleter QAbstractItemView { background-color: #2b2b2b; color: #f2f2f2; border: 1px solid #555555; selection-background-color: #555555; selection-color: #ffffff; }"
    );

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}