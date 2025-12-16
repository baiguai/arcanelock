#include "MainWindow.h"
#include <QKeyEvent>
#include <QApplication>
#include <QHeaderView>
#include <QStandardItem>
#include <QStatusBar>
#include <QItemSelectionModel>
#include <QMetaType>
#include <QFormLayout> // For laying out editable fields
#include <QLineEdit> // For editable text fields
#include <QTextEdit> // For multiline notes
#include <QDebug> // Keep for general debugging, can remove later if desired
#include <QSizePolicy> // For setting size policies
#include <QStackedWidget> // For managing stacked widgets
#include <QFileDialog> // Required for QFileDialog

// Define a simple struct to hold password record data
struct PasswordRecord {
    QString name;
    QString username;
    QString password;
    QString url;
    QString notes;

    bool isEmpty() const {
        return name.isEmpty() && username.isEmpty() && password.isEmpty() && url.isEmpty() && notes.isEmpty();
    }
};

// Declare the struct as a metatype so it can be stored in QVariant
Q_DECLARE_METATYPE(PasswordRecord)

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Arcane Lock");
    setMinimumSize(800, 600);

    // Create splitter for left (treeview) and right (record display) panels
    m_splitter = new QSplitter(this);
    setCentralWidget(m_splitter);

    // --- Left Panel: TreeView ---
    m_treeView = new QTreeView(this);
    m_splitter->addWidget(m_treeView);
    // Set size policy to ensure it expands horizontally but allows shrinking
    m_treeView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Set up a basic model for the tree view
    m_treeModel = new QStandardItemModel(this);
    m_treeView->setModel(m_treeModel);

    // Add some dummy data to the tree view with more nesting and PasswordRecord data
    QStandardItem *rootItem = m_treeModel->invisibleRootItem();

    QStandardItem *category1 = new QStandardItem("Work");
    rootItem->appendRow(category1);

    QStandardItem *item1_1 = new QStandardItem("GitHub");
    PasswordRecord githubRecord = {"GitHub", "mygithubuser", "githubpass123", "https://github.com", "My GitHub account"};
    item1_1->setData(QVariant::fromValue(githubRecord), Qt::UserRole);
    category1->appendRow(item1_1);

    QStandardItem *item1_2 = new QStandardItem("Jira");
    PasswordRecord jiraRecord = {"Jira", "myjiradeveloper", "jirapassword!23", "https://jira.company.com", "Project management tool"};
    item1_2->setData(QVariant::fromValue(jiraRecord), Qt::UserRole);
    category1->appendRow(item1_2);

    QStandardItem *item1_3 = new QStandardItem("Work Projects");
    category1->appendRow(item1_3);

    QStandardItem *subitem1_3_1 = new QStandardItem("Project X Login");
    PasswordRecord projectXRecord = {"Project X", "dev_user", "securepassX", "https://projectx.company.com", "Login for Project X dev environment"};
    subitem1_3_1->setData(QVariant::fromValue(projectXRecord), Qt::UserRole);
    item1_3->appendRow(subitem1_3_1);

    QStandardItem *subitem1_3_2 = new QStandardItem("Project Y VPN");
    PasswordRecord projectYRecord = {"Project Y VPN", "vpnuser", "vpnsecureY", "vpn.company.com", "VPN access for Project Y"};
    subitem1_3_2->setData(QVariant::fromValue(projectYRecord), Qt::UserRole);
    item1_3->appendRow(subitem1_3_2);


    QStandardItem *category2 = new QStandardItem("Personal");
    rootItem->appendRow(category2);

    QStandardItem *item2_1 = new QStandardItem("Email");
    PasswordRecord emailRecord = {"Personal Email", "myemail@example.com", "mysecretemailpass", "https://mail.example.com", "My personal email account"};
    item2_1->setData(QVariant::fromValue(emailRecord), Qt::UserRole);
    category2->appendRow(item2_1);

    QStandardItem *item2_2 = new QStandardItem("Banking");
    PasswordRecord bankingRecord = {"Bank Account", "bankuser", "bankpass123!", "https://mybank.com", "Online banking login"};
    item2_2->setData(QVariant::fromValue(bankingRecord), Qt::UserRole);
    category2->appendRow(item2_2);

    QStandardItem *item2_3 = new QStandardItem("Hobbies");
    category2->appendRow(item2_3);

    QStandardItem *subitem2_3_1 = new QStandardItem("Gaming Platform");
    PasswordRecord gamingRecord = {"Gaming Platform", "gamer_tag", "gamepass456", "https://gamingplatform.com", "My gaming account"};
    subitem2_3_1->setData(QVariant::fromValue(gamingRecord), Qt::UserRole);
    item2_3->appendRow(subitem2_3_1);

    QStandardItem *subitem2_3_2 = new QStandardItem("E-book Reader");
    PasswordRecord ebookRecord = {"E-book Reader", "ebookuser", "bookloverpass", "https://ebooks.com", "Account for e-books"};
    subitem2_3_2->setData(QVariant::fromValue(ebookRecord), Qt::UserRole);
    item2_3->appendRow(subitem2_3_2);


    m_treeView->expandAll(); // Start expanded
    m_treeView->header()->hide(); // Hide header for a cleaner look
    m_treeView->setEditTriggers(QAbstractItemView::NoEditTriggers); // Make items read-only
    m_treeView->setFocusPolicy(Qt::StrongFocus); // Ensure tree view can receive focus

    // Connect selection changed signal to slot
    connect(m_treeView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &MainWindow::onTreeSelectionChanged);


    // --- Right Panel: Stacked Widget for Record Display and Editable View ---
    m_rightPanelStackedWidget = new QStackedWidget(this);
    m_splitter->addWidget(m_rightPanelStackedWidget);
    m_rightPanelStackedWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_recordDisplay = new QTextEdit(this);
    m_recordDisplay->setReadOnly(true);
    m_recordDisplay->setText("Select an item from the tree view to see details.");
    m_rightPanelStackedWidget->addWidget(m_recordDisplay);

    setupEditableRecordView(); // Initialize the editable view
    m_rightPanelStackedWidget->addWidget(m_editableRecordView);

    m_rightPanelStackedWidget->setCurrentIndex(0); // Show read-only view initially


    // Set initial sizes for the splitter (e.g., 20% for tree, 80% for display)
    QList<int> sizes;
    sizes << width() * 0.2 << width() * 0.8;
    m_splitter->setSizes(sizes);

    // --- Status Bar (using QLabel for now) ---
    m_statusLabel = new QLabel("", this); // Initialize with empty text
    statusBar()->addWidget(m_statusLabel);

    // Initialize current mode and update status bar
    m_currentMode = Mode::TREE; // Default mode
    updateStatusLabel();

    // Install event filter for the entire application to catch key presses
    // This allows catching 'q' even if other widgets have focus
    qApp->installEventFilter(this);
}

MainWindow::~MainWindow()
{
    // Destructor is empty as Qt's parent-child mechanism handles deletion of child widgets
}

void MainWindow::setMode(Mode newMode)
{
    if (m_currentMode == newMode)
        return;

    m_currentMode = newMode;
    updateStatusLabel();

    // Set focus based on mode
    switch (m_currentMode) {
        case Mode::TREE:
            m_treeView->setFocus();
            m_rightPanelStackedWidget->setCurrentIndex(0); // Show read-only view
            break;
        case Mode::NORMAL:
            m_rightPanelStackedWidget->setCurrentIndex(0); // Show read-only view
            break;
        case Mode::INSERT:
            if (m_nameEdit) { // Check if m_nameEdit is valid before setting focus
                m_nameEdit->setFocus();
            }
            m_rightPanelStackedWidget->setCurrentIndex(1); // Show editable view
            break;
        case Mode::VISUAL:
            break;
    }
}

void MainWindow::updateStatusLabel()
{
    QString modeText;
    switch (m_currentMode) {
        case Mode::TREE: modeText = "TREE"; break;
        case Mode::NORMAL: modeText = "NORMAL"; break;
        case Mode::INSERT: modeText = "INSERT"; break;
        case Mode::VISUAL: modeText = "VISUAL"; break;
    }
    m_statusLabel->setText(QString("MODE: %1").arg(modeText));
}

void MainWindow::expandAllNodes() {
    m_treeView->expandAll();
}

void MainWindow::collapseAllNodes() {
    m_treeView->collapseAll();
}

// --- Tree Item Manipulation Implementations ---
void MainWindow::moveItemToParentOrRoot() {
    QModelIndex currentIndex = m_treeView->currentIndex();
    if (!currentIndex.isValid()) return;

    QStandardItem *currentItem = m_treeModel->itemFromIndex(currentIndex);
    if (!currentItem) return;

    QStandardItem *parentItem = currentItem->parent();
    if (!parentItem) { // Already a top-level item
        return;
    }

    int currentRow = currentItem->row();

    QList<QStandardItem*> itemsToMove = parentItem->takeRow(currentRow);
    if (itemsToMove.isEmpty()) {
        return;
    }

    QStandardItem *grandparentItem = parentItem->parent();
    QStandardItem *newContainer = grandparentItem ? grandparentItem : m_treeModel->invisibleRootItem();

    newContainer->appendRow(itemsToMove);

    QModelIndex newIndex = m_treeModel->index(newContainer->rowCount() - 1, 0, newContainer->index());
    m_treeView->setCurrentIndex(newIndex);
    m_treeView->scrollTo(newIndex);
}

void MainWindow::moveItemDown() {
    QModelIndex currentIndex = m_treeView->currentIndex();
    if (!currentIndex.isValid()) return;

    QStandardItem *currentItem = m_treeModel->itemFromIndex(currentIndex);
    if (!currentItem) return;

    QStandardItem *parentItem = currentItem->parent();
    QStandardItem *containerItem = parentItem ? parentItem : m_treeModel->invisibleRootItem();

    int currentRow = currentItem->row();
    int rowCount = containerItem->rowCount();
    if (currentRow >= rowCount - 1) { // Cannot move down if it's the last child
        return;
    }

    QList<QStandardItem*> itemsToMove = containerItem->takeRow(currentRow);
    if (itemsToMove.isEmpty()) {
        return;
    }

    containerItem->insertRow(currentRow + 1, itemsToMove);

    QModelIndex newIndex = m_treeModel->index(currentRow + 1, 0, parentItem ? parentItem->index() : QModelIndex());
    m_treeView->setCurrentIndex(newIndex);
    m_treeView->scrollTo(newIndex);
}

void MainWindow::moveItemUp() {
    QModelIndex currentIndex = m_treeView->currentIndex();
    if (!currentIndex.isValid()) return;

    QStandardItem *currentItem = m_treeModel->itemFromIndex(currentIndex);
    if (!currentItem) return;

    QStandardItem *parentItem = currentItem->parent();
    QStandardItem *containerItem = parentItem ? parentItem : m_treeModel->invisibleRootItem();

    int currentRow = currentItem->row();
    if (currentRow <= 0) { // Cannot move up if it's the first child
        return;
    }

    QList<QStandardItem*> itemsToMove = containerItem->takeRow(currentRow);
    if (itemsToMove.isEmpty()) {
        return;
    }

    containerItem->insertRow(currentRow - 1, itemsToMove);

    QModelIndex newIndex = m_treeModel->index(currentRow - 1, 0, parentItem ? parentItem->index() : QModelIndex());
    m_treeView->setCurrentIndex(newIndex);
    m_treeView->scrollTo(newIndex);
}

void MainWindow::moveItemIntoSiblingFolder() {
    QModelIndex currentIndex = m_treeView->currentIndex();
    if (!currentIndex.isValid()) return;

    QStandardItem *currentItem = m_treeModel->itemFromIndex(currentIndex);
    if (!currentItem) return;

    QStandardItem *parentItem = currentItem->parent();
    QStandardItem *containerItem = parentItem ? parentItem : m_treeModel->invisibleRootItem();

    int currentRow = currentItem->row();
    if (currentRow == 0) { // Cannot move into a sibling folder if it's the first child
        return;
    }

    QStandardItem *siblingItem = containerItem->child(currentRow - 1);
    if (!siblingItem) {
        return;
    }

    QList<QStandardItem*> itemsToMove = containerItem->takeRow(currentRow);
    if (itemsToMove.isEmpty()) {
        return;
    }

    siblingItem->appendRow(itemsToMove);

    m_treeView->expand(siblingItem->index());

    QModelIndex newIndex = m_treeModel->index(siblingItem->rowCount() - 1, 0, siblingItem->index());
    m_treeView->setCurrentIndex(newIndex);
    m_treeView->scrollTo(newIndex);
}

void MainWindow::deleteSelectedItem() {
    QModelIndex currentIndex = m_treeView->currentIndex();
    if (!currentIndex.isValid()) return;

    QStandardItem *currentItem = m_treeModel->itemFromIndex(currentIndex);
    if (!currentItem) return;

    QStandardItem *parentItem = currentItem->parent();
    QModelIndex parentIndex = parentItem ? parentItem->index() : QModelIndex();

    int currentRow = currentItem->row();
    m_treeModel->removeRow(currentRow, parentIndex);
}

void MainWindow::setupEditableRecordView()
{
    m_editableRecordView = new QWidget(this);
    QFormLayout *formLayout = new QFormLayout(m_editableRecordView);
    
    // Correct initialization: new QLineEdit/QTextEdit objects
    m_nameEdit = new QLineEdit(this);
    m_usernameEdit = new QLineEdit(this);
    m_passwordEdit = new QLineEdit(this);
    m_urlEdit = new QLineEdit(this);
    m_notesEdit = new QTextEdit(this);

    // Set object names for easier identification if needed, and for debugging
    m_nameEdit->setObjectName("nameEdit");
    m_usernameEdit->setObjectName("usernameEdit");
    m_passwordEdit->setObjectName("passwordEdit");
    m_urlEdit->setObjectName("urlEdit");
    m_notesEdit->setObjectName("notesEdit");

    // Apply dark theme styling to new widgets
    m_nameEdit->setStyleSheet("QLineEdit { background-color: #3c3c3c; color: #f2f2f2; border: 1px solid #555555; padding: 5px; }");
    m_usernameEdit->setStyleSheet("QLineEdit { background-color: #3c3c3c; color: #f2f2f2; border: 1px solid #555555; padding: 5px; }");
    m_passwordEdit->setStyleSheet("QLineEdit { background-color: #3c3c3c; color: #f2f2f2; border: 1px solid #555555; padding: 5px; }");
    m_urlEdit->setStyleSheet("QLineEdit { background-color: #3c3c3c; color: #f2f2f2; border: 1px solid #555555; padding: 5px; }");
    m_notesEdit->setStyleSheet("QTextEdit { background-color: #3c3c3c; color: #f2f2f2; border: 1px solid #555555; padding: 5px; }");

    formLayout->addRow("Name:", m_nameEdit);
    formLayout->addRow("Username:", m_usernameEdit);
    formLayout->addRow("Password:", m_passwordEdit);
    formLayout->addRow("URL:", m_urlEdit);
    formLayout->addRow("Notes:", m_notesEdit);
}

void MainWindow::enterInsertMode(const QModelIndex &index)
{
    if (!index.isValid()) return;

    m_splitterSizes = m_splitter->sizes(); // Save current splitter sizes

    QStandardItem *selectedItem = m_treeModel->itemFromIndex(index);
    if (!selectedItem || !selectedItem->data(Qt::UserRole).canConvert<PasswordRecord>()) {
        qDebug() << "Cannot enter INSERT mode: No valid password record selected.";
        return;
    }

    m_currentEditedItem = selectedItem;
    PasswordRecord record = m_currentEditedItem->data(Qt::UserRole).value<PasswordRecord>();

    m_nameEdit->setText(record.name);
    m_usernameEdit->setText(record.username);
    m_passwordEdit->setText(record.password);
    m_urlEdit->setText(record.url);
    m_notesEdit->setText(record.notes);

    setMode(Mode::INSERT);
    // Restore sizes immediately after setMode, once widgets are visible
    // This QTimer::singleShot will be removed as part of the QStackedWidget implementation
    // if (!m_splitterSizes.isEmpty()) {
    //     QTimer::singleShot(0, this, [this]{ m_splitter->setSizes(m_splitterSizes); });
    // }
}

void MainWindow::exitInsertMode()
{
    m_currentEditedItem = nullptr;
    m_nameEdit->clear();
    m_usernameEdit->clear();
    m_passwordEdit->clear();
    m_urlEdit->clear();
    m_notesEdit->clear();

    setMode(Mode::TREE);
    // Restore sizes immediately after setMode, once widgets are visible
    // This QTimer::singleShot will be removed as part of the QStackedWidget implementation
    // if (!m_splitterSizes.isEmpty()) {
    //     QTimer::singleShot(0, this, [this]{ m_splitter->setSizes(m_splitterSizes); });
    // }
}

void MainWindow::saveRecord()
{
    if (!m_currentEditedItem) {
        qDebug() << "No item being edited to save.";
        return;
    }

    PasswordRecord updatedRecord;
    updatedRecord.name = m_nameEdit->text();
    updatedRecord.username = m_usernameEdit->text();
    updatedRecord.password = m_passwordEdit->text();
    updatedRecord.url = m_urlEdit->text();
    updatedRecord.notes = m_notesEdit->toPlainText();

    m_currentEditedItem->setData(QVariant::fromValue(updatedRecord), Qt::UserRole);
    m_currentEditedItem->setText(updatedRecord.name);

    qDebug() << "Record saved for:" << updatedRecord.name;
    onTreeSelectionChanged(m_currentEditedItem->index(), QModelIndex());
    exitInsertMode();
}

void MainWindow::newDatabase() {
    // Clear the current model
    m_treeModel->clear();
    m_treeModel->setHorizontalHeaderLabels({"Items"}); // Re-set header if cleared

    // Clear the current file path
    m_currentFilePath.clear();

    // Update status bar
    statusBar()->showMessage(tr("New database created."), 3000);
    qDebug() << "New database created. Current file path cleared.";

    // Clear record display
    m_recordDisplay->setText("Select an item from the tree view to see details.");
}

void MainWindow::saveDatabase() {
    if (m_currentFilePath.isEmpty()) {
        // If no file path is set, act as "Save As"
        saveDatabaseAs();
    } else {
        // Save to the current file path
        saveModelToFile(m_currentFilePath);
    }
}

void MainWindow::saveDatabaseAs() {
    QString filePath = QFileDialog::getSaveFileName(this,
                                                    tr("Save Password Database"),
                                                    "",
                                                    tr("Arcane Lock Database (*.alock);;All Files (*)"));

    if (!filePath.isEmpty()) {
        m_currentFilePath = filePath;
        saveModelToFile(m_currentFilePath);
    } else {
        statusBar()->showMessage(tr("Save operation cancelled."), 3000);
    }
}

void MainWindow::onTreeSelectionChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);

    if (m_currentMode == Mode::INSERT) {
        return;
    }

    if (!current.isValid()) {
        m_recordDisplay->setText("Select an item from the tree view to see details.");
        return;
    }

    QStandardItem *selectedItem = m_treeModel->itemFromIndex(current);
    if (!selectedItem) {
        m_recordDisplay->setText("Error: Could not retrieve item data.");
        return;
    }

    if (selectedItem->data(Qt::UserRole).canConvert<PasswordRecord>()) {
        PasswordRecord record = selectedItem->data(Qt::UserRole).value<PasswordRecord>();
        if (record.isEmpty()) {
            m_recordDisplay->setText("This is a folder or category. Select a password entry to see details.");
            return;
        }

        QString displayHtml = "<style>"
                              "body { font-family: sans-serif; background-color: #2b2b2b; color: #f2f2f2; }"
                              "h3 { color: #f2f2f2; margin-bottom: 5px; }"
                              "p { margin: 0; padding: 2px 0; }"
                              "b { color: #aaaaaa; }"
                              "</style>"
                              "<body>"
                              "<h3>Password Record: %1</h3>"
                              "<p><b>Name:</b> %2</p>"
                              "<p><b>Username:</b> %3</p>"
                              "<p><b>Password:</b> %4</p>"
                              "<p><b>URL:</b> %5</p>"
                              "<p><b>Notes:</b> %6</p>"
                              "</body>";
        
        m_recordDisplay->setHtml(displayHtml.arg(record.name, record.name, record.username, record.password, record.url, record.notes));

    }
}

#include <functional> // Required for std::function

void MainWindow::saveModelToFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        statusBar()->showMessage(tr("Cannot write file %1:\n%2.").arg(filePath).arg(file.errorString()), 5000);
        return;
    }

    QTextStream out(&file);
    // Write a simple header to indicate this is an ArcaneLock file
    out << "# ArcaneLock Password Database\n";
    out << "# Format: Item Name\n";
    out << "#   field: value\n";
    out << "#   notes: |\n";
    out << "#     line 1\n";
    out << "#     line 2\n";
    out << "\n";

    // Helper function to recursively save QStandardItem and its children
    // Using std::function and lambda to capture 'this' and access member variables
    std::function<void(QTextStream &, QStandardItem *, int)> saveItemRecursive =
        [&](QTextStream &outStream, QStandardItem *item, int depth) {
        if (!item) return;

        // Indent based on depth
        for (int i = 0; i < depth; ++i) {
            outStream << "  ";
        }

        // Write item text
        outStream << "- " << item->text() << "\n";

        // If it has PasswordRecord data, write it
        if (item->data(Qt::UserRole).canConvert<PasswordRecord>()) {
            PasswordRecord record = item->data(Qt::UserRole).value<PasswordRecord>();
            if (!record.isEmpty()) {
                for (int i = 0; i < depth + 1; ++i) { outStream << "  "; } outStream << "  name: " << record.name << "\n";
                for (int i = 0; i < depth + 1; ++i) { outStream << "  "; } outStream << "  username: " << record.username << "\n";
                for (int i = 0; i < depth + 1; ++i) { outStream << "  "; } outStream << "  password: " << record.password << "\n"; // Placeholder: NO ENCRYPTION
                for (int i = 0; i < depth + 1; ++i) { outStream << "  "; } outStream << "  url: " << record.url << "\n";
                for (int i = 0; i < depth + 1; ++i) { outStream << "  "; } outStream << "  notes: |\n";
                QStringList notesLines = record.notes.split('\n');
                for (const QString &line : notesLines) {
                    for (int i = 0; i < depth + 2; ++i) { outStream << "  "; } outStream << line << "\n";
                }
            }
        }

        // Recursively save children
        for (int i = 0; i < item->rowCount(); ++i) {
            saveItemRecursive(outStream, item->child(i), depth + 1);
        }
    };

    QStandardItem *rootItem = m_treeModel->invisibleRootItem();
    for (int i = 0; i < rootItem->rowCount(); ++i) {
        saveItemRecursive(out, rootItem->child(i), 0);
    }

    file.close();
    statusBar()->showMessage(tr("File saved to %1").arg(filePath), 3000);
    qDebug() << "Model saved to:" << filePath;
}
    bool MainWindow::eventFilter(QObject *obj, QEvent *event)
    {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        Qt::Key key = static_cast<Qt::Key>(keyEvent->key());
        Qt::KeyboardModifiers modifiers = keyEvent->modifiers();

        // Common key bindings (Q to quit)
        if (key == Qt::Key_Q && m_currentMode != Mode::INSERT) {
            QApplication::quit();
            return true;
        }

        // New keybindings for file operations
        if (key == Qt::Key_N && m_currentMode == Mode::TREE) {
            newDatabase();
            return true;
        } else if (key == Qt::Key_S) {
            if (modifiers & Qt::ShiftModifier) {
                saveDatabaseAs();
            } else {
                saveDatabase();
            }
            return true;
        }

        if (m_currentMode == Mode::TREE) {
            if (modifiers & Qt::ShiftModifier) { // Handle Shift + H/J/K/L, D, E, C
                if (key == Qt::Key_H) {
                    moveItemToParentOrRoot();
                    return true;
                } else if (key == Qt::Key_J) {
                    moveItemDown();
                    return true;
                } else if (key == Qt::Key_K) {
                    moveItemUp();
                    return true;
                } else if (key == Qt::Key_L) {
                    moveItemIntoSiblingFolder();
                    return true;
                } else if (key == Qt::Key_D) {
                    deleteSelectedItem();
                    return true;
                } else if (key == Qt::Key_E) {
                    expandAllNodes();
                    return true;
                } else if (key == Qt::Key_C) {
                    collapseAllNodes();
                    return true;
                }
            } else { // Handle h/j/k/l for navigation without Shift, and 'i' for insert
                if (key == Qt::Key_I) { // Enter INSERT mode
                    QModelIndex currentIndex = m_treeView->currentIndex();
                    if (currentIndex.isValid() && m_treeModel->itemFromIndex(currentIndex)->data(Qt::UserRole).canConvert<PasswordRecord>()) {
                        enterInsertMode(currentIndex);
                        return true;
                    }
                }

                Qt::Key simulatedKey = Qt::Key_unknown;
                bool handled = false;

                if (key == Qt::Key_J) { // Down
                    simulatedKey = Qt::Key_Down;
                    handled = true;
                } else if (key == Qt::Key_K) { // Up
                    simulatedKey = Qt::Key_Up;
                    handled = true;
                } else if (key == Qt::Key_H) { // Left
                    simulatedKey = Qt::Key_Left;
                    QModelIndex currentIndex = m_treeView->currentIndex();
                    if (currentIndex.isValid() && m_treeView->isExpanded(currentIndex)) {
                        m_treeView->collapse(currentIndex);
                        handled = true;
                    } else {
                        handled = true;
                    }
                } else if (key == Qt::Key_L) { // Right
                    simulatedKey = Qt::Key_Right;
                    QModelIndex currentIndex = m_treeView->currentIndex();
                    if (currentIndex.isValid() && !m_treeView->isExpanded(currentIndex) && m_treeView->model()->hasChildren(currentIndex)) {
                        m_treeView->expand(currentIndex);
                        handled = true;
                    } else {
                        handled = true;
                    }
                }

                if (handled && simulatedKey != Qt::Key_unknown) {
                    QKeyEvent *simulatedArrowEvent = new QKeyEvent(QEvent::KeyPress, simulatedKey, Qt::NoModifier);
                    QApplication::sendEvent(m_treeView, simulatedArrowEvent);
                    delete simulatedArrowEvent;
                    return true;
                } else if (handled) {
                    return true;
                }
            }
        } else if (m_currentMode == Mode::INSERT) {
            if (key == Qt::Key_Escape) {
                exitInsertMode();
                return true;
            } else if (key == Qt::Key_Return && (modifiers & Qt::ControlModifier)) {
                saveRecord();
                return true;
            } else if (key == Qt::Key_Tab) {
                if (modifiers & Qt::ShiftModifier) {
                    QWidget *focusedWidget = QApplication::focusWidget();
                    if (focusedWidget == m_nameEdit) {
                        m_notesEdit->setFocus();
                    } else if (focusedWidget == m_usernameEdit) {
                        m_nameEdit->setFocus();
                    } else if (focusedWidget == m_passwordEdit) {
                        m_usernameEdit->setFocus();
                    } else if (focusedWidget == m_urlEdit) {
                        m_passwordEdit->setFocus();
                    } else if (focusedWidget == m_notesEdit) {
                        m_urlEdit->setFocus();
                    } else {
                        m_notesEdit->setFocus();
                    }
                } else {
                    QWidget *focusedWidget = QApplication::focusWidget();
                    if (focusedWidget == m_nameEdit) {
                        m_usernameEdit->setFocus();
                    } else if (focusedWidget == m_usernameEdit) {
                        m_passwordEdit->setFocus();
                    } else if (focusedWidget == m_passwordEdit) {
                        m_urlEdit->setFocus();
                    } else if (focusedWidget == m_urlEdit) {
                        m_notesEdit->setFocus();
                    } else if (focusedWidget == m_notesEdit) {
                        m_nameEdit->setFocus();
                    } else {
                        m_nameEdit->setFocus();
                    }
                }
                return true;
            }
        }
    } else if (event->type() == QEvent::FocusIn) {
        if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(obj)) {
            lineEdit->selectAll();
        } else if (QTextEdit *textEdit = qobject_cast<QTextEdit*>(obj)) {
            textEdit->selectAll();
        }
        return false; // Crucially, return false so the event continues to be processed by the widget
    }
    return QMainWindow::eventFilter(obj, event);
}