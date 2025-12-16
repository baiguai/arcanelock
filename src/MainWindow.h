#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QTreeView>
#include <QTextEdit>
#include <QLabel>
#include <QStandardItemModel> // For the tree view data
#include <QStackedWidget> // New: For managing stacked widgets

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum class Mode {
        TREE,
        NORMAL,
        INSERT,
        VISUAL // Visual mode not explicitly requested but useful for future selection
    };

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots: // New slot section
    void onTreeSelectionChanged(const QModelIndex &current, const QModelIndex &previous);
    void saveRecord(); // New: Slot to save the edited record
    void newDatabase(); // New: Slot to clear the current database and start a new one
    void saveDatabase(); // New: Slot to save the current database
    void saveDatabaseAs(); // New: Slot to save the current database to a new file

private:
    void setMode(Mode newMode);
    void updateStatusLabel(); // Helper to update the status bar text
    void setupEditableRecordView(); // New: Setup the editable fields in the right panel
    void enterInsertMode(const QModelIndex &index); // New: Enter insert mode for a specific record
    void exitInsertMode(); // New: Exit insert mode
    void saveModelToFile(const QString &filePath); // New: Helper to save the tree model to a file

    // Tree item manipulation methods
    void moveItemToParentOrRoot();
    void moveItemDown();
    void moveItemUp();
    void moveItemIntoSiblingFolder();
    void deleteSelectedItem(); // New: Delete the currently selected tree item
    void expandAllNodes();   // New: Expand all tree view nodes
    void collapseAllNodes(); // New: Collapse all tree view nodes

    QSplitter *m_splitter;
    QTreeView *m_treeView;
    QStackedWidget *m_rightPanelStackedWidget; // Manages read-only and editable views
    QTextEdit *m_recordDisplay; // Read-only display of password record (now a child of m_rightPanelStackedWidget)
    QWidget *m_editableRecordView; // Container for editable fields (now a child of m_rightPanelStackedWidget)
    QLineEdit *m_nameEdit;
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QLineEdit *m_urlEdit;
    QTextEdit *m_notesEdit;
    QLabel *m_statusLabel; // For mode or status display, similar to the reference

    QStandardItemModel *m_treeModel; // Model for the tree view
    Mode m_currentMode; // Current operational mode of the application
    QStandardItem *m_currentEditedItem; // Pointer to the item currently being edited
    QList<int> m_splitterSizes; // Stores the splitter sizes to restore them
    QString m_currentFilePath; // Stores the path of the current database file
};

#endif // MAINWINDOW_H
