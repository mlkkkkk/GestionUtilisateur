// loginwindow.h
#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QVBoxLayout>
#include "connection.h"

// Forward declaration
class UserWindow;

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

signals:
    void loginSuccess();

private slots:
    void handleLogin();
    void togglePasswordVisibility();
    void showForgotPasswordDialog();

private:
    void setupUI();
    bool validateLogin(const QString &email, const QString &password);
    void showSecurityQuestionsDialog(const QString &email);

    QLineEdit *emailEdit;
    QLineEdit *passwordEdit;
    QPushButton *loginButton;
    QPushButton *togglePasswordBtn;
    QPushButton *forgotPasswordBtn;
    QLabel *titleLabel;
    QLabel *errorLabel;
};

#endif
