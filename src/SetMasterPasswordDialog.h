#ifndef SETMASTERPASSWORDDIALOG_H
#define SETMASTERPASSWORDDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

class SetMasterPasswordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SetMasterPasswordDialog(QWidget *parent = nullptr);
    QString getPassword() const;

private slots:
    void onOkClicked();
    void onCancelClicked();

private:
    QLineEdit *m_passwordEdit;
    QLineEdit *m_confirmPasswordEdit;
    QLabel *m_errorLabel;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
};

#endif // SETMASTERPASSWORDDIALOG_H
