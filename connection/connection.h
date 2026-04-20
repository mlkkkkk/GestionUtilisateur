#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

class Connection
{
private:
    Connection() {}
    static Connection* instance;
    QSqlDatabase db;

public:
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    static Connection* getInstance()
    {
        if (instance == nullptr) {
            instance = new Connection();
        }
        return instance;
    }

    bool createconnect()
    {
        db = QSqlDatabase::addDatabase("QODBC");
        db.setDatabaseName("Driver={Oracle in XE};DBQ=XE;Uid=malek;Pwd=malek123;");

        if (db.open()) {
            qDebug() << "Connexion Singleton reussie !";
            return true;
        } else {
            qDebug() << "Connexion echouee :" << db.lastError().text();
            return false;
        }
    }

    // Compatibilité avec le reste du projet
    bool establishConnection() { return createconnect(); }
    bool isOpen() { return db.isOpen(); }
    QSqlDatabase getDatabase() { return db; }

    void closeConnection()
    {
        if (db.isOpen()) {
            db.close();
        }
    }

    static void deleteInstance()
    {
        delete instance;
        instance = nullptr;
    }
};

#endif // CONNECTION_H
