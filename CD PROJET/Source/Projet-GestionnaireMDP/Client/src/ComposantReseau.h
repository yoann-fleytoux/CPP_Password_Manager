#ifndef COMPOSANTRESEAU_H
#define COMPOSANTRESEAU_H

#include <QObject>
#include <QTcpSocket>

class ComposantCryptographie;

class ComposantReseau : public QObject
{
    Q_OBJECT

public:
    ComposantReseau(const QString ip, const int port, ComposantCryptographie * c);
    ~ComposantReseau();

    bool connecter();

    QByteArray envoyerMessage(const QByteArray message);
    QByteArray getClePubliqueRSA();

private:
    QTcpSocket * socket;
    ComposantCryptographie * compCrypt;
    QString ipServ;
    int portServ;

    QByteArray envoyerMessage_private(QByteArray message);

    QByteArray intToByteArray(unsigned int valeur);
    unsigned int byteArrayToInt(const QByteArray ba);
};

#endif // COMPOSANTRESEAU_H
