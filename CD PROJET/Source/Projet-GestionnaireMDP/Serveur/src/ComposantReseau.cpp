#include "ComposantReseau.h"
#include "ComposantReseauConnexion.h"
#include "ComposantPrincipal.h"

ComposantReseau::ComposantReseau(int port, QString clePriv, QString clePub, ComposantPrincipal * parent) {

    clePriveeRSA = clePriv;
    clePubliqueRSA = clePub;
    composantPrincipal = parent;

    // Ouverture de la socket d'écoute
    listen(QHostAddress::Any,port);

    // Toute connexion entrante entraine l'exécution de nouvelleConnexion()
    QObject::connect(this, SIGNAL(newConnection()),this,SLOT(nouvelleConnexion()));
}

void ComposantReseau::nouvelleConnexion() {

    // Récupération de la connexion entrante
    ComposantReseauConnexion * connexion = new ComposantReseauConnexion(nextPendingConnection(), clePriveeRSA, clePubliqueRSA);

    // Toute arrivée de données de la connexion sera retransmise au composant principal
    QObject::connect(connexion, SIGNAL(traitementMessage(QByteArray)),
                     composantPrincipal, SLOT(traitementMessage(QByteArray)));
}
