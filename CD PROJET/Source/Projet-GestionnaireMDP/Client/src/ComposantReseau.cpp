#include "ComposantReseau.h"
#include "../../Serveur/src/ComposantCryptographie.h"

#include <QThread>
#include <QMessageBox>
#include <cmath>

// Constructeur de la connexion au serveur avec ip et port
ComposantReseau::ComposantReseau(const QString ip, const int port, ComposantCryptographie * c) :
    socket(new QTcpSocket()),
    compCrypt(c),
    ipServ(ip),
    portServ(port)
{
}

// Destructeur
ComposantReseau::~ComposantReseau() {
    // On ferme la socket si elle est ouverte
    if(compCrypt && socket->isOpen()) {
        envoyerMessage("disconnect");
        socket->disconnectFromHost();
        if(socket->state() != QAbstractSocket::UnconnectedState)
            socket->waitForDisconnected(5000);
    }
    delete socket;
}

// Télécharge et renvoie la clé publique RSA
QByteArray ComposantReseau::getClePubliqueRSA() {
    socket->connectToHost(ipServ, portServ);
    if(socket->waitForConnected()) {
        QByteArray reponse = envoyerMessage_private("clepubliquersa");
        return reponse;
    }
    return QByteArray();
}

// Ouvre la connexion et initie le cryptage AES
bool ComposantReseau::connecter() {

    // Ouverture connexion
    socket->connectToHost(ipServ, portServ);
    if(socket->waitForConnected()) {

        // Génération clé AES
        compCrypt->genererCleAES();

        // Préparation message de clé AES
        QByteArray messageCleAES = "cleaes";
        messageCleAES.append(compCrypt->getCleAES());
        if(messageCleAES.size() == 6) return false;

        // Cryptage du message en RSA et envoi
        QByteArray messageCleAESCrypte = compCrypt->encrypterRSA(messageCleAES);
        QByteArray reponse = envoyerMessage_private(messageCleAESCrypte);

        return reponse == "cleaesok";
    }
    return false;
}

// Envoie un message au serveur après l'avoir crypté en AES
QByteArray ComposantReseau::envoyerMessage(const QByteArray message) {

    // Cryptage du message en AES
    QByteArray messageCrypte = compCrypt->encrypterAES(message);

    // Envoi et décryptage de la réponse
    QByteArray reponseCryptee = envoyerMessage_private(messageCrypte);
    QByteArray reponse = compCrypt->decrypterAES(reponseCryptee);

    return reponse;
}

// Envoie un message au serveur
QByteArray ComposantReseau::envoyerMessage_private(QByteArray message) {
    QByteArray reponse;

    // Ajout de la taille du message + 4 au début
    message.prepend(intToByteArray(message.size() + 4));

    // Envoi du message
    socket->write(message);
    socket->flush();

    // Réception de tous les blocs de données de la réponse
    socket->waitForReadyRead();
    reponse.append(socket->readAll());

    int taille = byteArrayToInt(reponse.mid(0, 4));

    while(reponse.size() != taille) {
        socket->waitForReadyRead();
        reponse.append(socket->readAll());
    }

    return reponse.mid(4);
}

// Convertit un entier en un tableau d'octets
QByteArray ComposantReseau::intToByteArray(unsigned int valeur) {
    QByteArray ba;

    unsigned char aux;

    for(int c = 3; c >= 0; c--) {
        aux = (unsigned char)(valeur / (unsigned int)pow(256, c));
        ba.append(aux);
        valeur -= (unsigned int)aux * (unsigned int)pow(256, c);
    }

    return ba;
}

// Convertit un tableau d'octets en un entier
unsigned int ComposantReseau::byteArrayToInt(const QByteArray ba) {
    unsigned int valeur = 0;

    if(ba.size() != 4) return valeur;

    unsigned char aux;

    for(int c = 3; c >= 0; c--) {
        aux = ba.at(3 - c);
        valeur += (unsigned int)aux * (unsigned int)pow(256, c);
    }

    return valeur;
}
