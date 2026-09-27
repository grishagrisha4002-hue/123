#ifndef LOGVIEWER_H
#define LOGVIEWER_H

#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>

class LogViewer : public QDialog
{
    Q_OBJECT
public:
    explicit LogViewer(QWidget *parent = nullptr);

    void appendLog(const QString &logEntry);

private slots:
    void onSearchTextChanged(const QString &text);
    void onSaveToFileClicked();
    void onClearClicked();

private:
    QTextEdit *m_textEdit;
    QLineEdit *m_searchEdit;
    QPushButton *m_saveButton;
    QPushButton *m_clearButton;
    QPushButton *m_closeButton;
    QStringList m_allLogs;

    void refreshDisplay();
};

#endif // LOGVIEWER_H
