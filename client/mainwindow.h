#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QListWidgetItem>

struct Contact {
    int id;
    QString name;
    QString email;
    QString phone;
    QString address;
    QString category;
};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAdd();
    void onDelete();
    void onUpdate();
    void onReadyRead();
    void onLoad();

private:
    Ui::MainWindow *ui;
    QTcpSocket *socket;
    QString buffer;
    QList<Contact> contacts;  // Храним все контакты

    void sendRequest(const QString& cmd);
    void showEditDialog(const Contact& contact);
    void updateContactsList();
};

#endif
