#include "ComposantPrincipal.h"
#include "ComposantReseau.h"
#include "ComposantSauvegarde.h"
#include "GestionGroupes.h"
#include "GestionMotsDePasse.h"
#include "GestionServeurs.h"
#include "GestionUtilisateurs.h"
#include "../../Serveur/src/ComposantCryptographie.h"

#ifdef Q_OS_WIN
#include "ui_ComposantPrincipal_win.h"
#else
#include "ui_ComposantPrincipal_unix.h"
#endif

#include <QMessageBox>
#include <QDir>

ComposantPrincipal::ComposantPrincipal(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ComposantPrincipal),
    compRes(NULL), compSave(NULL), gestGroupe(NULL),
    gestMDP(NULL), gestServ(NULL), gestUtil(NULL)

{

    // Initialisation des variables membre
    ui->setupUi(this);
    reinitialiser();

    // Lecture du fichier de configuration
    fichierConfig.setFileName(QDir::homePath() + QString("/spm/spm.conf"));

    if(fichierConfig.exists()) {
        if (!fichierConfig.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "Erreur fichier", QString("Impossible d'ouvrir le fichier de configuration :\n\n")
                                                          + fichierConfig.fileName());
            exit(1);
        }
        if(fichierConfig.size() != 0) {
            QTextStream config(&fichierConfig);
            QString line;
            while (!config.atEnd()) {
                line = config.readLine();
                QStringList list = line.split("=");
                if(list[0] == "serveur") ipServ = list.at(1).trimmed();
                else if(list[0] == "port") portServ = list.at(1).toInt();
                else if(list[0] == "cleServeur") cleServ = list.at(1).trimmed();
            }

            // Affichage de la page de connexion
            ui->stackedWidget->setCurrentIndex(0);
            return;
        }
        else fichierConfig.close();
    }
    // Aucun fichier de configuration : installation du client
    ui->stackedWidget->setCurrentIndex(2);

}

// Réinitialisation des variables membre et interfaces
void ComposantPrincipal::reinitialiser() {
    idUtilisateur = -1;
    nomUtilisateur = "";
    estAdministrateur = false;
    listeMotsDePasse.clear();
    ui->listeMotsDePasse->clear();
    ui->stackedWidget->setCurrentIndex(0);
    ui->menuAdministration->menuAction()->setVisible(false);
    ui->actionDeconnexion->setEnabled(false);
    ui->connexionPageConnexion->setEnabled(true);
    ui->connexionPageConnexion->setText("Connecter");
    ui->identifiantPageConnexion->clear();
    ui->mdpPageConnexion->clear();
}

// Destruction des composants
ComposantPrincipal::~ComposantPrincipal()
{
    delete ui;
    if(compRes != NULL) delete compRes;
    if(compSave != NULL) delete compSave;
    if(gestGroupe != NULL) delete gestGroupe;
    if(gestMDP != NULL) delete gestMDP;
    if(gestServ != NULL) delete gestServ;
    if(gestUtil != NULL) delete gestUtil;
}

// Ecriture de la configuration client
void ComposantPrincipal::on_enregistrerPageConfiguration_clicked()
{
    // L'IP et le port sont récuperés
    ipServ = ui->serveurPageConfiguration->text();
    portServ = ui->portPageConfiguration->value();

    // On vérifie l'existence du dossier spm
    QDir pathToConf(QDir::homePath() + QString("/spm"));
    if(!pathToConf.exists()) {
        if(!pathToConf.mkpath(QDir::homePath() + QString("/spm"))) {
            QMessageBox::critical(this, "Erreur système de fichiers", QString("Impossible de créer le dossier spm :\n\n")
                                                                      + QDir::homePath() + QString("/spm"));
            return;
        }
    }

    // On ouvre le fichier
    if (!fichierConfig.open(QFile::WriteOnly | QFile::Truncate)) {
        QMessageBox::critical(this, "Erreur fichier", QString("Impossible d'ouvrir le fichier de configuration :\n\n")
                                                      + fichierConfig.fileName());
        return;
    }
    QTextStream sortie(&fichierConfig);

    // Téléchargement de la clé publique du serveur
    compRes = new ComposantReseau(ipServ,portServ,NULL);
    QByteArray cle = compRes->getClePubliqueRSA();
    if(cle.size() == 0) {
        fichierConfig.close();
        delete compRes;
        compRes = NULL;
        QMessageBox::critical(this, "Erreur réseau", "Impossible de télécharger la clé publique RSA.");
        return;
    }

    // Enregistrement de la clé publique du serveur
    cleServ = QDir::homePath() + QString("/spm/server-pubkey.pem");
    QFile fichierCleRSA(cleServ);
    if(!fichierCleRSA.open(QFile::WriteOnly | QFile::Truncate)) {
        fichierConfig.close();
        delete compRes;
        compRes = NULL;
        QMessageBox::critical(this, "Erreur réseau", QString("Impossible d'écrire dans le fichier ") + cleServ);
        return;
    }
    fichierCleRSA.write(cle);
    fichierCleRSA.close();

    // On écrit la configuration
    sortie << "serveur=" << ipServ << endl;
    sortie << "port=" << portServ << endl;
    sortie << "cleServeur=" << cleServ << flush;
    fichierConfig.close();

    // On affiche la page de connexion
    ui->stackedWidget->setCurrentIndex(0);
}

// Connexion de l'utilisateur
void ComposantPrincipal::on_connexionPageConnexion_clicked()
{

    // Création des composants de cryptographie et réseau
    compCrypt = new ComposantCryptographie();
    compCrypt->loadClePubliqueRSA(cleServ);
    compRes = new ComposantReseau(ipServ,portServ,compCrypt);

    // Connexion au serveur
    ui->connexionPageConnexion->setEnabled(false);
    ui->connexionPageConnexion->setText("Connexion en cours");
    if(!compRes->connecter()) {
        QMessageBox::critical(this, "Erreur réseau", "Le serveur ne répond pas.\nVérifiez votre connexion réseau.", QMessageBox::Ok);
        ui->connexionPageConnexion->setEnabled(true);
        ui->connexionPageConnexion->setText("Connecter");
        delete compRes;
        compRes = NULL;
        delete compCrypt;
        compCrypt = NULL;
        return;
    }

    // Création message de connexion
    QString user = ui->identifiantPageConnexion->text();
    QString pass = ui->mdpPageConnexion->text();
    QByteArray message = "connexion";
    message.append(31);
    message.append(user);
    message.append(31);
    message.append(pass);
    QByteArray reponse = compRes->envoyerMessage(message);

    QList<QByteArray> champs = reponse.split(31);

    // Vérification de la réponse du serveur
    if(champs.at(0) == "connexionacceptee") {
        if(champs.at(1) == "admin") {
            ui->menuAdministration->menuAction()->setVisible(true);
            gestUtil = new GestionUtilisateurs(compRes, this);
            gestServ = new GestionServeurs(compRes, this);
            gestMDP = new GestionMotsDePasse(compRes, this);
            gestGroupe = new GestionGroupes(compRes, this);
            compSave = new ComposantSauvegarde(compRes, this);
            estAdministrateur = true;
        }
        idUtilisateur = champs.at(2).toInt();
        nomUtilisateur = champs.at(3);
        ui->identifiantPageConnexion->setFocus();
        ui->actionDeconnexion->setEnabled(true);
        ui->stackedWidget->setCurrentIndex(1);
        ui->mdpPageConnexion->clear();
        rafraichirListeMotsDePasse();
    }
    else if(champs.at(0) == "connexionrefusee") {
        QMessageBox::critical(this, "Connexion refusée", "Connexion refusée.\nVérifiez vos informations de connexion.", QMessageBox::Ok);
        delete compRes;
        delete compCrypt;
    }
    else {
        QMessageBox::critical(this, "Erreur inconnue", reponse, QMessageBox::Ok);
        delete compRes;
        delete compCrypt;
    }
    ui->connexionPageConnexion->setEnabled(true);
    ui->connexionPageConnexion->setText("Connecter");
}

// Récupération de la liste des mots de passe accessibles à l'utilisateur
void ComposantPrincipal::rafraichirListeMotsDePasse() {

    // Envoi de la requête et réception de la réponse
    QByteArray message = "listemotsdepasse";
    QByteArray reponse = compRes->envoyerMessage(message);

    QList<QByteArray> lignes = reponse.split(30);

    listeMotsDePasse.clear();
    ui->listeMotsDePasse->clear();

    // Affichage des résultats
    if(lignes.at(0) == "listemotsdepasse" && lignes.at(1).size() > 0) {
        QList<QByteArray> champs;
        MotDePasse motDePasse;

        for(int m = 1; m < lignes.size(); m++) {
            champs = lignes.at(m).split(31);
            motDePasse.nomServeur = champs.at(0).trimmed();
            motDePasse.hoteServeur = champs.at(1).trimmed();
            motDePasse.nom = champs.at(2).trimmed();
            motDePasse.identifiant = champs.at(3).trimmed();
            motDePasse.motDePasse = champs.at(4).trimmed();
            listeMotsDePasse.append(motDePasse);
            ui->listeMotsDePasse->addItem(motDePasse.nomServeur + " : " +
                                          motDePasse.nom + " @ " + motDePasse.hoteServeur);
        }
    }

    // Initialisations des zones d'affichage
    ui->labelNomServeur->clear();
    ui->labelHoteServeur->clear();
    ui->labelNom->clear();
    ui->labelIdentifiant->clear();
    ui->motDePasse->clear();
    ui->motDePasse->setEchoMode(QLineEdit::Password);
    ui->afficherMasquer->setText("Afficher");
    ui->afficherMasquer->setEnabled(false);

}

// Sélection d'un mot de passe : affichage des informations
void ComposantPrincipal::on_listeMotsDePasse_clicked(const QModelIndex &index)
{
    int i = index.row();
    if(i == -1 || i >= listeMotsDePasse.size()) return;

    MotDePasse motDePasse = listeMotsDePasse.at(i);

    // Affichage des informations du mot de passe
    ui->labelNomServeur->setText(motDePasse.nomServeur);
    ui->labelHoteServeur->setText(motDePasse.hoteServeur);
    ui->labelNom->setText(motDePasse.nom);
    ui->labelIdentifiant->setText(motDePasse.identifiant);
    ui->motDePasse->setText(motDePasse.motDePasse);

    // Le mot de passe est masqué
    ui->motDePasse->setEchoMode(QLineEdit::Password);
    ui->afficherMasquer->setText("Afficher");
    ui->afficherMasquer->setEnabled(true);
}

// Action du menu Fichier - Déconnexion
void ComposantPrincipal::on_actionDeconnexion_triggered()
{
    if(idUtilisateur != -1) {
        // Le composant réseau envoie le message de déconnexion lors de sa destruction
        if(estAdministrateur) {
            delete gestUtil;
            gestUtil = NULL;
            delete gestGroupe;
            gestGroupe = NULL;
            delete gestMDP;
            gestMDP = NULL;
            delete gestServ;
            gestServ = NULL;
            delete compSave;
            compSave = NULL;
        }
        if(compRes != NULL) {
            delete compRes;
            compRes = NULL;
        }
        if(compCrypt != NULL) {
            delete compCrypt;
            compCrypt = NULL;
        }
        reinitialiser();
    }

}

// Action du menu Fichier - Quitter
void ComposantPrincipal::on_actionQuitter_triggered()
{
    on_actionDeconnexion_triggered();
    this->close();
}

// Action du menu Administration - Gestion des utilisateurs
void ComposantPrincipal::on_actionGestionUtilisateurs_triggered()
{
    // Actualisation de la liste si nécessaire
    if(!gestUtil->isVisible()) gestUtil->rafraichirListeUtilisateurs();
    gestUtil->show();
}

// Action du menu Administration - Gestion des serveurs
void ComposantPrincipal::on_actionGestionServeurs_triggered()
{
    // Actualisation de la liste si nécessaire
    if(!gestServ->isVisible()) gestServ->rafraichirListeServeurs();
    gestServ->show();
}

// Action du menu Administration - Gestion des mots de passe
void ComposantPrincipal::on_actionGestionMDP_triggered()
{
    // Actualisation de la liste si nécessaire
    if(!gestMDP->isVisible()) gestMDP->rafraichirListeMotsDePasse();
    gestMDP->show();
}

// Action du menu Administration - Gestion des groupes
void ComposantPrincipal::on_actionGestionGroupes_triggered()
{
    // Actualisation de la liste si nécessaire
    if(!gestGroupe->isVisible()) gestGroupe->rafraichirListeGroupes();
    gestGroupe->show();
}

// Action du menu Administration - Sauvegarde du serveur
void ComposantPrincipal::on_actionSauvegarder_triggered()
{
    compSave->show();
}

// Action du bouton Actualiser pour la liste des mots de passe
void ComposantPrincipal::on_actualiser_clicked()
{
    rafraichirListeMotsDePasse();
}

// Action du bouton Afficher/Masquer pour le mot de passe sélectionné
void ComposantPrincipal::on_afficherMasquer_clicked()
{
    if(ui->listeMotsDePasse->currentRow() == -1) return;

    // Si le mot de passe est masqué on l'affiche, et inversement
    if(ui->motDePasse->echoMode() == QLineEdit::Password) {
        ui->motDePasse->setEchoMode(QLineEdit::Normal);
        ui->afficherMasquer->setText("Masquer");
    }
    else {
        ui->motDePasse->setEchoMode(QLineEdit::Password);
        ui->afficherMasquer->setText("Afficher");
    }
}

// La touche Entrée fait passer au champ mot de passe si on est dans le champ identifiant
void ComposantPrincipal::on_identifiantPageConnexion_returnPressed()
{
    ui->mdpPageConnexion->setFocus();
}

// La touche Entrée lance la connexion si on est dans le champ mot de passe
void ComposantPrincipal::on_mdpPageConnexion_returnPressed()
{
    on_connexionPageConnexion_clicked();
}
