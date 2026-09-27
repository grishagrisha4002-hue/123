#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>

class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = nullptr);

    void setValues(const QString &host, quint16 port, const QString &nickname);
    QString host() const;
    quint16 port() const;
    QString nickname() const;

private slots:
    void onConnectClicked();

private:
    QLineEdit *m_hostEdit;
    QSpinBox *m_portSpinBox;
    QLineEdit *m_nickEdit;
    QPushButton *m_connectButton;
    QPushButton *m_cancelButton;
};

#endif // LOGINDIALOG_H
