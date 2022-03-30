#ifndef TCPDATAWRAP_H
#define TCPDATAWRAP_H

#include <QObject>
#include <QTcpSocket>
#include <QDataStream>
#include <QDebug>
#include <QtEndian>
#define MINSIZE 4
class TcpDataWrap : public QObject
{
    Q_OBJECT

public:
    explicit TcpDataWrap(QObject *parent = nullptr);
    void mySend(QTcpSocket* sendSocket,QString sdata);
    QString myRecv(QTcpSocket* recvSocket);
    char* buffer=new char[8];
signals:

};

#endif // TCPDATAWRAP_H
