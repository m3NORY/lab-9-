#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDebug>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    socket = new QTcpSocket(this);

    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
    connect(socket, &QTcpSocket::errorOccurred, this, [this]() {
        QMessageBox::warning(this, "Ошибка", "Ошибка подключения к серверу");
    });
    connect(socket, &QTcpSocket::connected, this, [this]() {
        qDebug() << "Connected to server";
        onLoad();
    });

    socket->connectToHost("127.0.0.1", 12345);

    connect(ui->btnAdd, &QPushButton::clicked, this, &MainWindow::onAdd);
    connect(ui->btnDelete, &QPushButton::clicked, this, &MainWindow::onDelete);
    connect(ui->btnUpdate, &QPushButton::clicked, this, &MainWindow::onUpdate);
    connect(ui->btnLoad, &QPushButton::clicked, this, &MainWindow::onLoad);
    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::onLoad);
    connect(ui->categoryCombo, &QComboBox::currentTextChanged, this, &MainWindow::onLoad);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::sendRequest(const QString& cmd)
{
    if (socket->state() != QAbstractSocket::ConnectedState) {
        QMessageBox::warning(this, "Ошибка", "Сервер не подключён");
        return;
    }
    socket->write((cmd + "\n").toUtf8());
    socket->flush();
}

void MainWindow::onLoad()
{
    QString category = ui->categoryCombo->currentText();
    QString search = ui->searchLineEdit->text();
    sendRequest(QString("GET;%1;%2;100").arg(category).arg(search));
}

void MainWindow::onAdd()
{
    if (ui->editName->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Имя обязательно");
        return;
    }

    QString msg = QString("ADD;%1;%2;%3;%4;%5")
                      .arg(ui->editName->text())
                      .arg(ui->editEmail->text())
                      .arg(ui->editPhone->text())
                      .arg(ui->editAddress->text())
                      .arg(ui->editCategory->currentText());

    sendRequest(msg);

    ui->editName->clear();
    ui->editEmail->clear();
    ui->editPhone->clear();
    ui->editAddress->clear();
}

void MainWindow::updateContactsList()
{
    ui->listWidget->clear();

    for (const Contact& contact : contacts) {
        QString display = QString("%1; %2 | %3 | %4 | %5 | %6")
                              .arg(contact.id, 3)
                              .arg(contact.name, 15)
                              .arg(contact.email, 20)
                              .arg(contact.phone, 12)
                              .arg(contact.address, 15)
                              .arg(contact.category, 8);

        QListWidgetItem* item = new QListWidgetItem(display);
        item->setData(Qt::UserRole, contact.id);
        ui->listWidget->addItem(item);
    }

    if (contacts.isEmpty()) {
        ui->listWidget->addItem("Нет контактов");
    }

    ui->statusBar->showMessage(QString("Загружено %1 контактов").arg(contacts.size()));
}


void MainWindow::showEditDialog(const Contact& contact)
{
    QDialog dialog(this);
    dialog.setWindowTitle("Редактирование контакта");
    dialog.setModal(true);
    dialog.setMinimumWidth(400);

    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);

    QFormLayout* formLayout = new QFormLayout();

    QLineEdit* nameEdit = new QLineEdit(contact.name);
    QLineEdit* emailEdit = new QLineEdit(contact.email);
    QLineEdit* phoneEdit = new QLineEdit(contact.phone);
    QLineEdit* addressEdit = new QLineEdit(contact.address);
    QComboBox* categoryCombo = new QComboBox();
    categoryCombo->addItems({"friend", "family", "work"});

    int index = categoryCombo->findText(contact.category);
    if (index >= 0) categoryCombo->setCurrentIndex(index);

    formLayout->addRow("Имя:", nameEdit);
    formLayout->addRow("Email:", emailEdit);
    formLayout->addRow("Телефон:", phoneEdit);
    formLayout->addRow("Адрес:", addressEdit);
    formLayout->addRow("Категория:", categoryCombo);

    mainLayout->addLayout(formLayout);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* saveBtn = new QPushButton("Сохранить");
    QPushButton* cancelBtn = new QPushButton("Отмена");

    saveBtn->setStyleSheet("background-color: #4CAF50; color: white;");
    cancelBtn->setStyleSheet("background-color: #f44336; color: white;");

    buttonLayout->addWidget(saveBtn);
    buttonLayout->addWidget(cancelBtn);
    mainLayout->addLayout(buttonLayout);

    connect(saveBtn, &QPushButton::clicked, [&]() {
        if (nameEdit->text().isEmpty()) {
            QMessageBox::warning(&dialog, "Ошибка", "Имя обязательно для заполнения");
            return;
        }

        QString msg = QString("UPDATE;%1;%2;%3;%4;%5;%6")
                          .arg(contact.id)
                          .arg(nameEdit->text())
                          .arg(emailEdit->text())
                          .arg(phoneEdit->text())
                          .arg(addressEdit->text())
                          .arg(categoryCombo->currentText());

        sendRequest(msg);
        dialog.accept();
    });

    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    dialog.exec();
}

void MainWindow::onUpdate()
{
    QListWidgetItem* currentItem = ui->listWidget->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, "Ошибка", "Выберите контакт для редактирования");
        return;
    }

    int contactId = currentItem->data(Qt::UserRole).toInt();

    Contact selectedContact;
    bool found = false;

    for (const Contact& contact : contacts) {
        if (contact.id == contactId) {
            selectedContact = contact;
            found = true;
            break;
        }
    }

    if (found) {
        showEditDialog(selectedContact);
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось получить данные контакта");
    }
}

void MainWindow::onDelete()
{
    QListWidgetItem* currentItem = ui->listWidget->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, "Ошибка", "Выберите контакт для удаления");
        return;
    }

    if (QMessageBox::question(this, "Подтверждение", "Удалить контакт?") == QMessageBox::Yes) {
        int contactId = currentItem->data(Qt::UserRole).toInt();
        sendRequest("DEL;" + QString::number(contactId));
    }
}

void MainWindow::onReadyRead()
{
    QByteArray data = socket->readAll();
    QString response = QString::fromUtf8(data);

    qDebug() << "Response:" << response;

    if (response.trimmed() == "OK") {
        onLoad();
        return;
    }

    if (response.trimmed() == "ERR") {
        QMessageBox::warning(this, "Ошибка", "Ошибка операции");
        return;
    }

    contacts.clear();
    QStringList lines = response.split("\n", Qt::SkipEmptyParts);

    for (const QString& line : lines) {
        if (line.contains("No contacts")) {
            updateContactsList();
            return;
        }

        QStringList parts = line.split(";");
        if (parts.size() >= 6) {
            Contact contact;
            contact.id = parts[0].trimmed().toInt();
            contact.name = parts[1].trimmed();
            contact.email = parts[2].trimmed();
            contact.phone = parts[3].trimmed();
            contact.address = parts[4].trimmed();
            contact.category = parts[5].trimmed();
            contacts.append(contact);
        }
    }

    updateContactsList();
}
