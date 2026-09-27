#include "logviewer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

LogViewer::LogViewer(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Журнал событий и логи сервера");
    resize(640, 480);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *topLayout = new QHBoxLayout();
    QLabel *searchLabel = new QLabel("Поиск в логах:", this);
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Фильтр по тексту...");
    m_clearButton = new QPushButton("Очистить вид", this);

    topLayout->addWidget(searchLabel);
    topLayout->addWidget(m_searchEdit);
    topLayout->addWidget(m_clearButton);
    mainLayout->addLayout(topLayout);

    m_textEdit = new QTextEdit(this);
    m_textEdit->setReadOnly(true);
    m_textEdit->setFontFamily("Courier New");
    mainLayout->addWidget(m_textEdit);

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    m_saveButton = new QPushButton("Экспорт в файл...", this);
    m_closeButton = new QPushButton("Закрыть", this);

    bottomLayout->addWidget(m_saveButton);
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_closeButton);
    mainLayout->addLayout(bottomLayout);

    connect(m_searchEdit, &QLineEdit::textChanged, this, &LogViewer::onSearchTextChanged);
    connect(m_clearButton, &QPushButton::clicked, this, &LogViewer::onClearClicked);
    connect(m_saveButton, &QPushButton::clicked, this, &LogViewer::onSaveToFileClicked);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
}

void LogViewer::appendLog(const QString &logEntry)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString fullLine = QString("[%1] %2").arg(timestamp, logEntry);
    m_allLogs.append(fullLine);

    if (m_searchEdit->text().isEmpty() || fullLine.contains(m_searchEdit->text(), Qt::CaseInsensitive)) {
        m_textEdit->append(fullLine);
    }
}

void LogViewer::onSearchTextChanged(const QString &text)
{
    Q_UNUSED(text);
    refreshDisplay();
}

void LogViewer::refreshDisplay()
{
    m_textEdit->clear();
    QString filter = m_searchEdit->text().trimmed();

    for (const QString &line : m_allLogs) {
        if (filter.isEmpty() || line.contains(filter, Qt::CaseInsensitive)) {
            m_textEdit->append(line);
        }
    }
}

void LogViewer::onClearClicked()
{
    m_textEdit->clear();
}

void LogViewer::onSaveToFileClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить лог", "chat_session.log", "Log Files (*.log *.txt)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить файл.");
        return;
    }

    QTextStream out(&file);
    for (const QString &line : m_allLogs) {
        out << line << "\n";
    }
    file.close();

    QMessageBox::information(this, "Успех", "Лог успешно сохранен в файл.");
}
