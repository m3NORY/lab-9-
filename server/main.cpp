#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtSql>
#include <QDebug>

class ContactServer : public QTcpServer
{
    Q_OBJECT

private:
    QSqlDatabase db;

    void initDB()
    {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("contacts.db");

        if (!db.open()) {
            qDebug() << "DB ERROR:" << db.lastError().text();
            return;
        }

        QSqlQuery q;

        bool ok = q.exec(R"(
            CREATE TABLE IF NOT EXISTS contacts (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT NOT NULL,
                email TEXT,
                phone TEXT,
                address TEXT,
                category TEXT CHECK(category IN ('friend', 'family', 'work')),
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        )");

        if (ok) {
            qDebug() << "Database initialized successfully";
        }


        q.exec("CREATE INDEX IF NOT EXISTS idx_category ON contacts(category)");
        q.exec("CREATE INDEX IF NOT EXISTS idx_name ON contacts(name)");

        QSqlQuery countQuery;
        countQuery.exec("SELECT COUNT(*) FROM contacts");
        if (countQuery.next() && countQuery.value(0).toInt() == 0) {
            qDebug() << "Adding test data...";
            qDebug() << "Test data added";
        }
    }

    QString handle(QString req)
    {
        QSqlQuery q;
        req = req.trimmed();
        QStringList p = req.split(";");
        qDebug() << "Command:" << p[0];

        if (p[0] == "ADD") {
            if (p.size() < 6) {
                qDebug() << "ADD: not enough parameters";
                return "ERR\n";
            }

            q.prepare("INSERT INTO contacts(name, email, phone, address, category) VALUES(?, ?, ?, ?, ?)");
            q.addBindValue(p.value(1));  // name
            q.addBindValue(p.value(2));  // email
            q.addBindValue(p.value(3));  // phone
            q.addBindValue(p.value(4));  // address
            q.addBindValue(p.value(5));  // category

            if (q.exec()) {
                qDebug() << "Contact added successfully";
                return "OK\n";
            } else {
                qDebug() << "Add error:" << q.lastError().text();
                return "ERR\n";
            }
        }



        if (p[0] == "GET") {
            QString sql = "SELECT id, name, email, phone, address, category FROM contacts WHERE 1=1";

            if (p.size() > 1 && !p[1].isEmpty() && p[1] != "All") {
                sql += " AND category = '" + p[1] + "'";
            }

            if (p.size() > 2 && !p[2].isEmpty()) {
                sql += " AND (name LIKE '%" + p[2] + "%' OR email LIKE '%" + p[2] + "%')";
            }

            sql += " ORDER BY name ASC";

            int limit = 100;
            if (p.size() > 3 && !p[3].isEmpty()) {
                limit = p[3].toInt();
            }
            sql += " LIMIT " + QString::number(limit);
            qDebug() << "SQL:" << sql;
            if (!q.exec(sql)) {
                qDebug() << "Error:" << q.lastError().text();
                return "ERR\n";
            }

            QString res;
            while (q.next()) {
                res += QString("%1;%2;%3;%4;%5;%6\n")
                .arg(q.value(0).toString())  // id
                    .arg(q.value(1).toString())  // name
                    .arg(q.value(2).toString())  // email
                    .arg(q.value(3).toString())  // phone
                    .arg(q.value(4).toString())  // address
                    .arg(q.value(5).toString()); // category
            }

            qDebug() << "Returned" << q.size() << "contacts";
            return res.isEmpty() ? "No contacts found\n" : res;
        }


        if (p[0] == "UPDATE") {
            if (p.size() < 7) {
                qDebug() << "UPDATE: not enough parameters";
                return "ERR\n";
            }

            q.prepare("UPDATE contacts SET name=?, email=?, phone=?, address=?, category=? WHERE id=?");
            q.addBindValue(p.value(2));  // name
            q.addBindValue(p.value(3));  // email
            q.addBindValue(p.value(4));  // phone
            q.addBindValue(p.value(5));  // address
            q.addBindValue(p.value(6));  // category
            q.addBindValue(p.value(1).toInt());  // id

            if (q.exec()) {
                qDebug() << "Contact updated successfully, rows affected:" << q.numRowsAffected();
                return "OK\n";
            } else {
                qDebug() << "Update error:" << q.lastError().text();
                return "ERR\n";
            }
        }

        if (p[0] == "DEL") {
            if (p.size() < 2) {
                return "ERR\n";
            }
            q.prepare("DELETE FROM contacts WHERE id=?");
            q.addBindValue(p.value(1).toInt());

            if (q.exec()) {
                qDebug() << "Contact deleted, rows affected:" << q.numRowsAffected();
                return "OK\n";
            } else {
                qDebug() << "Delete error:" << q.lastError().text();
                return "ERR\n";
            }
        }

        return "ERR\n";
    }

public:
    ContactServer(QObject* parent = nullptr) : QTcpServer(parent) {
        initDB();
    }

    bool start(quint16 port) {
        if (listen(QHostAddress::Any, port)) {
            qDebug() << "Server started on port" << port;
            qDebug() << "Waiting for connections";
            return true;
        } else {
            qDebug() << "Failed to start server on port" << port;
            return false;
        }
    }

protected:
    void incomingConnection(qintptr socketDescriptor) override {
        QTcpSocket* socket = new QTcpSocket(this);
        socket->setSocketDescriptor(socketDescriptor);

        qDebug() << "Client connected:" << socket->peerAddress().toString();

        connect(socket, &QTcpSocket::readyRead, [this, socket]() {
            QString req = QString::fromUtf8(socket->readAll());
            QString res = handle(req);
            socket->write(res.toUtf8());
            socket->flush();
        });

        connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ContactServer server;
    server.start(12345);

    return a.exec();
}

#include "main.moc"
