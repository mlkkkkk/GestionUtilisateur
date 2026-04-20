#ifndef MATRIELE_H
#define MATRIELE_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QListWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QStatusBar>
#include <QTimer>
#include <QMap>
#include <QPropertyAnimation>
#include <QList>
#include <QSet>
#include <QColor>
#include <QBrush>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QPrinter>
#include <QTextDocument>
#include <QFileDialog>
#include <QDateTime>
#include <QPageLayout>
#include <QPageSize>
#include <QTextStream>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QFrame>
#include <QChartView>
#include <QPieSeries>
#include <QPieSlice>
#include <QBarSeries>
#include <QBarSet>
#include <QLegend>
#include <QBarCategoryAxis>
#include <QLineSeries>
#include <QDateTimeAxis>
#include <QValueAxis>
#include <QChart>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
namespace QtCharts {}
using namespace QtCharts;
#endif


    struct Material {
    int id;
    QString name;
    QString type;
    int quantity;
    int threshold;
    QDate addedDate;
    QString supplier;
    QString status;
};

class Matriele : public QMainWindow {
    Q_OBJECT

public:
    explicit Matriele(QWidget *parent = nullptr);
    ~Matriele();

private slots:
    void changePage(int index);
    void addMaterial();
    void modifyMaterial();
    void deleteMaterial();
    void searchMaterial();
    void sortMaterial();
    void clearMaterialFields();
    void exportToPdf();
    void exportToCsv();
    void showStatistics();
    void generateQRCode();
    void updateBlinkingState();
    void checkLowStock();
    void testBlinking();
    void importFromFile();
    void saveData();
    void loadData();
    void animateField();

private:
    void setupMaterialPage();
    void setupStatusBar();
    void setupShortcuts();
    void updateMaterialTable();
    void updateStatusBar();
    void loadMaterialsFromDB();
    QPushButton* createStyledButton(const QString& text, const QString& color);
    bool validateMaterialFields();
    bool isValidName(const QString& name);
    void showNotification(const QString& message, bool isError = false);
    void showAdvancedStatistics();
    QChart* createTypePieChart();
    QChart* createStatusPieChart();
    QChart* createQuantityBarChart();
    QChart* createStockLineChart();
    QString generateHtmlReport();

    // Material members
    QList<Material> materialsList;
    QTableWidget* materialTable;
    QLineEdit *idEdit, *nameEdit, *quantityEdit, *thresholdEdit, *supplierEdit;
    QComboBox *typeBox, *statusBox;
    QDateEdit *dateEdit;
    QLineEdit *searchEdit;
    QComboBox *sortComboBox;
    QPushButton *modifyBtn, *exportBtn, *statsBtn, *qrBtn, *importBtn, *saveBtn;

    // Timers
    QTimer *blinkTimer;
    QTimer *autoSaveTimer;
    QTimer *lowStockCheckTimer;

    bool blinkState;
    QSet<int> lowStockIds;

    int nextId;
    bool dataModified;

    QStackedWidget* pagesWidget;
    QListWidget* navList;
    QStatusBar* m_statusBar;
    QLabel* statusLabel;
    QLabel* statsLabel;

    QMap<QLineEdit*, QPropertyAnimation*> fieldAnimations;

    static const QString APP_VERSION;
    static const QString APP_NAME;
    static const int AUTO_SAVE_INTERVAL;
};

#endif // MATRIELE_H
