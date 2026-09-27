#include "chatwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QDateTime>

ChatWindow::ChatWindow(const QString &host, quint16 port, const QString &nickname, QWidget *parent)
    : QWidget(parent),
      m_host(host),
      m_port(port),
      m_nickname(nickname),
      m_nickHandshakeDone(false),
      m_exitApp(false),
      m_isReturningToLogin(false),
      m_socket(new QTcpSocket(this)),
      m_logViewer(new LogViewer(this))
{
    setWindowTitle(QString("TCP Чат — %1").arg(m_nickname));
    resize(700, 520);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Верхняя панель со статусом и кнопками
    QHBoxLayout *topLayout = new QHBoxLayout();
    m_statusLabel = new QLabel("Подключение...", this);
    m_statusLabel->setStyleSheet("font-weight: bold; color: orange;");

    m_disconnectButton = new QPushButton("Отключиться", this);
    m_disconnectButton->setStyleSheet("QPushButton { color: #d63031; font-weight: bold; padding: 3px 8px; }");
    m_disconnectButton->setToolTip("Отключиться от сервера и вернуться к окну входа");

    m_onlineLabel = new QLabel("Онлайн: 0", this);
    m_onlineLabel->setStyleSheet("color: #0066cc; font-weight: bold;");

    m_nickLabel = new QLabel(QString("Ник: <b>%1</b>").arg(m_nickname), this);

    m_changeNickButton = new QPushButton("Сменить ник (/name)", this);
    m_logViewerButton = new QPushButton("Просмотр логов", this);

    topLayout->addWidget(m_statusLabel);
    topLayout->addSpacing(8);
    topLayout->addWidget(m_disconnectButton);
    topLayout->addSpacing(15);
    topLayout->addWidget(m_onlineLabel);
    topLayout->addSpacing(15);
    topLayout->addWidget(m_nickLabel);
    topLayout->addStretch();
    topLayout->addWidget(m_changeNickButton);
    topLayout->addWidget(m_logViewerButton);
    mainLayout->addLayout(topLayout);

    // Основная область чата
    m_chatArea = new QTextEdit(this);
    m_chatArea->setReadOnly(true);
    m_chatArea->setStyleSheet("QTextEdit { background-color: #fdfdfd; font-size: 13px; line-height: 1.4; border: 1px solid #ccc; border-radius: 4px; padding: 6px; }");
    mainLayout->addWidget(m_chatArea);

    // Нижняя панель ввода сообщения
    QHBoxLayout *inputLayout = new QHBoxLayout();
    m_messageInput = new QLineEdit(this);
    m_messageInput->setPlaceholderText("Введите сообщение или команду (например: /name НовыйНик)...");
    m_sendButton = new QPushButton("Отправить", this);
    m_sendButton->setDefault(true);

    inputLayout->addWidget(m_messageInput);
    inputLayout->addWidget(m_sendButton);
    mainLayout->addLayout(inputLayout);

    // Соединение сигналов сокета
    connect(m_socket, &QTcpSocket::connected, this, &ChatWindow::onConnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &ChatWindow::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &ChatWindow::onDisconnected);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(m_socket, &QTcpSocket::errorOccurred, this, &ChatWindow::onErrorOccurred);
#else
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
            this, &ChatWindow::onErrorOccurred);
#endif

    // Соединение элементов интерфейса
    connect(m_disconnectButton, &QPushButton::clicked, this, &ChatWindow::onDisconnectClicked);
    connect(m_sendButton, &QPushButton::clicked, this, &ChatWindow::onSendMessageClicked);
    connect(m_messageInput, &QLineEdit::returnPressed, this, &ChatWindow::onSendMessageClicked);
    connect(m_changeNickButton, &QPushButton::clicked, this, &ChatWindow::onChangeNickClicked);
    connect(m_logViewerButton, &QPushButton::clicked, this, &ChatWindow::onOpenLogViewerClicked);

    // Запуск подключения
    m_logViewer->appendLog(QString("Инициализация подключения к %1:%2").arg(m_host).arg(m_port));
    m_socket->connectToHost(m_host, m_port);
}

ChatWindow::~ChatWindow()
{
    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();
    }
}

void ChatWindow::closeEvent(QCloseEvent *event)
{
    if (!m_isReturningToLogin) {
        m_exitApp = true;
    }
    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();
    }
    emit windowClosed();
    event->accept();
}

void ChatWindow::onDisconnectClicked()
{
    m_isReturningToLogin = true;
    m_exitApp = false;
    m_logViewer->appendLog("Нажата кнопка 'Отключиться'. Возврат к окну входа.");

    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();
    }

    emit returnToLogin();
    close();
}

void ChatWindow::onConnected()
{
    m_statusLabel->setText(QString("● Подключено к %1:%2").arg(m_host).arg(m_port));
    m_statusLabel->setStyleSheet("font-weight: bold; color: green;");
    m_messageInput->setEnabled(true);
    m_sendButton->setEnabled(true);
    m_logViewer->appendLog("Успешное соединение с сервером.");
}

void ChatWindow::onReadyRead()
{
    m_buffer.append(m_socket->readAll());

    while (m_buffer.contains('\n')) {
        int newlineIndex = m_buffer.indexOf('\n');
        QByteArray rawLine = m_buffer.left(newlineIndex);
        m_buffer.remove(0, newlineIndex + 1);

        QString line = QString::fromUtf8(rawLine).trimmed();
        if (line.isEmpty()) continue;

        m_logViewer->appendLog(QString("Входящее сообщение: %1").arg(line));

        // Сервер занят (лишний клиент)
        if (line.contains("Сервер занят", Qt::CaseInsensitive)) {
            appendFormattedMessage(QString("<span style='color:red; font-weight:bold;'>%1</span>").arg(line));
            QMessageBox::warning(this, "Сервер занят", line);
            continue;
        }

        // Никнейм уже занят другим клиентом
        if (line.contains("уже занят", Qt::CaseInsensitive)) {
            appendFormattedMessage(QString("<span style='color:red; font-weight:bold;'>%1</span>").arg(line));
            QMessageBox::warning(this, "Ошибка: ник занят", line);
            if (line.contains("Вход запрещен", Qt::CaseInsensitive)) {
                // Возвращаем пользователя к окну входа для смены ника
                onDisconnectClicked();
                return;
            }
            continue;
        }

        // Запрос ника сервером
        if (!m_nickHandshakeDone && line.contains("введите ваш ник", Qt::CaseInsensitive)) {
            appendFormattedMessage(QString("<span style='color:gray;'><i>%1</i></span>").arg(line));
            // Автоматически отправляем введенный в окне входа ник
            sendRawMessage(m_nickname);
            m_nickHandshakeDone = true;
            m_logViewer->appendLog(QString("Отправлен ник: %1").arg(m_nickname));
            continue;
        }

        // Обновление счетчика онлайн
        if (line.startsWith("Текущее количество подключенных клиентов:", Qt::CaseInsensitive)) {
            QString countStr = line.section(':', 1).trimmed();
            m_onlineLabel->setText(QString("Онлайн: %1 / 5").arg(countStr));
            appendFormattedMessage(QString("<span style='color:#007799; font-size: 11px;'>ℹ %1</span>").arg(line));
            continue;
        }

        // Системные события
        if (line.startsWith("К чату присоединился", Qt::CaseInsensitive)) {
            appendFormattedMessage(QString("<span style='color:green;'><b>→ %1</b></span>").arg(line));
        } else if (line.startsWith("Клиент", Qt::CaseInsensitive) && line.contains("сменил ник на")) {
            appendFormattedMessage(QString("<span style='color:#e67e22;'><b>✎ %1</b></span>").arg(line));
        } else if (line.startsWith("Отключился", Qt::CaseInsensitive)) {
            appendFormattedMessage(QString("<span style='color:#c0392b;'><b>← %1</b></span>").arg(line));
        } else {
            // Обычное сообщение чата
            appendFormattedMessage(QString("<span style='color:#2c3e50;'>%1</span>").arg(line));
        }
    }
}

void ChatWindow::onDisconnected()
{
    m_statusLabel->setText("○ Отключено от сервера");
    m_statusLabel->setStyleSheet("font-weight: bold; color: red;");
    m_onlineLabel->setText("Онлайн: 0");
    m_messageInput->setEnabled(false);
    m_sendButton->setEnabled(false);
    m_logViewer->appendLog("Соединение с сервером разорвано.");
    appendFormattedMessage("<span style='color:red;'><i>Соединение с сервером разорвано. Нажмите «Отключиться», чтобы вернуться к окну входа.</i></span>");
}

void ChatWindow::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    QString err = m_socket->errorString();
    m_statusLabel->setText(QString("Ошибка: %1").arg(err));
    m_statusLabel->setStyleSheet("font-weight: bold; color: red;");
    m_logViewer->appendLog(QString("Ошибка сокета: %1").arg(err));
}

void ChatWindow::onSendMessageClicked()
{
    QString text = m_messageInput->text().trimmed();
    if (text.isEmpty()) return;

    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        QMessageBox::warning(this, "Ошибка", "Нет соединения с сервером.");
        return;
    }

    // Если пользователь сам написал /name <НовоеИмя>
    if (text.startsWith("/name", Qt::CaseInsensitive)) {
        QString newNick = text.section(' ', 1).trimmed();
        if (!newNick.isEmpty()) {
            m_nickname = newNick;
            m_nickLabel->setText(QString("Ник: <b>%1</b>").arg(m_nickname));
            setWindowTitle(QString("TCP Чат — %1").arg(m_nickname));
        }
    }

    sendRawMessage(text);
    m_logViewer->appendLog(QString("Отправлено сообщение: %1").arg(text));

    // Добавляем свое отправленное сообщение в чат
    appendFormattedMessage(QString("<span style='color:#16a085;'><b>Вы:</b> %1</span>").arg(text));

    m_messageInput->clear();
    m_messageInput->setFocus();
}

void ChatWindow::onChangeNickClicked()
{
    bool ok = false;
    QString newNick = QInputDialog::getText(this, "Смена ника", "Введите новый никнейм:",
                                           QLineEdit::Normal, m_nickname, &ok);
    if (ok && !newNick.trimmed().isEmpty()) {
        newNick = newNick.trimmed();
        m_nickname = newNick;
        m_nickLabel->setText(QString("Ник: <b>%1</b>").arg(m_nickname));
        setWindowTitle(QString("TCP Чат — %1").arg(m_nickname));

        QString cmd = QString("/name %1").arg(newNick);
        sendRawMessage(cmd);
        m_logViewer->appendLog(QString("Запрос на смену ника: %1").arg(cmd));
    }
}

void ChatWindow::onOpenLogViewerClicked()
{
    m_logViewer->show();
    m_logViewer->raise();
    m_logViewer->activateWindow();
}

void ChatWindow::appendFormattedMessage(const QString &htmlMessage)
{
    QString timeStr = QDateTime::currentDateTime().toString("hh:mm:ss");
    m_chatArea->append(QString("<span style='color:#888; font-size:11px;'>[%1]</span> %2")
                       .arg(timeStr, htmlMessage));
}

void ChatWindow::sendRawMessage(const QString &msg)
{
    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        QByteArray data = msg.toUtf8();
        if (!data.endsWith('\n')) {
            data.append('\n');
        }
        m_socket->write(data);
        m_socket->flush();
    }
}
