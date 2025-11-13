#include "server.h"
#include <QDebug>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QAbstractSocket>
#include <QtNetwork/QNetworkInterface>

Server::Server(QObject *parent)
    : QObject{parent}
{
    initServer();
    connect(tcpServer, &QTcpServer::newConnection, this, &Server::printClientData);
}

void Server::printClientData()
{
    QTcpSocket *clientConnection = tcpServer->nextPendingConnection();
    connect(clientConnection, &QAbstractSocket::disconnected,
            clientConnection, &QObject::deleteLater);

    connect(clientConnection, &QAbstractSocket::disconnected, [clientConnection](){
        qDebug() << "Cliente desconectado.\nIP:" << clientConnection->peerAddress()
        << "\nPuerto:"  << clientConnection->peerPort();
    });

    qDebug() << "Nuevo cliente conectado desde IP"
             << clientConnection->peerAddress().toString()
             << "puerto" << clientConnection->peerPort();

    connect(clientConnection, &QTcpSocket::readyRead, [clientConnection]() {
        QByteArray accumulatedData;
        accumulatedData.append(clientConnection->readAll());
        int newlineIndex;
        while ((newlineIndex = accumulatedData.indexOf('\n')) != -1) {
            QByteArray line = accumulatedData.left(newlineIndex).trimmed(); // línea hasta \n
            accumulatedData.remove(0, newlineIndex + 1); // elimina línea procesada

            if (!line.isEmpty() && line.at(0) == '\0') {
                line.remove(0, 1); // remover bytes nulos iniciales
            }
            qDebug() << "Línea recibida:" << line;
        }
    });
}

void Server::initServer()
{
    tcpServer = new QTcpServer(this);
    if (!tcpServer->listen(QHostAddress::Any, 45554)) {
        qCritical() << "Error iniciando el servidor" << this;
        return;
    }

    QString ipAddress;
    const QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    for (const QHostAddress &entry : ipAddressesList) {
        if (entry != QHostAddress::LocalHost && entry.toIPv4Address()) {
            ipAddress = entry.toString();
            break;
        }
    }
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();

    qDebug() << "El servidor está corriendo en" << ipAddress << tcpServer->serverPort();
}
