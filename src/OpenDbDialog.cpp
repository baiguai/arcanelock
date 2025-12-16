#include "OpenDbDialog.h"
#include <QVBoxLayout>
#include <QListWidgetItem>
#include <QKeyEvent> // Required for QKeyEvent

OpenDbDialog::OpenDbDialog(const QStringList &recentFiles, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Open Database"));
    setMinimumSize(400, 300);

    m_listWidget = new QListWidget(this);

    // Add "Browse..." option at the top
    QListWidgetItem *browseItem = new QListWidgetItem(tr("Browse..."));
    m_listWidget->addItem(browseItem);

    // Add recent files
    m_listWidget->addItems(recentFiles);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_listWidget);
    setLayout(layout);

    connect(m_listWidget, &QListWidget::itemDoubleClicked, this, &OpenDbDialog::onItemSelected);
}

QString OpenDbDialog::getSelectedPath() const
{
    return m_selectedPath;
}

void OpenDbDialog::onItemSelected(QListWidgetItem *item)
{
    if (item) {
        if (item->text() == tr("Browse...")) {
            m_selectedPath = "BROWSE";
        } else {
            m_selectedPath = item->text();
        }
        accept();
    }
}

void OpenDbDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        QListWidgetItem *currentItem = m_listWidget->currentItem();
        if (currentItem) {
            if (currentItem->text() == tr("Browse...")) {
                m_selectedPath = "BROWSE";
            } else {
                m_selectedPath = currentItem->text();
            }
            accept();
        }
    } else {
        QDialog::keyPressEvent(event); // Call base class implementation for other keys
    }
}
