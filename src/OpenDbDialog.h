#ifndef OPENDBDIALOG_H
#define OPENDBDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QStringList>

class OpenDbDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OpenDbDialog(const QStringList &recentFiles, QWidget *parent = nullptr);
    QString getSelectedPath() const;

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onItemSelected(QListWidgetItem *item);

private:
    QListWidget *m_listWidget;
    QString m_selectedPath;
};

#endif // OPENDBDIALOG_H
