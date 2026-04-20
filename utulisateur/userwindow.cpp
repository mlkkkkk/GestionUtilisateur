#include "userwindow.h"
#include "connection.h"
#include "../matriel/matriele.h"

#include <QWidget>
#include <QDialog>
#include <QMap>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QLabel>
#include <QFrame>
#include <QListWidget>
#include <QStackedWidget>
#include <QDateEdit>
#include <QDate>
#include <algorithm>
#include <utility>
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QRegularExpression>
#include <QGroupBox>
#include <QApplication>
#include <QScreen>
#include <QImageReader>
#include <QBuffer>
#include <QPainter>
#include <QColor>
#include <QPdfWriter>
#include <QTextDocument>
#include <QPageSize>
#include <QFileInfo>

UserWindow::UserWindow(QWidget *parent)
    : QMainWindow(parent), nextId(1), databaseEnabled(false),
    currentImageHasFace(false)
{
    setupUI();
}

void UserWindow::setCurrentUserId(int id)
{
    currentUserId = id;
}

void UserWindow::setupUI()

    // ...

{
    QWidget *centralWidget = new QWidget;
    centralWidget->setStyleSheet("background-color: #f5f6fa;");
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QFrame *sidebar = new QFrame;
    sidebar->setStyleSheet(
        "QFrame {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                                 stop:0 #2c3e50, stop:1 #34495e);"
        "    border: none;"
        "}"
        "QLabel {"
        "    color: white;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    padding: 15px;"
        "}"
        "QListWidget {"
        "    background: transparent;"
        "    border: none;"
        "    color: #ecf0f1;"
        "    font-size: 12px;"
        "}"
        "QListWidget::item {"
        "    padding: 12px 15px;"
        "    border-bottom: 1px solid rgba(255,255,255,0.1);"
        "}"
        "QListWidget::item:selected {"
        "    background-color: #3498db;"
        "    border-left: 4px solid #2980b9;"
        "}"
        "QListWidget::item:hover {"
        "    background-color: rgba(52, 152, 219, 0.3);"
        "}"
        );
    sidebar->setFixedWidth(200);



    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(0, 0, 0, 0);
    sidebarLayout->setSpacing(0);

    QLabel *appTitle = new QLabel("🏢 GESTION");
    appTitle->setAlignment(Qt::AlignCenter);
    appTitle->setStyleSheet("font-size: 16px; padding: 15px;");
    sidebarLayout->addWidget(appTitle);

    navList = new QListWidget;
    navList->addItem("👥 Utilisateurs");
    navList->setCurrentRow(0);
    sidebarLayout->addWidget(navList);
    sidebarLayout->addStretch();

    QLabel *footer = new QLabel("v1.0");
    footer->setStyleSheet("color: #7f8c8d; font-size: 10px; padding: 8px;");
    footer->setAlignment(Qt::AlignCenter);
    sidebarLayout->addWidget(footer);

    pagesWidget = new QStackedWidget;

    QWidget *usersPage = new QWidget;
    usersPage->setStyleSheet("background-color: #f5f6fa;");

    QVBoxLayout *pageLayout = new QVBoxLayout(usersPage);
    pageLayout->setContentsMargins(20, 20, 20, 20);
    pageLayout->setSpacing(15);

    QFrame *header = new QFrame;
    header->setStyleSheet(
        "QFrame {"
        "    background-color: white;"
        "    border-radius: 8px;"
        "    padding: 12px;"
        "}"
        );

    QHBoxLayout *headerLayout = new QHBoxLayout(header);

    QLabel *pageTitle = new QLabel("👥 GESTION DES UTILISATEURS");
    pageTitle->setStyleSheet("color: #2c3e50; font-size: 18px; font-weight: bold;");

    headerLayout->addWidget(pageTitle);
    headerLayout->addStretch();

    searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText("🔍 Rechercher...");
    searchEdit->setStyleSheet(
        "QLineEdit {"
        "    background-color: #ecf0f1;"
        "    border: 1px solid #dfe6e9;"
        "    border-radius: 5px;"
        "    padding: 6px 10px;"
        "    font-size: 12px;"
        "    min-width: 180px;"
        "    color: black;"
        "}"
        );

    QPushButton *searchBtn = createStyledButton("Chercher", "#3498db");
    QPushButton *deleteBtn = createStyledButton("Supprimer", "#e74c3c");

    QPushButton *sortByDateBtn = createStyledButton("📅 Date", "#9b59b6");
    QPushButton *sortByNameBtn = createStyledButton("👤 Nom", "#1abc9c");

    headerLayout->addWidget(searchEdit);
    headerLayout->addWidget(searchBtn);
    headerLayout->addWidget(sortByDateBtn);
    headerLayout->addWidget(sortByNameBtn);

    QPushButton *exportBtn = createStyledButton("📊 Export", "#9b59b6");
    QPushButton *statsBtn = createStyledButton("📈 Statistiques", "#f39c12");
    headerLayout->addWidget(exportBtn);
    headerLayout->addWidget(statsBtn);
    headerLayout->addWidget(deleteBtn);

    pageLayout->addWidget(header);

    QHBoxLayout *contentLayout = new QHBoxLayout;
    contentLayout->setSpacing(15);

    QFrame *formFrame = new QFrame;
    formFrame->setStyleSheet(
        "QFrame {"
        "    background-color: white;"
        "    border-radius: 8px;"
        "    padding: 15px;"
        "}"
        );

    QVBoxLayout *formLayout = new QVBoxLayout(formFrame);
    formLayout->setSpacing(8);

    QLabel *formTitle = new QLabel("Ajouter/Modifier un utilisateur");
    formTitle->setStyleSheet("color: #3498db; font-size: 14px; font-weight: bold; margin-bottom: 8px;");
    formLayout->addWidget(formTitle);

    QString lineEditStyle =
        "QLineEdit, QDateEdit {"
        "    color: black;"
        "    background-color: white;"
        "    border: 1px solid #ced4da;"
        "    border-radius: 4px;"
        "    padding: 6px 10px;"
        "    font-size: 12px;"
        "}"
        "QLineEdit:focus, QDateEdit:focus {"
        "    border: 1px solid #3498db;"
        "    outline: none;"
        "}";

    QString comboBoxStyle =
        "QComboBox {"
        "    color: black;"
        "    background-color: white;"
        "    border: 1px solid #ced4da;"
        "    border-radius: 4px;"
        "    padding: 6px 10px;"
        "    font-size: 12px;"
        "}"
        "QComboBox::drop-down {"
        "    border: none;"
        "    width: 18px;"
        "}"
        "QComboBox::down-arrow {"
        "    image: none;"
        "    border-left: 4px solid transparent;"
        "    border-right: 4px solid transparent;"
        "    border-top: 5px solid #2c3e50;"
        "}"
        "QComboBox QAbstractItemView {"
        "    color: black;"
        "    background-color: white;"
        "    border: 1px solid #ced4da;"
        "    selection-background-color: #3498db;"
        "    selection-color: white;"
        "}";

    idEdit = new QLineEdit;
    idEdit->setReadOnly(true);
    idEdit->setText(QString::number(nextId));
    idEdit->setStyleSheet(
        "QLineEdit {"
        "    background-color: #e9ecef;"
        "    color: #6c757d;"
        "    border: 1px solid #ced4da;"
        "    border-radius: 4px;"
        "    padding: 6px 10px;"
        "    font-size: 12px;"
        "}"
        );

    nameEdit = new QLineEdit;
    nameEdit->setPlaceholderText("Nom (Majuscule)");
    nameEdit->setStyleSheet(lineEditStyle);

    emailEdit = new QLineEdit;
    emailEdit->setPlaceholderText("email@domaine.com");
    emailEdit->setStyleSheet(lineEditStyle);

    phoneEdit = new QLineEdit;
    phoneEdit->setPlaceholderText("Téléphone (8 chiffres)");
    phoneEdit->setStyleSheet(lineEditStyle);
    phoneEdit->setMaxLength(8);

    passwordEdit = new QLineEdit;
    passwordEdit->setPlaceholderText("Mot de passe (6+ caractères, 1 chiffre)");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setStyleSheet(lineEditStyle);

    roleBox = new QComboBox;
    roleBox->addItems({"Admin Utilisateur", "Admin Matériel", "Admin Clients", "Admin Commandes", "Admin Fournisseur"});
    roleBox->setStyleSheet(comboBoxStyle);
    statusBox = new QComboBox;
    statusBox->addItems({"Actif", "Inactif"});
    statusBox->setStyleSheet(comboBoxStyle);

    creationDateEdit = new QDateEdit;
    creationDateEdit->setDate(QDate::currentDate());
    creationDateEdit->setDisplayFormat("dd/MM/yyyy");
    creationDateEdit->setStyleSheet(lineEditStyle);

    QGroupBox *securityGroup = new QGroupBox("Questions de sécurité");
    securityGroup->setStyleSheet(
        "QGroupBox {"
        "    font-weight: bold;"
        "    color: #2c3e50;"
        "    border: 1px solid #ced4da;"
        "    border-radius: 6px;"
        "    margin-top: 8px;"
        "    padding-top: 8px;"
        "    font-size: 11px;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 8px;"
        "    padding: 0 5px 0 5px;"
        "}"
        );

    QVBoxLayout *securityLayout = new QVBoxLayout(securityGroup);
    securityLayout->setSpacing(5);

    QLabel *colorLabel = new QLabel("🎨 Couleur préférée :");
    colorLabel->setStyleSheet("color: #2c3e50; font-size: 11px;");
    favoriteColorEdit = new QLineEdit;
    favoriteColorEdit->setPlaceholderText("Ex: Bleu");
    favoriteColorEdit->setStyleSheet(lineEditStyle);

    QLabel *petLabel = new QLabel("🐕 Nom animal :");
    petLabel->setStyleSheet("color: #2c3e50; font-size: 11px;");
    petNameEdit = new QLineEdit;
    petNameEdit->setPlaceholderText("Ex: Max");
    petNameEdit->setStyleSheet(lineEditStyle);

    QLabel *carLabel = new QLabel("🚗 Matricule :");
    carLabel->setStyleSheet("color: #2c3e50; font-size: 11px;");
    carPlateEdit = new QLineEdit;
    carPlateEdit->setPlaceholderText("Ex: 123 TUN 456");
    carPlateEdit->setStyleSheet(lineEditStyle);

    securityLayout->addWidget(colorLabel);
    securityLayout->addWidget(favoriteColorEdit);
    securityLayout->addWidget(petLabel);
    securityLayout->addWidget(petNameEdit);
    securityLayout->addWidget(carLabel);
    securityLayout->addWidget(carPlateEdit);

    QGroupBox *imageGroup = new QGroupBox("Photo de profil");
    imageGroup->setStyleSheet(
        "QGroupBox {"
        "    font-weight: bold;"
        "    color: #2c3e50;"
        "    border: 1px solid #ced4da;"
        "    border-radius: 6px;"
        "    margin-top: 8px;"
        "    padding-top: 8px;"
        "    font-size: 11px;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 8px;"
        "    padding: 0 5px 0 5px;"
        "}"
        );

    QVBoxLayout *imageLayout = new QVBoxLayout(imageGroup);
    imageLayout->setSpacing(8);

    imagePreviewLabel = new QLabel;
    imagePreviewLabel->setFixedSize(150, 150);
    imagePreviewLabel->setStyleSheet(
        "QLabel {"
        "    border: 2px solid #ced4da;"
        "    border-radius: 75px;"
        "    background-color: #f8f9fa;"
        "}"
        );
    imagePreviewLabel->setAlignment(Qt::AlignCenter);
    imagePreviewLabel->setText("Aucune image");
    imagePreviewLabel->setScaledContents(false);

    QHBoxLayout *previewLayout = new QHBoxLayout;
    previewLayout->addStretch();
    previewLayout->addWidget(imagePreviewLabel);
    previewLayout->addStretch();
    imageLayout->addLayout(previewLayout);

    faceValidationLabel = new QLabel("Aucune image sélectionnée");
    faceValidationLabel->setAlignment(Qt::AlignCenter);
    faceValidationLabel->setStyleSheet("color: #7f8c8d; font-size: 11px;");
    imageLayout->addWidget(faceValidationLabel);

    QHBoxLayout *imageButtonsLayout = new QHBoxLayout;
    selectImageBtn = createStyledButton("📷 Choisir photo", "#3498db");
    clearImageBtn = createStyledButton("🗑️ Effacer", "#95a5a6");

    selectImageBtn->setFixedHeight(32);
    clearImageBtn->setFixedHeight(32);

    imageButtonsLayout->addWidget(selectImageBtn);
    imageButtonsLayout->addWidget(clearImageBtn);
    imageLayout->addLayout(imageButtonsLayout);

    QFormLayout *inputForm = new QFormLayout;
    inputForm->setSpacing(6);
    inputForm->setContentsMargins(0, 0, 0, 0);

    QString labelStyle =
        "QLabel {"
        "    color: #2c3e50;"
        "    font-weight: 600;"
        "    font-size: 11px;"
        "    padding: 2px 0;"
        "}";

    QLabel *idLabel = new QLabel("ID:");
    idLabel->setStyleSheet(labelStyle);
    QLabel *nameLabel = new QLabel("Nom:");
    nameLabel->setStyleSheet(labelStyle);
    QLabel *emailLabel = new QLabel("Email:");
    emailLabel->setStyleSheet(labelStyle);
    QLabel *phoneLabel = new QLabel("Tél:");
    phoneLabel->setStyleSheet(labelStyle);
    QLabel *passwordLabel = new QLabel("Mot de passe:");
    passwordLabel->setStyleSheet(labelStyle);
    QLabel *roleLabel = new QLabel("Rôle:");
    roleLabel->setStyleSheet(labelStyle);
    QLabel *statusLabel = new QLabel("Statut:");
    statusLabel->setStyleSheet(labelStyle);
    QLabel *dateLabel = new QLabel("Date:");
    dateLabel->setStyleSheet(labelStyle);

    inputForm->addRow(idLabel, idEdit);
    inputForm->addRow(nameLabel, nameEdit);
    inputForm->addRow(emailLabel, emailEdit);
    inputForm->addRow(phoneLabel, phoneEdit);
    inputForm->addRow(passwordLabel, passwordEdit);
    inputForm->addRow(roleLabel, roleBox);
    inputForm->addRow(statusLabel, statusBox);
    inputForm->addRow(dateLabel, creationDateEdit);

    int fieldHeight = 32;
    idEdit->setFixedHeight(fieldHeight);
    nameEdit->setFixedHeight(fieldHeight);
    emailEdit->setFixedHeight(fieldHeight);
    phoneEdit->setFixedHeight(fieldHeight);
    passwordEdit->setFixedHeight(fieldHeight);
    roleBox->setFixedHeight(fieldHeight);
    statusBox->setFixedHeight(fieldHeight);
    creationDateEdit->setFixedHeight(fieldHeight);
    favoriteColorEdit->setFixedHeight(fieldHeight);
    petNameEdit->setFixedHeight(fieldHeight);
    carPlateEdit->setFixedHeight(fieldHeight);

    inputForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    int labelWidth = 90;
    idLabel->setFixedWidth(labelWidth);
    nameLabel->setFixedWidth(labelWidth);
    emailLabel->setFixedWidth(labelWidth);
    phoneLabel->setFixedWidth(labelWidth);
    passwordLabel->setFixedWidth(labelWidth);
    roleLabel->setFixedWidth(labelWidth);
    statusLabel->setFixedWidth(labelWidth);
    dateLabel->setFixedWidth(labelWidth);

    formLayout->addLayout(inputForm);
    formLayout->addWidget(securityGroup);
    formLayout->addWidget(imageGroup);

    QSpacerItem *verticalSpacer = new QSpacerItem(20, 8, QSizePolicy::Minimum, QSizePolicy::Fixed);
    formLayout->addItem(verticalSpacer);

    QHBoxLayout *formButtons = new QHBoxLayout;
    formButtons->setSpacing(8);
    formButtons->setContentsMargins(0, 5, 0, 0);

    QPushButton *addBtn = createStyledButton("➕ Ajouter", "#2ecc71");
    modifyBtn = createStyledButton("✏️ Modifier", "#3498db");
    QPushButton *clearBtn = createStyledButton("🔄 Effacer", "#95a5a6");

    int buttonHeight = 32;
    addBtn->setFixedHeight(buttonHeight);
    modifyBtn->setFixedHeight(buttonHeight);
    clearBtn->setFixedHeight(buttonHeight);

    addBtn->setMinimumWidth(90);
    modifyBtn->setMinimumWidth(90);
    clearBtn->setMinimumWidth(90);

    formButtons->addWidget(addBtn);
    formButtons->addWidget(modifyBtn);
    formButtons->addWidget(clearBtn);

    formLayout->addLayout(formButtons);
    formLayout->addStretch();

    contentLayout->addWidget(formFrame, 1);

    QFrame *tableFrame = new QFrame;
    tableFrame->setStyleSheet(
        "QFrame {"
        "    background-color: white;"
        "    border-radius: 8px;"
        "}"
        );

    QVBoxLayout *tableFrameLayout = new QVBoxLayout(tableFrame);
    tableFrameLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *tableTitle = new QLabel("Liste des utilisateurs");
    tableTitle->setStyleSheet("color: #2c3e50; font-size: 13px; font-weight: bold; padding: 12px 15px 8px 15px;");
    tableFrameLayout->addWidget(tableTitle);

    usersTable = new QTableWidget(0, 6);
    QStringList headers = {"ID", "NOM", "EMAIL", "RÔLE", "STATUT", "DATE"};
    usersTable->setHorizontalHeaderLabels(headers);

    usersTable->setStyleSheet(
        "QTableWidget {"
        "    background-color: white;"
        "    border: none;"
        "    font-size: 11px;"
        "}"
        "QTableWidget::item {"
        "    padding: 8px;"
        "    border-bottom: 1px solid #f1f2f6;"
        "    color: #2c3e50;"
        "}"
        "QHeaderView::section {"
        "    background-color: #2c3e50;"
        "    color: white;"
        "    padding: 8px;"
        "    border: none;"
        "    font-size: 11px;"
        "    font-weight: bold;"
        "}"
        );

    usersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    usersTable->verticalHeader()->setVisible(false);
    usersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    usersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    tableFrameLayout->addWidget(usersTable, 1);
    contentLayout->addWidget(tableFrame, 2);

    pageLayout->addLayout(contentLayout, 1);

    pagesWidget->addWidget(usersPage);

    // Integation Matériel
    navList->addItem("📦 Matériel");
    Matriele *materialPage = new Matriele(this);
    pagesWidget->addWidget(materialPage);

    connect(navList, &QListWidget::currentRowChanged, pagesWidget, &QStackedWidget::setCurrentIndex);

    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(pagesWidget, 1);

    setWindowTitle("Système de Gestion - Utilisateurs");
    resize(1920, 1080);

    QRect screenGeometry = QGuiApplication::primaryScreen()->availableGeometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);

    // Connexion Oracle
    if (Connection::getInstance()->isOpen()) {
        databaseEnabled = true;
        qDebug() << "Chargement des données depuis Oracle...";
        loadUsersFromDatabase();
    } else {
        databaseEnabled = false;
        qDebug() << "Mode hors ligne - utilisation des données locales";
        User user1 = {1, "Admin Principal", "admin@entreprise.com", "Admin Utilisateur", "admin123", "12345678", "Actif", "Bleu", "Max", "123 TUN 456", QDate(2024, 1, 15), QImage(), false};
        User user2 = {2, "Gestionnaire Matériel", "materiel@entreprise.com", "Admin Matériel", "materiel123", "23456789", "Actif", "Vert", "Bella", "234 TUN 567", QDate(2024, 3, 10), QImage(), false};
        User user3 = {3, "Gestionnaire Clients", "clients@entreprise.com", "Admin Clients", "clients123", "34567890", "Actif", "Rouge", "Charlie", "345 TUN 678", QDate(2024, 2, 20), QImage(), false};
        User user4 = {4, "Gestionnaire Commandes", "commandes@entreprise.com", "Admin Commandes", "commandes123", "45678901", "Actif", "Jaune", "Luna", "456 TUN 789", QDate(2024, 5, 5), QImage(), false};
        User user5 = {5, "Gestionnaire Fournisseur", "fournisseur@entreprise.com", "Admin Fournisseur", "fournisseur123", "56789012", "Inactif", "Noir", "Rocky", "567 TUN 890", QDate(2024, 4, 1), QImage(), false};
        usersList << user1 << user2 << user3 << user4 << user5;
        nextId = 6;
        idEdit->setText(QString::number(nextId));
    }

    updateUsersTable();

    QObject::connect(addBtn,        &QPushButton::clicked, this, &UserWindow::addUser);
    QObject::connect(deleteBtn,     &QPushButton::clicked, this, &UserWindow::deleteUser);
    QObject::connect(modifyBtn,     &QPushButton::clicked, this, &UserWindow::modifyUser);
    QObject::connect(clearBtn,      &QPushButton::clicked, this, &UserWindow::clearFields);
    QObject::connect(searchBtn,     &QPushButton::clicked, this, &UserWindow::searchUser);
    QObject::connect(sortByDateBtn, &QPushButton::clicked, this, &UserWindow::sortUsersByDate);
    QObject::connect(sortByNameBtn, &QPushButton::clicked, this, &UserWindow::sortUsersByName);
    QObject::connect(exportBtn,     &QPushButton::clicked, this, &UserWindow::exportUsers);
    QObject::connect(statsBtn,      &QPushButton::clicked, this, &UserWindow::showRoleStats);
    QObject::connect(selectImageBtn,&QPushButton::clicked, this, &UserWindow::selectProfileImage);
    QObject::connect(clearImageBtn, &QPushButton::clicked, this, &UserWindow::clearProfileImage);

    QObject::connect(usersTable, &QTableWidget::itemSelectionChanged, [this]() {
        int row = usersTable->currentRow();
        if (row >= 0) {
            int id = usersTable->item(row, 0)->text().toInt();
            for (const User &user : std::as_const(usersList)) {
                if (user.id == id) {
                    idEdit->setText(QString::number(user.id));
                    nameEdit->setText(user.name);
                    emailEdit->setText(user.email);
                    phoneEdit->setText(user.phone);
                    passwordEdit->clear();

                    int roleIndex = roleBox->findText(user.role);
                    if (roleIndex >= 0) roleBox->setCurrentIndex(roleIndex);

                    // Afficher Actif/Inactif (avec majuscule) dans le combo
                    QString displayStatus = user.status.isEmpty() ? user.status
                                                                  : user.status.at(0).toUpper() + user.status.mid(1).toLower();
                    int statusIndex = statusBox->findText(displayStatus);
                    if (statusIndex >= 0) statusBox->setCurrentIndex(statusIndex);

                    creationDateEdit->setDate(user.creationDate);
                    favoriteColorEdit->setText(user.favoriteColor);
                    petNameEdit->setText(user.petName);
                    carPlateEdit->setText(user.carPlate);

                    if (!user.profileImage.isNull()) {
                        currentProfileImage = user.profileImage;
                        currentImageHasFace = user.hasFace;

                        QPixmap pixmap = QPixmap::fromImage(user.profileImage);
                        QPixmap scaled = pixmap.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation);

                        QPixmap circularPixmap(scaled.size());
                        circularPixmap.fill(Qt::transparent);
                        QPainter painter(&circularPixmap);
                        painter.setRenderHint(QPainter::Antialiasing);
                        painter.setBrush(QBrush(scaled));
                        painter.setPen(Qt::NoPen);
                        painter.drawEllipse(0, 0, scaled.width(), scaled.height());
                        painter.end();

                        imagePreviewLabel->setPixmap(circularPixmap);
                        if (user.hasFace) {
                            faceValidationLabel->setText("✅ Visage humain validé");
                            faceValidationLabel->setStyleSheet("color: #27ae60; font-size: 11px; font-weight: bold;");
                        } else {
                            faceValidationLabel->setText("⚠️ Photo sans visage");
                            faceValidationLabel->setStyleSheet("color: #f39c12; font-size: 11px; font-weight: bold;");
                        }
                    } else {
                        clearProfileImage();
                    }
                    break;
                }
            }
        }
    });
}

QPushButton* UserWindow::createStyledButton(const QString& text, const QString& color) {
    QPushButton *button = new QPushButton(text);
    button->setStyleSheet(QString(
                              "QPushButton {"
                              "    background-color: %1;"
                              "    color: white;"
                              "    border: none;"
                              "    border-radius: 5px;"
                              "    padding: 6px 12px;"
                              "    font-size: 11px;"
                              "    font-weight: 500;"
                              "}"
                              "QPushButton:hover {"
                              "    opacity: 0.9;"
                              "}"
                              ).arg(color));
    button->setCursor(Qt::PointingHandCursor);
    return button;
}

// ==================== VALIDATION ====================

bool UserWindow::validateName(const QString &name)
{
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Erreur de validation", "Le nom ne peut pas être vide.");
        return false;
    }
    if (name.length() < 2) {
        QMessageBox::warning(this, "Erreur de validation", "Le nom doit contenir au moins 2 caractères.");
        return false;
    }
    if (name.length() > 100) {
        QMessageBox::warning(this, "Erreur de validation", "Le nom ne peut pas dépasser 100 caractères.");
        return false;
    }
    if (!name[0].isUpper()) {
        QMessageBox::warning(this, "Erreur de validation",
                             "La première lettre du nom doit être une majuscule.\nExemple: Jean, Marie, Ahmed");
        return false;
    }
    QRegularExpression nameRegex("^[A-Z][A-Za-zÀ-ÿ\\s\\-']+$");
    if (!nameRegex.match(name).hasMatch()) {
        QMessageBox::warning(this, "Erreur de validation",
                             "Le nom doit commencer par une majuscule et ne contenir que des lettres, espaces, tirets et apostrophes.");
        return false;
    }
    return true;
}

bool UserWindow::validateEmail(const QString &email)
{
    if (email.isEmpty()) {
        QMessageBox::warning(this, "Erreur de validation", "L'email ne peut pas être vide.");
        return false;
    }
    if (email.length() > 150) {
        QMessageBox::warning(this, "Erreur de validation", "L'email ne peut pas dépasser 150 caractères.");
        return false;
    }
    QRegularExpression emailRegex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    if (!emailRegex.match(email).hasMatch()) {
        QMessageBox::warning(this, "Erreur de validation",
                             "Format d'email invalide. Exemple: utilisateur@domaine.com");
        return false;
    }
    return true;
}

bool UserWindow::validatePassword(const QString &password)
{
    if (password.isEmpty()) {
        QMessageBox::warning(this, "Erreur de validation", "Le mot de passe ne peut pas être vide.");
        return false;
    }
    if (password.length() < 6) {
        QMessageBox::warning(this, "Erreur de validation", "Le mot de passe doit contenir au moins 6 caractères.");
        return false;
    }
    if (password.length() > 128) {
        QMessageBox::warning(this, "Erreur de validation", "Le mot de passe ne peut pas dépasser 128 caractères.");
        return false;
    }
    bool hasLetter = false;
    for (QChar c : password) { if (c.isLetter()) { hasLetter = true; break; } }
    if (!hasLetter) {
        QMessageBox::warning(this, "Erreur de validation", "Le mot de passe doit contenir au moins une lettre.");
        return false;
    }
    bool hasDigit = false;
    for (QChar c : password) { if (c.isDigit()) { hasDigit = true; break; } }
    if (!hasDigit) {
        QMessageBox::warning(this, "Erreur de validation", "Le mot de passe doit contenir au moins un chiffre.");
        return false;
    }
    return true;
}

bool UserWindow::validatePhone(const QString &phone)
{
    if (phone.isEmpty()) {
        QMessageBox::warning(this, "Erreur de validation", "Le numéro de téléphone ne peut pas être vide.");
        return false;
    }
    if (phone.length() != 8) {
        QMessageBox::warning(this, "Erreur de validation",
                             "Le numéro de téléphone doit contenir exactement 8 chiffres.\nExemple: 12345678");
        return false;
    }
    for (QChar c : phone) {
        if (!c.isDigit()) {
            QMessageBox::warning(this, "Erreur de validation",
                                 "Le numéro de téléphone ne doit contenir que des chiffres.");
            return false;
        }
    }
    return true;
}

bool UserWindow::validateSecurityAnswers()
{
    if (favoriteColorEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Erreur de validation", "Veuillez indiquer votre couleur préférée.");
        return false;
    }
    if (petNameEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Erreur de validation", "Veuillez indiquer le nom de votre animal de compagnie.");
        return false;
    }
    if (carPlateEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Erreur de validation", "Veuillez indiquer la matricule de votre voiture.");
        return false;
    }
    if (carPlateEdit->text().length() < 5) {
        QMessageBox::warning(this, "Erreur de validation", "La matricule de la voiture est trop courte.");
        return false;
    }
    return true;
}

// FIX: non-bloquante — photo optionnelle, détection informative seulement
bool UserWindow::validateFaceImage(const QImage &image)
{
    if (image.isNull()) {
        currentImageHasFace = false;
        return true; // photo optionnelle
    }
    currentImageHasFace = detectFace(image);
    if (!currentImageHasFace) {
        faceValidationLabel->setText("⚠️ Photo acceptée (visage non confirmé)");
        faceValidationLabel->setStyleSheet("color: #f39c12; font-size: 11px; font-weight: bold;");
    }
    return true; // toujours valide, même sans visage détecté
}

bool UserWindow::detectFace(const QImage &image)
{
    if (image.isNull()) return false;

    QImage processed = preprocessImage(image);
    int width = processed.width();
    int height = processed.height();

    if (width < 100 || height < 100) {
        faceValidationLabel->setText("❌ Image trop petite (min 100x100)");
        faceValidationLabel->setStyleSheet("color: #e74c3c; font-size: 11px; font-weight: bold;");
        return false;
    }

    int skinPixelCount = 0;
    int totalPixels = width * height;
    int step = qMax(1, width / 50);

    for (int y = 0; y < height; y += step) {
        for (int x = 0; x < width; x += step) {
            QColor pixel = processed.pixelColor(x, y);
            int r = pixel.red(), g = pixel.green(), b = pixel.blue();
            if (r > g && r > b && r > 60 && g > 40 && b > 30) {
                if (abs(r - g) < 80 && abs(r - b) < 80) skinPixelCount++;
            }
        }
    }

    float skinPercentage = (float)skinPixelCount / (totalPixels / (step * step)) * 100;
    bool hasFaceLikeRegion = skinPercentage > 5.0f && skinPercentage < 40.0f;
    bool hasGoodAspectRatio = (float)width / height > 0.6f && (float)width / height < 0.9f;

    if (hasFaceLikeRegion && hasGoodAspectRatio) {
        faceValidationLabel->setText("✅ Visage humain détecté");
        faceValidationLabel->setStyleSheet("color: #27ae60; font-size: 11px; font-weight: bold;");
        return true;
    } else if (hasFaceLikeRegion) {
        faceValidationLabel->setText("⚠️ Visage détecté mais qualité médiocre");
        faceValidationLabel->setStyleSheet("color: #f39c12; font-size: 11px; font-weight: bold;");
        return true;
    } else {
        faceValidationLabel->setText("❌ Aucun visage humain détecté");
        faceValidationLabel->setStyleSheet("color: #e74c3c; font-size: 11px; font-weight: bold;");
        return false;
    }
}

QImage UserWindow::preprocessImage(const QImage &image)
{
    QImage processed = image.convertToFormat(QImage::Format_RGB32);
    if (processed.width() > 400 || processed.height() > 400)
        processed = processed.scaled(400, 400, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QImage result(processed.size(), QImage::Format_RGB32);
    for (int y = 0; y < processed.height(); ++y) {
        for (int x = 0; x < processed.width(); ++x) {
            QColor color = processed.pixelColor(x, y);
            int r = qBound(0, (int)((color.red()   - 128) * 1.2 + 128), 255);
            int g = qBound(0, (int)((color.green() - 128) * 1.2 + 128), 255);
            int b = qBound(0, (int)((color.blue()  - 128) * 1.2 + 128), 255);
            result.setPixelColor(x, y, QColor(r, g, b));
        }
    }
    return result;
}

void UserWindow::selectProfileImage()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Sélectionner une photo de profil",
                                                    "",
                                                    "Images (*.png *.jpg *.jpeg *.bmp)");
    if (fileName.isEmpty()) return;

    QImage image;
    if (!image.load(fileName)) {
        QMessageBox::warning(this, "Erreur", "Impossible de charger l'image.");
        return;
    }
    if (image.width() < 100 || image.height() < 100) {
        QMessageBox::warning(this, "Erreur", "L'image est trop petite (minimum 100x100 pixels).");
        return;
    }
    if (image.width() > 2000 || image.height() > 2000) {
        QMessageBox::warning(this, "Erreur", "L'image est trop grande (maximum 2000x2000 pixels).");
        return;
    }

    // Réduire l'image pour l'insertion DB pour ne pas dépasser les limites (ORA-01461)
    currentProfileImage = image.scaled(400, 400, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QPixmap pixmap = QPixmap::fromImage(currentProfileImage);
    QPixmap scaled = pixmap.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QPixmap circularPixmap(scaled.size());
    circularPixmap.fill(Qt::transparent);
    QPainter painter(&circularPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(scaled));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, scaled.width(), scaled.height());
    painter.end();

    imagePreviewLabel->setPixmap(circularPixmap);
    currentImageHasFace = true; // Si l'image existe on met Y dans A_VISAGE
    faceValidationLabel->setText("✅ Image acceptée");
    faceValidationLabel->setStyleSheet("color: #27ae60; font-size: 11px; font-weight: bold;");
}

void UserWindow::clearProfileImage()
{
    currentProfileImage = QImage();
    currentImageHasFace = false;
    imagePreviewLabel->setPixmap(QPixmap());
    imagePreviewLabel->setText("Aucune image");
    faceValidationLabel->setText("Aucune image sélectionnée");
    faceValidationLabel->setStyleSheet("color: #7f8c8d; font-size: 11px;");
}

// ==================== FONCTIONS UTILISATEURS ====================

void UserWindow::addUser()
{
    if (!validateName(nameEdit->text()))          return;
    if (!validateEmail(emailEdit->text()))         return;
    if (!validatePhone(phoneEdit->text()))         return;
    if (!validatePassword(passwordEdit->text()))   return;
    if (!validateSecurityAnswers())                return;

    User newUser;

    newUser.name          = nameEdit->text().trimmed();
    newUser.email         = emailEdit->text().trimmed();
    newUser.phone         = phoneEdit->text().trimmed();
    newUser.password      = passwordEdit->text();
    newUser.role          = roleBox->currentText();
    newUser.status        = statusBox->currentText();
    newUser.creationDate  = creationDateEdit->date();
    newUser.favoriteColor = favoriteColorEdit->text().trimmed();
    newUser.petName       = petNameEdit->text().trimmed();
    newUser.carPlate      = carPlateEdit->text().trimmed();

    newUser.profileImage  = currentProfileImage;
    newUser.hasFace       = !currentProfileImage.isNull();

    if (databaseEnabled) {
        QSqlDatabase db = Connection::getInstance()->getDatabase();

        if (!db.isOpen()) {
            QMessageBox::warning(this, "Erreur", "Base de données non connectée.");
            return;
        }

        QSqlQuery checkEmail(db);
        checkEmail.prepare("SELECT COUNT(*) FROM UTILISATEUR WHERE EMAIL = :email");
        checkEmail.bindValue(":email", newUser.email);

        if (!checkEmail.exec()) {
            QMessageBox::warning(this, "Erreur Oracle", checkEmail.lastError().text());
            return;
        }

        if (checkEmail.next() && checkEmail.value(0).toInt() > 0) {
            QMessageBox::warning(this, "Email dupliqué", "Cet email est déjà utilisé.");
            return;
        }

        QSqlQuery checkPhone(db);
        checkPhone.prepare("SELECT COUNT(*) FROM UTILISATEUR WHERE TELEPHONE = :telephone");
        checkPhone.bindValue(":telephone", newUser.phone);

        if (!checkPhone.exec()) {
            QMessageBox::warning(this, "Erreur Oracle", checkPhone.lastError().text());
            return;
        }

        if (checkPhone.next() && checkPhone.value(0).toInt() > 0) {
            QMessageBox::warning(this, "Téléphone dupliqué", "Ce numéro de téléphone est déjà utilisé.");
            return;
        }

        QSqlQuery qMax(db);
        if (!qMax.exec("SELECT NVL(MAX(ID_UTILISATEUR), 0) FROM UTILISATEUR")) {
            QMessageBox::warning(this, "Erreur Oracle", qMax.lastError().text());
            return;
        }

        if (qMax.next()) {
            newUser.id = qMax.value(0).toInt() + 1;
        } else {
            newUser.id = nextId;
        }

        // Utilisation du helper testé et fonctionnel
        if (!saveUserToDatabase(newUser)) {
            return;
        }

        usersList.append(newUser);
    } else {
        newUser.id = nextId;

        for (const User &u : std::as_const(usersList)) {
            if (u.email == newUser.email) {
                QMessageBox::warning(this, "Email dupliqué", "Cet email est déjà utilisé.");
                return;
            }

            if (u.phone == newUser.phone) {
                QMessageBox::warning(this, "Téléphone dupliqué", "Ce numéro de téléphone est déjà utilisé.");
                return;
            }
        }

        usersList.append(newUser);
    }

    nextId = newUser.id + 1;
    idEdit->setText(QString::number(nextId));

    updateUsersTable();
    clearFields();

    QMessageBox::information(this, "Succès",
                             QString("Utilisateur '%1' ajouté avec succès !").arg(newUser.name));
}

void UserWindow::modifyUser()
{
    int currentId = idEdit->text().toInt();
    if (currentId <= 0) {
        QMessageBox::warning(this, "Erreur", "Sélectionnez un utilisateur à modifier.");
        return;
    }

    if (!validateName(nameEdit->text()))    return;
    if (!validateEmail(emailEdit->text()))  return;
    if (!validatePhone(phoneEdit->text()))  return;
    if (!validateSecurityAnswers())         return;

    // Photo optionnelle — non-bloquante
    if (!currentProfileImage.isNull())
        validateFaceImage(currentProfileImage);

    for (int i = 0; i < usersList.size(); ++i) {
        if (usersList[i].id == currentId) {

            if (usersList[i].email != emailEdit->text()) {
                if (databaseEnabled) {
                    QSqlDatabase db = Connection::getInstance()->getDatabase();
                    QSqlQuery query(db);
                    query.prepare("SELECT COUNT(*) FROM UTILISATEUR WHERE EMAIL = :email AND ID_UTILISATEUR != :id");
                    query.bindValue(":email", emailEdit->text());
                    query.bindValue(":id", currentId);
                    if (query.exec() && query.next() && query.value(0).toInt() > 0) {
                        QMessageBox::warning(this, "Email dupliqué", "Cet email est déjà utilisé.");
                        return;
                    }
                } else {
                    for (const User &u : std::as_const(usersList)) {
                        if (u.email == emailEdit->text() && u.id != currentId) {
                            QMessageBox::warning(this, "Email dupliqué", "Cet email est déjà utilisé.");
                            return;
                        }
                    }
                }
            }

            if (usersList[i].phone != phoneEdit->text()) {
                if (databaseEnabled) {
                    QSqlDatabase db = Connection::getInstance()->getDatabase();
                    QSqlQuery query(db);
                    query.prepare("SELECT COUNT(*) FROM UTILISATEUR WHERE TELEPHONE = :phone AND ID_UTILISATEUR != :id");
                    query.bindValue(":phone", phoneEdit->text());
                    query.bindValue(":id", currentId);
                    if (query.exec() && query.next() && query.value(0).toInt() > 0) {
                        QMessageBox::warning(this, "Téléphone dupliqué", "Ce numéro de téléphone est déjà utilisé.");
                        return;
                    }
                } else {
                    for (const User &u : std::as_const(usersList)) {
                        if (u.phone == phoneEdit->text() && u.id != currentId) {
                            QMessageBox::warning(this, "Téléphone dupliqué", "Ce numéro de téléphone est déjà utilisé.");
                            return;
                        }
                    }
                }
            }

            usersList[i].name  = nameEdit->text();
            usersList[i].email = emailEdit->text();
            usersList[i].phone = phoneEdit->text();

            if (!passwordEdit->text().isEmpty()) {
                if (!validatePassword(passwordEdit->text())) return;
                usersList[i].password = passwordEdit->text();
            }

            usersList[i].role         = roleBox->currentText();
            usersList[i].status       = statusBox->currentText();
            usersList[i].creationDate = creationDateEdit->date();
            usersList[i].favoriteColor= favoriteColorEdit->text();
            usersList[i].petName      = petNameEdit->text();
            usersList[i].carPlate     = carPlateEdit->text();

            if (!currentProfileImage.isNull()) {
                usersList[i].profileImage = currentProfileImage;
                usersList[i].hasFace      = true; // Simplifié : Y si l'image est là
            }

            if (databaseEnabled) {
                updateUserInDatabase(usersList[i]);
                
                // "modify should happens the same": On réécrit complètement la table 
                QSqlDatabase db = Connection::getInstance()->getDatabase();
                db.transaction();
                QSqlQuery delQuery(db);
                if (delQuery.exec("DELETE FROM UTILISATEUR")) {
                    for (const User &u : std::as_const(usersList)) {
                        saveUserToDatabase(u);
                    }
                    db.commit();
                } else {
                    db.rollback();
                }
            }
            break;
        }
    }

    updateUsersTable();
    QMessageBox::information(this, "Succès", "Utilisateur modifié avec succès !");
}

void UserWindow::deleteUser()
{
    int row = usersTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Information", "Sélectionnez un utilisateur à supprimer.");
        return;
    }

    int id = usersTable->item(row, 0)->text().toInt();
    QString name = usersTable->item(row, 1)->text();

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirmation",
                                                              QString("Êtes-vous sûr de vouloir supprimer l'utilisateur '%1' ?").arg(name),
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No) return;

    for (int i = 0; i < usersList.size(); ++i) {
        if (usersList[i].id == id) { 
            usersList.removeAt(i); 
            break; 
        }
    }

    if (databaseEnabled) {
        deleteUserFromDatabase(id);
        
        // Exactement comme Modify et Trie : on recrée physiquement la base
        QSqlDatabase db = Connection::getInstance()->getDatabase();
        db.transaction();
        QSqlQuery delQuery(db);
        if (delQuery.exec("DELETE FROM UTILISATEUR")) {
            for (const User &u : std::as_const(usersList)) {
                saveUserToDatabase(u);
            }
            db.commit();
        } else {
            db.rollback();
        }
    }

    updateUsersTable();
    clearFields();
    QMessageBox::information(this, "Succès", "Utilisateur supprimé avec succès !");
}

void UserWindow::searchUser()
{
    QString searchText = searchEdit->text().trimmed();

    if (databaseEnabled) {
        QSqlDatabase db = Connection::getInstance()->getDatabase();
        if (!db.isOpen()) return;

        QString sql = "SELECT ID_UTILISATEUR, NOM, EMAIL, TELEPHONE, MOT_DE_PASSE, ROLE, STATUT, "
                      "DATE_CREATION, COULEUR_FAVORITE, NOM_ANIMAL, MATRICULE_VOITURE, "
                      "PHOTO_PROFIL, A_VISAGE FROM UTILISATEUR ";

        if (!searchText.isEmpty()) {
            sql += "WHERE LOWER(NOM) LIKE :search OR LOWER(EMAIL) LIKE :search OR LOWER(TELEPHONE) LIKE :search OR LOWER(ROLE) LIKE :search ";
        }
        sql += "ORDER BY ID_UTILISATEUR";

        QSqlQuery query(db);
        query.prepare(sql);
        if (!searchText.isEmpty()) {
            QString wildcard = "%" + searchText.toLower() + "%";
            query.bindValue(":search", wildcard);
        }

        if (query.exec()) {
            populateUsersListFromQuery(query);
            if (usersList.isEmpty() && !searchText.isEmpty()) {
                QMessageBox::information(this, "Recherche", "Aucun utilisateur trouvé dans la base de données.");
            }
        } else {
            QMessageBox::warning(this, "Erreur base de données", query.lastError().text());
        }
    } else {
        if (searchText.isEmpty()) { 
            // Handle local filter clear if needed, but simple updateUsersTable works if we kept the full list somewhere.
            // Since offline mode doesn't keep an unfiltered list, we just update.
            updateUsersTable(); 
            return; 
        }

        usersTable->setRowCount(0);
        for (const User &user : std::as_const(usersList)) {
            if (user.name.contains(searchText,  Qt::CaseInsensitive) ||
                user.email.contains(searchText, Qt::CaseInsensitive) ||
                user.phone.contains(searchText, Qt::CaseInsensitive) ||
                user.role.contains(searchText,  Qt::CaseInsensitive)) {
                int row = usersTable->rowCount();
                usersTable->insertRow(row);
                usersTable->setItem(row, 0, new QTableWidgetItem(QString::number(user.id)));
                usersTable->setItem(row, 1, new QTableWidgetItem(user.name));
                usersTable->setItem(row, 2, new QTableWidgetItem(user.email));
                usersTable->setItem(row, 3, new QTableWidgetItem(user.role));
                usersTable->setItem(row, 4, new QTableWidgetItem(user.status));
                usersTable->setItem(row, 5, new QTableWidgetItem(user.creationDate.toString("dd/MM/yyyy")));
            }
        }
        if (usersTable->rowCount() == 0)
            QMessageBox::information(this, "Recherche", "Aucun utilisateur trouvé.");
    }
}

void UserWindow::clearFields()
{
    nameEdit->clear();
    emailEdit->clear();
    phoneEdit->clear();
    passwordEdit->clear();
    favoriteColorEdit->clear();
    petNameEdit->clear();
    carPlateEdit->clear();
    roleBox->setCurrentIndex(0);
    statusBox->setCurrentIndex(0);
    creationDateEdit->setDate(QDate::currentDate());
    usersTable->clearSelection();
    clearProfileImage();
    idEdit->setText(QString::number(nextId));
}

void UserWindow::updateUsersTable()
{
    usersTable->setRowCount(0);
    for (const User &user : std::as_const(usersList)) {
        int row = usersTable->rowCount();
        usersTable->insertRow(row);
        usersTable->setItem(row, 0, new QTableWidgetItem(QString::number(user.id)));
        usersTable->setItem(row, 1, new QTableWidgetItem(user.name));
        usersTable->setItem(row, 2, new QTableWidgetItem(user.email));
        usersTable->setItem(row, 3, new QTableWidgetItem(user.role));
        usersTable->setItem(row, 4, new QTableWidgetItem(user.status));
        usersTable->setItem(row, 5, new QTableWidgetItem(user.creationDate.toString("dd/MM/yyyy")));
    }
}

void UserWindow::sortUsersByDate()
{
    if (databaseEnabled) {
        QSqlDatabase db = Connection::getInstance()->getDatabase();
        if (db.isOpen()) {
            QSqlQuery query(db);
            // 1. Lire avec le tri souhaité
            if (query.exec("SELECT ID_UTILISATEUR, NOM, EMAIL, TELEPHONE, MOT_DE_PASSE, ROLE, STATUT, "
                           "DATE_CREATION, COULEUR_FAVORITE, NOM_ANIMAL, MATRICULE_VOITURE, "
                           "PHOTO_PROFIL, A_VISAGE FROM UTILISATEUR ORDER BY DATE_CREATION ASC")) {
                populateUsersListFromQuery(query);
                
                // 2. Réorganiser physiquement la BDD !
                db.transaction();
                QSqlQuery delQuery(db);
                if (delQuery.exec("DELETE FROM UTILISATEUR")) {
                    for (const User &u : std::as_const(usersList)) {
                        saveUserToDatabase(u);
                    }
                    db.commit();
                } else {
                    db.rollback();
                }
            }
        }
    } else {
        std::sort(usersList.begin(), usersList.end(), [](const User &a, const User &b) {
            return a.creationDate < b.creationDate;
        });
        updateUsersTable();
    }
}

void UserWindow::sortUsersByName()
{
    if (databaseEnabled) {
        QSqlDatabase db = Connection::getInstance()->getDatabase();
        if (db.isOpen()) {
            QSqlQuery query(db);
            if (query.exec("SELECT ID_UTILISATEUR, NOM, EMAIL, TELEPHONE, MOT_DE_PASSE, ROLE, STATUT, "
                           "DATE_CREATION, COULEUR_FAVORITE, NOM_ANIMAL, MATRICULE_VOITURE, "
                           "PHOTO_PROFIL, A_VISAGE FROM UTILISATEUR ORDER BY NOM ASC")) {
                populateUsersListFromQuery(query);

                // Réorganisation physique
                db.transaction();
                QSqlQuery delQuery(db);
                if (delQuery.exec("DELETE FROM UTILISATEUR")) {
                    for (const User &u : std::as_const(usersList)) {
                        saveUserToDatabase(u);
                    }
                    db.commit();
                } else {
                    db.rollback();
                }
            }
        }
    } else {
        std::sort(usersList.begin(), usersList.end(), [](const User &a, const User &b) {
            if (a.name == b.name) return a.id < b.id;
            return a.name < b.name;
        });
        updateUsersTable();
    }
}

void UserWindow::exportUsers()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter les utilisateurs", "", "Fichiers PDF (*.pdf)");
    if (fileName.isEmpty()) return;

    if (QFileInfo(fileName).suffix().isEmpty()) {
        fileName.append(".pdf");
    }

    QPdfWriter pdfWriter(fileName);
    pdfWriter.setPageSize(QPageSize(QPageSize::A4));
    pdfWriter.setResolution(300);

    QTextDocument document;
    QString html = "<html><head><style>"
                   "table { width: 100%; border-collapse: collapse; margin-top: 20px; font-family: sans-serif; }"
                   "th { background-color: #2c3e50; color: white; padding: 10px; text-align: left; font-size: 12px; }"
                   "td { border: 1px solid #bdc3c7; padding: 8px; font-size: 11px; }"
                   "h1 { color: #2c3e50; text-align: center; font-family: sans-serif; margin-bottom: 20px; }"
                   "</style></head><body>"
                   "<h1>LISTE DES UTILISATEURS</h1>"
                   "<table>"
                   "<tr><th>ID</th><th>Nom</th><th>Email</th><th>Téléphone</th><th>Rôle</th><th>Statut</th><th>Date</th></tr>";

    if (databaseEnabled) {
        QSqlDatabase db = Connection::getInstance()->getDatabase();
        if (db.isOpen()) {
            QSqlQuery query(db);
            // On exporte dans l'ordre de la BDD actuelle !
            if (query.exec("SELECT ID_UTILISATEUR, NOM, EMAIL, TELEPHONE, ROLE, STATUT, "
                           "DATE_CREATION, COULEUR_FAVORITE, NOM_ANIMAL, MATRICULE_VOITURE "
                           "FROM UTILISATEUR")) {
                int count = 0;
                while (query.next()) {
                    html += "<tr>";
                    html += "<td>" + query.value(0).toString() + "</td>";
                    html += "<td>" + query.value(1).toString() + "</td>";
                    html += "<td>" + query.value(2).toString() + "</td>";
                    html += "<td>" + query.value(3).toString() + "</td>";
                    html += "<td>" + query.value(4).toString() + "</td>";
                    html += "<td>" + query.value(5).toString() + "</td>";
                    html += "<td>" + query.value(6).toDate().toString("dd/MM/yyyy") + "</td>";
                    html += "</tr>";
                    count++;
                }
                html += "</table></body></html>";
                document.setHtml(html);
                document.print(&pdfWriter);
                QMessageBox::information(this, "Succès", QString("%1 utilisateurs exportés en PDF avec succès depuis la Base de Données !").arg(count));
                return;
            }
        }
    }

    // Hors Ligne
    for (const User &user : std::as_const(usersList)) {
        html += "<tr>";
        html += "<td>" + QString::number(user.id) + "</td>";
        html += "<td>" + user.name + "</td>";
        html += "<td>" + user.email + "</td>";
        html += "<td>" + user.phone + "</td>";
        html += "<td>" + user.role + "</td>";
        html += "<td>" + user.status + "</td>";
        html += "<td>" + user.creationDate.toString("dd/MM/yyyy") + "</td>";
        html += "</tr>";
    }
    html += "</table></body></html>";
    document.setHtml(html);
    document.print(&pdfWriter);
    QMessageBox::information(this, "Succès", QString("%1 utilisateurs exportés en PDF avec succès depuis la base locale !").arg(usersList.size()));
}

// ==================== BASE DE DONNÉES ====================

void UserWindow::populateUsersListFromQuery(QSqlQuery &query)
{
    usersList.clear();
    while (query.next()) {
        User user;
        user.id           = query.value(0).toInt();
        user.name         = query.value(1).toString();
        user.email        = query.value(2).toString();
        user.phone        = query.value(3).toString();
        user.password     = query.value(4).toString();
        user.role         = query.value(5).toString();
        
        QString rawStatus = query.value(6).toString();
        user.status       = rawStatus.isEmpty() ? rawStatus
                                          : rawStatus.at(0).toUpper() + rawStatus.mid(1).toLower();
        user.creationDate  = query.value(7).toDate();
        user.favoriteColor = query.value(8).toString();
        user.petName       = query.value(9).toString();
        user.carPlate      = query.value(10).toString();

        QString photoPath = query.value(11).toString();
        if (!photoPath.isEmpty() && QFile::exists(photoPath)) {
            user.profileImage.load(photoPath);
            user.hasFace = (query.value(12).toString() == "Y");
        } else {
            user.hasFace = false;
        }

        usersList.append(user);
        if (user.id >= nextId) nextId = user.id + 1;
    }
    updateUsersTable();
    idEdit->setText(QString::number(nextId));
    qDebug() << "Liste d'utilisateurs mise à jour, taille :" << usersList.size();
}

void UserWindow::loadUsersFromDatabase()
{
    if (!databaseEnabled) return;

    QSqlDatabase db = Connection::getInstance()->getDatabase();
    if (!db.isOpen()) {
        QMessageBox::warning(this, "Erreur Base de données", "Base de données non connectée !");
        return;
    }

    QSqlQuery query(db);
    if (query.exec("SELECT ID_UTILISATEUR, NOM, EMAIL, TELEPHONE, MOT_DE_PASSE, ROLE, STATUT, "
                   "DATE_CREATION, COULEUR_FAVORITE, NOM_ANIMAL, MATRICULE_VOITURE, "
                   "PHOTO_PROFIL, A_VISAGE FROM UTILISATEUR ORDER BY ID_UTILISATEUR")) {
        populateUsersListFromQuery(query);
    } else {
        qDebug() << "Erreur chargement utilisateurs:" << query.lastError().text();
        QMessageBox::warning(this, "Erreur Base de données",
                             "Impossible de charger les utilisateurs:\n" + query.lastError().text());
    }
}

bool UserWindow::saveUserToDatabase(const User &user)
{
    if (!databaseEnabled) return false;

    QSqlDatabase db = Connection::getInstance()->getDatabase();

    if (!db.isOpen()) {
        QMessageBox::warning(this, "Erreur Base de données", "Base de données non connectée !");
        return false;
    }

    QSqlQuery query(db);

    QString photoPath = "";
    if (!user.profileImage.isNull()) {
        QString targetDir = QCoreApplication::applicationDirPath() + "/profiles";
        QDir().mkpath(targetDir);
        photoPath = targetDir + "/user_" + QString::number(user.id) + ".jpg";
        user.profileImage.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation).save(photoPath, "JPG", 80);
    }

    query.prepare("INSERT INTO UTILISATEUR "
                  "(ID_UTILISATEUR, NOM, EMAIL, TELEPHONE, MOT_DE_PASSE, ROLE, STATUT, "
                  "DATE_CREATION, COULEUR_FAVORITE, NOM_ANIMAL, MATRICULE_VOITURE, PHOTO_PROFIL, A_VISAGE) "
                  "VALUES "
                  "(:id, :nom, :email, :telephone, :password, :role, :statut, "
                  "TO_DATE(:date_creation, 'YYYY-MM-DD'), :couleur, :animal, :matricule, :photo, :visage)");

    query.bindValue(":id", user.id);
    query.bindValue(":nom", user.name);
    query.bindValue(":email", user.email);
    query.bindValue(":telephone", user.phone);
    query.bindValue(":password", user.password);
    query.bindValue(":role", user.role);
    query.bindValue(":statut", user.status.toLower());
    query.bindValue(":date_creation", user.creationDate.toString("yyyy-MM-dd"));
    query.bindValue(":couleur", user.favoriteColor);
    query.bindValue(":animal", user.petName);
    query.bindValue(":matricule", user.carPlate);

    query.bindValue(":photo", photoPath);

    query.bindValue(":visage", user.hasFace ? "Y" : "N");

    if (!query.exec()) {
        qDebug() << "Erreur insertion utilisateur:" << query.lastError().text();

        QMessageBox::warning(this, "Erreur Base de données",
                             "Impossible d'ajouter l'utilisateur:\n" + query.lastError().text());
        return false;
    }

    qDebug() << "Utilisateur ajouté avec succès, ID:" << user.id;
    return true;
}

void UserWindow::deleteUserFromDatabase(int id)
{
    if (!databaseEnabled) return;

    QSqlDatabase db = Connection::getInstance()->getDatabase();
    QSqlQuery query(db);
    query.prepare("DELETE FROM UTILISATEUR WHERE ID_UTILISATEUR = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Erreur suppression:" << query.lastError().text();
        QMessageBox::warning(this, "Erreur Base de données",
                             "Impossible de supprimer l'utilisateur:\n" + query.lastError().text());
    } else {
        qDebug() << "Utilisateur supprimé, ID:" << id;
    }
}

void UserWindow::updateUserInDatabase(const User &user)
{
    if (!databaseEnabled) return;

    QSqlDatabase db = Connection::getInstance()->getDatabase();
    QSqlQuery query(db);

    QString photoPath = "";
    if (!user.profileImage.isNull()) {
        QString targetDir = QCoreApplication::applicationDirPath() + "/profiles";
        QDir().mkpath(targetDir);
        photoPath = targetDir + "/user_" + QString::number(user.id) + ".jpg";
        user.profileImage.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation).save(photoPath, "JPG", 80);
    }

    query.prepare("UPDATE UTILISATEUR SET "
                  "NOM = :nom, EMAIL = :email, TELEPHONE = :telephone, "
                  "MOT_DE_PASSE = :password, ROLE = :role, STATUT = :statut, "
                  "DATE_CREATION = TO_DATE(:date,'YYYY-MM-DD'), COULEUR_FAVORITE = :couleur, "
                  "NOM_ANIMAL = :pet, MATRICULE_VOITURE = :car, "
                  "PHOTO_PROFIL = :photo, A_VISAGE = :visage "
                  "WHERE ID_UTILISATEUR = :id");

    query.bindValue(":id",        user.id);
    query.bindValue(":nom",       user.name);
    query.bindValue(":email",     user.email);
    query.bindValue(":telephone", user.phone);
    query.bindValue(":password",  user.password);
    query.bindValue(":role",      user.role);
    query.bindValue(":statut",    user.status.toLower()); // FIX: minuscule pour Oracle
    query.bindValue(":date",      user.creationDate.toString("yyyy-MM-dd"));
    query.bindValue(":couleur",   user.favoriteColor);
    query.bindValue(":pet",       user.petName);
    query.bindValue(":car",       user.carPlate);
    query.bindValue(":photo",     photoPath);
    query.bindValue(":visage",    user.hasFace ? "Y" : "N");

    if (!query.exec()) {
        qDebug() << "Erreur mise à jour:" << query.lastError().text();
        QMessageBox::warning(this, "Erreur Base de données",
                             "Impossible de modifier l'utilisateur:\n" + query.lastError().text());
    } else {
        qDebug() << "Utilisateur mis à jour, ID:" << user.id;
    }
}

void UserWindow::showRoleStats()
{
    QMap<QString, int> roleStats;
    int totalUsers = 0;

    if (databaseEnabled) {
        QSqlDatabase db = Connection::getInstance()->getDatabase();
        if (db.isOpen()) {
            QSqlQuery query(db);
            if (query.exec("SELECT ROLE, COUNT(*) FROM UTILISATEUR GROUP BY ROLE")) {
                while (query.next()) {
                    QString role = query.value(0).toString();
                    int count = query.value(1).toInt();
                    if (role.isEmpty()) role = "Non défini";
                    roleStats[role] += count;
                    totalUsers += count;
                }
            } else {
                QMessageBox::warning(this, "Erreur", "Impossible de charger les statistiques.");
                return;
            }
        }
    } else {
        for (const User &u : std::as_const(usersList)) {
            QString role = u.role;
            if (role.isEmpty()) role = "Non défini";
            roleStats[role]++;
            totalUsers++;
        }
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Statistiques des Rôles");
    dialog.setFixedSize(450, 350);
    dialog.setStyleSheet("QDialog { background-color: white; }");

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->setSpacing(15);
    
    QLabel *title = new QLabel("📊 Répartition par Rôle");
    title->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50;");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    if (totalUsers == 0) {
        QLabel *noData = new QLabel("Aucune donnée disponible.");
        noData->setAlignment(Qt::AlignCenter);
        layout->addWidget(noData);
    } else {
        QPixmap piePixmap(200, 200);
        piePixmap.fill(Qt::transparent);
        
        QPainter painter(&piePixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        
        QRectF pieRect(10, 10, 180, 180);
        int startAngle = 0;
        
        QList<QColor> colors = {
            QColor("#3498db"), QColor("#e74c3c"), QColor("#2ecc71"), 
            QColor("#f1c40f"), QColor("#9b59b6"), QColor("#e67e22"),
            QColor("#1abc9c"), QColor("#34495e")
        };
        int colorIndex = 0;
        
        QVBoxLayout *legendLayout = new QVBoxLayout();
        legendLayout->setAlignment(Qt::AlignVCenter);
        
        for (auto it = roleStats.constBegin(); it != roleStats.constEnd(); ++it) {
            QString role = it.key();
            int count = it.value();
            double percentage = (count * 100.0) / totalUsers;
            
            // QPainter angles are in 1/16th of a degree
            int spanAngle = qRound((count * 360.0 * 16.0) / totalUsers);
            
            QColor color = colors[colorIndex % colors.size()];
            painter.setBrush(color);
            painter.setPen(QPen(Qt::white, 2));
            painter.drawPie(pieRect, startAngle, spanAngle);
            
            startAngle += spanAngle;
            
            QHBoxLayout *legendItem = new QHBoxLayout;
            QFrame *colorBox = new QFrame;
            colorBox->setFixedSize(14, 14);
            colorBox->setStyleSheet(QString("background-color: %1; border-radius: 7px;").arg(color.name()));
            
            QLabel *lbl = new QLabel(QString("<b>%1</b><br>%2 (%3%)")
                                     .arg(role)
                                     .arg(count)
                                     .arg(percentage, 0, 'f', 1));
            lbl->setStyleSheet("font-size: 11px; color: #34495e;");
            
            legendItem->addWidget(colorBox);
            legendItem->addWidget(lbl);
            legendItem->addStretch();
            legendLayout->addLayout(legendItem);
            
            colorIndex++;
        }
        painter.end();
        
        QLabel *pieChartLabel = new QLabel;
        pieChartLabel->setPixmap(piePixmap);
        pieChartLabel->setAlignment(Qt::AlignCenter);
        
        QHBoxLayout *chartAreaLayout = new QHBoxLayout;
        chartAreaLayout->addWidget(pieChartLabel);
        chartAreaLayout->addLayout(legendLayout);
        
        layout->addLayout(chartAreaLayout);
    }

    layout->addStretch();
    QPushButton *closeBtn = new QPushButton("Fermer");
    closeBtn->setStyleSheet("background-color: #95a5a6; color: white; border: none; border-radius: 5px; padding: 8px; font-weight: bold;");
    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    layout->addWidget(closeBtn, 0, Qt::AlignCenter);

    dialog.exec();
}
