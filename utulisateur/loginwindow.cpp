#include "loginwindow.h"
#include "userwindow.h"
#include "connection.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QSizePolicy>
#include <QColor>

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Connexion - Système de Gestion");
    setFixedSize(550, 750);
    setupUI();

    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(25);
    shadowEffect->setColor(QColor(0, 0, 0, 100));
    shadowEffect->setOffset(0, 0);
    setGraphicsEffect(shadowEffect);
}

LoginWindow::~LoginWindow()
{
}

void LoginWindow::setupUI()
{
    setStyleSheet(
        "QWidget {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #667eea, stop:1 #764ba2);"
        "    font-family: 'Segoe UI', Arial, sans-serif;"
        "}"
        "QFrame {"
        "    background-color: white;"
        "    border-radius: 25px;"
        "}"
        "QLineEdit {"
        "    background-color: #f8f9fa;"
        "    border: 2px solid #e9ecef;"
        "    border-radius: 12px;"
        "    padding: 15px 20px;"
        "    font-size: 16px;"
        "    color: #495057;"
        "    selection-background-color: #667eea;"
        "    min-height: 25px;"
        "    min-width: 300px;"
        "    max-width: 350px;"
        "}"
        "QLineEdit:focus {"
        "    border: 2px solid #667eea;"
        "    background-color: white;"
        "}"
        "QLineEdit:hover {"
        "    border: 2px solid #764ba2;"
        "}"
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #667eea, stop:1 #764ba2);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 12px;"
        "    padding: 16px;"
        "    font-size: 18px;"
        "    font-weight: bold;"
        "    min-height: 30px;"
        "    min-width: 350px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #5a67d8, stop:1 #6b46a0);"
        "}"
        "QLabel {"
        "    color: #2d3748;"
        "}"
        );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);

    QFrame *loginFrame = new QFrame;
    loginFrame->setStyleSheet(
        "QFrame {"
        "    background-color: white;"
        "    border-radius: 25px;"
        "    padding: 30px;"
        "}"
        );

    QVBoxLayout *frameLayout = new QVBoxLayout(loginFrame);
    frameLayout->setSpacing(20);
    frameLayout->setAlignment(Qt::AlignHCenter);

    titleLabel = new QLabel("🔐 CONNEXION");
    titleLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 32px;"
        "    font-weight: bold;"
        "    color: #2d3748;"
        "    margin-bottom: 5px;"
        "}"
        );
    titleLabel->setAlignment(Qt::AlignCenter);
    frameLayout->addWidget(titleLabel);

    QLabel *subtitleLabel = new QLabel("Système de Gestion Collaborative");
    subtitleLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 16px;"
        "    color: #718096;"
        "    margin-bottom: 15px;"
        "}"
        );
    subtitleLabel->setAlignment(Qt::AlignCenter);
    frameLayout->addWidget(subtitleLabel);

    QLabel *emailLabel = new QLabel("📧 Adresse Email");
    emailLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 16px;"
        "    font-weight: 600;"
        "    color: #4a5568;"
        "    margin-top: 10px;"
        "    margin-bottom: 5px;"
        "}"
        );
    frameLayout->addWidget(emailLabel);

    emailEdit = new QLineEdit;
    emailEdit->setPlaceholderText("client@entreprise.com");
    emailEdit->setEchoMode(QLineEdit::Normal);
    emailEdit->setMinimumHeight(55);
    emailEdit->setMinimumWidth(350);
    emailEdit->setMaximumWidth(400);
    emailEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    frameLayout->addWidget(emailEdit, 0, Qt::AlignHCenter);

    QLabel *passwordLabel = new QLabel("🔑 Mot de passe");
    passwordLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 16px;"
        "    font-weight: 600;"
        "    color: #4a5568;"
        "    margin-top: 15px;"
        "    margin-bottom: 5px;"
        "}"
        );
    frameLayout->addWidget(passwordLabel);

    QHBoxLayout *passwordLayout = new QHBoxLayout;
    passwordLayout->setSpacing(10);
    passwordLayout->setContentsMargins(0, 0, 0, 0);

    passwordEdit = new QLineEdit;
    passwordEdit->setPlaceholderText("••••••••");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setMinimumHeight(55);
    passwordEdit->setMinimumWidth(290);
    passwordEdit->setMaximumWidth(340);
    passwordEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    togglePasswordBtn = new QPushButton("👁️");
    togglePasswordBtn->setFixedSize(60, 55);
    togglePasswordBtn->setCursor(Qt::PointingHandCursor);
    togglePasswordBtn->setStyleSheet(
        "QPushButton {"
        "    background: #e2e8f0;"
        "    border-radius: 12px;"
        "    font-size: 20px;"
        "    padding: 0px;"
        "    margin: 0px;"
        "    min-width: 60px;"
        "}"
        "QPushButton:hover {"
        "    background: #cbd5e0;"
        "}"
        );

    passwordLayout->addWidget(passwordEdit);
    passwordLayout->addWidget(togglePasswordBtn);
    frameLayout->addLayout(passwordLayout);

    forgotPasswordBtn = new QPushButton("Mot de passe oublié ?");
    forgotPasswordBtn->setCursor(Qt::PointingHandCursor);
    forgotPasswordBtn->setStyleSheet(
        "QPushButton {"
        "    background: transparent;"
        "    color: #667eea;"
        "    font-size: 14px;"
        "    font-weight: normal;"
        "    text-decoration: underline;"
        "    padding: 8px;"
        "    margin: 0px;"
        "    min-width: auto;"
        "}"
        "QPushButton:hover {"
        "    color: #764ba2;"
        "    background: transparent;"
        "}"
        );
    frameLayout->addWidget(forgotPasswordBtn, 0, Qt::AlignRight);

    errorLabel = new QLabel;
    errorLabel->setStyleSheet(
        "QLabel {"
        "    color: #e53e3e;"
        "    font-size: 15px;"
        "    padding: 12px;"
        "    background-color: #fff5f5;"
        "    border-radius: 10px;"
        "    margin: 15px 0px 5px 0px;"
        "    min-width: 320px;"
        "}"
        );
    errorLabel->setAlignment(Qt::AlignCenter);
    errorLabel->setVisible(false);
    errorLabel->setMinimumHeight(45);
    errorLabel->setWordWrap(true);
    frameLayout->addWidget(errorLabel, 0, Qt::AlignHCenter);

    loginButton = new QPushButton("SE CONNECTER");
    loginButton->setCursor(Qt::PointingHandCursor);
    loginButton->setMinimumHeight(60);
    loginButton->setMinimumWidth(350);
    loginButton->setMaximumWidth(400);
    loginButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    frameLayout->addWidget(loginButton, 0, Qt::AlignHCenter);

    QFrame *infoFrame = new QFrame;
    infoFrame->setStyleSheet(
        "QFrame {"
        "    background-color: #f7fafc;"
        "    border-radius: 12px;"
        "    border: 1px solid #e2e8f0;"
        "    padding: 15px;"
        "    margin-top: 10px;"
        "    min-width: 350px;"
        "}"
        );

    QVBoxLayout *infoLayout = new QVBoxLayout(infoFrame);

    QLabel *infoTitle = new QLabel("👥 Compte Oracle");
    infoTitle->setStyleSheet(
        "QLabel {"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    color: #4a5568;"
        "    margin-bottom: 10px;"
        "}"
        );
    infoTitle->setAlignment(Qt::AlignCenter);
    infoLayout->addWidget(infoTitle);

    QLabel *user1 = new QLabel(
        "<div style='text-align: center;'>"
        "<span style='color: #2d3748; font-size: 14px;'>📧 <b>client@entreprise.com</b></span><br>"
        "<span style='color: #2d3748; font-size: 14px;'>🔑 <b>tayssir1</b></span>"
        "</div>"
        );
    user1->setTextFormat(Qt::RichText);
    user1->setAlignment(Qt::AlignCenter);
    user1->setStyleSheet("margin: 5px; padding: 5px;");
    infoLayout->addWidget(user1);

    frameLayout->addWidget(infoFrame, 0, Qt::AlignHCenter);

    mainLayout->addWidget(loginFrame);

    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::handleLogin);
    connect(togglePasswordBtn, &QPushButton::clicked, this, &LoginWindow::togglePasswordVisibility);
    connect(forgotPasswordBtn, &QPushButton::clicked, this, &LoginWindow::showForgotPasswordDialog);
    connect(emailEdit, &QLineEdit::returnPressed, this, &LoginWindow::handleLogin);
    connect(passwordEdit, &QLineEdit::returnPressed, this, &LoginWindow::handleLogin);
}

void LoginWindow::togglePasswordVisibility()
{
    if (passwordEdit->echoMode() == QLineEdit::Password) {
        passwordEdit->setEchoMode(QLineEdit::Normal);
        togglePasswordBtn->setText("🔒");
    } else {
        passwordEdit->setEchoMode(QLineEdit::Password);
        togglePasswordBtn->setText("👁️");
    }
}

bool LoginWindow::validateLogin(const QString &email, const QString &password)
{
    QSqlDatabase db = Connection::getInstance()->getDatabase();

    if (!db.isOpen()) {
        return false;
    }

    QSqlQuery query(db);
    query.prepare(
        "SELECT ID_UTILISATEUR "
        "FROM UTILISATEUR "
        "WHERE LOWER(TRIM(EMAIL)) = LOWER(?) "
        "AND TRIM(MOT_DE_PASSE) = ?"
        );

    query.addBindValue(email.trimmed());
    query.addBindValue(password.trimmed());

    if (!query.exec()) {
        QMessageBox::warning(this, "Erreur Oracle", query.lastError().text());
        return false;
    }

    return query.next();
}

void LoginWindow::showForgotPasswordDialog()
{
    QString email = emailEdit->text().trimmed();

    if (email.isEmpty()) {
        QMessageBox::warning(this, "Email requis", "Veuillez entrer votre adresse email.");
        return;
    }

    QSqlDatabase db = Connection::getInstance()->getDatabase();

    if (!db.isOpen()) {
        QMessageBox::warning(this, "Erreur Base de données",
                             "La base de données n'est pas connectée.");
        return;
    }

    QSqlQuery query(db);
    query.prepare(
        "SELECT ID_UTILISATEUR "
        "FROM UTILISATEUR "
        "WHERE LOWER(TRIM(EMAIL)) = LOWER(?)"
        );

    query.addBindValue(email);

    if (!query.exec()) {
        QMessageBox::warning(this, "Erreur Oracle", query.lastError().text());
        return;
    }

    if (!query.next()) {
        QMessageBox::warning(this, "Email non trouvé",
                             "Aucun compte associé à cette adresse email.\nEmail saisi: " + email);
        return;
    }

    showSecurityQuestionsDialog(email);
}

void LoginWindow::showSecurityQuestionsDialog(const QString &email)
{
    QSqlDatabase db = Connection::getInstance()->getDatabase();

    if (!db.isOpen()) {
        QMessageBox::warning(this, "Erreur Base de données",
                             "La base de données n'est pas connectée.");
        return;
    }

    QSqlQuery query(db);
    query.prepare(
        "SELECT ID_UTILISATEUR, MOT_DE_PASSE, COULEUR_FAVORITE, NOM_ANIMAL, MATRICULE_VOITURE "
        "FROM UTILISATEUR "
        "WHERE LOWER(TRIM(EMAIL)) = LOWER(?)"
        );

    query.addBindValue(email.trimmed());

    if (!query.exec()) {
        QMessageBox::warning(this, "Erreur Oracle", query.lastError().text());
        return;
    }

    if (!query.next()) {
        QMessageBox::warning(this, "Email non trouvé",
                             "Aucun compte associé à cette adresse email.");
        return;
    }

    int userId = query.value(0).toInt();
    QString password = query.value(1).toString();
    QString favoriteColor = query.value(2).toString().trimmed();
    QString petName = query.value(3).toString().trimmed();
    QString carPlate = query.value(4).toString().trimmed();

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Vérification de sécurité");
    dialog->setFixedSize(500, 450);
    dialog->setStyleSheet(
        "QDialog { background-color: white; }"
        "QLabel { color: #2c3e50; font-size: 14px; }"
        "QLineEdit {"
        "    color: black;"
        "    background-color: white;"
        "    border: 1px solid #ced4da;"
        "    border-radius: 6px;"
        "    padding: 10px;"
        "    font-size: 13px;"
        "}"
        "QPushButton {"
        "    background-color: #3498db;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 10px 20px;"
        "    font-size: 13px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #2980b9; }"
        );

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->setSpacing(20);
    layout->setContentsMargins(30, 30, 30, 30);

    QLabel *titleLabel = new QLabel("🔐 Vérification d'identité");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #2c3e50;");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    QLabel *infoLabel = new QLabel("Pour récupérer votre mot de passe, répondez aux questions suivantes :");
    infoLabel->setWordWrap(true);
    infoLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(infoLabel);

    QFrame *separator = new QFrame;
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("background-color: #e2e8f0; max-height: 1px;");
    layout->addWidget(separator);

    QLabel *colorQuestion = new QLabel("🎨 Quelle est votre couleur préférée ?");
    colorQuestion->setStyleSheet("font-weight: bold; margin-top: 10px;");
    QLineEdit *colorAnswer = new QLineEdit;
    colorAnswer->setPlaceholderText("Entrez votre réponse...");
    layout->addWidget(colorQuestion);
    layout->addWidget(colorAnswer);

    QLabel *petQuestion = new QLabel("🐕 Quel est le nom de votre animal de compagnie ?");
    petQuestion->setStyleSheet("font-weight: bold; margin-top: 10px;");
    QLineEdit *petAnswer = new QLineEdit;
    petAnswer->setPlaceholderText("Entrez votre réponse...");
    layout->addWidget(petQuestion);
    layout->addWidget(petAnswer);

    QLabel *carQuestion = new QLabel("🚗 Quelle est la matricule de votre voiture ?");
    carQuestion->setStyleSheet("font-weight: bold; margin-top: 10px;");
    QLineEdit *carAnswer = new QLineEdit;
    carAnswer->setPlaceholderText("Entrez votre réponse...");
    layout->addWidget(carQuestion);
    layout->addWidget(carAnswer);

    layout->addStretch();

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *verifyBtn = new QPushButton("✅ Vérifier");
    QPushButton *cancelBtn = new QPushButton("❌ Annuler");

    verifyBtn->setStyleSheet("background-color: #2ecc71;");
    cancelBtn->setStyleSheet("background-color: #95a5a6;");

    buttonLayout->addWidget(verifyBtn);
    buttonLayout->addWidget(cancelBtn);
    layout->addLayout(buttonLayout);

    connect(verifyBtn, &QPushButton::clicked,
            [this, dialog, userId, password, favoriteColor, petName, carPlate,
             colorAnswer, petAnswer, carAnswer]() {

                QString color = colorAnswer->text().trimmed();
                QString pet = petAnswer->text().trimmed();
                QString car = carAnswer->text().trimmed();

                bool colorOk = color.compare(favoriteColor, Qt::CaseInsensitive) == 0;
                bool petOk = pet.compare(petName, Qt::CaseInsensitive) == 0;
                bool carOk = car.compare(carPlate, Qt::CaseInsensitive) == 0;

                if (colorOk && petOk && carOk) {
                    dialog->accept();

                    QMessageBox::information(this, "Mot de passe récupéré",
                                             "Votre mot de passe est : " + password);

                    emit loginSuccess(userId);
                    close();
                } else {
                    QString errorMessage = "❌ Vos réponses sont incorrectes.\n\n";

                    if (!colorOk) {
                        errorMessage += "• Couleur préférée incorrecte\n";
                    }

                    if (!petOk) {
                        errorMessage += "• Nom de l'animal incorrect\n";
                    }

                    if (!carOk) {
                        errorMessage += "• Matricule de la voiture incorrecte\n";
                    }

                    QMessageBox::warning(dialog, "Erreur de vérification", errorMessage);
                }
            });

    connect(cancelBtn, &QPushButton::clicked, dialog, &QDialog::reject);

    dialog->exec();
    dialog->deleteLater();
}

void LoginWindow::handleLogin()
{
    QString email = emailEdit->text().trimmed();
    QString password = passwordEdit->text().trimmed();

    if (email.isEmpty() || password.isEmpty()) {
        errorLabel->setText("❌ Veuillez remplir tous les champs");
        errorLabel->setVisible(true);
        return;
    }

    QSqlDatabase db = Connection::getInstance()->getDatabase();
    
    if (db.isOpen()) {
        QSqlQuery query(db);
        query.prepare(
            "SELECT ID_UTILISATEUR "
            "FROM UTILISATEUR "
            "WHERE LOWER(TRIM(EMAIL)) = LOWER(?) "
            "AND TRIM(MOT_DE_PASSE) = ?"
        );
        query.addBindValue(email);
        query.addBindValue(password);
        
        if (query.exec() && query.next()) {
            int userId = query.value(0).toInt();
            errorLabel->setVisible(false);
            emit loginSuccess(userId);
            this->close();
        } else {
            errorLabel->setText("❌ Mot de passe incorrect");
            errorLabel->setVisible(true);
            QMessageBox::warning(this, "Erreur de connexion", "Mot de passe incorrect.");
        }
    } else {
        // Mode hors ligne
        if (email == "client@entreprise.com" && password == "tayssir1") {
            errorLabel->setVisible(false);
            emit loginSuccess(2);
            this->close();
        } else if (email == "admin@entreprise.com" && password == "admin123") {
            errorLabel->setVisible(false);
            emit loginSuccess(1);
            this->close();
        } else {
            errorLabel->setText("❌ Mot de passe incorrect");
            errorLabel->setVisible(true);
            QMessageBox::warning(this, "Erreur de connexion", "Mot de passe incorrect.");
        }
    }
}
