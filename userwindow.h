// userwindow.h
#ifndef USERWINDOW_H
#define USERWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QDate>
#include <QImage>
#include <QPixmap>

class QLineEdit;
class QComboBox;
class QTableWidget;
class QPushButton;
class QListWidget;
class QLabel;
class QDateEdit;
class QGroupBox;
class QCheckBox;

class UserWindow : public QMainWindow
{
    Q_OBJECT

public:
    // Structures de données
    struct User {
        int id;
        QString name;
        QString email;
        QString role;
        QString password;
        QString phone;
        QString status;
        QString favoriteColor;
        QString petName;
        QString carPlate;
        QDate creationDate;
        QImage profileImage;
        bool hasFace;
    };

    UserWindow(QWidget *parent = nullptr);

private slots:
    // Fonctions pour la page Utilisateurs
    void addUser();
    void deleteUser();
    void clearFields();
    void modifyUser();
    void searchUser();
    void sortUsersByDate();
    void sortUsersByName();
    void exportUsers();
    void selectProfileImage();
    void clearProfileImage();

    // Fonctions de base de données
    void loadUsersFromDatabase();
    void saveUserToDatabase(const User &user);
    void deleteUserFromDatabase(int id);
    void updateUserInDatabase(const User &user);

private:
    void setupUI();
    void updateUsersTable();
    QPushButton* createStyledButton(const QString& text, const QString& color = "#3498db");

    // Validation methods
    bool validateName(const QString &name);
    bool validateEmail(const QString &email);
    bool validatePassword(const QString &password);
    bool validatePhone(const QString &phone);
    bool validateSecurityAnswers();
    bool validateFaceImage(const QImage &image);

    // Face detection methods
    bool detectFace(const QImage &image);
    bool isHumanFace(const QImage &image);
    QImage preprocessImage(const QImage &image);

    // Navigation
    QListWidget *navList;
    QStackedWidget *pagesWidget;

    // Page Utilisateurs
    QLineEdit *nameEdit;
    QLineEdit *emailEdit;
    QLineEdit *passwordEdit;
    QLineEdit *phoneEdit;
    QLineEdit *idEdit;
    QLineEdit *searchEdit;
    QLineEdit *favoriteColorEdit;
    QLineEdit *petNameEdit;
    QLineEdit *carPlateEdit;
    QComboBox *roleBox;
    QComboBox *statusBox;
    QDateEdit *creationDateEdit;
    QTableWidget *usersTable;
    QPushButton *modifyBtn;

    // Image related widgets
    QLabel *imagePreviewLabel;
    QPushButton *selectImageBtn;
    QPushButton *clearImageBtn;
    QLabel *faceValidationLabel;
    QImage currentProfileImage;
    bool currentImageHasFace;

    // Listes de données
    QList<User> usersList;

    int nextId;
    bool databaseEnabled;
};

#endif
