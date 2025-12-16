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
#include <sodium.h> // Required for libsodium cryptography
#include "SetMasterPasswordDialog.h" // Required for setting master password
#include <QInputDialog> // Required for password prompt
#include <QItemDelegate> // Required for connecting to editor signals
#include <QCompleter> // Required for search completer
#include <functional> // Required for std::function for recursive lambda
#include <QTimer> // Required for QTimer::singleShot
#include <QClipboard> // Required for clipboard access


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

#include <QSettings>
#include <QDir>

// Declare the struct as a metatype so it can be stored in QVariant
Q_DECLARE_METATYPE(PasswordRecord)
// Also declare QStandardItem* as a metatype
Q_DECLARE_METATYPE(QStandardItem*)


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
    
    // Connect item delegate signals to handle end of editing
    connect(m_treeView->itemDelegate(), &QAbstractItemDelegate::commitData,
            this, &MainWindow::onEditingFinished);
    connect(m_treeView->itemDelegate(), &QAbstractItemDelegate::closeEditor,
            this, &MainWindow::onEditingFinished);


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

    // Search Bar
    m_searchBar = new QLineEdit(this);
    m_searchBar->setPlaceholderText("Search...");
    m_searchBar->hide();
    statusBar()->addPermanentWidget(m_searchBar);

    m_searchCompleterModel = new QStandardItemModel(this);
    m_searchCompleter = new QCompleter(m_searchCompleterModel, this);
    m_searchCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_searchBar->setCompleter(m_searchCompleter);

    connect(m_searchBar, &QLineEdit::textChanged, this, &MainWindow::performSearch);
    connect(m_searchCompleter, QOverload<const QModelIndex &>::of(&QCompleter::activated),
            this, &MainWindow::jumpToSearchResult);
    // Connect returnPressed to handle selection from search bar
    connect(m_searchBar, &QLineEdit::returnPressed, this, &MainWindow::onSearchBarReturnPressed);


    // Initialize current mode and update status bar
    m_currentMode = Mode::TREE; // Default mode
    updateStatusLabel();

    // Install event filter for the entire application to catch key presses
    // This allows catching 'q' even if other widgets have focus
    qApp->installEventFilter(this);

    loadRecentFiles();
}

MainWindow::~MainWindow()
{
    // Destructor is empty as Qt's parent-child mechanism handles deletion of child widgets
}

void MainWindow::onEditingFinished()
{
    m_isEditingTreeItem = false;
}

void MainWindow::loadRecentFiles()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "ArcaneLock", "ArcaneLock");
    m_recentFiles = settings.value("recentFiles").toStringList();
}

void MainWindow::saveRecentFiles()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "ArcaneLock", "ArcaneLock");
    settings.setValue("recentFiles", m_recentFiles);
}

void MainWindow::addRecentFile(const QString &filePath)
{
    m_recentFiles.removeAll(filePath);
    m_recentFiles.prepend(filePath);
    while (m_recentFiles.size() > 20) {
        m_recentFiles.removeLast();
    }
    saveRecentFiles();
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

    QModelIndex itemIndex = m_currentEditedItem->index(); // Store index before pointer is nulled

    m_currentEditedItem->setData(QVariant::fromValue(updatedRecord), Qt::UserRole);
    m_currentEditedItem->setText(updatedRecord.name);

    qDebug() << "Record saved for:" << updatedRecord.name;
    exitInsertMode(); // This will null m_currentEditedItem
    onTreeSelectionChanged(itemIndex, QModelIndex()); // Use the stored index
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
        // For existing files, prompt for password
        bool ok;
        m_isModalDialogActive = true;
        QString password = QInputDialog::getText(this, tr("Master Password"),
                                                 tr("Enter master password to save:"), QLineEdit::Password,
                                                 QString(), &ok);
        m_isModalDialogActive = false;
        if (ok && !password.isEmpty()) {
            saveModelToFile(m_currentFilePath, password);
        } else {
            statusBar()->showMessage(tr("Save cancelled. Master password not provided."), 3000);
        }
    }
}

void MainWindow::saveDatabaseAs() {
    // For new files, prompt to set master password
    SetMasterPasswordDialog passwordDialog(this);
    m_isModalDialogActive = true;
    int result = passwordDialog.exec();
    m_isModalDialogActive = false;

    if (result == QDialog::Accepted) {
        QString masterPassword = passwordDialog.getPassword();

        m_isModalDialogActive = true; // Set flag before opening dialog
        QString filePath = QFileDialog::getSaveFileName(this,
                                                        tr("Save Password Database"),
                                                        "",
                                                        tr("Arcane Lock Database (*.alock);;All Files (*)"));
        m_isModalDialogActive = false; // Reset flag after dialog is closed

        if (!filePath.isEmpty()) {
            m_currentFilePath = filePath;
            saveModelToFile(m_currentFilePath, masterPassword);
        } else {
            statusBar()->showMessage(tr("Save operation cancelled."), 3000);
        }
    } else {
        statusBar()->showMessage(tr("Save operation cancelled. Master password not set."), 3000);
    }
}

void MainWindow::openDatabase()
{
    m_isModalDialogActive = true;
    OpenDbDialog openDialog(m_recentFiles, this);
    m_isModalDialogActive = false;

    if (openDialog.exec() == QDialog::Accepted) {
        QString selectedPath = openDialog.getSelectedPath();
        if (selectedPath == "BROWSE") {
            m_isModalDialogActive = true;
            QString filePath = QFileDialog::getOpenFileName(this,
                                                          tr("Open Password Database"),
                                                          "",
                                                          tr("Arcane Lock Database (*.alock);;All Files (*)"));
            m_isModalDialogActive = false;
            if (!filePath.isEmpty()) {
                loadFile(filePath);
            }
        } else {
            loadFile(selectedPath);
        }
    }
}

void MainWindow::createFolder()
{
    QModelIndex currentIndex = m_treeView->currentIndex();
    QStandardItem *parentItem = m_treeModel->invisibleRootItem();

    if (currentIndex.isValid()) {
        QStandardItem *selectedItem = m_treeModel->itemFromIndex(currentIndex);
        if (selectedItem->data(Qt::UserRole).canConvert<PasswordRecord>()) {
            // If it's a record, add as sibling
            parentItem = selectedItem->parent() ? selectedItem->parent() : parentItem;
        } else {
            // If it's a folder, add as child
            parentItem = selectedItem;
        }
    }

    QStandardItem *newItem = new QStandardItem("New Folder");
    parentItem->appendRow(newItem);
    m_treeView->setCurrentIndex(newItem->index());
    m_isEditingTreeItem = true;
    m_treeView->edit(newItem->index()); // Allow immediate renaming
}

void MainWindow::createRecord()
{
    QModelIndex currentIndex = m_treeView->currentIndex();
    QStandardItem *parentItem = m_treeModel->invisibleRootItem();

    if (currentIndex.isValid()) {
        QStandardItem *selectedItem = m_treeModel->itemFromIndex(currentIndex);
        if (selectedItem->data(Qt::UserRole).canConvert<PasswordRecord>()) {
            // If it's a record, add as sibling
            parentItem = selectedItem->parent() ? selectedItem->parent() : parentItem;
        } else {
            // If it's a folder, add as child
            parentItem = selectedItem;
        }
    }

    QStandardItem *newItem = new QStandardItem("New Record");
    PasswordRecord newRecord = {"New Record", "", "", "", ""};
    newItem->setData(QVariant::fromValue(newRecord), Qt::UserRole);

    parentItem->appendRow(newItem);
    m_treeView->setCurrentIndex(newItem->index());
    enterInsertMode(newItem->index());
}

void MainWindow::loadFile(const QString &filePath)
{
    // Prompt for password to load file
    bool ok;
    m_isModalDialogActive = true;
    QString password = QInputDialog::getText(this, tr("Master Password"),
                                             tr("Enter master password to open:"), QLineEdit::Password,
                                             QString(), &ok);
    m_isModalDialogActive = false;

    if (ok && !password.isEmpty()) {
        loadModelFromFile(filePath, password);
        addRecentFile(filePath);
        m_currentFilePath = filePath;
        statusBar()->showMessage(tr("Loaded %1").arg(filePath), 3000);
    } else {
        statusBar()->showMessage(tr("Open cancelled. Master password not provided."), 3000);
    }
}

void MainWindow::loadModelFromFile(const QString &filePath, const QString &masterPassword)
{
    if (sodium_init() < 0) {
        statusBar()->showMessage(tr("libsodium initialization failed."), 5000);
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        statusBar()->showMessage(tr("Cannot open file %1:\n%2.").arg(filePath).arg(file.errorString()), 5000);
        return;
    }

    QByteArray fileContent = file.readAll();
    file.close();

    // 1. Read Header
    QByteArray header = fileContent.mid(0, 8); // "ALOCK_V1" is 8 bytes
    if (header != "ALOCK_V1") {
        statusBar()->showMessage(tr("Error: Not a valid ArcaneLock encrypted file (or unknown version)."), 5000);
        return;
    }
    int offset = header.size();

    // 2. Read Argon2 Hash String
    QByteArray hashedPasswordData = fileContent.mid(offset, crypto_pwhash_STRBYTES);
    offset += crypto_pwhash_STRBYTES;

    // 3. Read Encryption Salt
    QByteArray encryptionSaltData = fileContent.mid(offset, crypto_pwhash_SALTBYTES);
    offset += crypto_pwhash_SALTBYTES;

    // 4. Read Nonce
    QByteArray nonceData = fileContent.mid(offset, crypto_secretbox_NONCEBYTES);
    offset += crypto_secretbox_NONCEBYTES;

    // 5. Read Ciphertext
    QByteArray ciphertext = fileContent.mid(offset);

    // Verify master password
    if (crypto_pwhash_str_verify(hashedPasswordData.constData(),
                                 masterPassword.toUtf8().constData(), masterPassword.toUtf8().length()) != 0) {
        statusBar()->showMessage(tr("Incorrect master password."), 5000);
        return;
    }

    // Derive encryption key
    unsigned char encryptionKey[crypto_secretbox_KEYBYTES];
    if (crypto_pwhash(encryptionKey, sizeof encryptionKey,
                      masterPassword.toUtf8().constData(), masterPassword.toUtf8().length(),
                      reinterpret_cast<const unsigned char*>(encryptionSaltData.constData()),
                      crypto_pwhash_OPSLIMIT_MODERATE,
                      crypto_pwhash_MEMLIMIT_MODERATE,
                      crypto_pwhash_ALG_ARGON2ID13) != 0) {
        statusBar()->showMessage(tr("Key derivation failed during decryption."), 5000);
        return;
    }

    // Decrypt the ciphertext
    QByteArray decryptedPlaintext(ciphertext.size() - crypto_secretbox_MACBYTES, Qt::Uninitialized);
    if (crypto_secretbox_open_easy(reinterpret_cast<unsigned char*>(decryptedPlaintext.data()),
                                   reinterpret_cast<const unsigned char*>(ciphertext.constData()), ciphertext.size(),
                                   reinterpret_cast<const unsigned char*>(nonceData.constData()),
                                   encryptionKey) != 0) {
        statusBar()->showMessage(tr("Decryption failed. Data may be corrupted or password incorrect."), 5000);
        return;
    }

    // Deserialize the decrypted plaintext into the model
    m_treeModel->clear();
    m_treeModel->setHorizontalHeaderLabels({"Items"});

    QString decryptedDataString = QString::fromUtf8(decryptedPlaintext);
    QTextStream in(&decryptedDataString);
    QList<QStandardItem*> parentStack;
    parentStack.append(m_treeModel->invisibleRootItem());

    QStandardItem *currentItem = nullptr;
    PasswordRecord currentRecord;

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().isEmpty() || line.trimmed().startsWith('#')) {
            continue;
        }

        int indentation = 0;
        while (indentation < line.length() && line[indentation] == ' ') {
            indentation++;
        }
        int level = indentation / 2;

        if (line.trimmed().startsWith("- ")) {
            // New item
            if (currentItem) {
                if (!currentRecord.isEmpty()) {
                    currentItem->setData(QVariant::fromValue(currentRecord), Qt::UserRole);
                }
                currentRecord = PasswordRecord();
            }

            QString itemName = line.trimmed().mid(2);
            currentItem = new QStandardItem(itemName);

            while (level < parentStack.size() - 1) {
                parentStack.removeLast();
            }
            parentStack.last()->appendRow(currentItem);
            parentStack.append(currentItem);

        } else if (currentItem) {
            // Part of the current item's record
            QString trimmedLine = line.trimmed();
            int colonIndex = trimmedLine.indexOf(':');
            if (colonIndex > 0) {
                QString key = trimmedLine.left(colonIndex).trimmed();
                QString value = trimmedLine.mid(colonIndex + 1).trimmed();

                if (key == "name") currentRecord.name = value;
                else if (key == "username") currentRecord.username = value;
                else if (key == "password") currentRecord.password = value;
                else if (key == "url") currentRecord.url = value;
                else if (key == "notes" && value == "|") {
                    QStringList notesLines;
                    while (!in.atEnd()) {
                        qint64 lastPos = in.pos(); // Save current position
                        QString noteLine = in.readLine();
                        int noteIndentation = 0;
                        while (noteIndentation < noteLine.length() && noteLine[noteIndentation] == ' ') {
                            noteIndentation++;
                        }
                        // Check if the note line is more indented than the expected level for record fields
                        // This assumes note lines are indented by at least `indentation + 2`
                        if (noteIndentation > indentation + 2) {
                            notesLines.append(noteLine.trimmed());
                        } else {
                            // If not, it means the note block has ended. Seek back to read this line again
                            // as it might be a new record item or another field.
                            in.seek(lastPos);
                            break;
                        }
                    }
                    currentRecord.notes = notesLines.join('\n');
                }
            }
        }
    }

    if (currentItem && !currentRecord.isEmpty()) {
        currentItem->setData(QVariant::fromValue(currentRecord), Qt::UserRole);
    }

    // No need to close file here, already done after reading all content.
    m_treeView->expandAll();
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
        
        m_recordDisplay->setHtml(displayHtml.arg(record.name, record.name, record.username, QString(record.password.length(), '*'), record.url, record.notes));

    } else {
        m_recordDisplay->setText("This is a folder or category. Select a password entry to see details.");
    }
}

void MainWindow::showSearchBar()
{
    m_searchBar->show();
    m_searchBar->setFocus();
}

void MainWindow::performSearch(const QString &text)
{
    m_searchCompleterModel->clear();
    if (text.isEmpty()) {
        m_searchCompleter->popup()->hide(); // Hide completer if search text is empty
        return;
    }

    // Recursive search lambda
    std::function<void(QStandardItem *, const QString &, QStandardItemModel *)> searchTreeRecursiveLambda = 
        [&](QStandardItem *item, const QString &searchText, QStandardItemModel *resultModel) {
        if (!item) return;

        // Check if the current item is a password record
        if (item->data(Qt::UserRole).canConvert<PasswordRecord>()) {
            PasswordRecord record = item->data(Qt::UserRole).value<PasswordRecord>();
            // Check if any field contains the search text (case-insensitive)
            if (record.name.contains(searchText, Qt::CaseInsensitive) ||
                record.username.contains(searchText, Qt::CaseInsensitive) ||
                record.url.contains(searchText, Qt::CaseInsensitive) ||
                record.notes.contains(searchText, Qt::CaseInsensitive)) 
            {
                // Create a new item for the completer model
                QStandardItem *resultItem = new QStandardItem(item->text());
                // Store a pointer to the original item in the main tree model
                resultItem->setData(QVariant::fromValue(item), Qt::UserRole);
                resultModel->appendRow(resultItem);
                qDebug() << "Search: Added result:" << item->text() << "Original item ptr:" << (void*)item; // DEBUG
            }
        }
        // Recursively search children
        for (int i = 0; i < item->rowCount(); ++i) {
            searchTreeRecursiveLambda(item->child(i), searchText, resultModel);
        }
    };

    // Start recursive search from the invisible root item of the main tree model
    searchTreeRecursiveLambda(m_treeModel->invisibleRootItem(), text, m_searchCompleterModel);
    m_searchCompleter->complete();
}

void MainWindow::jumpToSearchResult(const QModelIndex &index)
{
    qDebug() << "jumpToSearchResult called with index:" << index; // DEBUG
    if (!index.isValid()) {
        qDebug() << "jumpToSearchResult: Invalid index passed."; // DEBUG
        return;
    }
    
    // Get the completer item that was activated
    QStandardItem *completerItem = m_searchCompleterModel->itemFromIndex(index);
    qDebug() << "jumpToSearchResult: completerItem valid:" << (completerItem != nullptr); // DEBUG
    if (!completerItem) return;

    // Retrieve the original QStandardItem pointer stored in the completer item's UserRole
    QStandardItem *originalItem = completerItem->data(Qt::UserRole).value<QStandardItem*>();
    qDebug() << "jumpToSearchResult: originalItem valid:" << (originalItem != nullptr); // DEBUG
    if (!originalItem) return;

    qDebug() << "jumpToSearchResult: Original item text:" << originalItem->text() << "Ptr:" << (void*)originalItem; // DEBUG

    QModelIndex treeIndex = originalItem->index();
    qDebug() << "jumpToSearchResult: treeIndex valid:" << treeIndex.isValid(); // DEBUG
    if (!treeIndex.isValid()) {
        qDebug() << "jumpToSearchResult: treeIndex is NOT valid. This is a problem."; // DEBUG
        // This might happen if the originalItem is no longer part of the model.
        // It could also happen if the model was cleared or reset since the search.
        // For debugging, let's try to find it from the root if originalItem is still valid.
        // In a real app, if originalItem's index() is invalid, it means it's detached.

        // Attempt to re-find the item in the main model. This is a workaround for debugging,
        // and points to a deeper issue if originalItem->index() is consistently invalid.
        QList<QStandardItem*> foundItems = m_treeModel->findItems(originalItem->text(), Qt::MatchRecursive);
        if (!foundItems.isEmpty()) {
            // Find the item that matches the pointer if possible
            for (QStandardItem* foundItem : foundItems) {
                if (foundItem == originalItem) {
                    treeIndex = foundItem->index();
                    qDebug() << "jumpToSearchResult: Refound item with valid treeIndex:" << treeIndex.isValid(); // DEBUG
                    break;
                }
            }
        }

        if (!treeIndex.isValid()) { // If still not valid after re-finding, then give up.
            qDebug() << "jumpToSearchResult: Still no valid treeIndex after re-finding. Aborting."; // DEBUG
            return;
        }
    }
    
    // Expand all parents of the original item in the main tree view
    QModelIndex parent = treeIndex.parent();
    while (parent.isValid()) {
        qDebug() << "jumpToSearchResult: Expanding parent:" << m_treeModel->itemFromIndex(parent)->text(); // DEBUG
        m_treeView->expand(parent);
        parent = parent.parent();
    }

    qDebug() << "jumpToSearchResult: Setting current index to:" << m_treeModel->itemFromIndex(treeIndex)->text(); // DEBUG
    m_treeView->setCurrentIndex(treeIndex);
    qDebug() << "jumpToSearchResult: Scrolling to:" << m_treeModel->itemFromIndex(treeIndex)->text(); // DEBUG
    m_treeView->scrollTo(treeIndex);
    qDebug() << "jumpToSearchResult: Setting focus to treeView."; // DEBUG
    m_treeView->setFocus();
    
    // Use a single shot timer to ensure the UI updates before hiding the search bar
    QTimer::singleShot(0, this, [this]() {
        m_searchBar->hide(); // Hide search bar after selection
        m_searchBar->clear(); // Clear search bar text
        qDebug() << "jumpToSearchResult: Search bar hidden and cleared."; // DEBUG
    });
}

void MainWindow::onSearchBarReturnPressed()
{
    qDebug() << "onSearchBarReturnPressed called."; // DEBUG
    QModelIndex currentIndex = m_searchCompleter->popup()->currentIndex();
    qDebug() << "onSearchBarReturnPressed: Completer popup current index valid:" << currentIndex.isValid(); // DEBUG
    if (currentIndex.isValid()) {
        jumpToSearchResult(currentIndex);
    } else {
        // If no item is highlighted, just hide and clear the search bar
        m_searchBar->hide();
        m_searchBar->clear();
        m_treeView->setFocus(); // Return focus to the tree view
        qDebug() << "onSearchBarReturnPressed: No item highlighted, search bar hidden."; // DEBUG
    }
}

void MainWindow::copyPasswordToClipboard()
{
    qDebug() << "copyPasswordToClipboard called."; // DEBUG
    QModelIndex currentIndex = m_treeView->currentIndex();
    if (!currentIndex.isValid()) {
        statusBar()->showMessage(tr("No item selected to copy password from."), 3000);
        qDebug() << "copyPasswordToClipboard: No item selected."; // DEBUG
        return;
    }

    QStandardItem *selectedItem = m_treeModel->itemFromIndex(currentIndex);
    if (!selectedItem) {
        statusBar()->showMessage(tr("Error: Could not retrieve selected item data."), 3000);
        qDebug() << "copyPasswordToClipboard: Could not retrieve selected item."; // DEBUG
        return;
    }

    if (selectedItem->data(Qt::UserRole).canConvert<PasswordRecord>()) {
        PasswordRecord record = selectedItem->data(Qt::UserRole).value<PasswordRecord>();
        if (!record.isEmpty()) {
            QApplication::clipboard()->setText(record.password);
            statusBar()->showMessage(tr("Password for '%1' copied to clipboard.").arg(record.name), 3000);
            qDebug() << "copyPasswordToClipboard: Password copied for:" << record.name; // DEBUG
        } else {
            statusBar()->showMessage(tr("Selected item is not a password record."), 3000);
            qDebug() << "copyPasswordToClipboard: Selected item is not a password record (empty record)."; // DEBUG
        }
    } else {
        statusBar()->showMessage(tr("Selected item is not a password record."), 3000);
        qDebug() << "copyPasswordToClipboard: Selected item is not a password record (cannot convert)."; // DEBUG
    }
}


QByteArray MainWindow::serializeModelToByteArray() {
    QString strData;
    QTextStream out(&strData);

    // Recursive serialize lambda
    std::function<void(QTextStream &, QStandardItem *, int)> serializeItemRecursiveToStringLambda = 
        [&](QTextStream &outStreamLambda, QStandardItem *item, int depth) {
        if (!item) return;

        // Indent based on depth
        for (int i = 0; i < depth; ++i) {
            outStreamLambda << "  ";
        }

        // Write item text
        outStreamLambda << "- " << item->text() << "\n";

        // If it has PasswordRecord data, write it
        if (item->data(Qt::UserRole).canConvert<PasswordRecord>()) {
            PasswordRecord record = item->data(Qt::UserRole).value<PasswordRecord>();
            if (!record.isEmpty()) {
                for (int i = 0; i < depth + 1; ++i) { outStreamLambda << "  "; } outStreamLambda << "  name: " << record.name << "\n";
                for (int i = 0; i < depth + 1; ++i) { outStreamLambda << "  "; } outStreamLambda << "  username: " << record.username << "\n";
                for (int i = 0; i < depth + 1; ++i) { outStreamLambda << "  "; } outStreamLambda << "  password: " << record.password << "\n"; // Placeholder: NO ENCRYPTION
                for (int i = 0; i < depth + 1; ++i) { outStreamLambda << "  "; } outStreamLambda << "  url: " << record.url << "\n";
                for (int i = 0; i < depth + 1; ++i) { outStreamLambda << "  "; } outStreamLambda << "  notes: |\n";
                QStringList notesLines = record.notes.split('\n');
                for (const QString &line : notesLines) {
                    for (int i = 0; i < depth + 2; ++i) { outStreamLambda << "  "; } outStreamLambda << line << "\n";
                }
            }
        }

        // Recursively serialize children
        for (int i = 0; i < item->rowCount(); ++i) {
            serializeItemRecursiveToStringLambda(outStreamLambda, item->child(i), depth + 1);
        }
    };

    out << "# ArcaneLock Password Database\n";
    out << "# Format: Item Name\n";
    out << "#   field: value\n";
    out << "#   notes: |\n";
    out << "#     line 1\n";
    out << "#     line 2\n";
    out << "\n";

    QStandardItem *rootItem = m_treeModel->invisibleRootItem();
    for (int i = 0; i < rootItem->rowCount(); ++i) {
        serializeItemRecursiveToStringLambda(out, rootItem->child(i), 0);
    }
    return strData.toUtf8();
}

#include <QDialog>
#include <functional> // Required for std::function

void MainWindow::saveModelToFile(const QString &filePath, const QString &masterPassword) {
    if (sodium_init() < 0) {
        statusBar()->showMessage(tr("libsodium initialization failed."), 5000);
        return;
    }

    QByteArray plaintext = serializeModelToByteArray();
    
    // 1. Argon2 password hashing for verification (contains its own salt and params)
    char hashedPasswordStr[crypto_pwhash_STRBYTES];
    if (crypto_pwhash_str(hashedPasswordStr,
                          masterPassword.toUtf8().constData(), masterPassword.toUtf8().length(),
                          crypto_pwhash_OPSLIMIT_MODERATE,
                          crypto_pwhash_MEMLIMIT_MODERATE) != 0) {
        statusBar()->showMessage(tr("Password hashing for verification failed."), 5000);
        return;
    }
    QByteArray hashedPasswordData(hashedPasswordStr, crypto_pwhash_STRBYTES);

    // 2. Generate a separate salt for key derivation
    unsigned char encryptionSalt[crypto_pwhash_SALTBYTES];
    randombytes_buf(encryptionSalt, sizeof encryptionSalt);

    // 3. Derive a raw encryption key from master password and encryption salt
    unsigned char encryptionKey[crypto_secretbox_KEYBYTES];
    if (crypto_pwhash(encryptionKey, sizeof encryptionKey,
                      masterPassword.toUtf8().constData(), masterPassword.toUtf8().length(),
                      encryptionSalt,
                      crypto_pwhash_OPSLIMIT_MODERATE,
                      crypto_pwhash_MEMLIMIT_MODERATE,
                      crypto_pwhash_ALG_ARGON2ID13) != 0) {
        statusBar()->showMessage(tr("Key derivation for encryption failed."), 5000);
        return;
    }

    // 4. Generate a random nonce
    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    randombytes_buf(nonce, sizeof nonce);

    // 5. Encrypt the plaintext
    QByteArray ciphertext(plaintext.size() + crypto_secretbox_MACBYTES, Qt::Uninitialized);
    if (crypto_secretbox_easy(reinterpret_cast<unsigned char*>(ciphertext.data()),
                              reinterpret_cast<const unsigned char*>(plaintext.constData()), plaintext.size(),
                              nonce, encryptionKey) != 0) {
        statusBar()->showMessage(tr("Encryption failed."), 5000);
        return;
    }

    // Write to file
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        statusBar()->showMessage(tr("Cannot write file %1:\n%2.").arg(filePath).arg(file.errorString()), 5000);
        return;
    }

    QByteArray header = "ALOCK_V1"; // 8 bytes for header
    file.write(header);
    file.write(hashedPasswordData);       // Argon2 hash string for verification
    file.write(QByteArray(reinterpret_cast<const char*>(encryptionSalt), sizeof encryptionSalt)); // Salt for key derivation
    file.write(QByteArray(reinterpret_cast<const char*>(nonce), sizeof nonce)); // Nonce for encryption
    file.write(ciphertext);

    file.close();
    statusBar()->showMessage(tr("File saved and encrypted to %1").arg(filePath), 3000);
    qDebug() << "Model saved and encrypted to:" << filePath;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    // If a modal dialog is active, don't process any of our custom keybindings.
    if (m_isModalDialogActive) {
        return QMainWindow::eventFilter(obj, event);
    }

    if (event->type() == QEvent::KeyPress) {
        // This prevents capturing keys in QFileDialog, for example, which are not children of MainWindow
        QWidget* widget = qobject_cast<QWidget*>(obj);
        if (widget && widget->window() != this && widget->window()->isModal()) {
            return QMainWindow::eventFilter(obj, event);
        }

        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        Qt::Key key = static_cast<Qt::Key>(keyEvent->key());
        Qt::KeyboardModifiers modifiers = keyEvent->modifiers();

        // Handle search bar interaction
        if (m_searchBar->isVisible()) {
            if (key == Qt::Key_Escape) {
                m_searchBar->hide();
                m_searchBar->clear();
                m_treeView->setFocus();
                return true;
            }
            // If the search bar has focus, let it handle its own key events normally
            if (m_searchBar->hasFocus()) {
                return QMainWindow::eventFilter(obj, event);
            }
        }


        if (m_currentMode == Mode::TREE) {
            // If an item is being edited in the tree view, don't process any other keybindings
            if (m_isEditingTreeItem) {
                return QMainWindow::eventFilter(obj, event);
            }

            // All TREE mode keybindings go here
            if (key == Qt::Key_I && !(modifiers & Qt::ShiftModifier)) { // 'i' for insert/rename
                QModelIndex currentIndex = m_treeView->currentIndex();
                if (currentIndex.isValid()) {
                    QStandardItem *item = m_treeModel->itemFromIndex(currentIndex);
                    if (item->data(Qt::UserRole).canConvert<PasswordRecord>()) {
                        enterInsertMode(currentIndex); // Edit record
                    } else {
                        m_isEditingTreeItem = true;
                        m_treeView->edit(currentIndex); // Edit folder name
                    }
                    return true;
                }
            } else if (key == Qt::Key_Y && !(modifiers & Qt::ShiftModifier)) { // 'y' for yank password
                copyPasswordToClipboard();
                return true;
            }

            // File and item creation
            if (key == Qt::Key_Q) {
                QApplication::quit();
                return true;
            } else if (key == Qt::Key_N) {
                newDatabase();
                return true;
            } else if (key == Qt::Key_O) {
                openDatabase();
                return true;
            } else if (key == Qt::Key_A) {
                if (modifiers & Qt::ShiftModifier) {
                    createFolder();
                } else {
                    createRecord();
                }
                return true;
            } else if (key == Qt::Key_S) {
                if (modifiers & Qt::ShiftModifier) {
                    saveDatabaseAs();
                } else {
                    saveDatabase();
                }
                return true;
            } else if (key == Qt::Key_Slash) {
                showSearchBar();
                return true;
            }

            // Item manipulation (Shift pressed)
            if (modifiers & Qt::ShiftModifier) {
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
            } else { // No Shift modifier
                // Navigation
                Qt::Key simulatedKey = Qt::Key_unknown;
                bool handled = false;

                if (key == Qt::Key_J) { // Down
                    simulatedKey = Qt::Key_Down;
                    handled = true;
                } else if (key == Qt::Key_K) { // Up
                    simulatedKey = Qt::Key_Up;
                    handled = true;
                } else if (key == Qt::Key_H) { // Left (collapse)
                    QModelIndex currentIndex = m_treeView->currentIndex();
                    if (currentIndex.isValid() && m_treeView->isExpanded(currentIndex)) {
                        m_treeView->collapse(currentIndex);
                    } else if (currentIndex.isValid() && currentIndex.parent().isValid()) {
                        m_treeView->setCurrentIndex(currentIndex.parent());
                    }
                    return true;
                } else if (key == Qt::Key_L) { // Right (expand)
                    QModelIndex currentIndex = m_treeView->currentIndex();
                    if (currentIndex.isValid() && m_treeView->model()->hasChildren(currentIndex)) {
                        m_treeView->expand(currentIndex);
                    }
                    return true;
                }

                if (handled) {
                    QKeyEvent *simulatedArrowEvent = new QKeyEvent(QEvent::KeyPress, simulatedKey, Qt::NoModifier);
                    QApplication::sendEvent(m_treeView, simulatedArrowEvent);
                    delete simulatedArrowEvent;
                    return true;
                }
            }
        } else if (m_currentMode == Mode::INSERT) {
            // INSERT mode keybindings
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
        return false;
    }
    return QMainWindow::eventFilter(obj, event);
}