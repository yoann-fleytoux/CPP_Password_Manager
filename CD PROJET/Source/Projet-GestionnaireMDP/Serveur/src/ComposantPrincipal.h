#ifndef COMPOSANTPRINCIPAL_H
#define COMPOSANTPRINCIPAL_H

#include <QtCore>
#include <QObject>

class ComposantBaseDeDonnees;
class ComposantCryptographie;
class ComposantReseau;

class ComposantPrincipal : public QObject
{
    Q_OBJECT

public:
    ComposantPrincipal(int argc, char *argv[]);
    ~ComposantPrincipal();

private:
    ComposantBaseDeDonnees * composantBDD;
    ComposantReseau * composantReseau;

    QString fichierConfiguration;
    QString fichierBaseDeDonnees;
    QString fichierClePriveeRSA;
    QString fichierClePubliqueRSA;
    int portEcoute;

    void lancerServeur();
    void installerServeur(int argc, char *argv[]);
    void sauvegarderServeur(int argc, char *argv[]);
    void restaurerServeur(int argc, char *argv[]);

    QByteArray sauvegardeServeur();

    QByteArray intToByteArray(unsigned int valeur);
    unsigned int byteArrayToInt(const QByteArray ba);
    QByteArray genererMotDePasse(const int longueur);

public slots:
    void traitementMessage(QByteArray message);
};

#endif // COMPOSANTPRINCIPAL_H
