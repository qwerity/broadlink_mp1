#pragma once

#include <QtNetwork>

class BroadLinkMP1
{
public:
    BroadLinkMP1(QString &ipStr, QByteArray &destMAC);
    ~BroadLinkMP1() { udpSocket.close(); }

    bool auth();

    void setPower(bool on, int socketID = -1);

private:
    QByteArray decryptData(QByteArray &encryptedData);
    QByteArray encryptData(QByteArray &data);

    QByteArray sendPacket(uint8_t command, QByteArray payload);

    void setPowerMask(uint8_t sid_mask, bool on);

private:
    QUdpSocket udpSocket;
    QHostAddress ip;
    const uint16_t port = 80;

    QByteArray mac;

    QByteArray key;
    QByteArray id;

    uint16_t seqNumber = 0;
};
