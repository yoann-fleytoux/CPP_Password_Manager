#ifndef COMPOSANTCRYPTOGRAPHIE_H
#define COMPOSANTCRYPTOGRAPHIE_H

// Librairies OpenSSL
#include <openssl/pem.h>
#include <openssl/aes.h>
#include <openssl/rand.h>

// Librairies Qt
#include <QObject>

class ComposantCryptographie : public QObject
{
    Q_OBJECT

public:
    ComposantCryptographie();
    ~ComposantCryptographie();

    // RSA
    bool loadClePubliqueRSA(const QString fichier);
    bool loadClePriveeRSA(const QString fichier);
    QByteArray getClePubliqueRSA();
    QByteArray decrypterRSA(const QByteArray messageCrypte);
    QByteArray encrypterRSA(const QByteArray messageClair);
    static bool genererClesRSA(const QString fichierClePriveeRSA, const QString fichierClePubliqueRSA);

    // AES
    bool genererCleAES();
    QByteArray getCleAES();
    bool setCleAES(const QByteArray cle);
    QByteArray decrypterAES(const QByteArray messageCrypte);
    QByteArray encrypterAES(const QByteArray messageClair);

private:
    // RSA
    QString fichierCleRSA;
    RSA * clePriveeRSA;
    RSA * clePubliqueRSA;

    // AES
    EVP_CIPHER_CTX * encAES;
    EVP_CIPHER_CTX * decAES;
    unsigned char * cleAES;
    bool setCleAES();

};

#endif
