#ifndef COMPOSANTRESEAU_H
#define COMPOSANTRESEAU_H

#include <QTcpServer>

class ComposantCryptographie;
class ComposantPrincipal;

class ComposantReseau : public QTcpServer
{
    Q_OBJECT

public:
    ComposantReseau(int port, QString clePriv, QString clePub, ComposantPrincipal *parent);

private:
    ComposantPrincipal * composantPrincipal;
    QString clePriveeRSA;
    QString clePubliqueRSA;

private slots:
    void nouvelleConnexion();
};

#endif // COMPOSANTRESEAU_H
