#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set a dark stylesheet for the entire application
    app.setStyleSheet(
        "QMainWindow { background-color: #000; color: #fff; }"
        "QDialog { background-color: #000; }"
        "QTreeView { background-color: #000; color: #fff; alternate-background-color: #000; border: 1px solid #555555; }"
        "QTreeView::item:selected { background-color: #555555; color: #ffffff; }"
        "QTextEdit { background-color: #000; color: #fff; border: 1px solid #555555; }"
        "QLineEdit { background-color: #000; color: #fff; border: 1px solid #555555; padding: 3px 5px; min-height: 24px; }" // Adjusted height and padding
        "QTreeView QLineEdit { min-height: 24px; }" // Ensure this applies to the in-place editor
        "QLabel { color: #f2f2f2; }"
        "QPushButton { background-color: #222; color: #fff; border: 1px solid #555555; padding: 5px; min-width: 70px; }"
        "QPushButton:hover { background-color: #222; }"
        "QPushButton:pressed { background-color: #222; }"
        "QListWidget { background-color: #000; color: #fff; border: 1px solid #555555; }"
        "QListWidget::item:selected { background-color: #555555; color: #ffffff; }"
        "QSplitter::handle { background-color: #333; }"
        "QSplitter::handle:hover { background-color: #666; }"
        "QStatusBar { background-color: #000; color: #ccc; border-top: 1px solid #555555; }"
        // New styles for QCompleter and its popup
        "QCompleter { background-color: #000; color: #f2f2f2; border: 1px solid #555555; }"
        "QCompleter QAbstractItemView { background-color: #000; color: #f2f2f2; border: 1px solid #555555; selection-background-color: #555555; selection-color: #ffffff; }"
    );

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
