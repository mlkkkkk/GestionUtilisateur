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
class QSqlQuery;

class UserWindow : public QMainWindow
{
    Q_OBJECT
public:
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
    void setCurrentUserId(int id);        // ← déclaration simple
    int  getCurrentUserId() const { return currentUserId; }

private slots:
    void addUser();
    void deleteUser();
    void clearFields();
    void modifyUser();
    void searchUser();
    void sortUsersByDate();
    void sortUsersByName();
    void exportUsers();
    void showRoleStats();
    void selectProfileImage();
    void clearProfileImage();
    void loadUsersFromDatabase();
    bool saveUserToDatabase(const User &user);
    void deleteUserFromDatabase(int id);
    void updateUserInDatabase(const User &user);

private:
    void populateUsersListFromQuery(QSqlQuery &query);
    void setupUI();
    void updateUsersTable();
    QPushButton* createStyledButton(const QString& text, const QString& color = "#3498db");
    bool validateName(const QString &name);
    bool validateEmail(const QString &email);
    bool validatePassword(const QString &password);
    bool validatePhone(const QString &phone);
    bool validateSecurityAnswers();
    bool validateFaceImage(const QImage &image);
    bool detectFace(const QImage &image);
    bool isHumanFace(const QImage &image);
    QImage preprocessImage(const QImage &image);

    QListWidget  *navList;
    QStackedWidget *pagesWidget;
    QLineEdit    *nameEdit;
    QLineEdit    *emailEdit;
    QLineEdit    *passwordEdit;
    QLineEdit    *phoneEdit;
    QLineEdit    *idEdit;
    QLineEdit    *searchEdit;
    QLineEdit    *favoriteColorEdit;
    QLineEdit    *petNameEdit;
    QLineEdit    *carPlateEdit;
    QComboBox    *roleBox;
    QComboBox    *statusBox;
    QDateEdit    *creationDateEdit;
    QTableWidget *usersTable;
    QPushButton  *modifyBtn;
    QLabel       *imagePreviewLabel;
    QPushButton  *selectImageBtn;
    QPushButton  *clearImageBtn;
    QLabel       *faceValidationLabel;
    QImage        currentProfileImage;
    bool          currentImageHasFace;
    QList<User>   usersList;
    int           nextId;
    bool          databaseEnabled;
    int           currentUserId;
};

#endif // USERWINDOW_H
