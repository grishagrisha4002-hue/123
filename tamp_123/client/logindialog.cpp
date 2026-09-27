#include "logindialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Подключение к серверу");
    setFixedSize(360, 240);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *headerLabel = new QLabel("<h2>Вход в TCP Чат</h2>", this);
    headerLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(headerLabel);

    QFormLayout *formLayout = new QFormLayout();

    m_hostEdit = new QLineEdit("127.0.0.1", this);
    m_hostEdit->setPlaceholderText("например 127.0.0.1");

    m_portSpinBox = new QSpinBox(this);
    m_portSpinBox->setRange(1, 65535);
    m_portSpinBox->setValue(5555);

    m_nickEdit = new QLineEdit("Клиент_1", this);
    m_nickEdit->setPlaceholderText("Ваш никнейм");

    formLayout->addRow("IP / Хост:", m_hostEdit);
    formLayout->addRow("Порт:", m_portSpinBox);
    formLayout->addRow("Никнейм:", m_nickEdit);

    mainLayout->addLayout(formLayout);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_connectButton = new QPushButton("Подключиться", this);
    m_connectButton->setDefault(true);
    m_cancelButton = new QPushButton("Отмена", this);

    btnLayout->addWidget(m_connectButton);
    btnLayout->addWidget(m_cancelButton);
    mainLayout->addLayout(btnLayout);

    connect(m_connectButton, &QPushButton::clicked, this, &LoginDialog::onConnectClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void LoginDialog::setValues(const QString &host, quint16 port, const QString &nickname)
{
    m_hostEdit->setText(host);
    m_portSpinBox->setValue(port);
    m_nickEdit->setText(nickname);
}

QString LoginDialog::host() const
{
    return m_hostEdit->text().trimmed();
}

quint16 LoginDialog::port() const
{
    return static_cast<quint16>(m_portSpinBox->value());
}

QString LoginDialog::nickname() const
{
    return m_nickEdit->text().trimmed();
}

void LoginDialog::onConnectClicked()
{
    if (m_hostEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Пожалуйста, укажите адрес хоста.");
        m_hostEdit->setFocus();
        return;
    }

    if (m_nickEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Пожалуйста, введите никнейм.");
        m_nickEdit->setFocus();
        return;
    }

    accept();
}
