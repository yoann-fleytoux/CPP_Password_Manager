#include "ComposantPrincipal.h"
#include "ComposantBaseDeDonnees.h"
#include "ComposantCryptographie.h"
#include "ComposantReseau.h"
#include "ComposantReseauConnexion.h"
#include "ComposantDocumentation.h"

#include <QFile>
#include <QDir>
#include <QTextStream>
#include <cmath>
#include <iostream>


// Le constructeur analyse les paramètres de la ligne de commande
ComposantPrincipal::ComposantPrincipal(int argc, char *argv[])
{

    // Fichier de configuration par défaut
    fichierConfiguration = QDir::homePath() + QString("/spmd/spmd.conf");

    if(argc == 1) {
        lancerServeur();
        return;
    }

    // Si des paramètres sont présents, on prépare la sortie standard pour écriture
    QTextStream out(stdout);

    // Lancement du serveur avec un fichier de configuration spécifique
    if(strcmp(argv[1], "--config") == 0) {

        if(argc != 3) {
            out << "Ligne de commande incorrecte." << endl;
            ComposantDocumentation::aide();
            exit(1);
        }
        if(!QFile::exists(argv[2])) {
            out << "Impossible de lancer le serveur : fichier de configuration introuvable." << endl;
            exit(1);
        }
        fichierConfiguration = argv[2];
        lancerServeur();
        return;
    }

    // Installation du serveur
    if(strcmp(argv[1], "--install") == 0) {
        installerServeur(argc, argv);
        exit(0);
    }

    // Sauvegarde du serveur
    if(strcmp(argv[1], "--backup") == 0) {
        sauvegarderServeur(argc, argv);
        exit(0);
    }

    // Restauration d'une sauvegarde du serveur
    if(strcmp(argv[1], "--restore") == 0) {
        restaurerServeur(argc, argv);
        exit(0);
    }

    // Affichage de l'aide
    if(strcmp(argv[1], "--help") == 0) {
        ComposantDocumentation::aide();
        exit(0);
    }

    // Par défaut, affichage de l'aide
    out << endl << "Ligne de commande incorrecte." << endl;
    ComposantDocumentation::aide();
    exit(1);
    return;

}

ComposantPrincipal::~ComposantPrincipal() {
}

// Lancement normal du serveur
void ComposantPrincipal::lancerServeur() {

    // On prépare la sortie standard pour écriture
    QTextStream out(stdout);

    // Vérification de la présence du fichier de configuration
    QFile fichierConfig(fichierConfiguration);
    if(!fichierConfig.exists()) {
        out << "Impossible de lancer le serveur : le fichier de configuration est introuvable." << endl;
        exit(1);
    }
    if(!fichierConfig.open(QIODevice::ReadOnly)) {
        out << "Impossible de lancer le serveur : le fichier de configuration ne peut etre ouvert." << endl;
        exit(1);
    }

    // Chargement de la configuration
    QString fichier(fichierConfig.readAll());
    fichierConfig.close();

    QStringList lignes = fichier.split("\n");
    QStringList ligne;
    for(int l = 0; l < lignes.size(); l++) {
        ligne = lignes.at(l).split("=");
        if(ligne.at(0) == "fichierClePubliqueRSA") fichierClePubliqueRSA = ligne.at(1).trimmed();
        else if(ligne.at(0) == "fichierClePriveeRSA") fichierClePriveeRSA = ligne.at(1).trimmed();
        else if(ligne.at(0) == "fichierBaseDeDonnees") fichierBaseDeDonnees = ligne.at(1).trimmed();
        else if(ligne.at(0) == "portEcoute") portEcoute = ligne.at(1).toInt();
    }

    // Création composant base de données avec le fichier fichierBaseDeDonnees
    composantBDD = new ComposantBaseDeDonnees(fichierBaseDeDonnees, fichierClePriveeRSA, fichierClePubliqueRSA);
    if(!composantBDD->ouvrirBaseDeDonnees()) {
        out << "Impossible de lancer le serveur : la base de donnees ne peut etre ouverte." << endl;
        exit(1);
    }

    // Création composant réseau avec les fichiers de cles RSA
    composantReseau = new ComposantReseau(portEcoute, fichierClePriveeRSA, fichierClePubliqueRSA, this);
}

// Traitement des messages provenant de la connexion réseau
void ComposantPrincipal::traitementMessage(QByteArray message) {

    // On récupère la connexion qui a demandé le traitement du message
    ComposantReseauConnexion * connexion = (ComposantReseauConnexion *)(QObject::sender());

    // Si l'utilisateur n'est pas identifié, on tente une identification
    if(connexion->getIdUtilisateur() == -1) {
        QList<QByteArray> champs = message.split(31);
        if(champs.at(0) == "connexion") {
            int id;
            QString nom;
            bool admin;
            composantBDD->connecterUtilisateur(champs.at(1), champs.at(2), id, nom, admin);
            if(id != -1) {
                connexion->setUtilisateur(id, admin);
                QByteArray reponse = "connexionacceptee";
                reponse.append(31);
                if(admin) reponse.append("admin");
                else reponse.append("user");
                reponse.append(31);
                reponse.append(QString::number(id));
                reponse.append(31);
                reponse.append(nom);
                connexion->envoyerMessage(reponse);
            }
            else connexion->envoyerMessage("connexionrefusee");
            return;
        }
        else {
            connexion->envoyerMessage("nonconnecte");
            return;
        }
    }
    // Sinon, on analyse la requête pour exécuter le bon traitement
    else {
        QList<QByteArray> champs = message.split(30);
        if(champs.at(0) == "listemotsdepasse") {
            QByteArray liste = composantBDD->listeMotsDePasse(connexion->getIdUtilisateur());
            connexion->envoyerMessage(liste);
            return;
        }
        else if(connexion->getEstAdministrateur()) {
            if(champs.at(0).startsWith("admin_liste")) {
                QByteArray liste;

                // Listes générales
                if(champs.at(0) == "admin_listeutilisateurs") liste = composantBDD->admin_listeUtilisateurs();
                else if(champs.at(0) == "admin_listegroupes") liste = composantBDD->admin_listeGroupes();
                else if(champs.at(0) == "admin_listemotsdepasse") liste = composantBDD->admin_listeMotsDePasse();
                else if(champs.at(0) == "admin_listeserveurs") liste = composantBDD->admin_listeServeurs();

                // Listes particulières
                else if(champs.at(0) == "admin_listegroupesutilisateur") liste = composantBDD->admin_listeGroupesUtilisateur(champs.at(1).toInt());
                else if(champs.at(0) == "admin_listeutilisateursgroupe") liste = composantBDD->admin_listeUtilisateursGroupe(champs.at(1).toInt());
                else if(champs.at(0) == "admin_listemotsdepassegroupe") liste = composantBDD->admin_listeMotsDePasseGroupe(champs.at(1).toInt());
                else if(champs.at(0) == "admin_listegroupesmotdepasse") liste = composantBDD->admin_listeGroupesMotDePasse(champs.at(1).toInt());


                connexion->envoyerMessage(liste);
                return;
            }
            else if(champs.at(0) == "admin_gererutilisateur") {
                QByteArray reponse;
                if(champs.at(1) == "creation") reponse = composantBDD->admin_gererUtilisateur("creation", champs.at(2).toInt(), champs.at(3), champs.at(4), champs.at(5), champs.at(6) == "true");
                else if(champs.at(1) == "modification") reponse = composantBDD->admin_gererUtilisateur("modification", champs.at(2).toInt(), champs.at(3), champs.at(4), champs.at(5), champs.at(6) == "true");
                else if(champs.at(1) == "suppression") reponse = composantBDD->admin_gererUtilisateur("suppression", champs.at(2).toInt());
                else if(champs.at(1) == "groupes") {
                    QVector<int> groupesID;
                    QList<QByteArray> groupesListe = champs.at(3).split(31);
                    for(int g = 0; g < groupesListe.size(); g++)
                        groupesID.append(groupesListe.at(g).toInt());
                    reponse = composantBDD->admin_gererGroupesUtilisateur(champs.at(2).toInt(), groupesID);
                }
                connexion->envoyerMessage(reponse);
                return;
            }
            else if(champs.at(0) == "admin_gerergroupe") {
                QByteArray reponse;
                if(champs.at(1) == "creation") reponse = composantBDD->admin_gererGroupe("creation", champs.at(2).toInt(), champs.at(3));
                else if(champs.at(1) == "modification") reponse = composantBDD->admin_gererGroupe("modification", champs.at(2).toInt(), champs.at(3));
                else if(champs.at(1) == "suppression") reponse = composantBDD->admin_gererGroupe("suppression", champs.at(2).toInt());
                else if(champs.at(1) == "utilisateurs") {
                    QVector<int> utilisateursID;
                    QList<QByteArray> utilisateursListe = champs.at(3).split(31);
                    for(int g = 0; g < utilisateursListe.size(); g++)
                        utilisateursID.append(utilisateursListe.at(g).toInt());
                    reponse = composantBDD->admin_gererUtilisateursGroupe(champs.at(2).toInt(), utilisateursID);
                }
                else if(champs.at(1) == "motsdepasse") {
                    QVector<int> motsdepasseID;
                    QList<QByteArray> motsdepasseListe = champs.at(3).split(31);
                    for(int g = 0; g < motsdepasseListe.size(); g++)
                        motsdepasseID.append(motsdepasseListe.at(g).toInt());
                    reponse = composantBDD->admin_gererMotsDePasseGroupe(champs.at(2).toInt(), motsdepasseID);
                }
                connexion->envoyerMessage(reponse);
                return;
            }
            else if(champs.at(0) == "admin_gerermotdepasse") {
                QByteArray reponse;
                if(champs.at(1) == "creation") reponse = composantBDD->admin_gererMotDePasse("creation", champs.at(2).toInt(), champs.at(3), champs.at(4), champs.at(5), champs.at(6).toInt());
                else if(champs.at(1) == "modification") reponse = composantBDD->admin_gererMotDePasse("modification", champs.at(2).toInt(), champs.at(3), champs.at(4), champs.at(5), champs.at(6).toInt());
                else if(champs.at(1) == "suppression") reponse = composantBDD->admin_gererMotDePasse("suppression", champs.at(2).toInt());
                else if(champs.at(1) == "groupes") {
                    QVector<int> groupesID;
                    QList<QByteArray> groupesListe = champs.at(3).split(31);
                    for(int g = 0; g < groupesListe.size(); g++)
                        groupesID.append(groupesListe.at(g).toInt());
                    reponse = composantBDD->admin_gererGroupesMotDePasse(champs.at(2).toInt(), groupesID);
                }
                connexion->envoyerMessage(reponse);
                return;
            }
            else if(champs.at(0) == "admin_gererserveur") {
                QByteArray reponse;
                if(champs.at(1) == "creation") reponse = composantBDD->admin_gererServeur("creation", champs.at(2).toInt(), champs.at(3), champs.at(4));
                else if(champs.at(1) == "modification") reponse = composantBDD->admin_gererServeur("modification", champs.at(2).toInt(), champs.at(3), champs.at(4));
                else if(champs.at(1) == "suppression") reponse = composantBDD->admin_gererServeur("suppression", champs.at(2).toInt());

                connexion->envoyerMessage(reponse);
                return;
            }
            else if(champs.at(0) == "admin_sauvegarder") {
                QByteArray reponse;
                reponse = sauvegardeServeur();
                connexion->envoyerMessage(reponse);
                return;
            }
        }
        connexion->envoyerMessage("commandeincorrecte");
    }
}

// Renvoie la sauvegarde du serveur au format "sauvegardeok" + mot de passe généré (16 caractères) + contenu crypté sauvegarde
QByteArray ComposantPrincipal::sauvegardeServeur() {
    QByteArray sauvegarde;

    QFile fConfig(fichierConfiguration);
    QFile fClePriveeRSA(fichierClePriveeRSA);
    QFile fClePubliqueRSA(fichierClePubliqueRSA);
    QFile fBaseDeDonnees(fichierBaseDeDonnees);

    // Vérification de l'ouverture des fichiers
    if(!fConfig.open(QIODevice::ReadOnly) ||
       !fClePriveeRSA.open(QIODevice::ReadOnly) ||
       !fClePubliqueRSA.open(QIODevice::ReadOnly) ||
       !fBaseDeDonnees.open(QIODevice::ReadOnly)) return QByteArray();

    // Sauvegarde fichier de configuration
    sauvegarde.append(intToByteArray(fConfig.bytesAvailable()));
    sauvegarde.append(fConfig.readAll());

    // Sauvegarde fichier de clé privée RSA
    sauvegarde.append(intToByteArray(fClePriveeRSA.bytesAvailable()));
    sauvegarde.append(fClePriveeRSA.readAll());

    // Sauvegarde fichier de clé publique RSA
    sauvegarde.append(intToByteArray(fClePubliqueRSA.bytesAvailable()));
    sauvegarde.append(fClePubliqueRSA.readAll());

    // Sauvegarde fichier de la base de données
    sauvegarde.append(intToByteArray(fBaseDeDonnees.bytesAvailable()));
    sauvegarde.append(fBaseDeDonnees.readAll());

    // Fermeture des fichiers
    fConfig.close();
    fClePriveeRSA.close();
    fClePubliqueRSA.close();
    fBaseDeDonnees.close();

    // Génération du mot de passe utilisé en clé AES
    QByteArray motDePasse = genererMotDePasse(16);

    ComposantCryptographie c;
    if(!c.setCleAES(motDePasse + motDePasse))
        return QByteArray();

    // Création de la sauvegarde au format "sauvegardeok" + mot de passe généré (16 caractères) + contenu crypté sauvegarde
    QByteArray sauvegardeCryptee = "sauvegardeok";
    sauvegardeCryptee.append(motDePasse);
    sauvegardeCryptee.append(c.encrypterAES(sauvegarde));

    // En cas d'erreur
    if(sauvegardeCryptee.size() <= 28) return QByteArray();

    return sauvegardeCryptee;
}

// Convertit un entier en un tableau d'octets
QByteArray ComposantPrincipal::intToByteArray(unsigned int valeur) {
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
unsigned int ComposantPrincipal::byteArrayToInt(const QByteArray ba) {
    unsigned int valeur = 0;

    if(ba.size() != 4) return valeur;

    unsigned char aux;

    for(int c = 3; c >= 0; c--) {
        aux = ba.at(3 - c);
        valeur += (unsigned int)aux * (unsigned int)pow(256, c);
    }

    return valeur;
}

// Génère un mot de passe aléatoirement d'une longueur donnée
QByteArray ComposantPrincipal::genererMotDePasse(const int longueur) {
    QByteArray motDePasse;

    // Caractères utilisés dans le mot de passe
    QByteArray caracteres = "abcdefghijklmnopqrstuvwxyz";
    caracteres += "0123456789";
    caracteres += "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    caracteres += "0123456789";

    int ch;

    // Choix aléatoire d'un caractère jusqu'à obtenir la longueur voulue
    srand(time(NULL));
    for(int c = 0; c < longueur; c++) {
        ch = rand() % caracteres.size();
        motDePasse.append(caracteres.at(ch));
    }

    return motDePasse;
}

// Installation du serveur avec les paramètres donnés en ligne de commande
void ComposantPrincipal::installerServeur(int argc, char *argv[]) {

    QTextStream out(stdout);

    int p = 2;

    // Fichiers et paramètres par défaut
    fichierBaseDeDonnees = QDir::homePath() + QString("/spmd/database.sqlite3");
    fichierClePriveeRSA = QDir::homePath() + QString("/spmd/private_key.pem");
    fichierClePubliqueRSA = QDir::homePath() + QString("/spmd/public_key.pem");
    portEcoute = 3665;
    bool genererCles = true;

    // Analyse de chaque argument de la ligne de commande
    while(p < argc) {
        if(strcmp(argv[p], "--config") == 0 && p + 1 < argc) {
            fichierConfiguration = argv[p+1];
            p += 2;
        }
        else if(strcmp(argv[p], "--priv-key") == 0 && p + 1 < argc) {
            fichierClePriveeRSA = argv[p+1];
            p += 2;
        }
        else if(strcmp(argv[p], "--pub-key") == 0 && p + 1 < argc) {
            fichierClePubliqueRSA = argv[p+1];
            p += 2;
        }
        else if(strcmp(argv[p], "--database") == 0 && p + 1 < argc) {
            fichierBaseDeDonnees = argv[p+1];
            p += 2;
        }
        else if(strcmp(argv[p], "--port") == 0 && p + 1 < argc) {
            portEcoute = QString(argv[p+1]).toInt();
            p += 2;
        }
        else if(strcmp(argv[p], "--use-existing-keys") == 0) {
            genererCles = false;
            p++;
        }
        else p++;
    }

    // On vérifie l'existence du dossier spmd
    QDir pathToConf(QDir::homePath() + QString("/spmd"));
    if(!pathToConf.exists()) {
        if(!pathToConf.mkpath(QDir::homePath() + QString("/spmd"))) {
            out << "Impossible de créer le dossier " << QDir::homePath() + QString("/spmd") << " . Abandon." << endl;
            exit(1);
        }
    }

    // Ouverture fichier configuration
    if(QFile::exists(fichierConfiguration)) {
        char c = 0;
        while(c != 'o' && c != 'n') {
            out << "Le fichier " << fichierConfiguration << " existe, remplacer ? (o/n)" << endl;
            std::cin >> c;
        }
        if(c == 'n') {
            out << "Abandon." << endl;
            exit(1);
        }
    }

    QFile fConfig(fichierConfiguration);
    if(!fConfig.open(QFile::WriteOnly | QFile::Truncate)) {
        out << "Impossible d'ecrire dans le fichier " << fichierConfiguration << ". Abandon." << endl;
        exit(1);
    }

    // Ouverture fichier base de donées
    if(QFile::exists(fichierBaseDeDonnees)) {
        char c = 0;
        while(c != 'o' && c != 'n') {
            out << "Le fichier " << fichierBaseDeDonnees << " existe, remplacer ? (o/n)" << endl;
            std::cin >> c;
        }
        if(c == 'n') {
            out << "Abandon." << endl;
            exit(1);
        }
        else if(!QFile::remove(fichierBaseDeDonnees)) {
            out << "Impossible d'ecrire dans le fichier " << fichierBaseDeDonnees << ". Abandon." << endl;
            exit(1);
        }
    }

    // En cas de génération de clés RSA
    if(genererCles) {
        // Ouverture fichier clé privée
        if(QFile::exists(fichierClePriveeRSA)) {
            char c = 0;
            while(c != 'o' && c != 'n') {
                out << "Le fichier " << fichierClePriveeRSA << " existe, remplacer ? (o/n)" << endl;
                std::cin >> c;
            }
            if(c == 'n') {
                out << "Abandon." << endl;
                exit(1);
            }
            else if(!QFile::remove(fichierClePriveeRSA)) {
                out << "Impossible d'ecrire dans le fichier " << fichierClePriveeRSA << ". Abandon." << endl;
                exit(1);
            }
        }

        // Ouverture fichier clé publique
        if(QFile::exists(fichierClePubliqueRSA)) {
            char c = 0;
            while(c != 'o' && c != 'n') {
                out << "Le fichier " << fichierClePubliqueRSA << " existe, remplacer ? (o/n)" << endl;
                std::cin >> c;
            }
            if(c == 'n') {
                out << "Abandon." << endl;
                exit(1);
            }
            else if(!QFile::remove(fichierClePubliqueRSA)) {
                out << "Impossible d'ecrire dans le fichier " << fichierClePubliqueRSA << ". Abandon." << endl;
                exit(1);
            }
        }

        if(!ComposantCryptographie::genererClesRSA(fichierClePriveeRSA, fichierClePubliqueRSA)) {
            out << "Impossible de creer les cles RSA. Abandon." << endl;
            exit(1);
        }
    }
    // Sinon on utilise les clés RSA existantes
    else {
        QFile fClePubliqueRSA(fichierClePubliqueRSA);
        QFile fClePriveeRSA(fichierClePriveeRSA);

        // Vérification de l'existence des clés
        if(!fClePubliqueRSA.exists()) {
            out << "Fichier de cle publique RSA introuvable. Abandon." << endl;
            exit(1);
        }
        if(!fClePriveeRSA.exists()) {
            out << "Fichier de cle privee RSA introuvable. Abandon." << endl;
            exit(1);
        }

        // Vérification de la possibilité de lecture des clés
        if(fClePriveeRSA.open(QFile::ReadOnly)) {
            if(!fClePubliqueRSA.open(QFile::ReadOnly)) {
                fClePriveeRSA.close();
                out << "Impossible d'ouvrir le fichier de cle publique RSA. Abandon." << endl;
                exit(1);
            }
        }
        else {
            out << "Impossible d'ouvrir le fichier de cle privee RSA. Abandon." << endl;
            exit(1);
        }
        fClePubliqueRSA.close();
        fClePriveeRSA.close();
    }

    // Création base de données à partir de la structure de base
    if(!QFile::copy("db_structure", fichierBaseDeDonnees)) {
        out << "Impossible d'ecrire dans la base de donnees " << fichierBaseDeDonnees << ". Abandon." << endl;
        exit(1);
    }
    composantBDD = new ComposantBaseDeDonnees(fichierBaseDeDonnees, fichierClePriveeRSA, fichierClePubliqueRSA);
    if(!composantBDD->ouvrirBaseDeDonnees()) {
        out << "Impossible d'ouvrir la base de donnees " << fichierBaseDeDonnees << ". Abandon." << endl;
        delete composantBDD;
        exit(1);
    }

    // Création utilisateur root aved unt mot de passe généré aléatoirement
    QString motDePasse = genererMotDePasse(10);
    if("creationok" != composantBDD->admin_gererUtilisateur("creation", -1, "Administrateur", "root", motDePasse, true)) {
        out << "Impossible de creer le compte root. Abandon." << endl;
        delete composantBDD;
        exit(1);
    }

    // Ecriture de la configuration
    QTextStream sortiefConfig(&fConfig);
    sortiefConfig << "fichierClePriveeRSA=" << fichierClePriveeRSA << endl;
    sortiefConfig << "fichierClePubliqueRSA=" << fichierClePubliqueRSA << endl;
    sortiefConfig << "fichierBaseDeDonnees=" << fichierBaseDeDonnees << endl;
    sortiefConfig << "portEcoute=" << QString::number(portEcoute) << flush;
    fConfig.close();

    // Affichage des informations du compte root
    out << "Le serveur a correctement ete installe. Le compte root a ete cree : " << endl;
    out << "Login : root" << endl << "Mot de passe : " << motDePasse << endl;
    out << "Par mesure de securite, vous pouvez changer le mot de passe du compte root." << endl;

    exit(0);
}

// Sauvegarde du serveur en ligne de commande
void ComposantPrincipal::sauvegarderServeur(int argc, char *argv[]) {
    QTextStream out(stdout);

    // Vérification du nombre de paramètres
    if(argc != 4 && argc != 6) {
        out << "Ligne de commande incorrecte." << endl;
        ComposantDocumentation::aide();
        exit(1);
    }

    int p = 2;

    QString fichierSauvegarde = "";

    // Analyse de chaque argument de la ligne de commande
    while(p < argc) {
        if(strcmp(argv[p], "--file") == 0 && p + 1 < argc) {
            fichierSauvegarde = argv[p+1];
            p += 2;
        }
        else if(strcmp(argv[p], "--config") == 0 && p + 1 < argc) {
            fichierConfiguration = argv[p+1];
            p += 2;
        }
        else p++;
    }

    // Vérification paramètres fournis
    if(fichierSauvegarde == "") {
        out << "Ligne de commande incorrecte." << endl;
        ComposantDocumentation::aide();
        exit(1);
    }

    // Vérification de l'existence du fichier de configuration
    QFile fichierConfig(fichierConfiguration);
    if(!fichierConfig.exists()) {
        out << "Impossible de sauvegarder le serveur : le fichier de configuration est introuvable." << endl;
        exit(1);
    }
    if(!fichierConfig.open(QIODevice::ReadOnly)) {
        out << "Impossible de sauvegarder le serveur : le fichier de configuration ne peut etre ouvert." << endl;
        exit(1);
    }

    // Vérification du fichier de sauvegarde
    if(QFile::exists(fichierSauvegarde)) {
        char c = 0;
        while(c != 'o' && c != 'n') {
            out << "Le fichier " << fichierSauvegarde << " existe, remplacer ? (o/n)" << endl;
            std::cin >> c;
        }
        if(c == 'n') {
            out << "Abandon." << endl;
            exit(1);
        }
        else if(!QFile::remove(fichierSauvegarde)) {
            out << "Impossible d'ecrire dans le fichier " << fichierSauvegarde << ". Abandon." << endl;
            exit(1);
        }
    }
    QFile fSauvegarde(fichierSauvegarde);
    if(!fSauvegarde.open(QFile::WriteOnly | QFile::Truncate)) {
        out << "Impossible d'ecrire dans le fichier " << fichierSauvegarde << ". Abandon." << endl;
        exit(1);
    }

    // Chargement de la configuration
    QString fichier(fichierConfig.readAll());
    fichierConfig.close();

    QStringList lignes = fichier.split("\n");
    QStringList ligne;
    for(int l = 0; l < lignes.size(); l++) {
        ligne = lignes.at(l).split("=");
        if(ligne.at(0) == "fichierClePubliqueRSA") fichierClePubliqueRSA = ligne.at(1).trimmed();
        else if(ligne.at(0) == "fichierClePriveeRSA") fichierClePriveeRSA = ligne.at(1).trimmed();
        else if(ligne.at(0) == "fichierBaseDeDonnees") fichierBaseDeDonnees = ligne.at(1).trimmed();
        else if(ligne.at(0) == "portEcoute") portEcoute = ligne.at(1).toInt();
    }

    // Création de la sauvegarde
    QByteArray sauvegarde = sauvegardeServeur();

    if(!sauvegarde.startsWith("sauvegardeok")) {
        out << "Impossible de sauvegarder le serveur : erreur technique. Veuillez réessayer." << endl;
        fSauvegarde.close();
        exit(1);
    }

    QString motDePasse = sauvegarde.mid(12, 16);

    // Ecriture de la sauvegarde
    fSauvegarde.write(sauvegarde.mid(28));
    fSauvegarde.close();

    // Affichage des informations de la sauvegarde
    out << "Le serveur a correctement ete sauvegarde dans le fichier " << fichierSauvegarde << endl;
    out << "Mot de passe de la sauvegarde : " << motDePasse << endl;

    exit(0);

}

// Restauration d'une sauvegarde en ligne de commande
void ComposantPrincipal::restaurerServeur(int argc, char *argv[]) {
    QTextStream out(stdout);

    // Vérification du nombre d'arguments
    if(argc != 6 && argc != 8) {
        out << "Ligne de commande incorrecte." << endl;
        ComposantDocumentation::aide();
        exit(1);
    }

    int p = 2;

    QString fichierSauvegarde = "";
    QString motDePasse = "";

    // Analyse de chaque argument de la ligne de commande
    while(p < argc) {
        if(strcmp(argv[p], "--file") == 0 && p + 1 < argc) {
            fichierSauvegarde = argv[p+1];
            p += 2;
        }
        else if(strcmp(argv[p], "--password") == 0 && p + 1 < argc) {
            motDePasse = argv[p+1];
            p += 2;
        }
        else if(strcmp(argv[p], "--config") == 0 && p + 1 < argc) {
            fichierConfiguration = argv[p+1];
            p += 2;
        }
        else p++;
    }

    // Vérification des paramètres fournis
    if(fichierSauvegarde == "" || motDePasse == "") {
        out << "Ligne de commande incorrecte." << endl;
        ComposantDocumentation::aide();
        exit(1);
    }

    // Initialisation du décryptage d'après le mot de passe
    ComposantCryptographie c;
    if(!c.setCleAES(motDePasse.toUtf8() + motDePasse.toUtf8())) {
        out << "Mot de passe non accepte. Veuillez reessayer." << endl;
        exit(1);
    }

    // On vérifie l'existence du dossier spmd
    QDir pathToConf(QDir::homePath() + QString("/spmd"));
    if(!pathToConf.exists()) {
        if(!pathToConf.mkpath(QDir::homePath() + QString("/spmd"))) {
            out << "Impossible de créer le dossier " << QDir::homePath() + QString("/spmd") << " . Abandon." << endl;
            exit(1);
        }
    }

    // Ouverture du fichier de sauvegarde
    QFile fSauvegarde(fichierSauvegarde);
    if(!fSauvegarde.open(QFile::ReadOnly)) {
        out << "Impossible de lire le fichier " << fichierSauvegarde << ". Abandon." << endl;
        exit(1);
    }

    // Lecture et décryptage de la sauvegarde
    QByteArray sauvegardeCryptee = fSauvegarde.readAll();
    fSauvegarde.close();
    QByteArray sauvegarde = c.decrypterAES(sauvegardeCryptee);

    if(sauvegarde.size() == 0) {
        out << "Impossible de decrypter la sauvegarde. Abandon." << endl;
        exit(1);
    }

    // Extraction de chaque fichier de la sauvegarde
    int position = 0;
    int taille = byteArrayToInt(sauvegarde.mid(position, 4));
    QByteArray contenuConfig = sauvegarde.mid(position+4, taille);

    position += taille + 4;
    taille = byteArrayToInt(sauvegarde.mid(position, 4));
    QByteArray contenuClePriveeRSA = sauvegarde.mid(position+4, taille);

    position += taille + 4;
    taille = byteArrayToInt(sauvegarde.mid(position, 4));
    QByteArray contenuClePubliqueRSA = sauvegarde.mid(position+4, taille);

    position += taille + 4;
    taille = byteArrayToInt(sauvegarde.mid(position, 4));
    QByteArray contenuBaseDeDonnees = sauvegarde.mid(position+4, taille);

    // Lecture de la configuration
    QStringList lignes = QString(contenuConfig).split("\n");
    QStringList ligne;
    for(int l = 0; l < lignes.size(); l++) {
        ligne = lignes.at(l).split("=");
        if(ligne.at(0) == "fichierClePubliqueRSA") fichierClePubliqueRSA = ligne.at(1).trimmed();
        else if(ligne.at(0) == "fichierClePriveeRSA") fichierClePriveeRSA = ligne.at(1).trimmed();
        else if(ligne.at(0) == "fichierBaseDeDonnees") fichierBaseDeDonnees = ligne.at(1).trimmed();
    }

    // Ouverture fichier configuration
    if(QFile::exists(fichierConfiguration)) {
        char c = 0;
        while(c != 'o' && c != 'n') {
            out << "Le fichier " << fichierConfiguration << " existe, remplacer ? (o/n)" << endl;
            std::cin >> c;
        }
        if(c == 'n') {
            out << "Abandon." << endl;
            exit(1);
        }
    }
    QFile fConfig(fichierConfiguration);
    if(!fConfig.open(QFile::WriteOnly | QFile::Truncate)) {
        out << "Impossible d'ecrire dans le fichier " << fichierConfiguration << ". Abandon." << endl;
        exit(1);
    }

    // Ouverture fichier clé privée RSA
    if(QFile::exists(fichierClePriveeRSA)) {
        char c = 0;
        while(c != 'o' && c != 'n') {
            out << "Le fichier " << fichierClePriveeRSA << " existe, remplacer ? (o/n)" << endl;
            std::cin >> c;
        }
        if(c == 'n') {
            out << "Abandon." << endl;
            exit(1);
        }
    }
    QFile fClePriveeRSA(fichierClePriveeRSA);
    if(!fClePriveeRSA.open(QFile::WriteOnly | QFile::Truncate)) {
        out << "Impossible d'ecrire dans le fichier " << fichierClePriveeRSA << ". Abandon." << endl;
        exit(1);
    }

    // Ouverture fichier clé publique RSA
    if(QFile::exists(fichierClePubliqueRSA)) {
        char c = 0;
        while(c != 'o' && c != 'n') {
            out << "Le fichier " << fichierClePubliqueRSA << " existe, remplacer ? (o/n)" << endl;
            std::cin >> c;
        }
        if(c == 'n') {
            out << "Abandon." << endl;
            exit(1);
        }
    }
    QFile fClePubliqueRSA(fichierClePubliqueRSA);
    if(!fClePubliqueRSA.open(QFile::WriteOnly | QFile::Truncate)) {
        out << "Impossible d'ecrire dans le fichier " << fichierClePubliqueRSA << ". Abandon." << endl;
        exit(1);
    }

    // Ouverture fichier base de données
    if(QFile::exists(fichierBaseDeDonnees)) {
        char c = 0;
        while(c != 'o' && c != 'n') {
            out << "Le fichier " << fichierBaseDeDonnees << " existe, remplacer ? (o/n)" << endl;
            std::cin >> c;
        }
        if(c == 'n') {
            out << "Abandon." << endl;
            exit(1);
        }
    }
    QFile fBaseDeDonnees(fichierBaseDeDonnees);
    if(!fBaseDeDonnees.open(QFile::WriteOnly | QFile::Truncate)) {
        out << "Impossible d'ecrire dans le fichier " << fichierBaseDeDonnees << ". Abandon." << endl;
        exit(1);
    }

    // Ecriture puis fermeture des fichiers
    fConfig.write(contenuConfig);
    fClePriveeRSA.write(contenuClePriveeRSA);
    fClePubliqueRSA.write(contenuClePubliqueRSA);
    fBaseDeDonnees.write(contenuBaseDeDonnees);
    fConfig.close();
    fClePriveeRSA.close();
    fClePubliqueRSA.close();
    fBaseDeDonnees.close();

    out << "Le serveur a correctement ete restaure." << endl;

    exit(0);
}
