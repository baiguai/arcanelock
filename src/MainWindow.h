#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QTreeView>
#include <QTextBrowser>
#include <QTextEdit>
#include <QLabel>
#include <QStandardItemModel> // For the tree view data
#include <QStackedWidget> // New: For managing stacked widgets
#include <QCompleter>
#include <QStringList> // Required for recent files list

#include "OpenDbDialog.h" // The new dialog for opening files

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
    void openDatabase(); // New: Slot to open a database
    void createFolder(); // New: Slot to create a new folder
    void createRecord(); // New: Slot to create a new password record
    void onEditingFinished(); // New: Slot to handle when tree view item editing is finished
    void showSearchBar(); // New: Slot to show and focus the search bar
    void performSearch(const QString &text); // New: Slot to perform the search
    void jumpToSearchResult(const QModelIndex &index); // New: Slot to jump to a search result
    void onSearchBarReturnPressed(); // New: Slot to handle return key press in search bar
    void copyPasswordToClipboard(); // New: Slot to copy selected password to clipboard
    void showHelpDialog(); // New: Slot to show the help dialog

private:
    void setMode(Mode newMode);
    void updateStatusLabel(); // Helper to update the status bar text
    void setupEditableRecordView(); // New: Setup the editable fields in the right panel
    void enterInsertMode(const QModelIndex &index); // New: Enter insert mode for a specific record
    void exitInsertMode(); // New: Exit insert mode
    void saveModelToFile(const QString &filePath, const QString &masterPassword); // New: Helper to save the tree model to a file
    bool loadModelFromFile(const QString &filePath, const QString &masterPassword); // New: Helper to load the tree model from a file
    void loadRecentFiles(); // New: Load the list of recent files
    void saveRecentFiles(); // New: Save the list of recent files
    void addRecentFile(const QString &filePath); // New: Add a file to the recent files list
    void loadFile(const QString &filePath, bool isStartup = false); // New: Load a specific file, with optional startup flag
    QByteArray serializeModelToByteArray(); // New: Helper to serialize the model into a QByteArray

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
    QTextBrowser *m_recordDisplay; // Read-only display of password record (now a child of m_rightPanelStackedWidget)
    QWidget *m_editableRecordView; // Container for editable fields (now a child of m_rightPanelStackedWidget)
    QLineEdit *m_nameEdit;
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QLineEdit *m_urlEdit;
    QTextEdit *m_notesEdit;
    QLabel *m_statusLabel; // For mode or status display, similar to the reference
    QLineEdit *m_searchBar; // New: Search bar
    QCompleter *m_searchCompleter; // New: Search completer
    QStandardItemModel *m_searchCompleterModel; // New: Model for search completer

    QStandardItemModel *m_treeModel; // Model for the tree view
    Mode m_currentMode; // Current operational mode of the application
    QStandardItem *m_currentEditedItem; // Pointer to the item currently being edited
    QList<int> m_splitterSizes; // Stores the splitter sizes to restore them
    QString m_currentFilePath; // Stores the path of the current database file
    bool m_isModalDialogActive = false; // Is a modal dialog like 'Save As' currently active?
    bool m_isEditingTreeItem = false; // Is an item in the tree view being edited?
    QStringList m_recentFiles; // Stores the list of recently opened files
    QString m_masterPassword; // Stores the master password
};

#endif // MAINWINDOW_H
