#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QtNetwork/QTcpServer>

#define PORT 45554

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);


private slots:
    void printClientData();

private:
    void initServer();


    QTcpServer *tcpServer = nullptr;
};

#endif // SERVER_H
