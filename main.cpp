#include <QApplication>
#include <QScreen>
#include <QStyle>
#include <QMessageBox>
#include "loginwindow.h"
#include "userwindow.h"
#include "connection.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyle("Fusion");

    app.setStyleSheet(
        "QMessageBox {"
        "    background-color: white;"
        "    min-width: 400px;"
        "}"
        "QMessageBox QLabel {"
        "    color: #2c3e50;"
        "    font-size: 14px;"
        "    padding: 15px;"
        "}"
        "QMessageBox QPushButton {"
        "    background-color: #3498db;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 8px;"
        "    padding: 10px 25px;"
        "    font-size: 14px;"
        "    min-width: 100px;"
        "    min-height: 35px;"
        "}"
        "QMessageBox QPushButton:hover {"
        "    background-color: #2980b9;"
        "}"
        );

    // Singleton : une seule instance de connexion pour tout le projet
    Connection* conn = Connection::getInstance();
    bool dbConnected = conn->establishConnection();

    if (!dbConnected) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Mode hors ligne");
        msgBox.setText("Impossible de se connecter à la base de données.");
        msgBox.setInformativeText("Voulez-vous continuer en mode démo (données locales) ?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        msgBox.setIcon(QMessageBox::Question);

        if (msgBox.exec() == QMessageBox::No) {
            Connection::deleteInstance();
            return -1;
        }
    }

    LoginWindow *login = new LoginWindow();
    UserWindow  *userWin = new UserWindow();

    QObject::connect(login, &LoginWindow::loginSuccess, [=](int userId) {
        userWin->setCurrentUserId(userId);
        userWin->show();
    });
    login->show();

    int result = app.exec();

    Connection::deleteInstance();

    return result;
}
