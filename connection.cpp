#include "connection.h"

Connection* Connection::instance = nullptr;

Connection::Connection()
{
    // Default parameters - TO MODIFY
    dataSourceName = "source projet";  // Your ODBC DSN
    userName = "malek";
    password = "malek123";
    email = "";
    adminRole = ROLE_ADMIN_CONNECTION;
    isLoggedIn = false;
    lastValidationError = "";

    db = QSqlDatabase::addDatabase("QODBC", "oracle_connection");
}

Connection::~Connection()
{
    closeConnection();
}

Connection* Connection::getInstance()
{
    if (instance == nullptr) {
        instance = new Connection();
    }
    return instance;
}

void Connection::deleteInstance()
{
    if (instance != nullptr) {
        delete instance;
        instance = nullptr;
    }
}

// Validation method for Data Source Name
bool Connection::validateDataSourceName(const QString &dsn)
{
    if (dsn.isEmpty()) {
        lastValidationError = "Le nom de la source de données (DSN) ne peut pas être vide.";
        return false;
    }

    if (dsn.length() > 255) {
        lastValidationError = "Le nom de la source de données est trop long (maximum 255 caractères).";
        return false;
    }

    // Check for invalid characters
    QRegularExpression invalidChars("[<>:\"/\\|?*]");
    if (invalidChars.match(dsn).hasMatch()) {
        lastValidationError = "Le nom de la source de données contient des caractères invalides.";
        return false;
    }

    return true;
}

// Validation method for User Name
bool Connection::validateUserName(const QString &user)
{
    if (user.isEmpty()) {
        lastValidationError = "Le nom d'utilisateur ne peut pas être vide.";
        return false;
    }

    if (user.length() > 128) {
        lastValidationError = "Le nom d'utilisateur est trop long (maximum 128 caractères).";
        return false;
    }

    // Check for valid Oracle username format (alphanumeric, _, #, $)
    QRegularExpression validFormat("^[a-zA-Z][a-zA-Z0-9_#$]{0,127}$");
    if (!validFormat.match(user).hasMatch()) {
        lastValidationError = "Format de nom d'utilisateur invalide. "
                              "Le nom doit commencer par une lettre et ne contenir que "
                              "des lettres, chiffres, _, #, $.";
        return false;
    }

    return true;
}

// Enhanced validation method for Password
// Requirements: minimum 8 characters, at least one uppercase letter
bool Connection::validatePassword(const QString &pass)
{
    if (pass.isEmpty()) {
        lastValidationError = "Le mot de passe ne peut pas être vide.";
        return false;
    }

    if (pass.length() > 128) {
        lastValidationError = "Le mot de passe est trop long (maximum 128 caractères).";
        return false;
    }

    // Check minimum length of 8 characters
    if (pass.length() < 8) {
        lastValidationError = "Le mot de passe doit contenir au moins 8 caractères.";
        return false;
    }

    // Check for at least one uppercase letter
    bool hasUppercase = false;
    for (QChar c : pass) {
        if (c.isUpper()) {
            hasUppercase = true;
            break;
        }
    }

    if (!hasUppercase) {
        lastValidationError = "Le mot de passe doit contenir au moins une lettre majuscule.";
        return false;
    }

    // Check for at least one digit (optional but recommended)
    bool hasDigit = false;
    for (QChar c : pass) {
        if (c.isDigit()) {
            hasDigit = true;
            break;
        }
    }

    if (!hasDigit) {
        lastValidationError = "Le mot de passe doit contenir au moins un chiffre.";
        return false;
    }

    // Check for spaces in password
    if (pass.contains(' ')) {
        lastValidationError = "Le mot de passe ne peut pas contenir d'espaces.";
        return false;
    }

    return true;
}

// New validation method for Email
// Requirements: must contain @ symbol
bool Connection::validateEmail(const QString &email)
{
    if (email.isEmpty()) {
        lastValidationError = "L'adresse email ne peut pas être vide.";
        return false;
    }

    if (email.length() > 255) {
        lastValidationError = "L'adresse email est trop longue (maximum 255 caractères).";
        return false;
    }

    // Check if email contains @ symbol
    if (!email.contains('@')) {
        lastValidationError = "L'adresse email doit contenir le symbole '@'.";
        return false;
    }

    // Check if email has something before and after @
    QStringList parts = email.split('@');
    if (parts.size() != 2) {
        lastValidationError = "Format d'email invalide. Utilisez: nom@domaine.com";
        return false;
    }

    if (parts[0].isEmpty()) {
        lastValidationError = "L'email doit avoir un nom avant le symbole '@'.";
        return false;
    }

    if (parts[1].isEmpty()) {
        lastValidationError = "L'email doit avoir un domaine après le symbole '@'.";
        return false;
    }

    // Optional: Check for valid email format using regex
    QRegularExpression emailRegex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    if (!emailRegex.match(email).hasMatch()) {
        lastValidationError = "Format d'email invalide. Exemple: utilisateur@domaine.com";
        return false;
    }

    return true;
}

// Validate admin role
bool Connection::validateAdminRole(int role)
{
    if (role < ROLE_ADMIN_CONNECTION || role > ROLE_ADMIN_FOURNISSEUR) {
        lastValidationError = "Rôle d'administrateur invalide.";
        return false;
    }
    return true;
}

// Validate all connection parameters
bool Connection::validateConnectionParams(const QString &dsn, const QString &user, const QString &pass, const QString &email)
{
    if (!validateDataSourceName(dsn)) {
        return false;
    }

    if (!validateUserName(user)) {
        return false;
    }

    if (!validatePassword(pass)) {
        return false;
    }

    if (!validateEmail(email)) {
        return false;
    }

    return true;
}

// Modified setConnectionParams with validation including email and role
bool Connection::setConnectionParams(const QString &dsn, const QString &user, const QString &pass, const QString &email, int role)
{
    // Validate parameters before setting them
    if (!validateConnectionParams(dsn, user, pass, email)) {
        qDebug() << "Validation échouée:" << lastValidationError;

        QMessageBox::warning(nullptr, "Erreur de validation",
                             "Paramètres de connexion invalides:\n" + lastValidationError);

        return false;
    }

    if (!validateAdminRole(role)) {
        QMessageBox::warning(nullptr, "Erreur de validation",
                             "Rôle d'administrateur invalide:\n" + lastValidationError);
        return false;
    }

    // Set the validated parameters
    dataSourceName = dsn;
    userName = user;
    password = pass;
    this->email = email;
    adminRole = role;

    qDebug() << "Paramètres de connexion validés et enregistrés";
    qDebug() << "Rôle administrateur:" << getAdminRoleString();

    return true;
}

// Test connection without affecting current connection
bool Connection::testConnection(const QString &dsn, const QString &user, const QString &pass)
{
    // Validate parameters (without email for this test)
    if (!validateDataSourceName(dsn) || !validateUserName(user) || !validatePassword(pass)) {
        return false;
    }

    // Create temporary connection for testing
    QSqlDatabase testDb = QSqlDatabase::addDatabase("QODBC", "test_connection");
    testDb.setDatabaseName(dsn);
    testDb.setUserName(user);
    testDb.setPassword(pass);

    bool success = testDb.open();

    if (success) {
        testDb.close();
        qDebug() << "Test de connexion réussi";
    } else {
        QSqlError error = testDb.lastError();
        lastValidationError = "Échec de connexion: " + error.text();
        qDebug() << "Test de connexion échoué:" << error.text();

        QMessageBox::critical(nullptr, "Erreur de connexion",
                              "Impossible de se connecter avec ces paramètres:\n\n"
                              "DSN: " + dsn + "\n"
                                          "Utilisateur: " + user + "\n\n"
                                           "Erreur: " + error.text());
    }

    // Remove temporary connection
    testDb = QSqlDatabase();
    QSqlDatabase::removeDatabase("test_connection");

    return success;
}

bool Connection::establishConnection()
{
    if (db.isOpen()) {
        return true;
    }

    // Validate current parameters before attempting connection
    if (!validateConnectionParams(dataSourceName, userName, password, email)) {
        QMessageBox::critical(nullptr, "Erreur de validation",
                              "Impossible d'établir la connexion:\n" + lastValidationError);
        return false;
    }

    db.setDatabaseName(dataSourceName);
    db.setUserName(userName);
    db.setPassword(password);

    if (!db.open()) {
        QSqlError error = db.lastError();
        qDebug() << "Erreur de connexion:" << error.text();

        QString errorMessage = "Impossible de se connecter à Oracle.\n\n"
                               "DSN: " + dataSourceName + "\n"
                                                  "Utilisateur: " + userName + "\n"
                                            "Email: " + email + "\n\n"
                                         "Erreur: " + error.text();

        QMessageBox::critical(nullptr, "Erreur de connexion", errorMessage);

        return false;
    }

    qDebug() << "Connexion réussie à Oracle";
    return true;
}

void Connection::closeConnection()
{
    if (db.isOpen()) {
        QString connectionName = db.connectionName();
        db.close();
        db = QSqlDatabase();
        QSqlDatabase::removeDatabase(connectionName);
    }
}

bool Connection::isOpen()
{
    return db.isOpen();
}

QSqlDatabase Connection::getDatabase()
{
    return db;
}

QString Connection::getLastError()
{
    if (!db.lastError().text().isEmpty()) {
        return db.lastError().text();
    }
    if (!lastValidationError.isEmpty()) {
        return lastValidationError;
    }
    return "Aucune erreur";
}

QString Connection::getValidationError() const
{
    return lastValidationError;
}

// Admin role methods
void Connection::setAdminRole(int role)
{
    if (validateAdminRole(role)) {
        adminRole = role;
        qDebug() << "Rôle administrateur changé à:" << getAdminRoleString();
    }
}

int Connection::getAdminRole() const
{
    return adminRole;
}

QString Connection::getAdminRoleString() const
{
    switch(adminRole) {
    case ROLE_ADMIN_CONNECTION:
        return "Administrateur de Connexion";
    case ROLE_ADMIN_MATERIEL:
        return "Administrateur de Matériel";
    case ROLE_ADMIN_CLIENT:
        return "Administrateur de Client";
    case ROLE_ADMIN_COMMANDE:
        return "Administrateur de Commande";
    case ROLE_ADMIN_FOURNISSEUR:
        return "Administrateur de Fournisseur";
    default:
        return "Rôle Inconnu";
    }
}

bool Connection::hasPermission(AdminRole requiredRole) const
{
    // Check if the current admin role has the required permission
    // You can implement complex permission logic here
    // For now, it's a simple role check
    return (adminRole == requiredRole);
}

bool Connection::hasAnyPermission(const QList<AdminRole>& requiredRoles) const
{
    // Check if current admin role has any of the required permissions
    for (AdminRole role : requiredRoles) {
        if (adminRole == role) {
            return true;
        }
    }
    return false;
}

// Email methods
QString Connection::getEmail() const
{
    return email;
}

// Login method with validation
bool Connection::login(const QString &email, const QString &password)
{
    // Validate email and password
    if (!validateEmail(email)) {
        QMessageBox::warning(nullptr, "Erreur de connexion",
                             "Email invalide:\n" + lastValidationError);
        return false;
    }

    if (!validatePassword(password)) {
        QMessageBox::warning(nullptr, "Erreur de connexion",
                             "Mot de passe invalide:\n" + lastValidationError);
        return false;
    }

    // Here you would typically check against database
    // For this example, we'll just check if email matches stored email
    if (this->email != email) {
        lastValidationError = "Email incorrect.";
        QMessageBox::warning(nullptr, "Erreur de connexion",
                             "Email ou mot de passe incorrect.");
        return false;
    }

    if (this->password != password) {
        lastValidationError = "Mot de passe incorrect.";
        QMessageBox::warning(nullptr, "Erreur de connexion",
                             "Email ou mot de passe incorrect.");
        return false;
    }

    isLoggedIn = true;
    qDebug() << "Connexion réussie pour:" << email;
    qDebug() << "Rôle:" << getAdminRoleString();

    return true;
}

bool Connection::logout()
{
    isLoggedIn = false;
    qDebug() << "Déconnexion réussie";
    return true;
}
