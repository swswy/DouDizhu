#include "tcpdatawrap.h"

TcpDataWrap::TcpDataWrap(QObject *parent) : QObject(parent)
{

}

void TcpDataWrap::mySend(QTcpSocket *sendSocket, QString sdata)
{
    QByteArray sendByte;
    QDataStream out(&sendByte, QIODevice::WriteOnly);
    //out.setVersion(QDataStream::Qt_5_3);
    //设置大端模式，C++、JAVA中都是使用的大端，一般只有linux的嵌入式使用的小端
    out.setByteOrder(QDataStream::BigEndian);
    //占位符,这里必须要先这样占位，然后后续读算出整体长度后在插入
    out << qint64(0);
    //回到文件开头，插入真实的数值
    out.device()->seek(0);
    qint64 len = (qint64)(sdata.size());
    out  << len;
    sendByte+=sdata.toUtf8();
    //qDebug()<<"di22hang"<<sendByte;
    sendSocket->write(sendByte);
}

QString TcpDataWrap::myRecv(QTcpSocket *recvSocket)
{

    qint64 datasize;
    datasize=recvSocket->read(buffer,8);
    datasize=qFromBigEndian<qint64>(buffer);
    QByteArray result;
    //qDebug()<<buffer;
    //qDebug()<<datasize;
    result=recvSocket->read(datasize);
    //if(recvSocket->bytesAvailable()) emit(dataLeft());
    return result;
}
