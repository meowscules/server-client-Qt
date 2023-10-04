#include "server.h"

Server::Server(): QTcpServer()
    , socket(new QTcpSocket)
{
    nextBlockSize = 0;
    if(this->listen(QHostAddress::Any, 2323))
        qDebug() << "is Listening" << this->isListening();
    else
        qDebug() << "is Listening" << this->isListening();
}

void Server::sendToClient(QString str)
{
    Data.clear();
    QDataStream out(&Data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_2);
    out << quint16(0) << QTime::currentTime() << str;
    out.device()->seek(0);
    out << quint16(Data.size() - sizeof(quint16));
    for(int i = 0; i < Sockets.size(); i++){
        Sockets[i]->write(Data);
    }
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    socket->setSocketDescriptor(socketDescriptor);
    connect(socket, &QTcpSocket::readyRead, this, &Server::slotReadyRead);
    //connect(socket, &QTcpSocket::disconnected, this, &Server::socketDisconnected);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

    Sockets.push_back(socket);
    qDebug() << "client connected" << socketDescriptor << "Sockets" << Sockets;
}

void Server::slotReadyRead()
{
    socket = (QTcpSocket*)sender();
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_6_2);
    if(in.status() == QDataStream::Ok){
        for(;;)
        {
            if(nextBlockSize == 0){
                qDebug() << "nextBlockSize = 0";
                if(socket->bytesAvailable()<2){
                    qDebug() << "Data < 2, break";
                    break;
                }
                in >> nextBlockSize;
                qDebug() << "nextBlockSize = " << nextBlockSize;
            }
            if (socket->bytesAvailable() < nextBlockSize){
                qDebug() << "Data not full, break";
                break;
            }
            QString str;
            QTime time;
            in >> time >> str;
            nextBlockSize = 0;
            qDebug() << str;
            sendToClient(str);
            break;
        }
    }
    else qDebug() << "Data stream error";
}

void Server::socketDisconnected()
{
    qDebug() << "Client disconnected " ;
}
