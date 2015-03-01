#include "ComposantReseauConnexion.h"
#include "ComposantCryptographie.h"

#include <QTcpSocket>
#include <cmath>

ComposantReseauConnexion::ComposantReseauConnexion(QTcpSocket *s, const QString clePriv, const QString clePub)
{
    socket = s;

    // Toute arrivée de données entrainera l'exécution de receptionMessage()
    QObject::connect(socket, SIGNAL(readyRead()),this,SLOT(receptionMessage()));

    // Si déconnexion, on supprime la connexion
    QObject::connect(socket, SIGNAL(disconnected()),this,SLOT(deleteLater()));

    // Création du composant de cryptographie pour la connexion
    composantCrypto = new ComposantCryptographie();
    composantCrypto->loadClePriveeRSA(clePriv);
    composantCrypto->loadClePubliqueRSA(clePub);

    // Initialisation session
    estCryptee = false;
    idUtilisateur = -1;
    estAdministrateur = false;
}

ComposantReseauConnexion::~ComposantReseauConnexion()
{
    delete socket;
    delete composantCrypto;
}

// Traitement des données reçues sur la connexion
void ComposantReseauConnexion::receptionMessage() {

    // Lecture du bloc de données
    QByteArray reponse = socket->readAll();

    unsigned int taille = byteArrayToInt(reponse.mid(0, 4));

    while((unsigned int)reponse.size() < taille) {
        socket->waitForReadyRead();
        reponse.append(socket->readAll());
    }

    QByteArray messageCrypte = reponse.mid(4);

    // Si la connexion n'est pas cryptée
    if(!estCryptee) {
        QByteArray reponse;
        QByteArray message = composantCrypto->decrypterRSA(messageCrypte);

        // Si le message ne contient pas la clé AES
        if(!message.startsWith("cleaes")) {

            // Le client avait demandé la clé publique RSA du serveur (en non crypté)
            if(messageCrypte == "clepubliquersa") {
                reponse = composantCrypto->getClePubliqueRSA();
            }
            // Erreur de message
            else reponse = "cleaeserreur";
        }

        // Sinon on récupère la clé AES
        else if(composantCrypto->setCleAES(message.mid(6))) {
            reponse = "cleaesok";
            estCryptee = true;
        }

        // Ajout de la taille du message + 4 au début
        reponse.prepend(intToByteArray(reponse.size() + 4));

        // Envoi de la réponse
        socket->write(reponse);
        socket->flush();
        return;
    }

    // Si la connexion était cryptée, on décrypte et on lance le traitement du message
    QByteArray message = composantCrypto->decrypterAES(messageCrypte);
    emit traitementMessage(message);
}

// Envoi d'un message au client
// Le message sera crypté en AES
void ComposantReseauConnexion::envoyerMessage(QByteArray message) {

    if(!estCryptee) return;

    // Cryptage du message en AES
    QByteArray messageCrypte = composantCrypto->encrypterAES(message);

    // Ajout de la taille du message + 4 au début
    messageCrypte.prepend(intToByteArray(messageCrypte.size() + 4));

    // Envoi de la réponse
    socket->write(messageCrypte);
    socket->flush();
}

// Prise en compte des informations de l'utilisateur connecté dans la session
void ComposantReseauConnexion::setUtilisateur(int id, bool admin) {
    idUtilisateur = id;
    estAdministrateur = admin;
}

// Récupère l'id de l'utilisateur connecté
int ComposantReseauConnexion::getIdUtilisateur() {
    return idUtilisateur;
}

// Indique si l'utilisateur connecté est administrateur
bool ComposantReseauConnexion::getEstAdministrateur() {
    return estAdministrateur;
}

// Convertit un entier en un tableau d'octets
QByteArray ComposantReseauConnexion::intToByteArray(unsigned int valeur) {
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
unsigned int ComposantReseauConnexion::byteArrayToInt(const QByteArray ba) {
    unsigned int valeur = 0;

    if(ba.size() != 4) return valeur;

    unsigned char aux;

    for(int c = 3; c >= 0; c--) {
        aux = ba.at(3 - c);
        valeur += (unsigned int)aux * (unsigned int)pow(256, c);
    }

    return valeur;
}
