#include "ComposantBaseDeDonnees.h"
#include "ComposantCryptographie.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QFile>

ComposantBaseDeDonnees::ComposantBaseDeDonnees(const QString fichier, const QString fClePriveeRSA, const QString fClePubliqueRSA) {
    fichierBaseDeDonnees = fichier;
    fichierClePriveeRSA = fClePriveeRSA;
    fichierClePubliqueRSA = fClePubliqueRSA;
    compCrypto = NULL;
}

// Ferme la base de données si elle est ouverte
// Détruit le composant de cryptographie si il existe
ComposantBaseDeDonnees::~ComposantBaseDeDonnees() {
    if(db.isOpen()) db.close();
    if(compCrypto != NULL) delete compCrypto;
}

// Ouvre la base de données à partir du chemin fichierBaseDeDonnees
// Initialise le composant de cryptographie pour les mots de passe
// Renvoie true si l'ouverture a réussie, false sinon
bool ComposantBaseDeDonnees::ouvrirBaseDeDonnees() {

    // Initialisation base de données
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(fichierBaseDeDonnees);

    // Ouverture de la base de données
    if(db.open()) {
        // Initialisation du composant de cryptographie
        compCrypto = new ComposantCryptographie();
        if(!compCrypto) return false;
        if(compCrypto->loadClePriveeRSA(fichierClePriveeRSA) &&
           compCrypto->loadClePubliqueRSA(fichierClePubliqueRSA))
        {
            return true;
        }
        // Si le chargement a échoué
        delete compCrypto;
        compCrypto = NULL;
    }

    // Si l'ouverture de la base de données
    // Ou le chargement des clés RSA a échoué
    return false;
}

// Vérifie les informations de connexion utilisateur fournies
void ComposantBaseDeDonnees::connecterUtilisateur(const QString utilisateur, const QString motDePasse, int & id, QString & nom, bool & admin) {

    // Initialisations
    id = -1;
    nom = "";
    admin = false;

    QByteArray motDePasseCrypte;
    QString motDePasseClair;

    if(db.isOpen()) {

        // Préparation de la requête - utilisateur selon identifiant
        QSqlQuery query(db);
        query.prepare("SELECT id_utilisateur, nom, motdepasse, administrateur "
                      "FROM utilisateurs "
                      "WHERE identifiant = :identifiant");
        query.bindValue(":identifiant", utilisateur, QSql::Out);
        query.exec();

        // Récupération des valeurs en cas de résultat
        if(query.next()) {

            // Décryptage du mot de passe associé à l'identifiant
            motDePasseCrypte = QByteArray::fromHex(query.value(2).toString().toUtf8());
            motDePasseClair = QString(compCrypto->decrypterRSA(motDePasseCrypte));

            // Affectation des variables de la session de connexion
            if(motDePasseClair == motDePasse) {
                id = query.value(0).toInt();
                nom = query.value(1).toString();
                admin = query.value(3).toInt() == 1;
            }
        }
    }
}

/********************************************** LISTES GENERALES **********************************************/

// Administration : liste tous les utilisateurs du système
QByteArray ComposantBaseDeDonnees::admin_listeUtilisateurs() {

    // Préparation de la réponse
    QByteArray liste;
    liste.append("admin_listeutilisateurs");
    liste.append(30);

    QByteArray motDePasseCrypte;
    QString motDePasseClair;

    if(db.isOpen()) {

        // True lorsqu'au moins un utilisateur existe
        bool ok = false;

        // Préparation et exécution de la requête - liste utilisateurs
        QSqlQuery query(db);
        query.prepare("SELECT id_utilisateur, nom, identifiant, motdepasse, administrateur "
                      "FROM utilisateurs ");
        query.exec();

        // Récupération des valeurs
        while(query.next()) {
            ok = true;

            // Décryptage du mot de passe de l'utilisateur
            motDePasseCrypte = QByteArray::fromHex(query.value(3).toString().toUtf8());
            motDePasseClair = QString(compCrypto->decrypterRSA(motDePasseCrypte));

            liste.append(QString::number(query.value(0).toInt()));
            liste.append(31);
            liste.append(query.value(1).toString());
            liste.append(31);
            liste.append(query.value(2).toString());
            liste.append(31);
            liste.append(motDePasseClair);
            liste.append(31);
            liste.append(query.value(4).toInt() == 1 ? "admin" : "user");
            liste.append(30);
        }
        if(ok) liste.remove(liste.size() - 1, 1);
    }

    return liste;
}

// Administration : liste tous les groupes du système
QByteArray ComposantBaseDeDonnees::admin_listeGroupes() {

    // Préparation de la réponse
    QByteArray liste;
    liste.append("admin_listegroupes");
    liste.append(30);

    if(db.isOpen()) {

        // True lorsqu'au moins un groupe existe
        bool ok = false;

        // Préparation et exécution de la requête - liste groupes
        QSqlQuery query(db);
        query.prepare("SELECT id_groupe, nom "
                      "FROM groupes ");
        query.exec();

        // Récupération des valeurs en cas de résultat
        while(query.next()) {
            ok = true;
            liste.append(QString::number(query.value(0).toInt()));
            liste.append(31);
            liste.append(query.value(1).toString());
            liste.append(30);
        }
        if(ok) liste.remove(liste.size() - 1, 1);
    }

    return liste;
}

// Administration : liste tous les mots de passe du système
QByteArray ComposantBaseDeDonnees::admin_listeMotsDePasse() {

    // Préparation de la réponse
    QByteArray liste;
    liste.append("admin_listemotsdepasse");
    liste.append(30);

    QByteArray motDePasseCrypte;
    QString motDePasseClair;

    if(db.isOpen()) {

        // True lorsqu'au moins un mot de passe existe
        bool ok = false;

        // Préparation et exécution de la requête - liste mots de passe
        QSqlQuery query(db);
        query.prepare("SELECT id_motdepasse, motsdepasse.nom, identifiant, motdepasse, serveurs.id_serveur, serveurs.nom, hote "
                      "FROM motsdepasse, serveurs "
                      "WHERE serveurs.id_serveur = motsdepasse.id_serveur");
        query.exec();

        // Récupération des valeurs en cas de résultat
        while(query.next()) {
            ok = true;

            // Décryptage du mot de passe
            motDePasseCrypte = QByteArray::fromHex(query.value(3).toString().toUtf8());
            motDePasseClair = QString(compCrypto->decrypterRSA(motDePasseCrypte));

            liste.append(QString::number(query.value(0).toInt()));
            liste.append(31);
            liste.append(query.value(1).toString());
            liste.append(31);
            liste.append(query.value(2).toString());
            liste.append(31);
            liste.append(motDePasseClair);
            liste.append(31);
            liste.append(QString::number(query.value(4).toInt()));
            liste.append(31);
            liste.append(query.value(5).toString());
            liste.append(31);
            liste.append(query.value(6).toString());
            liste.append(30);
        }
        if(ok) liste.remove(liste.size() - 1, 1);
    }

    return liste;
}

// Administration : liste tous les serveurs du système
QByteArray ComposantBaseDeDonnees::admin_listeServeurs() {

    // Préparation de la réponse
    QByteArray liste;
    liste.append("admin_listeserveurs");
    liste.append(30);

    if(db.isOpen()) {

        // True lorsqu'au moins un serveur existe
        bool ok = false;

        // Préparation et exécution de la requête - liste serveurs
        QSqlQuery query(db);
        query.prepare("SELECT id_serveur, nom, hote "
                      "FROM serveurs ");
        query.exec();

        // Récupération des valeurs en cas de résultat
        while(query.next()) {
            ok = true;
            liste.append(QString::number(query.value(0).toInt()));
            liste.append(31);
            liste.append(query.value(1).toString());
            liste.append(31);
            liste.append(query.value(2).toString());
            liste.append(30);
        }
        if(ok) liste.remove(liste.size() - 1, 1);
    }

    return liste;
}


/********************************************** LISTES PARTICULIERES **********************************************/

// Utilisateur : liste tous les mots de passe accessibles à un utilisateur
QByteArray ComposantBaseDeDonnees::listeMotsDePasse(const int idUtilisateur) {

    // Préparation de la réponse
    QByteArray liste;
    liste.append("listemotsdepasse");
    liste.append(30);

    QByteArray motDePasseCrypte;
    QString motDePasseClair;

    if(db.isOpen()) {

        // True lorsqu'au moins un mot de passe est accessible
        bool ok = false;

        // Préparation et exécution de la requête - liste mots de passe de l'utilisateur
        QSqlQuery query(db);
        query.prepare("SELECT DISTINCT serveurs.nom, hote, motsdepasse.nom, identifiant, motdepasse "
                      "FROM serveurs, motsdepasse, utilisateurs_groupes, groupes_motsdepasse "
                      "WHERE serveurs.id_serveur = motsdepasse.id_serveur "
                      "  AND motsdepasse.id_motdepasse = groupes_motsdepasse.id_motdepasse "
                      "  AND groupes_motsdepasse.id_groupe = utilisateurs_groupes.id_groupe "
                      "  AND utilisateurs_groupes.id_utilisateur = :id_utilisateur");
        query.bindValue(":id_utilisateur", idUtilisateur, QSql::Out);
        query.exec();

        // Récupération des valeurs en cas de résultat
        while(query.next()) {
            ok = true;

            // Décryptage du mot de passe
            motDePasseCrypte = QByteArray::fromHex(query.value(4).toString().toUtf8());
            motDePasseClair = QString(compCrypto->decrypterRSA(motDePasseCrypte));

            liste.append(query.value(0).toString());
            liste.append(31);
            liste.append(query.value(1).toString());
            liste.append(31);
            liste.append(query.value(2).toString());
            liste.append(31);
            liste.append(query.value(3).toString());
            liste.append(31);
            liste.append(motDePasseClair);
            liste.append(30);
        }
        if(ok) liste.remove(liste.size() - 1, 1);
    }

    return liste;
}

// Administration : liste les groupes dans lequel un utilisateur est affecté
QByteArray ComposantBaseDeDonnees::admin_listeGroupesUtilisateur(const int idUtilisateur) {

    // Préparation de la réponse
    QByteArray liste;
    liste.append("admin_listegroupesutilisateur");
    liste.append(30);

    if(db.isOpen()) {

        // True lorsque l'utilisateur est affecté à au moins un groupe
        bool ok = false;

        // Préparation et exécution de la requête - liste groupes de utilisateur
        QSqlQuery query(db);
        query.prepare("SELECT DISTINCT groupes.id_groupe, nom "
                      "FROM groupes, utilisateurs_groupes "
                      "WHERE groupes.id_groupe = utilisateurs_groupes.id_groupe "
                      "  AND utilisateurs_groupes.id_utilisateur = :id_utilisateur");
        query.bindValue(":id_utilisateur", idUtilisateur, QSql::Out);
        query.exec();

        // Récupération des valeurs en cas de résultat
        while(query.next()) {
            ok = true;
            liste.append(QString::number(query.value(0).toInt()));
            liste.append(31);
            liste.append(query.value(1).toString());
            liste.append(30);
        }
        if(ok) liste.remove(liste.size() - 1, 1);
    }

    return liste;
}

// Administration : liste les utilisateurs affectés à un groupe
QByteArray ComposantBaseDeDonnees::admin_listeUtilisateursGroupe(const int idGroupe) {

    // Préparation de la réponse
    QByteArray liste;
    liste.append("admin_listeutilisateursgroupe");
    liste.append(30);

    if(db.isOpen()) {

        // True lorsqu'au moins un utilisateur est affecté au groupe
        bool ok = false;

        // Préparation et exécution de la requête - liste utilisateurs du groupe
        QSqlQuery query(db);
        query.prepare("SELECT DISTINCT utilisateurs.id_utilisateur, nom, identifiant "
                      "FROM utilisateurs, utilisateurs_groupes "
                      "WHERE utilisateurs.id_utilisateur = utilisateurs_groupes.id_utilisateur "
                      "  AND utilisateurs_groupes.id_groupe = :id_groupe");
        query.bindValue(":id_groupe", idGroupe, QSql::Out);
        query.exec();

        // Récupération des valeurs en cas de résultat
        while(query.next()) {
            ok = true;
            liste.append(QString::number(query.value(0).toInt()));
            liste.append(31);
            liste.append(query.value(1).toString());
            liste.append(31);
            liste.append(query.value(2).toString());
            liste.append(30);
        }
        if(ok) liste.remove(liste.size() - 1, 1);
    }

    return liste;
}

// Administration : liste les mots de passe auxquels un groupe a accès
QByteArray ComposantBaseDeDonnees::admin_listeMotsDePasseGroupe(const int idGroupe) {

    // Préparation de la réponse
    QByteArray liste;
    liste.append("admin_listemotsdepassegroupe");
    liste.append(30);

    if(db.isOpen()) {

        // True lorsque le groupe a accès à au moins un mot de passe
        bool ok = false;

        // Préparation et exécution de la requête - liste mots de passe du groupe
        QSqlQuery query(db);
        query.prepare("SELECT DISTINCT motsdepasse.id_motdepasse, motsdepasse.nom, identifiant, serveurs.nom, hote "
                      "FROM motsdepasse, serveurs, groupes_motsdepasse "
                      "WHERE serveurs.id_serveur = motsdepasse.id_serveur "
                      "  AND motsdepasse.id_motdepasse = groupes_motsdepasse.id_motdepasse "
                      "  AND groupes_motsdepasse.id_groupe = :id_groupe");
        query.bindValue(":id_groupe", idGroupe, QSql::Out);
        query.exec();

        // Récupération des valeurs en cas de résultat
        while(query.next()) {
            ok = true;
            liste.append(QString::number(query.value(0).toInt()));
            liste.append(31);
            liste.append(query.value(1).toString());
            liste.append(31);
            liste.append(query.value(2).toString());
            liste.append(31);
            liste.append(query.value(3).toString());
            liste.append(31);
            liste.append(query.value(4).toString());
            liste.append(30);
        }
        if(ok) liste.remove(liste.size() - 1, 1);
    }

    return liste;
}

// Administration : liste les groupes qui ont accès à un mot de passe
QByteArray ComposantBaseDeDonnees::admin_listeGroupesMotDePasse(const int idMotDePasse) {

    // Préparation de la réponse
    QByteArray liste;
    liste.append("admin_listegroupesmotdepasse");
    liste.append(30);

    if(db.isOpen()) {

        // True lorsqu'au moins un groupe a accès au mot de passe
        bool ok = false;

        // Préparation et exécution de la requête - liste groupes du mot de passe
        QSqlQuery query(db);
        query.prepare("SELECT DISTINCT groupes.id_groupe, nom "
                      "FROM groupes, groupes_motsdepasse "
                      "WHERE groupes.id_groupe = groupes_motsdepasse.id_groupe "
                      "  AND groupes_motsdepasse.id_motdepasse = :id_motdepasse");
        query.bindValue(":id_motdepasse", idMotDePasse, QSql::Out);
        query.exec();

        // Récupération des valeurs en cas de résultat
        while(query.next()) {
            ok = true;
            liste.append(QString::number(query.value(0).toInt()));
            liste.append(31);
            liste.append(query.value(1).toString());
            liste.append(30);
        }
        if(ok) liste.remove(liste.size() - 1, 1);
    }

    return liste;
}

/********************************************** GESTION UTILISATEURS **********************************************/

// Administration : gestion des utilisateurs
QByteArray ComposantBaseDeDonnees::admin_gererUtilisateur(const QString mode, const int id, const QString nom, const QString identifiant, const QString motDePasse, const bool admin) {

    QByteArray reponse;

    if(mode == "creation") {

        // Cryptage du mot de passe
        QString motDePasseCrypte = (compCrypto->encrypterRSA(motDePasse.toUtf8()).toHex());

        // Préparation de la requête - création utilisateur
        QSqlQuery query(db);
        query.prepare("INSERT INTO utilisateurs (nom, identifiant, motdepasse, administrateur) "
                      "                VALUES (:nom, :identifiant, :motdepasse, :administrateur)");
        query.bindValue(":nom", nom, QSql::In);
        query.bindValue(":identifiant", identifiant, QSql::In);
        query.bindValue(":motdepasse", motDePasseCrypte, QSql::In);
        query.bindValue(":administrateur", admin ? 1 : 0, QSql::In);

        // Exécution de la requête
        if(query.exec()) reponse = "creationok";
        else {
            reponse = "creationerreurtechnique";
            reponse.append(30);
            reponse.append(query.lastError().text());
        }

    }
    else if(mode == "modification") {

        // Cryptage du mot de passe
        QString motDePasseCrypte = QString(compCrypto->encrypterRSA(motDePasse.toUtf8()).toHex());

        // Préparation de la requête - modification utilisateur
        QSqlQuery query(db);
        query.prepare("UPDATE utilisateurs "
                      "SET nom = :nom, identifiant = :identifiant, motdepasse = :motdepasse, administrateur = :administrateur "
                      "WHERE id_utilisateur = :id_utilisateur");
        query.bindValue(":nom", nom, QSql::In);
        query.bindValue(":identifiant", identifiant, QSql::In);
        query.bindValue(":motdepasse", motDePasseCrypte, QSql::In);
        query.bindValue(":administrateur", admin ? 1 : 0, QSql::In);
        query.bindValue(":id_utilisateur", id, QSql::In);

        // Exécution de la requête
        if(query.exec()) {
            if(query.numRowsAffected() == 1) reponse = "modificationok";
            else reponse = "modificationerreurinexistant";
        }
        else {
            reponse = "modificationerreurtechnique";
            reponse.append(30);
            reponse.append(query.lastError().text());
        }

    }
    else if(mode == "suppression") {

        // Plusieurs requêtes en modification ont lieu : on démarre une transaction
        db.transaction();

        // Préparation et exécution de la requête - suppression utilisateur
        QSqlQuery query1(db);
        query1.prepare("DELETE FROM utilisateurs "
                      "WHERE id_utilisateur = :id_utilisateur");
        query1.bindValue(":id_utilisateur", id, QSql::In);
        query1.exec();

        // Si l'utilisateur existait bien
        if(query1.numRowsAffected() == 1) {

            // Préparation et exécution de la requête - suppression liens avec groupes de l'utilisateur
            QSqlQuery query2(db);
            query2.prepare("DELETE FROM utilisateurs_groupes "
                          "WHERE id_utilisateur = :id_utilisateur");
            query2.bindValue(":id_utilisateur", id, QSql::In);
            query2.exec();

            // Fin de transaction
            if(db.commit())
                reponse = "suppressionok";
            else {
                db.rollback();
                reponse = "suppressionerreurtechnique";
                reponse.append(30);
                reponse.append(db.lastError().text());
            }

        }
        // Si l'utilisateur n'existait pas
        else {
            db.rollback();
            reponse = "suppressionerreurinexistant";
        }
    }

    return reponse;
}

// Administration : gestion des groupes d'un utilisateur
QByteArray ComposantBaseDeDonnees::admin_gererGroupesUtilisateur(const int idUtilisateur, const QVector<int> groupes) {
    QByteArray reponse;

    // Vérification de l'existence de l'utilisateur
    QSqlQuery query(db);
    query.prepare("SELECT * FROM utilisateurs "
                  "WHERE id_utilisateur = :id_utilisateur");
    query.bindValue(":id_utilisateur", idUtilisateur, QSql::In);
    query.exec();

    // Si l'utilisateur n'existe pas
    if(!query.next()) reponse = "groupesutilisateurerreurinexistant";

    // Si l'utilisateur existe
    else {

        // Plusieurs requêtes en modification ont lieu : on démarre une transaction
        db.transaction();

        // Préparation de la requête - suppression liens avec groupes de l'utilisateurs
        QSqlQuery query1(db);
        query1.prepare("DELETE FROM utilisateurs_groupes "
                      "WHERE id_utilisateur = :id_utilisateur");
        query1.bindValue(":id_utilisateur", idUtilisateur, QSql::In);
        query1.exec();

        // Préparation de la requête - ajout liens avec nouveaux groupes de l'utilisateur
        QSqlQuery query2(db);
        query2.prepare("INSERT INTO utilisateurs_groupes (id_utilisateur, id_groupe) "
                       "                         VALUES (:id_utilisateur, :id_groupe)");
        query2.bindValue(":id_utilisateur", idUtilisateur, QSql::In);

        // Exécution de la requête pour chaque id de nouveau groupe
        for(int g = 0; g < groupes.size(); g++) {
            query2.bindValue(":id_groupe", groupes.at(g), QSql::In);
            query2.exec();
        }

        // Fin de transaction
        if(db.commit())
            reponse = "groupesutilisateurok";
        else {
            db.rollback();
            reponse = "groupesutilisateurerreurtechnique";
            reponse.append(30);
            reponse.append(db.lastError().text());
        }

    }

    return reponse;
}

/********************************************** GESTION GROUPES **********************************************/

// Administration : gestion des groupes
QByteArray ComposantBaseDeDonnees::admin_gererGroupe(const QString mode, const int id, const QString nom) {
    QByteArray reponse;

    if(mode == "creation") {
        // Préparation de la requête - création groupe
        QSqlQuery query(db);
        query.prepare("INSERT INTO groupes (nom) "
                      "             VALUES (:nom)");
        query.bindValue(":nom", nom, QSql::In);

        // Exécution de la requête
        if(query.exec()) reponse = "creationok";
        else {
            reponse = "creationerreurtechnique";
            reponse.append(30);
            reponse.append(query.lastError().text());
        }
    }
    else if(mode == "modification") {
        // Préparation de la requête - modification groupe
        QSqlQuery query(db);
        query.prepare("UPDATE groupes "
                      "SET nom = :nom "
                      "WHERE id_groupe = :id_groupe");
        query.bindValue(":nom", nom, QSql::In);
        query.bindValue(":id_groupe", id, QSql::In);

        // Exécution de la requête
        if(query.exec()) {
            if(query.numRowsAffected() == 1) reponse = "modificationok";
            else reponse = "modificationerreurinexistant";
        }
        else {
            reponse = "modificationerreurtechnique";
            reponse.append(30);
            reponse.append(query.lastError().text());
        }
    }
    else if(mode == "suppression") {

        // Plusieurs requêtes en modification ont lieu : on démarre une transaction
        db.transaction();

        // Préparation et exécution de la requête - suppression groupe
        QSqlQuery query1(db);
        query1.prepare("DELETE FROM groupes "
                       "WHERE id_groupe = :id_groupe");
        query1.bindValue(":id_groupe", id, QSql::In);
        query1.exec();

        // Si le groupe existait bien
        if(query1.numRowsAffected() == 1) {

            // Préparation de la requête - suppression liens avec utilisateurs du groupe
            QSqlQuery query2(db);
            query2.prepare("DELETE FROM utilisateurs_groupes "
                           "WHERE id_groupe = :id_groupe");
            query2.bindValue(":id_groupe", id, QSql::In);
            query2.exec();

            // Préparation de la requête - suppression liens avec mots de passe du groupe
            QSqlQuery query3(db);
            query3.prepare("DELETE FROM groupes_motsdepasse "
                           "WHERE id_groupe = :id_groupe");
            query3.bindValue(":id_groupe", id, QSql::In);
            query3.exec();

            // Fin de transaction
            if(db.commit())
                reponse = "suppressionok";
            else {
                db.rollback();
                reponse = "suppressionerreurtechnique";
                reponse.append(30);
                reponse.append(db.lastError().text());
            }

        }
        // Si le groupe n'existait pas
        else {
            db.rollback();
            reponse = "suppressionerreurinexistant";
        }
    }

    return reponse;
}

// Administration : gestion des utilisateurs d'un groupe
QByteArray ComposantBaseDeDonnees::admin_gererUtilisateursGroupe(const int idGroupe, const QVector<int> utilisateurs) {
    QByteArray reponse;

    // Vérification de l'existence du groupe
    QSqlQuery query(db);
    query.prepare("SELECT * FROM groupes "
                  "WHERE id_groupe = :id_groupe");
    query.bindValue(":id_groupe", idGroupe, QSql::In);
    query.exec();

    // Si le groupe n'existe pas
    if(!query.next()) reponse = "utilisateursgroupeerreurinexistant";

    // Si le groupe existe bien
    else {

        // Plusieurs requêtes en modification ont lieu : on démarre une transaction
        db.transaction();

        // Préparation de la requête - suppression liens avec utilisateurs du groupe
        QSqlQuery query1(db);
        query1.prepare("DELETE FROM utilisateurs_groupes "
                       "WHERE id_groupe = :id_groupe");
        query1.bindValue(":id_groupe", idGroupe, QSql::In);
        query1.exec();

        // Préparation de la requête - ajout liens avec nouveaux utilisateurs du groupe
        QSqlQuery query2(db);
        query2.prepare("INSERT INTO utilisateurs_groupes (id_utilisateur, id_groupe) "
                       "                         VALUES (:id_utilisateur, :id_groupe)");
        query2.bindValue(":id_groupe", idGroupe, QSql::In);

        // Exécution de la requête pour chaque id de nouvel utilisateur
        for(int u = 0; u < utilisateurs.size(); u++) {
            query2.bindValue(":id_utilisateur", utilisateurs.at(u), QSql::In);
            query2.exec();
        }

        // Fin de transaction
        if(db.commit())
            reponse = "utilisateursgroupeok";
        else {
            db.rollback();
            reponse = "utilisateursgroupeerreurtechnique";
            reponse.append(30);
            reponse.append(db.lastError().text());
        }

    }

    return reponse;
}

// Administration : gestion des mots de passe d'un groupe
QByteArray ComposantBaseDeDonnees::admin_gererMotsDePasseGroupe(const int idGroupe, const QVector<int> motsDePasse) {
    QByteArray reponse;

    // Vérification de l'existence du groupe
    QSqlQuery query(db);
    query.prepare("SELECT * FROM groupes "
                  "WHERE id_groupe = :id_groupe");
    query.bindValue(":id_groupe", idGroupe, QSql::In);
    query.exec();

    // Si le groupe n'existe pas
    if(!query.next()) reponse = "motsdepassegroupeerreurinexistant";

    // Si le groupe existe
    else {

        // Plusieurs requêtes en modification ont lieu : on démarre une transaction
        db.transaction();

        // Préparation de la requête - suppression liens avec mots de passe du groupe
        QSqlQuery query1(db);
        query1.prepare("DELETE FROM groupes_motsdepasse "
                       "WHERE id_groupe = :id_groupe");
        query1.bindValue(":id_groupe", idGroupe, QSql::In);
        query1.exec();

        // Préparation de la requête - ajout liens avec nouveaux mots de passe
        QSqlQuery query2(db);
        query2.prepare("INSERT INTO groupes_motsdepasse (id_groupe, id_motdepasse) "
                       "                         VALUES (:id_groupe, :id_motdepasse)");
        query2.bindValue(":id_groupe", idGroupe, QSql::In);

        // Exécution de la requête pour chaque id de nouveau mot de passe
        for(int m = 0; m < motsDePasse.size(); m++) {
            query2.bindValue(":id_motdepasse", motsDePasse.at(m), QSql::In);
            query2.exec();
        }

        // Fin de transaction
        if(db.commit())
            reponse = "motsdepassegroupeok";
        else {
            db.rollback();
            reponse = "motsdepassegroupeerreurtechnique";
            reponse.append(30);
            reponse.append(db.lastError().text());
        }

    }

    return reponse;
}

/********************************************** GESTION MOTS DE PASSE **********************************************/

// Administration : gestion des mots de passe
QByteArray ComposantBaseDeDonnees::admin_gererMotDePasse(const QString mode, const int id, const QString nom, const QString identifiant, const QString motDePasse, const int serveur) {
    QByteArray reponse;

    if(mode == "creation") {

        // Cryptage du mot de passe
        QString motDePasseCrypte = QString(compCrypto->encrypterRSA(motDePasse.toUtf8()).toHex());

        // Préparation de la requête - création mot de passe
        QSqlQuery query(db);
        query.prepare("INSERT INTO motsdepasse (nom, identifiant, motdepasse, id_serveur) "
                      "               VALUES (:nom, :identifiant, :motdepasse, :id_serveur)");
        query.bindValue(":nom", nom, QSql::In);
        query.bindValue(":identifiant", identifiant, QSql::In);
        query.bindValue(":motdepasse", motDePasseCrypte, QSql::In);
        query.bindValue(":id_serveur", serveur, QSql::In);

        // Exécution de la requête
        if(query.exec()) reponse = "creationok";
        else {
            reponse = "creationerreurtechnique";
            reponse.append(30);
            reponse.append(query.lastError().text());
        }
    }
    else if(mode == "modification") {

        // Cryptage du mot de passe
        QString motDePasseCrypte = QString(compCrypto->encrypterRSA(motDePasse.toUtf8()).toHex());

        // Préparation de la requête - modification mot de passe
        QSqlQuery query(db);
        query.prepare("UPDATE motsdepasse "
                      "SET nom = :nom, identifiant = :identifiant, motdepasse = :motdepasse, id_serveur = :id_serveur "
                      "WHERE id_motdepasse = :id_motdepasse");
        query.bindValue(":nom", nom, QSql::In);
        query.bindValue(":identifiant", identifiant, QSql::In);
        query.bindValue(":motdepasse", motDePasseCrypte, QSql::In);
        query.bindValue(":id_serveur", serveur, QSql::In);
        query.bindValue(":id_motdepasse", id, QSql::In);

        // Exécution de la requête
        if(query.exec()) {
            if(query.numRowsAffected() == 1) reponse = "modificationok";
            else reponse = "modificationerreurinexistant";
        }
        else {
            reponse = "modificationerreurtechnique";
            reponse.append(30);
            reponse.append(query.lastError().text());
        }
    }
    else if(mode == "suppression") {

        // Plusieurs requêtes en modification ont lieu : on démarre une transaction
        db.transaction();

        // Préparation et exécution de la requête - suppression mot de passe
        QSqlQuery query1(db);
        query1.prepare("DELETE FROM motsdepasse "
                       "WHERE id_motdepasse = :id_motdepasse");
        query1.bindValue(":id_motdepasse", id, QSql::In);
        query1.exec();

        // Si le mot de passe existait
        if(query1.numRowsAffected() == 1) {

            // Préparation et exécution de la requête - suppression liens avec groupes du mot de passe
            QSqlQuery query2(db);
            query2.prepare("DELETE FROM groupes_motsdepasse "
                           "WHERE id_motdepasse = :id_motdepasse");
            query2.bindValue(":id_motdepasse", id, QSql::In);
            query2.exec();

            // Fin de transaction
            if(db.commit())
                reponse = "suppressionok";
            else {
                db.rollback();
                reponse = "suppressionerreurtechnique";
                reponse.append(30);
                reponse.append(db.lastError().text());
            }
        }
        // Si le mot de passe n'existait pas
        else {
            db.rollback();
            reponse = "suppressionerreurinexistant";
        }
    }

    return reponse;
}

// Administration : gestion des groupes d'un mot de passe
QByteArray ComposantBaseDeDonnees::admin_gererGroupesMotDePasse(const int idMotDePasse, const QVector<int> groupes) {
    QByteArray reponse;

    // Vérification de l'existence du mot de passe
    QSqlQuery query(db);
    query.prepare("SELECT * FROM motsdepasse "
                  "WHERE id_motdepasse = :id_motdepasse");
    query.bindValue(":id_motdepasse", idMotDePasse, QSql::In);
    query.exec();

    // Si le mot de passe n'existe pas
    if(!query.next()) reponse = "groupesmotdepasseerreurinexistant";

    // Si le mot de passe existe bien
    else {

        // Plusieurs requêtes en modification ont lieu : on démarre une transaction
        db.transaction();

        // Préparation et exécution de la requête - suppression liens avec groupes du mot de passe
        QSqlQuery query1(db);
        query1.prepare("DELETE FROM groupes_motsdepasse "
                       "WHERE id_motdepasse = :id_motdepasse");
        query1.bindValue(":id_motdepasse", idMotDePasse, QSql::In);
        query1.exec();

        // Préparation de la requête - ajout liens avec nouveaux groupes
        QSqlQuery query2(db);
        query2.prepare("INSERT INTO groupes_motsdepasse (id_groupe, id_motdepasse) "
                       "                         VALUES (:id_groupe, :id_motdepasse)");
        query2.bindValue(":id_motdepasse", idMotDePasse, QSql::In);

        // Exécution de la requête pour chaque id de nouveau groupe
        for(int g = 0; g < groupes.size(); g++) {
            query2.bindValue(":id_groupe", groupes.at(g), QSql::In);
            query2.exec();
        }

        // Fin de transaction
        if(db.commit())
            reponse = "groupesmotdepasseok";
        else {
            db.rollback();
            reponse = "groupesmotdepasseerreurtechnique";
            reponse.append(30);
            reponse.append(db.lastError().text());
        }
    }

    return reponse;
}

/********************************************** GESTION SERVEURS **********************************************/

// Administration : gestion des serveurs
QByteArray ComposantBaseDeDonnees::admin_gererServeur(const QString mode, const int id, const QString nom, const QString hote) {
    QByteArray reponse;

    if(mode == "creation") {
        // Préparation de la requête - création serveur
        QSqlQuery query(db);
        query.prepare("INSERT INTO serveurs (nom, hote) "
                      "             VALUES (:nom, :hote)");
        query.bindValue(":nom", nom, QSql::In);
        query.bindValue(":hote", hote, QSql::In);

        // Exécution de la requête
        if(query.exec()) reponse = "creationok";
        else {
            reponse = "creationerreurtechnique";
            reponse.append(30);
            reponse.append(query.lastError().text());
        }
    }
    else if(mode == "modification") {
        // Préparation de la requête - modification serveur
        QSqlQuery query(db);
        query.prepare("UPDATE serveurs "
                      "SET nom = :nom, hote = :hote "
                      "WHERE id_serveur = :id_serveur");
        query.bindValue(":nom", nom, QSql::In);
        query.bindValue(":hote", hote, QSql::In);
        query.bindValue(":id_serveur", id, QSql::In);

        // Exécution de la requête
        if(query.exec()) {
            if(query.numRowsAffected() == 1) reponse = "modificationok";
            else reponse = "modificationerreurinexistant";
        }
        else {
            reponse = "modificationerreurtechnique";
            reponse.append(30);
            reponse.append(query.lastError().text());
        }
    }
    else if(mode == "suppression") {

        // Plusieurs requêtes en modification ont lieu : on démarre une transaction
        db.transaction();

        // Préparation de la requête - suppression serveur
        QSqlQuery query1(db);
        query1.prepare("DELETE FROM serveurs "
                       "WHERE id_serveur = :id_serveur");
        query1.bindValue(":id_serveur", id, QSql::In);
        query1.exec();

        // Si le serveur existait bien
        if(query1.numRowsAffected() == 1) {

            // Préparation de la requête - liste mots de passe du serveur
            QSqlQuery query2(db);
            query2.prepare("SELECT id_motdepasse FROM motsdepasse "
                           "WHERE id_serveur = :id_serveur");
            query2.bindValue(":id_serveur", id, QSql::In);
            query2.exec();

            // Préparation de la requête - suppression liens avec groupes des mots de passe
            QSqlQuery query3(db);
            query3.prepare("DELETE FROM groupes_motsdepasse "
                           "WHERE id_motdepasse = :id_motdepasse");

            // Exécution de la requête pour chaque mot de passe du serveur supprimé
            while(query2.next()) {
                query3.bindValue(":id_motdepasse", query2.value(0).toInt());
                query3.exec();
            }

            // Préparation et exécution de la requête - suppression mots de passe du serveur supprimé
            QSqlQuery query4(db);
            query4.prepare("DELETE FROM motsdepasse "
                           "WHERE id_serveur = :id_serveur");
            query4.bindValue(":id_serveur", id, QSql::In);
            query4.exec();

            // Fin de transaction
            if(db.commit())
                reponse = "suppressionok";
            else {
                db.rollback();
                reponse = "suppressionerreurtechnique";
                reponse.append(30);
                reponse.append(db.lastError().text());
            }

        }
        // Si le serveur n'existait pas
        else {
            db.rollback();
            reponse = "suppressionerreurinexistant";
        }
    }

    return reponse;
}
