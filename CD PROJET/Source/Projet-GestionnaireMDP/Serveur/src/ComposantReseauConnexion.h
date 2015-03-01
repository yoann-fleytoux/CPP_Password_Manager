#ifndef COMPOSANTRESEAUCONNEXION_H
#define COMPOSANTRESEAUCONNEXION_H

#include <QObject>

class ComposantCryptographie;
class QTcpSocket;

class ComposantReseauConnexion : public QObject
{
    Q_OBJECT

public:
    ComposantReseauConnexion(QTcpSocket *s, const QString clePriv, const QString clePub);
    ~ComposantReseauConnexion();
    void setUtilisateur(int id, bool admin);
    int getIdUtilisateur();
    bool getEstAdministrateur();

    void envoyerMessage(QByteArray message);

private:
    bool estCryptee;
    int idUtilisateur;
    bool estAdministrateur;
    QTcpSocket * socket;
    ComposantCryptographie * composantCrypto;

    QByteArray intToByteArray(unsigned int valeur);
    unsigned int byteArrayToInt(const QByteArray ba);

private slots:
    void receptionMessage();

signals:
    void traitementMessage(QByteArray message);
};

#endif // COMPOSANTRESEAUCONNEXION_H
