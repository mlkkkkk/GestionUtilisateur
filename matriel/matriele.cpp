#include "matriele.h"
#include "qrcodegen.hpp"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFrame>
#include <QHeaderView>
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <QShortcut>
#include <QPainter>
#include <QDialog>
#include <algorithm>
#include <QUrl>
#include <QFileDialog>
#include <QTextStream>

const QString Matriele::APP_VERSION = "2.0.0";
const QString Matriele::APP_NAME = "Gestion Matériel";
const int Matriele::AUTO_SAVE_INTERVAL = 300000;

// ==================== CONSTRUCTEUR ====================

Matriele::Matriele(QWidget *parent) : QMainWindow(parent),
    blinkState(false), nextId(1), dataModified(false)
{
    setWindowTitle(QString("%1 v%2").arg(APP_NAME).arg(APP_VERSION));
    setMinimumSize(1200, 700);

    QWidget *centralWidget = new QWidget;
    centralWidget->setStyleSheet("background-color: #f5f6fa;");
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Sidebar
    QFrame *sidebar = new QFrame;
    sidebar->setStyleSheet(
        "QFrame { background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "stop:0 #2c3e50, stop:1 #34495e); border: none; }"
        "QLabel { color: white; font-size: 16px; font-weight: bold; }"
        "QListWidget { background: transparent; border: none; color: #ecf0f1; font-size: 14px; }"
        "QListWidget::item { padding: 15px 20px; border-bottom: 1px solid rgba(255,255,255,0.1); }"
        "QListWidget::item:selected { background-color: #3498db; border-left: 4px solid #2980b9; }"
        "QListWidget::item:hover { background-color: rgba(52, 152, 219, 0.3); }");
    sidebar->setFixedWidth(260);

    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(0, 0, 0, 0);
    sidebarLayout->setSpacing(0);

    QLabel *appTitle = new QLabel("🏢 " + APP_NAME);
    appTitle->setAlignment(Qt::AlignCenter);
    appTitle->setStyleSheet("font-size: 18px; padding: 25px;");
    sidebarLayout->addWidget(appTitle);

    navList = new QListWidget;
    navList->addItem("🛠️  Matériel");
    navList->setCurrentRow(0);
    connect(navList, &QListWidget::currentRowChanged, this, &Matriele::changePage);

    sidebarLayout->addWidget(navList);
    sidebarLayout->addStretch();

    QLabel *footer = new QLabel(QString("v%1 - © 2024").arg(APP_VERSION));
    footer->setStyleSheet("color: #7f8c8d; font-size: 10px; padding: 15px;");
    footer->setAlignment(Qt::AlignCenter);
    sidebarLayout->addWidget(footer);

    pagesWidget = new QStackedWidget;

    setupMaterialPage();
    setupStatusBar();
    setupShortcuts();

    // mainLayout->addWidget(sidebar);
    mainLayout->addWidget(pagesWidget, 1);

    loadMaterialsFromDB();
    updateMaterialTable();

    autoSaveTimer = new QTimer(this);
    connect(autoSaveTimer, &QTimer::timeout, this, &Matriele::saveData);
    autoSaveTimer->start(AUTO_SAVE_INTERVAL);

    blinkTimer = new QTimer(this);
    connect(blinkTimer, &QTimer::timeout, this, &Matriele::updateBlinkingState);
    blinkTimer->start(800);

    lowStockCheckTimer = new QTimer(this);
    connect(lowStockCheckTimer, &QTimer::timeout, this, &Matriele::checkLowStock);
    lowStockCheckTimer->start(3000);

    checkLowStock();
    QTimer::singleShot(100, this, &Matriele::updateBlinkingState);

    updateStatusBar();
}

Matriele::~Matriele() {
    qDeleteAll(fieldAnimations);
}

// ==================== CHARGEMENT DEPUIS ORACLE ====================

void Matriele::loadMaterialsFromDB() {
    materialsList.clear();

    QString searchText = searchEdit ? searchEdit->text().trimmed() : "";
    QString sortCriteria = sortComboBox ? sortComboBox->currentText() : "";

    QString orderBy = "ID_MATERIEL ASC";
    if (sortCriteria.contains("ID (décroissant)")) orderBy = "ID_MATERIEL DESC";
    else if (sortCriteria.contains("quantité (croissant)")) orderBy = "QUANTITE ASC";
    else if (sortCriteria.contains("quantité (décroissant)")) orderBy = "QUANTITE DESC";
    else if (sortCriteria.contains("type")) orderBy = "TYPE ASC";
    else if (sortCriteria.contains("nom")) orderBy = "NOM_MATERIEL ASC";
    else if (sortCriteria.contains("fournisseur")) orderBy = "FOURNISSEUR ASC";

    QString sqlQuery = "SELECT ID_MATERIEL, NOM_MATERIEL, TYPE, QUANTITE, SEUIL_MINIMUM, DATE_AJOUT, FOURNISSEUR, STATUT FROM MATERIEL";

    if (!searchText.isEmpty()) {
        sqlQuery += " WHERE (LOWER(NOM_MATERIEL) LIKE :search OR LOWER(TYPE) LIKE :search OR LOWER(FOURNISSEUR) LIKE :search)";
    }

    sqlQuery += " ORDER BY " + orderBy;

    QSqlQuery query;
    query.prepare(sqlQuery);
    
    if (!searchText.isEmpty()) {
        query.bindValue(":search", "%" + searchText.toLower() + "%");
    }

    if (!query.exec()) {
        QString err = query.lastError().text();
        if (err.contains("ORA-00904") || err.contains("FOURNISSEUR")) {
            QSqlQuery alterQuery;
            alterQuery.exec("ALTER TABLE MATERIEL ADD FOURNISSEUR VARCHAR2(150)");
            
            if (!query.exec()) {
                qDebug() << "Erreur re-chargement MATERIEL:" << query.lastError().text();
                return;
            }
        } else {
            qDebug() << "Erreur chargement MATERIEL:" << err;
            return;
        }
    }

    while (query.next()) {
        Material m;
        m.id        = query.value(0).toInt();
        m.name      = query.value(1).toString();
        m.type      = query.value(2).toString();
        m.quantity  = query.value(3).toInt();
        m.threshold = query.value(4).toInt();
        m.addedDate = query.value(5).toDate();
        m.supplier  = query.value(6).toString();
        m.status    = query.value(7).toString();
        materialsList.append(m);
    }

    if (!materialsList.isEmpty()) {
        QSqlQuery nextIdQuery("SELECT MAX(ID_MATERIEL) FROM MATERIEL");
        if (nextIdQuery.next()) {
            nextId = nextIdQuery.value(0).toInt() + 1;
        } else {
            nextId = 1;
        }
    }

    qDebug() << "Matériels chargés depuis Oracle:" << materialsList.size();
}

// ==================== SETUP UI ====================

void Matriele::setupMaterialPage() {
    QWidget *materialPage = new QWidget;
    materialPage->setStyleSheet("background-color: #f5f6fa;");

    QVBoxLayout *pageLayout = new QVBoxLayout(materialPage);
    pageLayout->setContentsMargins(30, 30, 30, 30);
    pageLayout->setSpacing(20);

    // Header
    QFrame *headerFrame = new QFrame;
    headerFrame->setObjectName("headerFrame");
    headerFrame->setStyleSheet("#headerFrame { background-color: white; border-radius: 15px; padding: 20px; }");
    QVBoxLayout *headerLayout = new QVBoxLayout(headerFrame);

    QLabel *titleLabel = new QLabel("📦 GESTION MATÉRIELLE");
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50;");
    headerLayout->addWidget(titleLabel);

    // Search bar
    QHBoxLayout *searchLayout = new QHBoxLayout;
    searchLayout->setSpacing(15);

    searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText("🔍 Rechercher par nom, type...");
    searchEdit->setStyleSheet(
        "QLineEdit { background-color: #f8f9fa; border: 2px solid #e9ecef; border-radius: 10px;"
        "padding: 12px 15px; font-size: 14px; color: #2c3e50; }"
        "QLineEdit:focus { border-color: #3498db; }");
    searchEdit->setMinimumWidth(300);
    connect(searchEdit, &QLineEdit::textChanged, this, &Matriele::searchMaterial);

    sortComboBox = new QComboBox;
    sortComboBox->addItems({"📊 Trier par ID (croissant)", "📊 Trier par ID (décroissant)",
                            "📈 Trier par quantité (croissant)", "📈 Trier par quantité (décroissant)",
                            "🏷️ Trier par type", "🔤 Trier par nom", "🏢 Trier par fournisseur"});
    sortComboBox->setStyleSheet(
        "QComboBox { background-color: #f8f9fa; border: 2px solid #e9ecef; border-radius: 10px;"
        "padding: 10px 15px; font-size: 13px; min-width: 220px; color: #2c3e50; }"
        "QComboBox:hover { border-color: #3498db; }");

    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(sortComboBox);
    searchLayout->addStretch();
    headerLayout->addLayout(searchLayout);

    // Action buttons
    QHBoxLayout *actionButtonsLayout = new QHBoxLayout;
    actionButtonsLayout->setSpacing(12);

    QPushButton *addBtn = createStyledButton("➕ Ajouter", "#2ecc71");
    modifyBtn = createStyledButton("✏️ Modifier", "#3498db");
    QPushButton *deleteBtn = createStyledButton("🗑️ Supprimer", "#e74c3c");

    modifyBtn->setEnabled(false);
    deleteBtn->setEnabled(false);

    actionButtonsLayout->addWidget(addBtn);
    actionButtonsLayout->addWidget(modifyBtn);
    actionButtonsLayout->addWidget(deleteBtn);
    actionButtonsLayout->addStretch();

    exportBtn = createStyledButton("📄 Exporter PDF", "#9b59b6");
    statsBtn = createStyledButton("📈 Statistiques", "#1abc9c");
    qrBtn = createStyledButton("📱 Code QR", "#34495e");
    QPushButton *sortBtn = createStyledButton("🔄 Trier", "#f39c12");
    saveBtn = createStyledButton("💾 Actualiser", "#27ae60");
    QPushButton *testBlinkBtn = createStyledButton("🔴 Test Clignotement", "#e74c3c");

    actionButtonsLayout->addWidget(exportBtn);
    actionButtonsLayout->addWidget(statsBtn);
    actionButtonsLayout->addWidget(qrBtn);
    actionButtonsLayout->addWidget(sortBtn);
    actionButtonsLayout->addWidget(saveBtn);
    actionButtonsLayout->addWidget(testBlinkBtn);

    headerLayout->addLayout(actionButtonsLayout);
    pageLayout->addWidget(headerFrame);

    // Main content
    QHBoxLayout *contentLayout = new QHBoxLayout;
    contentLayout->setSpacing(25);

    // Form
    QFrame *formFrame = new QFrame;
    formFrame->setObjectName("formFrame");
    formFrame->setStyleSheet("#formFrame { background-color: white; border-radius: 15px; padding: 25px; }");
    formFrame->setFixedWidth(450);

    QVBoxLayout *formLayout = new QVBoxLayout(formFrame);

    QLabel *formTitle = new QLabel("📝 Formulaire matériel");
    formTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #3498db; padding-bottom: 15px;");
    formLayout->addWidget(formTitle);

    QString inputStyle =
        "QLineEdit, QComboBox, QDateEdit { background-color: #f8f9fa; border: 2px solid #e9ecef;"
        "border-radius: 8px; padding: 10px 12px; font-size: 13px; color: #2c3e50; min-height: 25px; }"
        "QLineEdit:focus, QComboBox:focus, QDateEdit:focus { border-color: #3498db; }"
        "QLineEdit:read-only { background-color: #e9ecef; color: #6c757d; }";

    idEdit = new QLineEdit;
    idEdit->setReadOnly(true);
    idEdit->setStyleSheet(inputStyle);

    nameEdit = new QLineEdit;
    nameEdit->setPlaceholderText("Ex: Poutre en chêne");
    nameEdit->setStyleSheet(inputStyle);
    connect(nameEdit, &QLineEdit::textEdited, this, &Matriele::animateField);

    typeBox = new QComboBox;
    typeBox->addItems({"Bois", "Outil", "Consommable", "Electrique", "Electronique", "Metal", "Plastique"});
    typeBox->setStyleSheet(inputStyle);

    quantityEdit = new QLineEdit;
    quantityEdit->setPlaceholderText("Ex: 50");
    quantityEdit->setStyleSheet(inputStyle);
    quantityEdit->setValidator(new QIntValidator(0, 999999, this));
    connect(quantityEdit, &QLineEdit::textEdited, this, &Matriele::animateField);

    thresholdEdit = new QLineEdit;
    thresholdEdit->setPlaceholderText("Ex: 10");
    thresholdEdit->setStyleSheet(inputStyle);
    thresholdEdit->setValidator(new QIntValidator(0, 999999, this));
    connect(thresholdEdit, &QLineEdit::textEdited, this, &Matriele::animateField);

    dateEdit = new QDateEdit;
    dateEdit->setDate(QDate::currentDate());
    dateEdit->setCalendarPopup(true);
    dateEdit->setStyleSheet(inputStyle);

    supplierEdit = new QLineEdit;
    supplierEdit->setPlaceholderText("Ex: BoisCorp");
    supplierEdit->setStyleSheet(inputStyle);
    connect(supplierEdit, &QLineEdit::textEdited, this, &Matriele::animateField);

    statusBox = new QComboBox;
    statusBox->addItems({"Disponible", "Rupture", "Stock critique"});
    statusBox->setStyleSheet(inputStyle);

    QFormLayout *inputForm = new QFormLayout;
    inputForm->setSpacing(15);
    inputForm->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    inputForm->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);

    QLabel *idLabel = new QLabel("🔑 ID:");
    idLabel->setStyleSheet("font-weight: bold; color: #34495e;");
    QLabel *nameLabel = new QLabel("🏷️ Nom *:");
    nameLabel->setStyleSheet("font-weight: bold; color: #34495e;");
    QLabel *typeLabel = new QLabel("📋 Type:");
    typeLabel->setStyleSheet("font-weight: bold; color: #34495e;");
    QLabel *quantityLabel = new QLabel("🔢 Quantité *:");
    quantityLabel->setStyleSheet("font-weight: bold; color: #34495e;");
    QLabel *thresholdLabel = new QLabel("⚠️ Seuil *:");
    thresholdLabel->setStyleSheet("font-weight: bold; color: #34495e;");
    QLabel *dateLabel = new QLabel("📅 Date d'ajout:");
    dateLabel->setStyleSheet("font-weight: bold; color: #34495e;");
    QLabel *supplierLabel = new QLabel("🏢 Fournisseur *:");
    supplierLabel->setStyleSheet("font-weight: bold; color: #34495e;");
    QLabel *statusLabelForm = new QLabel("📊 Statut:");
    statusLabelForm->setStyleSheet("font-weight: bold; color: #34495e;");

    inputForm->addRow(idLabel, idEdit);
    inputForm->addRow(nameLabel, nameEdit);
    inputForm->addRow(typeLabel, typeBox);
    inputForm->addRow(quantityLabel, quantityEdit);
    inputForm->addRow(thresholdLabel, thresholdEdit);
    inputForm->addRow(dateLabel, dateEdit);
    inputForm->addRow(supplierLabel, supplierEdit);
    inputForm->addRow(statusLabelForm, statusBox);

    formLayout->addLayout(inputForm);
    formLayout->addStretch();

    QLabel *infoNote = new QLabel("⚠️ Les champs avec * sont obligatoires");
    infoNote->setStyleSheet("color: #e74c3c; font-size: 11px; padding-top: 15px; font-weight: bold;");
    formLayout->addWidget(infoNote);

    contentLayout->addWidget(formFrame);

    // Table
    QFrame *tableFrame = new QFrame;
    tableFrame->setObjectName("tableFrame");
    tableFrame->setStyleSheet("#tableFrame { background-color: white; border-radius: 15px; padding: 5px; }");

    QVBoxLayout *tableFrameLayout = new QVBoxLayout(tableFrame);

    QLabel *tableTitle = new QLabel("📋 Inventaire du matériel");
    tableTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50; padding: 15px;");
    tableFrameLayout->addWidget(tableTitle);

    materialTable = new QTableWidget(0, 8);
    materialTable->setHorizontalHeaderLabels({"ID", "NOM", "TYPE", "QUANTITÉ", "SEUIL", "DATE", "FOURNISSEUR", "STATUT"});
    materialTable->setStyleSheet(
        "QTableWidget { background-color: white; border: none; font-size: 13px; }"
        "QTableWidget::item { padding: 12px; border-bottom: 1px solid #e9ecef; }"
        "QTableWidget::item:selected { background-color: #e3f2fd; }"
        "QHeaderView::section { background-color: #2c3e50; color: white; padding: 12px; border: none; font-weight: bold; }");
    materialTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    materialTable->verticalHeader()->setVisible(false);
    materialTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    materialTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    materialTable->setAlternatingRowColors(false);

    tableFrameLayout->addWidget(materialTable);
    contentLayout->addWidget(tableFrame, 2);

    pageLayout->addLayout(contentLayout);

    // Connections boutons
    connect(addBtn,    &QPushButton::clicked, [this]() { if (validateMaterialFields()) addMaterial(); });
    connect(deleteBtn, &QPushButton::clicked, this, &Matriele::deleteMaterial);
    connect(modifyBtn, &QPushButton::clicked, [this]() { if (validateMaterialFields()) modifyMaterial(); });
    connect(exportBtn, &QPushButton::clicked, this, &Matriele::exportToPdf);
    connect(statsBtn,  &QPushButton::clicked, this, &Matriele::showStatistics);
    connect(qrBtn,     &QPushButton::clicked, this, &Matriele::generateQRCode);
    connect(saveBtn,   &QPushButton::clicked, this, &Matriele::saveData);
    connect(sortBtn,   &QPushButton::clicked, this, &Matriele::sortMaterial);
    connect(testBlinkBtn, &QPushButton::clicked, this, &Matriele::testBlinking);

    connect(materialTable, &QTableWidget::itemSelectionChanged, [this, deleteBtn]() {
        int row = materialTable->currentRow();
        bool validSelection = (row >= 0 && materialTable->item(row, 0) != nullptr);

        modifyBtn->setEnabled(validSelection);
        deleteBtn->setEnabled(validSelection);

        if (validSelection) {
            idEdit->setText(materialTable->item(row, 0)->text());
            nameEdit->setText(materialTable->item(row, 1)->text());
            typeBox->setCurrentText(materialTable->item(row, 2)->text());
            quantityEdit->setText(materialTable->item(row, 3)->text());
            thresholdEdit->setText(materialTable->item(row, 4)->text());
            dateEdit->setDate(QDate::fromString(materialTable->item(row, 5)->text(), "dd/MM/yyyy"));
            supplierEdit->setText(materialTable->item(row, 6)->text());
            statusBox->setCurrentText(materialTable->item(row, 7)->text());
        } else {
            idEdit->clear();
            nameEdit->clear();
            quantityEdit->clear();
            thresholdEdit->clear();
            supplierEdit->clear();
            typeBox->setCurrentIndex(0);
            statusBox->setCurrentIndex(0);
            dateEdit->setDate(QDate::currentDate());
        }
    });

    pagesWidget->addWidget(materialPage);
}

void Matriele::setupStatusBar() {
    m_statusBar = new QStatusBar;
    setStatusBar(m_statusBar);

    statusLabel = new QLabel("✅ Prêt");
    statsLabel = new QLabel;

    m_statusBar->addWidget(statusLabel);
    m_statusBar->addPermanentWidget(statsLabel);

    m_statusBar->setStyleSheet("QStatusBar { background-color: #2c3e50; color: white; padding: 5px 10px; }"
                               "QStatusBar QLabel { color: white; }");
}

void Matriele::setupShortcuts() {
    QShortcut *saveShortcut = new QShortcut(QKeySequence::Save, this);
    connect(saveShortcut, &QShortcut::activated, this, &Matriele::saveData);

    QShortcut *quitShortcut = new QShortcut(QKeySequence::Quit, this);
    connect(quitShortcut, &QShortcut::activated, this, &QWidget::close);
}

QPushButton* Matriele::createStyledButton(const QString& text, const QString& colorStr) {
    QPushButton *button = new QPushButton(text);
    QColor baseColor(colorStr);
    QColor hoverColor = baseColor.lighter(110);
    QColor pressedColor = baseColor.darker(110);

    button->setStyleSheet(QString(
        "QPushButton { background-color: %1; color: white; border: none; border-radius: 10px;"
        "padding: 10px 18px; font-size: 13px; font-weight: bold; min-width: 100px; }"
        "QPushButton:hover:!disabled { background-color: %2; }"
        "QPushButton:pressed:!disabled { background-color: %3; }"
        "QPushButton:disabled { background-color: #bdc3c7; color: #ecf0f1; }"
    ).arg(colorStr, hoverColor.name(), pressedColor.name()));
    
    button->setCursor(Qt::PointingHandCursor);
    button->setMinimumHeight(40);
    return button;
}

// ==================== VALIDATION ====================

bool Matriele::isValidName(const QString& name) {
    QString trimmedName = name.trimmed();
    // Allow numbers, spaces, letters, and accented characters common in real materials
    return trimmedName.length() >= 2 && trimmedName.length() <= 100;
}

bool Matriele::validateMaterialFields() {
    bool valid = true;
    QString errorMessage;

    if (nameEdit->text().trimmed().isEmpty()) {
        errorMessage += "• Le nom du matériel est requis\n";
        valid = false;
    } else if (!isValidName(nameEdit->text())) {
        errorMessage += "• Le nom doit contenir uniquement des lettres (A-Z, a-z)\n";
        valid = false;
    }

    if (quantityEdit->text().trimmed().isEmpty()) {
        errorMessage += "• La quantité est requise\n";
        valid = false;
    } else {
        bool ok;
        int value = quantityEdit->text().toInt(&ok);
        if (!ok || value < 0) {
            errorMessage += "• La quantité doit être un nombre positif\n";
            valid = false;
        }
    }

    if (thresholdEdit->text().trimmed().isEmpty()) {
        errorMessage += "• Le seuil minimum est requis\n";
        valid = false;
    } else {
        bool ok;
        int value = thresholdEdit->text().toInt(&ok);
        if (!ok || value < 0) {
            errorMessage += "• Le seuil doit être un nombre positif\n";
            valid = false;
        }
    }

    if (supplierEdit->text().trimmed().isEmpty()) {
        errorMessage += "• Le fournisseur est requis\n";
        valid = false;
    }

    if (!valid && !errorMessage.isEmpty()) {
        showNotification("Erreurs de validation :\n" + errorMessage, true);
    }

    return valid;
}

// ==================== ANIMATIONS ====================

void Matriele::animateField() {
    QLineEdit* field = qobject_cast<QLineEdit*>(sender());
    if (!field) return;

    if (field->property("isAnimating").toBool()) return;
    
    field->setProperty("isAnimating", true);
    QString originalStyle = field->styleSheet();
    field->setStyleSheet(originalStyle + "border: 2px solid #3498db; background-color: #e8f0fe;");
    
    QTimer::singleShot(300, [field, originalStyle]() {
        if (field) {
            field->setStyleSheet(originalStyle);
            field->setProperty("isAnimating", false);
        }
    });
}

void Matriele::showNotification(const QString& message, bool isError) {
    QMessageBox *msgBox = new QMessageBox(this);
    msgBox->setWindowTitle(isError ? "❌ Erreur" : "✅ Succès");
    msgBox->setText(message);
    msgBox->setIcon(isError ? QMessageBox::Warning : QMessageBox::Information);
    
    if (isError) {
        msgBox->exec();
    } else {
        msgBox->setAttribute(Qt::WA_DeleteOnClose);
        msgBox->setModal(false);
        msgBox->show();
        QTimer::singleShot(2500, msgBox, &QMessageBox::accept);
    }
}

// ==================== GESTION DES MATÉRIAUX ====================

void Matriele::addMaterial() {
    int qty       = quantityEdit->text().toInt();
    int threshold = thresholdEdit->text().toInt();

    QString status;
    if (qty <= 0)              status = "Rupture";
    else if (qty <= threshold) status = "Stock critique";
    else                       status = "Disponible";

    QSqlQuery query;
    query.prepare(
        "INSERT INTO MATERIEL (ID_MATERIEL, NOM_MATERIEL, TYPE, QUANTITE, SEUIL_MINIMUM, DATE_AJOUT, FOURNISSEUR, STATUT) "
        "VALUES (SEQ_MATERIEL.NEXTVAL, :nom, :type, :quantite, :seuil, "
        "TO_DATE(:date, 'YYYY-MM-DD'), :fournisseur, :statut)"
        );
    query.bindValue(":nom",         nameEdit->text().trimmed());
    query.bindValue(":type",        typeBox->currentText());
    query.bindValue(":quantite",    qty);
    query.bindValue(":seuil",       threshold);
    query.bindValue(":date",        dateEdit->date().toString("yyyy-MM-dd"));
    query.bindValue(":fournisseur", supplierEdit->text().trimmed());
    query.bindValue(":statut",      status);

    if (!query.exec()) {
        showNotification("❌ Erreur SQL (INSERT):\n" + query.lastError().text(), true);
        return;
    }
    
    QSqlQuery commitQuery;
    commitQuery.exec("COMMIT");

    loadMaterialsFromDB();
    updateMaterialTable();
    clearMaterialFields();
    checkLowStock();
    showNotification("✓ Matériel ajouté avec succès dans Oracle !", false);
}

void Matriele::modifyMaterial() {
    int row = materialTable->currentRow();
    if (row < 0) {
        showNotification("Sélectionnez un matériel à modifier.", true);
        return;
    }

    int id        = idEdit->text().toInt();
    int qty       = quantityEdit->text().toInt();
    int threshold = thresholdEdit->text().toInt();

    QString status;
    if (qty <= 0)              status = "Rupture";
    else if (qty <= threshold) status = "Stock critique";
    else                       status = "Disponible";

    QSqlQuery query;
    query.prepare(
        "UPDATE MATERIEL SET "
        "NOM_MATERIEL = :nom, "
        "TYPE = :type, "
        "QUANTITE = :quantite, "
        "SEUIL_MINIMUM = :seuil, "
        "DATE_AJOUT = TO_DATE(:date, 'YYYY-MM-DD'), "
        "FOURNISSEUR = :fournisseur, "
        "STATUT = :statut "
        "WHERE ID_MATERIEL = :id"
        );
    query.bindValue(":nom",         nameEdit->text().trimmed());
    query.bindValue(":type",        typeBox->currentText());
    query.bindValue(":quantite",    qty);
    query.bindValue(":seuil",       threshold);
    query.bindValue(":date",        dateEdit->date().toString("yyyy-MM-dd"));
    query.bindValue(":fournisseur", supplierEdit->text().trimmed());
    query.bindValue(":statut",      status);
    query.bindValue(":id",          id);

    if (!query.exec()) {
        showNotification("❌ Erreur SQL (UPDATE):\n" + query.lastError().text(), true);
        return;
    }

    QSqlQuery commitQuery;
    commitQuery.exec("COMMIT");

    // Update table dynamically without recreating it completely
    materialTable->item(row, 1)->setText(nameEdit->text().trimmed());
    materialTable->item(row, 2)->setText(typeBox->currentText());
    
    QTableWidgetItem *qtyItem = materialTable->item(row, 3);
    qtyItem->setText(QString::number(qty));
    
    materialTable->item(row, 4)->setText(QString::number(threshold));
    materialTable->item(row, 5)->setText(dateEdit->date().toString("dd/MM/yyyy"));
    materialTable->item(row, 6)->setText(supplierEdit->text().trimmed());
    
    QTableWidgetItem *statusItem = materialTable->item(row, 7);
    statusItem->setText(status);
    
    QFont font = qtyItem->font();
    if (qty <= threshold) {
        qtyItem->setForeground(QBrush(QColor(231, 76, 60)));
        statusItem->setForeground(QBrush(QColor(231, 76, 60)));
        font.setBold(true);
    } else {
        qtyItem->setForeground(QBrush(QColor(44, 62, 80)));
        statusItem->setForeground(QBrush(QColor(44, 62, 80)));
        font.setBold(false);
    }
    qtyItem->setFont(font);
    statusItem->setFont(font);

    // Keep memory mapping in sync
    loadMaterialsFromDB();
    
    checkLowStock();
    showNotification("✓ Matériel modifié avec succès dans Oracle !", false);
}

void Matriele::deleteMaterial() {
    int row = materialTable->currentRow();
    if (row < 0) {
        showNotification("Sélectionnez un matériel à supprimer.", true);
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirmation", "Voulez-vous vraiment supprimer ce matériel ?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        int id = materialTable->item(row, 0)->text().toInt();

        QSqlQuery query;
        query.prepare("DELETE FROM MATERIEL WHERE ID_MATERIEL = :id");
        query.bindValue(":id", id);

        if (!query.exec()) {
            showNotification("❌ Erreur SQL (DELETE):\n" + query.lastError().text(), true);
            return;
        }

        QSqlQuery commitQuery;
        commitQuery.exec("COMMIT");

        // Remove row immediately to avoid flickering
        materialTable->removeRow(row);

        loadMaterialsFromDB();
        clearMaterialFields();
        checkLowStock();
        showNotification("✓ Matériel supprimé avec succès !", false);
    }
}

void Matriele::searchMaterial() {
    loadMaterialsFromDB();
    updateMaterialTable();
}

void Matriele::sortMaterial() {
    QString sortCriteria = sortComboBox ? sortComboBox->currentText() : "";
    QString orderBy = "ID_MATERIEL ASC";
    if (sortCriteria.contains("ID (décroissant)")) orderBy = "ID_MATERIEL DESC";
    else if (sortCriteria.contains("quantité (croissant)")) orderBy = "QUANTITE ASC";
    else if (sortCriteria.contains("quantité (décroissant)")) orderBy = "QUANTITE DESC";
    else if (sortCriteria.contains("type")) orderBy = "TYPE ASC";
    else if (sortCriteria.contains("nom")) orderBy = "NOM_MATERIEL ASC";
    else if (sortCriteria.contains("fournisseur")) orderBy = "FOURNISSEUR ASC";

    // 1. Lire toutes les données dans l'ordre demandé
    QList<Material> tempSortedList;
    QSqlQuery selectQuery("SELECT ID_MATERIEL, NOM_MATERIEL, TYPE, QUANTITE, SEUIL_MINIMUM, DATE_AJOUT, FOURNISSEUR, STATUT FROM MATERIEL ORDER BY " + orderBy);
    while (selectQuery.next()) {
        Material m;
        m.id        = selectQuery.value(0).toInt();
        m.name      = selectQuery.value(1).toString();
        m.type      = selectQuery.value(2).toString();
        m.quantity  = selectQuery.value(3).toInt();
        m.threshold = selectQuery.value(4).toInt();
        m.addedDate = selectQuery.value(5).toDate();
        m.supplier  = selectQuery.value(6).toString();
        m.status    = selectQuery.value(7).toString();
        tempSortedList.append(m);
    }

    if (tempSortedList.isEmpty()) {
        showNotification("Rien à trier !", true);
        return;
    }

    // 2. Transaction pour réécrire la table physiquement
    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    QSqlQuery deleteQuery;
    if (!deleteQuery.exec("DELETE FROM MATERIEL")) {
        db.rollback();
        showNotification("❌ Impossible de réécrire la table :\n" + deleteQuery.lastError().text(), true);
        return;
    }

    QSqlQuery insertQuery;
    insertQuery.prepare(
        "INSERT INTO MATERIEL (ID_MATERIEL, NOM_MATERIEL, TYPE, QUANTITE, SEUIL_MINIMUM, DATE_AJOUT, FOURNISSEUR, STATUT) "
        "VALUES (:id, :nom, :type, :quantite, :seuil, TO_DATE(:date, 'YYYY-MM-DD'), :fournisseur, :statut)"
    );

    // 3. Réinsérer pour forcer l'ordre physique d'Oracle
    for (const Material &m : tempSortedList) {
        insertQuery.bindValue(":id", m.id);
        insertQuery.bindValue(":nom", m.name);
        insertQuery.bindValue(":type", m.type);
        insertQuery.bindValue(":quantite", m.quantity);
        insertQuery.bindValue(":seuil", m.threshold);
        insertQuery.bindValue(":date", m.addedDate.toString("yyyy-MM-dd"));
        insertQuery.bindValue(":fournisseur", m.supplier);
        insertQuery.bindValue(":statut", m.status);
        
        if (!insertQuery.exec()) {
            db.rollback();
            showNotification("❌ Erreur de réinsertion :\n" + insertQuery.lastError().text(), true);
            return;
        }
    }

    db.commit();

    loadMaterialsFromDB();
    updateMaterialTable();
    showNotification("✓ Tri DÉFINITIF et PHYSIQUE appliqué !", false);
}

// ==================== AFFICHAGE DU TABLEAU ====================

void Matriele::updateMaterialTable() {
    materialTable->setRowCount(0);

    for (const Material &material : materialsList) {
        int row = materialTable->rowCount();
        materialTable->insertRow(row);

        materialTable->setItem(row, 0, new QTableWidgetItem(QString::number(material.id)));
        materialTable->setItem(row, 1, new QTableWidgetItem(material.name));
        materialTable->setItem(row, 2, new QTableWidgetItem(material.type));

        QTableWidgetItem *qtyItem = new QTableWidgetItem(QString::number(material.quantity));
        if (material.quantity <= material.threshold) {
            qtyItem->setForeground(QBrush(QColor(231, 76, 60)));
            QFont font = qtyItem->font();
            font.setBold(true);
            qtyItem->setFont(font);
        }
        materialTable->setItem(row, 3, qtyItem);

        materialTable->setItem(row, 4, new QTableWidgetItem(QString::number(material.threshold)));
        materialTable->setItem(row, 5, new QTableWidgetItem(material.addedDate.toString("dd/MM/yyyy")));
        materialTable->setItem(row, 6, new QTableWidgetItem(material.supplier));

        QTableWidgetItem *statusItem = new QTableWidgetItem(material.status);
        if (material.quantity <= material.threshold) {
            statusItem->setForeground(QBrush(QColor(231, 76, 60)));
            QFont font = statusItem->font();
            font.setBold(true);
            statusItem->setFont(font);
        }
        materialTable->setItem(row, 7, statusItem);
    }

    updateBlinkingState();
    updateStatusBar();
}

void Matriele::clearMaterialFields() {
    nameEdit->clear();
    quantityEdit->clear();
    thresholdEdit->clear();
    supplierEdit->clear();
    typeBox->setCurrentIndex(0);
    statusBox->setCurrentIndex(0);
    dateEdit->setDate(QDate::currentDate());
    materialTable->clearSelection();
    idEdit->clear();
}

void Matriele::updateStatusBar() {
    int total = 0, lowStock = 0, outOfStock = 0;
    QSqlQuery query("SELECT COUNT(*), SUM(CASE WHEN QUANTITE <= SEUIL_MINIMUM THEN 1 ELSE 0 END), SUM(CASE WHEN QUANTITE = 0 THEN 1 ELSE 0 END) FROM MATERIEL");
    if (query.next()) {
        total = query.value(0).toInt();
        lowStock = query.value(1).toInt();
        outOfStock = query.value(2).toInt();
    }

    statsLabel->setText(QString("📊 Total: %1 | ⚠️ Stock critique: %2 | 🔴 Rupture: %3")
                            .arg(total).arg(lowStock).arg(outOfStock));
}

// ==================== CLIGNOTEMENT ROUGE ====================

void Matriele::checkLowStock()
{
    lowStockIds.clear();
    int lowStockCount = 0;
    int outOfStockCount = 0;

    QSqlQuery query("SELECT ID_MATERIEL, QUANTITE FROM MATERIEL WHERE QUANTITE <= SEUIL_MINIMUM");
    while (query.next()) {
        int id = query.value(0).toInt();
        int qty = query.value(1).toInt();
        lowStockIds.insert(id);
        lowStockCount++;
        if (qty == 0) outOfStockCount++;
    }

    if (!lowStockIds.isEmpty()) {
        QString alertMsg;
        if (outOfStockCount > 0) {
            alertMsg = QString("🔴 ALERTE : %1 matériel(s) critique (%2 en rupture)!")
                           .arg(lowStockCount).arg(outOfStockCount);
            statusLabel->setStyleSheet("color: #e74c3c; font-weight: bold; background-color: #2c3e50;");
        } else {
            alertMsg = QString("⚠️ ATTENTION : %1 matériel(s) en stock critique !").arg(lowStockCount);
            statusLabel->setStyleSheet("color: #f39c12; font-weight: bold; background-color: #2c3e50;");
        }
        statusLabel->setText(alertMsg);
    } else {
        statusLabel->setText("✅ Stock normal");
        statusLabel->setStyleSheet("color: #2ecc71; font-weight: normal; background-color: #2c3e50;");
    }
}

void Matriele::updateBlinkingState()
{
    if (!materialTable) return;

    blinkState = !blinkState;
    int rowCount = materialTable->rowCount();

    for (int row = 0; row < rowCount; ++row) {
        if (!materialTable->item(row, 0)) continue;

        int id = materialTable->item(row, 0)->text().toInt();
        bool isLowStock = lowStockIds.contains(id);

        if (isLowStock) {
            int qty = 0;
            if (materialTable->item(row, 3)) {
                QString qtStr = materialTable->item(row, 3)->text();
                qtStr.remove("❌ ").remove("❗ ").remove("⚠️ ").remove("🔴 ").remove("   ");
                qty = qtStr.toInt();
            }

            int threshold = 1;
            if (materialTable->item(row, 4)) {
                threshold = materialTable->item(row, 4)->text().toInt();
            }
            if (threshold <= 0) threshold = 1;

            double ratio = (double)qty / (double)threshold;

            QColor bgColorHover, textColor;
            QString iconStr, statusText;

            if (qty == 0) {
                // RUPTURE TOTALE : Clignotement rouge intense 
                bgColorHover = blinkState ? QColor(255, 230, 230) : QColor(255, 180, 180);
                textColor = QColor(192, 57, 43); // Dark Red
                iconStr = blinkState ? "🔴 " : "❌ ";
                statusText = "RUPTURE TOTALE";
            } else if (ratio <= 0.3) {
                // ÉTAT CRITIQUE SÉVÈRE (<= 30% du seuil) : Clignotement rouge/orange scintillant
                bgColorHover = blinkState ? QColor(253, 237, 237) : QColor(255, 204, 188); 
                textColor = QColor(211, 84, 0);
                iconStr = blinkState ? "❗ " : "   ";
                statusText = "CRITIQUE SÉVÈRE";
            } else {
                // ÉTAT CRITIQUE NORMAL (<= 100% du seuil) : Clignotement orange
                bgColorHover = blinkState ? QColor(255, 244, 229) : QColor(255, 236, 210);
                textColor = QColor(211, 84, 0); // Orange
                iconStr = blinkState ? "⚠️ " : "   ";
                statusText = "SEUIL ATTEINT";
            }

            for (int col = 0; col < materialTable->columnCount(); ++col) {
                QTableWidgetItem *item = materialTable->item(row, col);
                if (item) {
                    item->setBackground(QBrush(bgColorHover));
                    item->setForeground(QBrush(textColor));
                    QFont font = item->font();
                    font.setBold(true);
                    item->setFont(font);
                }
            }

            if (materialTable->item(row, 7))
                materialTable->item(row, 7)->setText(iconStr + statusText);

            if (materialTable->item(row, 3))
                materialTable->item(row, 3)->setText(iconStr + QString::number(qty));

        } else {
            QColor bgColor = (row % 2 == 0) ? Qt::white : QColor(248, 249, 250);
            for (int col = 0; col < materialTable->columnCount(); ++col) {
                QTableWidgetItem *item = materialTable->item(row, col);
                if (item) {
                    item->setBackground(QBrush(bgColor));
                    item->setForeground(QBrush(QColor(44, 62, 80)));
                    QFont font = item->font();
                    font.setBold(false);
                    item->setFont(font);
                }
            }

            if (materialTable->item(row, 7)) {
                for (const Material &m : materialsList) {
                    if (m.id == id) {
                        materialTable->item(row, 7)->setText(m.status);
                        break;
                    }
                }
            }

            if (materialTable->item(row, 3)) {
                QString qtyText = materialTable->item(row, 3)->text();
                qtyText.remove("❌ ").remove("❗ ").remove("⚠️ ").remove("🔴 ").remove("   ");
                materialTable->item(row, 3)->setText(qtyText);
            }
        }
    }

    materialTable->viewport()->update();
}

void Matriele::testBlinking()
{
    qDebug() << "=== TEST MANUEL DU CLIGNOTEMENT ===";
    updateBlinkingState();
}

// ==================== STATISTIQUES AVEC GRAPHIQUES ====================

void Matriele::showStatistics() {
    showAdvancedStatistics();
}

void Matriele::showAdvancedStatistics() {
    if (materialsList.isEmpty()) {
        showNotification("Aucune donnée disponible pour les statistiques", true);
        return;
    }

    QDialog *statsDialog = new QDialog(this);
    statsDialog->setWindowTitle("📊 Statistiques Avancées - BERUS");
    statsDialog->setMinimumSize(1000, 700);
    statsDialog->setStyleSheet("background-color: #f5f6fa;");

    QVBoxLayout *mainLayout = new QVBoxLayout(statsDialog);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(15);

    QLabel *titleLabel = new QLabel("📈 TABLEAU DE BORD STATISTIQUE");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #2c3e50; padding: 10px; background-color: white; border-radius: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    QTabWidget *tabWidget = new QTabWidget();
    tabWidget->setStyleSheet(
        "QTabWidget::pane { background-color: white; border-radius: 10px; }"
        "QTabBar::tab { background-color: #ecf0f1; padding: 10px 20px; margin: 5px; border-radius: 8px; }"
        "QTabBar::tab:selected { background-color: #3498db; color: white; }"
        );

    // 1. Diagramme circulaire - Types
    QWidget *typeTab = new QWidget();
    QVBoxLayout *typeLayout = new QVBoxLayout(typeTab);
    QChartView *typeChartView = new QChartView(createTypePieChart());
    typeChartView->setRenderHint(QPainter::Antialiasing);
    typeChartView->setMinimumHeight(400);
    typeLayout->addWidget(typeChartView);
    tabWidget->addTab(typeTab, "🏷️ Par Type");

    // 2. Diagramme circulaire - Statuts
    QWidget *statusTab = new QWidget();
    QVBoxLayout *statusLayout = new QVBoxLayout(statusTab);
    QChartView *statusChartView = new QChartView(createStatusPieChart());
    statusChartView->setRenderHint(QPainter::Antialiasing);
    statusChartView->setMinimumHeight(400);
    statusLayout->addWidget(statusChartView);
    tabWidget->addTab(statusTab, "📌 Par Statut");

    // 3. Diagramme à barres
    QWidget *barTab = new QWidget();
    QVBoxLayout *barLayout = new QVBoxLayout(barTab);
    QChartView *barChartView = new QChartView(createQuantityBarChart());
    barChartView->setRenderHint(QPainter::Antialiasing);
    barChartView->setMinimumHeight(450);
    barLayout->addWidget(barChartView);
    tabWidget->addTab(barTab, "📊 Quantités");

    // 4. Courbe d'évolution
    QWidget *lineTab = new QWidget();
    QVBoxLayout *lineLayout = new QVBoxLayout(lineTab);
    QChartView *lineChartView = new QChartView(createStockLineChart());
    lineChartView->setRenderHint(QPainter::Antialiasing);
    lineChartView->setMinimumHeight(450);
    lineLayout->addWidget(lineChartView);
    tabWidget->addTab(lineTab, "📈 Évolution");

    mainLayout->addWidget(tabWidget);

    // Résumé
    QFrame *summaryFrame = new QFrame();
    summaryFrame->setStyleSheet("QFrame { background-color: white; border-radius: 10px; padding: 15px; }");
    QHBoxLayout *summaryLayout = new QHBoxLayout(summaryFrame);

    int total = materialsList.size();
    int totalQuantity = 0;
    int lowStock = 0;
    int outOfStock = 0;
    int available = 0;

    for (const Material &m : materialsList) {
        totalQuantity += m.quantity;
        if (m.quantity <= m.threshold) lowStock++;
        if (m.quantity == 0) outOfStock++;
        if (m.status == "Disponible") available++;
    }

    QStringList summaryItems;
    summaryItems << QString("📦 Total: %1").arg(total);
    summaryItems << QString("🔢 Unités: %1").arg(totalQuantity);
    summaryItems << QString("✅ Disponible: %1").arg(available);
    summaryItems << QString("⚠️ Critique: %1").arg(lowStock);
    summaryItems << QString("🔴 Rupture: %1").arg(outOfStock);

    for (const QString &item : summaryItems) {
        QLabel *label = new QLabel(item);
        label->setStyleSheet("font-size: 13px; font-weight: bold; color: #2c3e50; padding: 5px 15px; background-color: #ecf0f1; border-radius: 8px;");
        summaryLayout->addWidget(label);
    }
    summaryLayout->addStretch();
    mainLayout->addWidget(summaryFrame);

    QPushButton *closeBtn = createStyledButton("Fermer", "#3498db");
    connect(closeBtn, &QPushButton::clicked, statsDialog, &QDialog::accept);
    mainLayout->addWidget(closeBtn);

    statsDialog->exec();
    delete statsDialog;
}

QChart* Matriele::createTypePieChart() {
    QMap<QString, int> typeCount;
    for (const Material &m : materialsList) {
        typeCount[m.type] = typeCount.value(m.type, 0) + 1;
    }

    QPieSeries *series = new QPieSeries();

    QList<QColor> colors = {
        QColor(52, 152, 219), QColor(46, 204, 113), QColor(231, 76, 60),
        QColor(155, 89, 182), QColor(241, 196, 15), QColor(230, 126, 34), QColor(26, 188, 156)
    };

    int colorIndex = 0;
    for (auto it = typeCount.begin(); it != typeCount.end(); ++it) {
        QPieSlice *slice = series->append(it.key(), it.value());
        if (colorIndex < colors.size()) {
            slice->setColor(colors[colorIndex]);
            slice->setLabelColor(colors[colorIndex]);
        }
        slice->setLabelVisible(true);
        double percent = (double)it.value() / materialsList.size() * 100.0;
        slice->setLabel(QString("%1\n%2 (%3%)").arg(it.key()).arg(it.value()).arg(percent, 0, 'f', 1));
        colorIndex++;
    }

    series->setLabelsPosition(QPieSlice::LabelOutside);
    series->setHoleSize(0.35);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("📊 Répartition par type de matériel");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignRight);
    chart->setBackgroundBrush(QBrush(Qt::white));

    return chart;
}

QChart* Matriele::createStatusPieChart() {
    QMap<QString, int> statusCount;
    for (const Material &m : materialsList) {
        statusCount[m.status] = statusCount.value(m.status, 0) + 1;
    }

    QPieSeries *series = new QPieSeries();

    QMap<QString, QColor> statusColors;
    statusColors["Disponible"] = QColor(46, 204, 113);
    statusColors["Stock critique"] = QColor(241, 196, 15);
    statusColors["Rupture"] = QColor(231, 76, 60);

    for (auto it = statusCount.begin(); it != statusCount.end(); ++it) {
        QPieSlice *slice = series->append(it.key(), it.value());
        if (statusColors.contains(it.key())) {
            slice->setColor(statusColors[it.key()]);
            slice->setLabelColor(statusColors[it.key()]);
        }
        slice->setLabelVisible(true);
        double percent = (double)it.value() / materialsList.size() * 100.0;
        slice->setLabel(QString("%1\n%2 (%3%)").arg(it.key()).arg(it.value()).arg(percent, 0, 'f', 1));
    }

    series->setLabelsPosition(QPieSlice::LabelOutside);
    series->setHoleSize(0.35);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("📈 État du stock");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignRight);
    chart->setBackgroundBrush(QBrush(Qt::white));

    return chart;
}

QChart* Matriele::createQuantityBarChart() {
    int maxItems = qMin(10, materialsList.size());

    QBarSeries *series = new QBarSeries();
    QBarSet *quantitySet = new QBarSet("Quantité");
    QBarSet *thresholdSet = new QBarSet("Seuil minimum");

    QStringList categories;

    quantitySet->setColor(QColor(52, 152, 219));
    thresholdSet->setColor(QColor(231, 76, 60));

    for (int i = 0; i < maxItems; i++) {
        const Material &m = materialsList[i];
        quantitySet->append(m.quantity);
        thresholdSet->append(m.threshold);

        QString shortName = m.name;
        if (shortName.length() > 15) {
            shortName = shortName.left(12) + "...";
        }
        categories << shortName;
    }

    series->append(quantitySet);
    series->append(thresholdSet);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("📊 Comparaison Quantité / Seuil (Top 10)");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setTitleText("Matériel");
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Quantité (unités)");
    axisY->setLabelFormat("%d");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->setBackgroundBrush(QBrush(Qt::white));

    return chart;
}

QChart* Matriele::createStockLineChart() {
    QMap<QDate, int> evolutionByDate;

    for (const Material &m : materialsList) {
        if (m.addedDate.isValid()) {
            evolutionByDate[m.addedDate] = evolutionByDate.value(m.addedDate, 0) + m.quantity;
        }
    }

    QList<QDate> sortedDates = evolutionByDate.keys();
    std::sort(sortedDates.begin(), sortedDates.end());

    QLineSeries *series = new QLineSeries();
    series->setName("Stock total ajouté");
    series->setColor(QColor(52, 152, 219));

    int cumulative = 0;
    for (const QDate &date : sortedDates) {
        cumulative += evolutionByDate[date];
        QDateTime dateTime(date, QTime(0, 0, 0));
        series->append(dateTime.toMSecsSinceEpoch(), cumulative);
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("📈 Évolution cumulative du stock");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QDateTimeAxis *axisX = new QDateTimeAxis();
    axisX->setTitleText("Date");
    axisX->setFormat("dd/MM/yyyy");
    axisX->setTickCount(qMin(8, qMax(2, sortedDates.size())));
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Stock cumulé (unités)");
    axisY->setLabelFormat("%d");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->setBackgroundBrush(QBrush(Qt::white));

    return chart;
}

// ==================== EXPORTATION PDF ====================

void Matriele::exportToPdf()
{
    if (materialsList.isEmpty()) {
        showNotification("Aucune donnée à exporter !", true);
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Exporter le rapport en PDF",
                                                    QDir::homePath() + "/rapport_materiel_berus.pdf",
                                                    "PDF Files (*.pdf)");

    if (fileName.isEmpty())
        return;

    QString html = generateHtmlReport();

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);

    QTextDocument document;
    document.setHtml(html);
    document.print(&printer);

    showNotification(QString("✓ Rapport PDF exporté !\n\nFichier : %1").arg(fileName), false);
}

QString Matriele::generateHtmlReport()
{
    int totalMaterials = materialsList.size();
    int totalQuantity = 0;
    int lowStock = 0;
    int outOfStock = 0;
    int available = 0;

    for (const Material &m : materialsList) {
        totalQuantity += m.quantity;
        if (m.quantity <= m.threshold) lowStock++;
        if (m.quantity == 0) outOfStock++;
        if (m.status == "Disponible") available++;
    }

    QString html;
    html += "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
    html += "<title>Rapport Matériel - BERUS</title><style>";
    html += "body { font-family: Arial; margin: 20px; }";
    html += "h1 { color: #2c3e50; border-bottom: 3px solid #3498db; }";
    html += ".stats { background: #ecf0f1; padding: 15px; border-radius: 10px; margin: 20px 0; }";
    html += "table { width: 100%; border-collapse: collapse; }";
    html += "th { background: #2c3e50; color: white; padding: 10px; }";
    html += "td { padding: 8px; border-bottom: 1px solid #ddd; }";
    html += ".critical { background-color: #f8d7da; color: #721c24; }";
    html += ".footer { margin-top: 30px; text-align: center; font-size: 10px; color: #95a5a6; }";
    html += "</style></head><body>";
    html += "<h1>📦 BERUS - RAPPORT D'INVENTAIRE</h1>";
    html += "<p>Date: " + QDate::currentDate().toString("dd/MM/yyyy") + " à " + QTime::currentTime().toString("HH:mm:ss") + "</p>";
    html += "<div class='stats'><h3>Statistiques</h3>";
    html += "<p>Total matériels: " + QString::number(totalMaterials) + "</p>";
    html += "<p>Unités en stock: " + QString::number(totalQuantity) + "</p>";
    html += "<p>Stock critique: " + QString::number(lowStock) + "</p>";
    html += "<p>En rupture: " + QString::number(outOfStock) + "</p></div>";
    html += "<table border='1'>";
    html += "<tr><th>ID</th><th>Nom</th><th>Type</th><th>Quantité</th><th>Seuil</th><th>Statut</th><th>Date</th></tr>";

    for (const Material &m : materialsList) {
        QString rowClass = (m.quantity <= m.threshold) ? "class='critical'" : "";
        html += "<tr " + rowClass + ">";
        html += "<td>" + QString::number(m.id) + "</td>";
        html += "<td>" + m.name + "</td>";
        html += "<td>" + m.type + "</td>";
        html += "<td>" + QString::number(m.quantity) + "</td>";
        html += "<td>" + QString::number(m.threshold) + "</td>";
        html += "<td>" + m.status + "</td>";
        html += "<td>" + m.addedDate.toString("dd/MM/yyyy") + "</td>";
        html += "</tr>";
    }

    html += "</table>";
    html += "<div class='footer'>BERUS - Système de Gestion de Matériel</div>";
    html += "</body></html>";

    return html;
}

void Matriele::exportToCsv()
{
    if (materialsList.isEmpty()) {
        showNotification("Aucune donnée à exporter !", true);
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Exporter en CSV",
                                                    QDir::homePath() + "/materiel_berus.csv",
                                                    "CSV Files (*.csv)");

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        showNotification("Impossible de créer le fichier CSV !", true);
        return;
    }

    QTextStream stream(&file);
    stream << "ID;NOM;TYPE;QUANTITE;SEUIL;STATUT;DATE\n";

    for (const Material &m : materialsList) {
        stream << m.id << ";" << m.name << ";" << m.type << ";"
               << m.quantity << ";" << m.threshold << ";" << m.status << ";"
               << m.addedDate.toString("dd/MM/yyyy") << "\n";
    }

    file.close();
    showNotification("✓ Export CSV réussi !", false);
}

// ==================== QR CODE ====================

void Matriele::generateQRCode() {
    int row = materialTable->currentRow();
    if (row < 0) {
        showNotification("Sélectionnez un matériel pour générer son Code QR.", true);
        return;
    }

    int id = materialTable->item(row, 0)->text().toInt();
    QString name     = materialTable->item(row, 1)->text();
    QString type     = materialTable->item(row, 2)->text();
    QString quantity = materialTable->item(row, 3)->text();
    QString status   = materialTable->item(row, 7)->text();

    QString plainText = QString("Détails du Matériel :\r\n\r\n"
                                "ID : %1\r\n"
                                "Nom : %2\r\n"
                                "Type : %3\r\n"
                                "Quantité : %4\r\n"
                                "Statut : %5")
                            .arg(id).arg(name).arg(type).arg(quantity).arg(status);

    // Apple blocks QR codes from opening the Notes app directly.
    // The only way to get this into Notes is to scan it as plain text, copy it, and paste it.
    QString qrData = plainText;

    using namespace qrcodegen;
    QrCode qr = QrCode::encodeText(qrData.toUtf8().constData(), QrCode::Ecc::MEDIUM);

    int scale = 5, margin = 2, size = qr.getSize();
    int imgSize = (size + 2 * margin) * scale;

    QImage img(imgSize, imgSize, QImage::Format_RGB32);
    img.fill(Qt::white);

    QPainter painter(&img);
    painter.setBrush(Qt::black);
    painter.setPen(Qt::NoPen);

    for (int y = 0; y < size; y++)
        for (int x = 0; x < size; x++)
            if (qr.getModule(x, y))
                painter.drawRect((x + margin) * scale, (y + margin) * scale, scale, scale);

    QDialog dialog(this);
    dialog.setWindowTitle("Code QR : " + name);
    dialog.setMinimumSize(400, 500);

    QVBoxLayout layout(&dialog);
    QLabel label;
    label.setPixmap(QPixmap::fromImage(img).scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    label.setAlignment(Qt::AlignCenter);
    layout.addWidget(&label);

    QLabel infoLabel;
    infoLabel.setText(plainText); // Afficher le texte lisible, pas l'URL encodée
    infoLabel.setAlignment(Qt::AlignCenter);
    infoLabel.setStyleSheet("font-family: monospace; padding: 10px; background-color: #f8f9fa; border-radius: 5px;");
    layout.addWidget(&infoLabel);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    
    QPushButton* saveTxtBtn = createStyledButton("💾 Sauvegarder .txt", "#2ecc71");
    connect(saveTxtBtn, &QPushButton::clicked, [&dialog, this, name, plainText]() {
        // Créer un dossier "notes" s'il n'existe pas
        QDir dir(QCoreApplication::applicationDirPath() + "/notes");
        if (!dir.exists()) dir.mkpath(".");

        QString fileName = QFileDialog::getSaveFileName(&dialog, "Enregistrer Fichier Texte", dir.absolutePath() + "/" + name + "_info.txt", "Text Files (*.txt)");
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << plainText;
                file.close();
                showNotification("✓ Fichier texte sauvegardé sur le PC !", false);
            }
        }
    });

    QPushButton* savePdfBtn = createStyledButton("📄 Sauvegarder .pdf", "#8e44ad");
    connect(savePdfBtn, &QPushButton::clicked, [&dialog, this, name, plainText]() {
        QDir dir(QCoreApplication::applicationDirPath() + "/notes");
        if (!dir.exists()) dir.mkpath(".");

        QString fileName = QFileDialog::getSaveFileName(&dialog, "Enregistrer Fichier PDF", dir.absolutePath() + "/" + name + "_fiche.pdf", "PDF Files (*.pdf)");
        if (!fileName.isEmpty()) {
            QPrinter printer(QPrinter::HighResolution);
            printer.setOutputFormat(QPrinter::PdfFormat);
            printer.setOutputFileName(fileName);
            
            QPainter pdfPainter(&printer);
            pdfPainter.setFont(QFont("Arial", 16, QFont::Bold));
            pdfPainter.drawText(200, 300, "Fiche Matériel : " + name);
            pdfPainter.setFont(QFont("Arial", 12));
            pdfPainter.drawText(200, 500, plainText);
            pdfPainter.end();
            showNotification("✓ Fiche PDF sauvegardée sur le PC !", false);
        }
    });

    QPushButton* closeBtn = createStyledButton("Fermer", "#3498db");
    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    
    btnLayout->addWidget(saveTxtBtn);
    btnLayout->addWidget(savePdfBtn);
    btnLayout->addWidget(closeBtn);
    layout.addLayout(btnLayout);

    dialog.exec();
}

// ==================== AUTRES FONCTIONS ====================

void Matriele::importFromFile() {
    showNotification("📂 Import - Fonctionnalité à implémenter", false);
}

void Matriele::saveData() {
    loadMaterialsFromDB();
    updateMaterialTable();
    showNotification("✓ Données actualisées depuis Oracle !", false);
}

void Matriele::loadData() {
    loadMaterialsFromDB();
    updateMaterialTable();
}

void Matriele::changePage(int index) {
    pagesWidget->setCurrentIndex(index);
    if (index == 1) {
        showStatistics();
        QTimer::singleShot(100, [this]() { navList->setCurrentRow(0); });
    }
}
