#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QMessageBox>
#include <QRegularExpression>

// Admin Roles Enum
enum AdminRole {
    ROLE_ADMIN_CONNECTION = 1,    // Admin de connexion
    ROLE_ADMIN_MATERIEL = 2,      // Admin de matériel
    ROLE_ADMIN_CLIENT = 3,        // Admin de client
    ROLE_ADMIN_COMMANDE = 4,      // Admin de commande
    ROLE_ADMIN_FOURNISSEUR = 5    // Admin de fournisseur
};

class Connection
{
private:
    Connection();
    static Connection *instance;
    QSqlDatabase db;
    QString dataSourceName;
    QString userName;
    QString password;
    QString email;
    QString lastValidationError;
    int adminRole;  // Store the admin role

    // Validation methods
    bool validateDataSourceName(const QString &dsn);
    bool validateUserName(const QString &user);
    bool validatePassword(const QString &pass);
    bool validateEmail(const QString &email);
    bool validateConnectionParams(const QString &dsn, const QString &user, const QString &pass, const QString &email);
    bool validateAdminRole(int role);

public:
    ~Connection();
    static Connection* getInstance();
    static void deleteInstance();
    bool establishConnection();
    void closeConnection();
    bool isOpen();
    QSqlDatabase getDatabase();

    // Modified method with validation including email
    bool setConnectionParams(const QString &dsn, const QString &user, const QString &pass, const QString &email, int role = ROLE_ADMIN_CONNECTION);

    QString getLastError();

    // Additional validation methods
    QString getValidationError() const;
    bool testConnection(const QString &dsn, const QString &user, const QString &pass);

    // Admin role methods
    void setAdminRole(int role);
    int getAdminRole() const;
    QString getAdminRoleString() const;
    bool hasPermission(AdminRole requiredRole) const;
    bool hasAnyPermission(const QList<AdminRole>& requiredRoles) const;

    // Email methods
    QString getEmail() const;

    // Login method with validation
    bool login(const QString &email, const QString &password);
    bool logout();

private:
    bool isLoggedIn;
};

#endif
