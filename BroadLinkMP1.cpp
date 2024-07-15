#include "BroadLinkMP1.h"
#include <cstring>
#include <cstdlib>

#include <openssl/aes.h>

#include <QDebug>

#define RETRIES_COUNT   (3)

static uint8_t brd_key[AES_BLOCK_SIZE] = {0x09, 0x76, 0x28, 0x34, 0x3f, 0xe9, 0x9e, 0x23, 0x76, 0x5c, 0x15, 0x13, 0xac, 0xcf, 0x8b, 0x02};
static uint8_t brd_iv[AES_BLOCK_SIZE]  = {0x56, 0x2e, 0x17, 0x99, 0x6d, 0x09, 0x3d, 0x28, 0xdd, 0xb3, 0xba, 0x69, 0x5a, 0x2e, 0x6f, 0x58};

BroadLinkMP1::BroadLinkMP1(QString &ipStr, QByteArray &destMAC) :
    ip(ipStr),
    mac(destMAC),
    key(QByteArray((const char*)brd_key, AES_BLOCK_SIZE)),
    id(QByteArray(4, 0))
{
    seqNumber = 0; //uint16_t(std::rand() % 0xffff);

    udpSocket.bind(ip, port, QAbstractSocket::ReuseAddressHint);
}

QByteArray BroadLinkMP1::decryptData(QByteArray &encryptedData)
{
    auto iv = QByteArray((const char*)brd_iv, 16);
//    qInfo() << "decrypt key" << key.toHex();
//    qInfo() << "decrypt iv" << iv.toHex();

    QByteArray clearData(encryptedData.size(), 0);

    AES_KEY aes_key;
    AES_set_decrypt_key((uint8_t *)key.data(), key.size() * 8, &aes_key);
    AES_cbc_encrypt((uint8_t *)encryptedData.data(), (uint8_t *)clearData.data(), size_t(encryptedData.size()), &aes_key, (uint8_t*)iv.data(), AES_DECRYPT);
//    qInfo() << "decrypt iv" << iv.toHex();

    return clearData;
}

QByteArray BroadLinkMP1::encryptData(QByteArray &data)
{
    auto iv = QByteArray((const char*)brd_iv, 16);
//    qInfo() << "encrypt key" << key.toHex();
//    qInfo() << "encrypt iv" << iv.toHex();

    QByteArray encryptedData(data.size(), 0);

    AES_KEY aes_key;
    AES_set_encrypt_key((uint8_t *)key.data(), key.size() * 8, &aes_key);
    AES_cbc_encrypt((const uint8_t *)data.constData(), (uint8_t*)encryptedData.data(), size_t(data.size()), &aes_key, (uint8_t*)iv.data(), AES_ENCRYPT);
//    qInfo() << "encrypt iv" << iv.toHex();

    return encryptedData;
}

QByteArray BroadLinkMP1::sendPacket(uint8_t command, QByteArray payload)
{
    seqNumber = static_cast<uint16_t>((seqNumber + 1) & 0xffff);

    auto packet = QByteArray(0x38, 0);
    packet[0x00] = 0x5a;
    packet[0x01] = 0xa5;
    packet[0x02] = 0xaa;
    packet[0x03] = 0x55;
    packet[0x04] = 0x5a;
    packet[0x05] = 0xa5;
    packet[0x06] = 0xaa;
    packet[0x07] = 0x55;

    packet[0x24] = 0x2a;
    packet[0x25] = 0x27;
    packet[0x26] = command;

    packet[0x28] = seqNumber & 0xff;
    packet[0x29] = seqNumber >> 8;
    packet[0x2a] = mac[5];
    packet[0x2b] = mac[4];
    packet[0x2c] = mac[3];
    packet[0x2d] = mac[2];
    packet[0x2e] = mac[1];
    packet[0x2f] = mac[0];
    packet[0x30] = id[0];
    packet[0x31] = id[1];
    packet[0x32] = id[2];
    packet[0x33] = id[3];

    // pad the payload for AES encryption
    if (payload.size() > 0)
    {
        auto numpad = ((payload.size() / 16) + 1) * 16;
        payload = payload.leftJustified(numpad, 0x00);
    }

    int checksum = 0xbeaf;
    for (int i = 0; i < payload.size(); i ++)
    {
        checksum += uint8_t(payload[i]);
        checksum &= 0xffff;
    }

//    qInfo() << "seqNumber" << seqNumber;
//    qInfo() << "payload" << payload.toHex();
    payload = encryptData(payload);
//    qInfo() << "enc payload" << payload.toHex();

    packet[0x34] = checksum & 0xff;
    packet[0x35] = checksum >> 8;

    for (int i = 0; i < payload.size(); i ++)
    {
        packet.append(payload[i]);
    }

    checksum = 0xbeaf;
    for (int i = 0; i < packet.size(); i ++)
    {
        checksum += uint8_t(packet[i]);
        checksum &= 0xffff;
    }
    packet[0x20] = checksum & 0xff;
    packet[0x21] = checksum >> 8;

//    qInfo() << "checksum" << checksum;
//    qInfo() << "packet" << packet.toHex();

    QByteArray response;
    for (int i = 0; i < RETRIES_COUNT; ++i)
    {
        auto sent_bytes = udpSocket.writeDatagram(packet, ip, port);
        if (sent_bytes < 0)
        {
            qInfo() << "can't send hex string, return value:" << sent_bytes;
            qInfo() << "error" << udpSocket.errorString();
            break;
        }

        udpSocket.waitForReadyRead(5000);

        if (udpSocket.hasPendingDatagrams())
        {
            QHostAddress sender;
            quint16 senderPort;

            response.reserve(udpSocket.pendingDatagramSize());
            response.resize(udpSocket.pendingDatagramSize());
            udpSocket.readDatagram(response.data(), response.size(), &sender, &senderPort);
            break;
        }
    }

    return response;
}

bool BroadLinkMP1::auth()
{
    QByteArray payload(0x50, 0);

    // my device id
    payload[0x04] = 0x31;
    payload[0x05] = 0x31;
    payload[0x06] = 0x31;
    payload[0x07] = 0x31;
    payload[0x08] = 0x31;
    payload[0x09] = 0x31;
    payload[0x0a] = 0x31;
    payload[0x0b] = 0x31;
    payload[0x0c] = 0x31;
    payload[0x0d] = 0x31;
    payload[0x0e] = 0x31;
    payload[0x0f] = 0x31;
    payload[0x10] = 0x31;
    payload[0x11] = 0x31;
    payload[0x12] = 0x31;
    payload[0x1e] = 0x01;
    payload[0x2d] = 0x01;
    payload[0x30] = 'T';
    payload[0x31] = 'e';
    payload[0x32] = 's';
    payload[0x33] = 't';
    payload[0x34] = ' ';
    payload[0x35] = ' ';
    payload[0x36] = '1';

    auto response = sendPacket(0x65, payload);
    if (response.isEmpty())
        return false;

    auto a = response.mid(0x38);
    payload = decryptData(a);

//    qInfo() << "response" << response.toHex();
//    qInfo() << "a" << a.toHex();
//    qInfo() << "payload" << payload.toHex();

    if (payload.isEmpty())
        return false;

    auto newKey = payload.mid(0x04, AES_BLOCK_SIZE);
    if (AES_BLOCK_SIZE != newKey.size())
        return false;

    this->id = payload.mid(0x00, 0x04);
    this->key = newKey;

    return true;
}

void BroadLinkMP1::setPowerMask(uint8_t sid_mask, bool on)
{
    QByteArray packet(16, 0);

    packet[0x00] = 0x0d;
    packet[0x02] = 0xa5;
    packet[0x03] = 0xa5;
    packet[0x04] = 0x5a;
    packet[0x05] = 0x5a;
    if (on)
    {
        packet[0x06] = 0xb2 + (sid_mask << 1);
    }
    else
    {
        packet[0x06] = 0xb2 + sid_mask;
    }
    packet[0x07] = 0xc0;
    packet[0x08] = 0x02;
    packet[0x0a] = 0x03;
    packet[0x0d] = sid_mask;
    if (on)
    {
        packet[0x0e] = sid_mask;
    }
    else
    {
        packet[0x0e] = 0;
    }

    const auto response = sendPacket(0x6a, packet);
    auto err = response[0x22] | (response[0x23] << 8);
    qInfo() << __FUNCTION__ << "error: " << err;
}

void BroadLinkMP1::setPower(bool on, int socketID)
{
    uint8_t socketMask = 0x01 << (socketID - 1);
    if (socketID == -1)
        socketMask = 0x0F;

    setPowerMask(socketMask, on);
}
