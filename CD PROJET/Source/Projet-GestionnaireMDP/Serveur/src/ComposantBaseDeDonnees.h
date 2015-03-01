#ifndef COMPOSANTBASEDEDONNEES_H
#define COMPOSANTBASEDEDONNEES_H

#include <QtCore>
#include <QSql>
#include <QSqlDatabase>

class ComposantCryptographie;

class ComposantBaseDeDonnees
{
public:
    ComposantBaseDeDonnees(const QString fichier, const QString fClePriveeRSA, const QString fClePubliqueRSA);
    ~ComposantBaseDeDonnees();
    bool ouvrirBaseDeDonnees();

    void connecterUtilisateur(const QString utilisateur, const QString motDePasse, int & id, QString &nom, bool & admin);

    // Listes générales
    QByteArray admin_listeUtilisateurs();
    QByteArray admin_listeGroupes();
    QByteArray admin_listeMotsDePasse();
    QByteArray admin_listeServeurs();

    // Listes particulières
    QByteArray listeMotsDePasse(const int idUtilisateur);
    QByteArray admin_listeGroupesUtilisateur(const int idUtilisateur);
    QByteArray admin_listeUtilisateursGroupe(const int idGroupe);
    QByteArray admin_listeMotsDePasseGroupe(const int idGroupe);
    QByteArray admin_listeGroupesMotDePasse(const int idMotDePasse);

    // Administration des utilisateurs
    QByteArray admin_gererUtilisateur(const QString mode, const int id = -1, const QString nom = "", const QString identifiant = "", const QString motDePasse = "", const bool admin = false);
    QByteArray admin_gererGroupesUtilisateur(const int idUtilisateur, const QVector<int> groupes);

    // Administration des groupes
    QByteArray admin_gererGroupe(const QString mode, const int id = -1, const QString nom = "");
    QByteArray admin_gererUtilisateursGroupe(const int idGroupe, const QVector<int> utilisateurs);
    QByteArray admin_gererMotsDePasseGroupe(const int idGroupe, const QVector<int> motsDePasse);

    // Administration des mots de passe
    QByteArray admin_gererMotDePasse(const QString mode, const int id = -1, const QString nom = "", const QString identifiant = "", const QString motDePasse = "", const int serveur = 0);
    QByteArray admin_gererGroupesMotDePasse(const int idMotDePasse, const QVector<int> groupes);

    // Administration des serveurs
    QByteArray admin_gererServeur(const QString mode, const int id = -1, const QString nom = "", const QString hote = "");

private:
    QSqlDatabase db;
    QString fichierBaseDeDonnees;
    QString fichierClePriveeRSA;
    QString fichierClePubliqueRSA;
    ComposantCryptographie * compCrypto;
};

#endif // COMPOSANTBASEDEDONNEES_H
