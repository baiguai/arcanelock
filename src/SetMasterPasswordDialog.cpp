#include "SetMasterPasswordDialog.h"
#include <QMessageBox>

SetMasterPasswordDialog::SetMasterPasswordDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Set Master Password"));
    setFixedSize(300, 180); // Fixed size for simplicity

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText(tr("Enter master password"));

    m_confirmPasswordEdit = new QLineEdit(this);
    m_confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    m_confirmPasswordEdit->setPlaceholderText(tr("Confirm master password"));

    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet("color: red;");
    m_errorLabel->hide(); // Hide initially

    m_okButton = new QPushButton(tr("OK"), this);
    m_cancelButton = new QPushButton(tr("Cancel"), this);

    connect(m_okButton, &QPushButton::clicked, this, &SetMasterPasswordDialog::onOkClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &SetMasterPasswordDialog::onCancelClicked);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("Master Password:"), this));
    layout->addWidget(m_passwordEdit);
    layout->addWidget(new QLabel(tr("Confirm Password:"), this));
    layout->addWidget(m_confirmPasswordEdit);
    layout->addWidget(m_errorLabel);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addStretch();

    layout->addLayout(buttonLayout);
    setLayout(layout);
}

QString SetMasterPasswordDialog::getPassword() const
{
    return m_passwordEdit->text();
}

void SetMasterPasswordDialog::onOkClicked()
{
    QString password = m_passwordEdit->text();
    QString confirmPassword = m_confirmPasswordEdit->text();

    if (password.isEmpty()) {
        m_errorLabel->setText(tr("Password cannot be empty."));
        m_errorLabel->show();
        return;
    }

    if (password != confirmPassword) {
        m_errorLabel->setText(tr("Passwords do not match."));
        m_errorLabel->show();
        return;
    }

    m_errorLabel->hide();
    accept();
}

void SetMasterPasswordDialog::onCancelClicked()
{
    reject();
}
