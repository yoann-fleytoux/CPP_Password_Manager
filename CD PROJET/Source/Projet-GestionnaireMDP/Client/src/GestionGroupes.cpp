#include "GestionGroupes.h"
#include "ComposantReseau.h"

#ifdef Q_OS_WIN
#include "ui_GestionGroupes_win.h"
#else
#include "ui_GestionGroupes_unix.h"
#endif

#include <QMessageBox>
#include <QCloseEvent>
#include <QKeyEvent>

GestionGroupes::GestionGroupes(ComposantReseau *p_compRes, QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint|Qt::WindowCloseButtonHint|Qt::WindowMinimizeButtonHint),
    ui(new Ui::GestionGroupes),
    compRes(p_compRes)
{
    ui->setupUi(this);
}

GestionGroupes::~GestionGroupes()
{
    delete ui;
}

// Récupère la liste des groupes du système
void GestionGroupes::rafraichirListeGroupes() {

    // Envoi de la requête et réception de la réponse
    QByteArray message = "admin_listegroupes";
    QByteArray reponse = compRes->envoyerMessage(message);

    QList<QByteArray> lignes = reponse.split(30);

    listeGroupes.clear();
    ui->listeGroupes->clear();

    // Affichage des résultats
    if(lignes.at(0) == "admin_listegroupes" && lignes.at(1).size() > 0) {
        QList<QByteArray> champs;

        for(int m = 1; m < lignes.size(); m++) {
            champs = lignes.at(m).split(31);
            groupe.id = champs.at(0).trimmed().toInt();
            groupe.nom = champs.at(1).trimmed();
            listeGroupes.append(groupe);
            ui->listeGroupes->addItem(groupe.nom);
        }
    }

    ui->listeGroupes->setCurrentRow(-1);

}

// Entre en mode création d'un groupe
void GestionGroupes::on_creer_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->titreCreerModifier->setText("Créer un groupe");

    groupe.id = -1;
    ui->nom->clear();
}

// Entre en mode modification d'un groupe
void GestionGroupes::on_modifier_clicked()
{
    int i = ui->listeGroupes->currentRow();
    if(i < 0 || i >= listeGroupes.size()) return;

    ui->stackedWidget->setCurrentIndex(1);
    ui->titreCreerModifier->setText("Modifier un groupe");

    groupe = listeGroupes.at(i);
    ui->nom->setText(groupe.nom);
}

// Suppression d'un groupe
void GestionGroupes::on_supprimer_clicked()
{
    int i = ui->listeGroupes->currentRow();
    if(i < 0 || i >= listeGroupes.size()) return;

    // Message de confirmation
    QMessageBox::StandardButton retour;
    retour = QMessageBox::question(this, "Confirmation", "La suppression du groupe sera définitive. Confirmer ?");

    if(retour != QMessageBox::Yes) return;

    // Envoi de la requête et réception de la réponse
    QByteArray message = "admin_gerergroupe";
    message.append(30);
    message.append("suppression");
    message.append(30);
    message.append(QString::number(listeGroupes.at(i).id));

    QByteArray reponse = compRes->envoyerMessage(message);

    // Affichage de l'état de la requête
    if(reponse == "suppressionok") {
        QMessageBox::information(this, "Suppression effectuée", "Le groupe a été supprimé");
        rafraichirListeGroupes();
    }

    else if(reponse == "suppressionerreurinexistant") {
        QMessageBox::critical(this, "Suppression échouée", "Le groupe n'existe pas (id_groupe introuvable).");
        rafraichirListeGroupes();
    }

    else if(reponse.startsWith("suppressionerreurtechnique")) {
        QMessageBox::critical(this, "Suppression échouée", "Une erreur technique a eu lieu :\n\n" + reponse.split(30).at(1));
    }
    else {
        QMessageBox::critical(this, "Erreur serveur", "Pas de réponse ou réponse malformée du serveur. Veuillez réessayer.");
    }
}

// Entre en mode gestion des utilisateurs du groupe
void GestionGroupes::on_gererUtilisateurs_clicked()
{
    int i = ui->listeGroupes->currentRow();
    if(i < 0 || i >= listeGroupes.size()) return;

    listeUtilisateursGroupe.clear();
    ui->utilisateursGroupe->clear();
    listeUtilisateurs.clear();
    ui->utilisateurs->clear();

    groupe = listeGroupes.at(i);

    ui->labelGroupe1->setText(groupe.nom);

    QByteArray message;
    QList<QByteArray> lignes;
    QList<QByteArray> champs;
    Utilisateur utilisateur;

    // Récupération des utilisateurs actuels du groupe
    QByteArray utilisateursGroupe;
    message = "admin_listeutilisateursgroupe";
    message.append(30);
    message.append(QString::number(groupe.id));
    utilisateursGroupe = compRes->envoyerMessage(message);

    lignes = utilisateursGroupe.split(30);

    // Affichage des résultats
    if(lignes.at(0) == "admin_listeutilisateursgroupe" && lignes.at(1).size() > 0) {
        for(int m = 1; m < lignes.size(); m++) {
            champs = lignes.at(m).split(31);
            utilisateur.id = champs.at(0).trimmed().toInt();
            utilisateur.nom = champs.at(1).trimmed();
            utilisateur.identifiant = champs.at(2).trimmed();
            listeUtilisateursGroupe.append(utilisateur);
            ui->utilisateursGroupe->addItem(utilisateur.nom + " (" + utilisateur.identifiant + ")");
        }
    }

    lignes.clear();
    champs.clear();

    // Récupération des utilisateurs du système
    QByteArray utilisateurs;
    utilisateurs = compRes->envoyerMessage("admin_listeutilisateurs");
    bool trouve;

    lignes = utilisateurs.split(30);

    // Affichage des résultats
    if(lignes.at(0) == "admin_listeutilisateurs" && lignes.at(1).size() > 0) {
        listeUtilisateurs.clear();
        ui->utilisateurs->clear();
        for(int m = 1; m < lignes.size(); m++) {
            champs = lignes.at(m).split(31);
            utilisateur.id = champs.at(0).trimmed().toInt();
            utilisateur.nom = champs.at(1).trimmed();
            utilisateur.identifiant = champs.at(2).trimmed();

            // On exclue les utilisateurs qui sont dans les utilisateurs du groupe
            for(int g = 0; g < listeUtilisateursGroupe.size(); g++) {
                if(listeUtilisateursGroupe.at(g).id == utilisateur.id) {
                    trouve = true;
                    break;
                }
            }
            if(!trouve) {
                listeUtilisateurs.append(utilisateur);
                ui->utilisateurs->addItem(utilisateur.nom + " (" + utilisateur.identifiant + ")");
            }
            trouve = false;
        }
    }

    ui->stackedWidget->setCurrentIndex(2);
}

// Entre en mode gestion des mots de passe du groupe
void GestionGroupes::on_gererMotsDePasse_clicked()
{
    int i = ui->listeGroupes->currentRow();
    if(i < 0 || i >= listeGroupes.size()) return;

    listeMotsDePasseGroupe.clear();
    ui->motsDePasseGroupe->clear();
    listeMotsDePasse.clear();
    ui->motsDePasse->clear();

    groupe = listeGroupes.at(i);

    ui->labelGroupe2->setText(groupe.nom);

    QByteArray message;
    QList<QByteArray> lignes;
    QList<QByteArray> champs;
    MotDePasse motDePasse;

    // Récupération des mots de passe actuels du groupe
    QByteArray motsDePasseGroupe;

    message = "admin_listemotsdepassegroupe";
    message.append(30);
    message.append(QString::number(groupe.id));
    motsDePasseGroupe = compRes->envoyerMessage(message);

    lignes = motsDePasseGroupe.split(30);

    // Affichage des résultats
    if(lignes.at(0) == "admin_listemotsdepassegroupe" && lignes.at(1).size() > 0) {
        for(int m = 1; m < lignes.size(); m++) {
            champs = lignes.at(m).split(31);
            motDePasse.id = champs.at(0).trimmed().toInt();
            motDePasse.nom = champs.at(1).trimmed();
            motDePasse.identifiant = champs.at(2).trimmed();
            motDePasse.nomServeur = champs.at(3).trimmed();
            motDePasse.hoteServeur = champs.at(4).trimmed();
            listeMotsDePasseGroupe.append(motDePasse);
            ui->motsDePasseGroupe->addItem(motDePasse.nomServeur + " : " +
                                            motDePasse.nom + " @ " + motDePasse.hoteServeur);
        }
    }

    lignes.clear();
    champs.clear();

    // Récupération des mots de passe du système
    QByteArray motsDePasse;
    motsDePasse = compRes->envoyerMessage("admin_listemotsdepasse");
    bool trouve;

    lignes = motsDePasse.split(30);

    // Affichage des résultats
    if(lignes.at(0) == "admin_listemotsdepasse" && lignes.at(1).size() > 0) {
        for(int m = 1; m < lignes.size(); m++) {
            champs = lignes.at(m).split(31);
            motDePasse.id = champs.at(0).trimmed().toInt();
            motDePasse.nom = champs.at(1).trimmed();
            motDePasse.identifiant = champs.at(2).trimmed();
            motDePasse.nomServeur = champs.at(5).trimmed();
            motDePasse.hoteServeur = champs.at(6).trimmed();

            // On exclue les mots de passe qui sont dans les mots de passe du groupe
            for(int g = 0; g < listeMotsDePasseGroupe.size(); g++) {
                if(listeMotsDePasseGroupe.at(g).id == motDePasse.id) {
                    trouve = true;
                    break;
                }
            }
            if(!trouve) {
                listeMotsDePasse.append(motDePasse);
                ui->motsDePasse->addItem(motDePasse.nomServeur + " : " +
                                         motDePasse.nom + " @ " + motDePasse.hoteServeur);
            }
            trouve = false;
        }
    }

    ui->stackedWidget->setCurrentIndex(3);
}

// Validation de la création / modification du groupe
void GestionGroupes::on_validerGroupe_clicked()
{
    QString mode = groupe.id == -1 ? "creation" : "modification";

    // Envoi de la requête et réception de la réponse
    QByteArray message = "admin_gerergroupe";
    message.append(30);
    message.append(mode);
    message.append(30);
    message.append(QString::number(groupe.id));
    message.append(30);
    message.append(ui->nom->text());

    QByteArray reponse = compRes->envoyerMessage(message);

    // Affichage de l'état de la requête
    if(mode == "creation") {
        if(reponse == "creationok") {
            ui->stackedWidget->setCurrentIndex(0);
            QMessageBox::information(this, "Création réussie", "Le groupe a bien été créé");
            rafraichirListeGroupes();
        }
        else if(reponse.startsWith("creationerreurtechnique")) {
            QMessageBox::critical(this, "Création échouée", "Une erreur technique a eu lieu :\n\n" + reponse.split(30).at(1));
        }
        else {
            QMessageBox::critical(this, "Erreur serveur", "Pas de réponse ou réponse malformée du serveur. Veuillez réessayer.");
        }
    }
    else if(mode == "modification") {
        if(reponse == "modificationok") {
            ui->stackedWidget->setCurrentIndex(0);
            QMessageBox::information(this, "Modification réussie", "Le groupe a bien été modifié");
            rafraichirListeGroupes();
        }
        else if(reponse == "modificationerreurinexistant") {
            ui->stackedWidget->setCurrentIndex(0);
            QMessageBox::critical(this, "Modification échouée", "Le groupe n'existe pas (id_groupe introuvable).");
            rafraichirListeGroupes();
        }
        else if(reponse.startsWith("modificationerreurtechnique")) {
            QMessageBox::critical(this, "Modification échouée", "Une erreur technique a eu lieu :\n\n" + reponse.split(30).at(1));
        }
        else {
            QMessageBox::critical(this, "Erreur serveur", "Pas de réponse ou réponse malformée du serveur. Veuillez réessayer.");
        }
    }
}

// Annulation de la création / modification du groupe
void GestionGroupes::on_annulerGroupe_clicked()
{
    QMessageBox::StandardButton retour;
    retour = QMessageBox::question(this, "Confirmation", "Les modifications seront abandonnées. Confirmer ?");

    if(retour != QMessageBox::Yes) return;

    ui->stackedWidget->setCurrentIndex(0);
}

// Ajout d'un utilisateur du système aux utilisateurs du groupe
void GestionGroupes::on_ajouterUtilisateur_clicked()
{
    int i = ui->utilisateurs->currentRow();
    if(i < 0 || i >= listeUtilisateurs.size()) return;

    Utilisateur utilisateur = listeUtilisateurs.at(i);
    listeUtilisateurs.remove(i);
    delete ui->utilisateurs->takeItem(i);
    listeUtilisateursGroupe.append(utilisateur);
    ui->utilisateursGroupe->addItem(utilisateur.nom + " (" + utilisateur.identifiant + ")");
}

// Retrait d'un utilisateur des utilisateurs du groupe
void GestionGroupes::on_enleverUtilisateur_clicked()
{
    int i = ui->utilisateursGroupe->currentRow();
    if(i < 0 || i >= listeUtilisateursGroupe.size()) return;

    Utilisateur utilisateur = listeUtilisateursGroupe.at(i);
    listeUtilisateursGroupe.remove(i);
    delete ui->utilisateursGroupe->takeItem(i);
    listeUtilisateurs.append(utilisateur);
    ui->utilisateurs->addItem(utilisateur.nom + " (" + utilisateur.identifiant + ")");
}

// Validation de la liste des utilisateurs du groupe
void GestionGroupes::on_validerUtilisateurs_clicked()
{

    // Envoi de la requête et réception de la réponse
    QByteArray message = "admin_gerergroupe";
    message.append(30);
    message.append("utilisateurs");
    message.append(30);
    message.append(QString::number(groupe.id));
    message.append(30);

    for(int g = 0; g < listeUtilisateursGroupe.size(); g++) {
        message.append(QString::number(listeUtilisateursGroupe.at(g).id));
        if(g != listeUtilisateursGroupe.size() - 1) message.append(31);
    }

    QByteArray reponse = compRes->envoyerMessage(message);

    // Affichage de l'état de la requête
    if(reponse == "utilisateursgroupeok") {
        ui->stackedWidget->setCurrentIndex(0);
        QMessageBox::information(this, "Modification réussie", "Les utilisateurs du groupe ont été mis à jour.");
        rafraichirListeGroupes();
    }
    else if(reponse == "utilisateursgroupeerreurinexistant") {
        ui->stackedWidget->setCurrentIndex(0);
        QMessageBox::critical(this, "Modification échouée", "Le groupe n'existe pas (id_groupe introuvable).");
        rafraichirListeGroupes();
    }
    else if(reponse.startsWith("utilisateursgroupeerreurtechnique")) {
        QMessageBox::critical(this, "Modification échouée", "Une erreur technique a eu lieu :\n\n" + reponse.split(30).at(1));
    }
    else {
        QMessageBox::critical(this, "Erreur serveur", "Pas de réponse ou réponse malformée du serveur. Veuillez réessayer.");
    }
}

// Annulation de la modification de la liste des utilisateurs du groupe
void GestionGroupes::on_annulerUtilisateurs_clicked()
{
    QMessageBox::StandardButton retour;
    retour = QMessageBox::question(this, "Confirmation", "Les modifications seront abandonnées. Confirmer ?");

    if(retour != QMessageBox::Yes) return;

    ui->stackedWidget->setCurrentIndex(0);
}

// Ajout d'un mot de passe du système aux mots de passe du groupe
void GestionGroupes::on_ajouterMotDePasse_clicked()
{
    int i = ui->motsDePasse->currentRow();
    if(i < 0 || i >= listeMotsDePasse.size()) return;

    MotDePasse motDePasse = listeMotsDePasse.at(i);
    listeMotsDePasse.remove(i);
    delete ui->motsDePasse->takeItem(i);
    listeMotsDePasseGroupe.append(motDePasse);
    ui->motsDePasseGroupe->addItem(motDePasse.nomServeur + " : " +
                                   motDePasse.nom + " @ " + motDePasse.hoteServeur);
}

// Ajout d'un mot de passe des mots de passe du groupe
void GestionGroupes::on_enleverMotDePasse_clicked()
{
    int i = ui->motsDePasseGroupe->currentRow();
    if(i < 0 || i >= listeMotsDePasseGroupe.size()) return;

    MotDePasse motDePasse = listeMotsDePasseGroupe.at(i);
    listeMotsDePasseGroupe.remove(i);
    delete ui->motsDePasseGroupe->takeItem(i);
    listeMotsDePasse.append(motDePasse);
    ui->motsDePasse->addItem(motDePasse.nomServeur + " : " +
                             motDePasse.nom + " @ " + motDePasse.hoteServeur);
}

// Validation de la liste des mots de passe du groupe
void GestionGroupes::on_validerMotsDePasse_clicked()
{

    // Envoi de la requête et réception de la réponse
    QByteArray message = "admin_gerergroupe";
    message.append(30);
    message.append("motsdepasse");
    message.append(30);
    message.append(QString::number(groupe.id));
    message.append(30);

    for(int g = 0; g < listeMotsDePasseGroupe.size(); g++) {
        message.append(QString::number(listeMotsDePasseGroupe.at(g).id));
        if(g != listeMotsDePasseGroupe.size() - 1) message.append(31);
    }

    QByteArray reponse = compRes->envoyerMessage(message);

    // Affichage de l'état de la requête
    if(reponse == "motsdepassegroupeok") {
        ui->stackedWidget->setCurrentIndex(0);
        QMessageBox::information(this, "Modification réussie", "Les mots de passe du groupe ont été mis à jour.");
        rafraichirListeGroupes();
    }
    else if(reponse == "motsdepassegroupeerreurinexistant") {
        ui->stackedWidget->setCurrentIndex(0);
        QMessageBox::critical(this, "Modification échouée", "Le groupe n'existe pas (id_groupe introuvable).");
        rafraichirListeGroupes();
    }
    else if(reponse.startsWith("motsdepassegroupeerreurtechnique")) {
        QMessageBox::critical(this, "Modification échouée", "Une erreur technique a eu lieu :\n\n" + reponse.split(30).at(1));
    }
    else {
        QMessageBox::critical(this, "Erreur serveur", "Pas de réponse ou réponse malformée du serveur. Veuillez réessayer.");
    }
}

// Annulation de la modification de la liste des mots de passe du groupe
void GestionGroupes::on_annulerMotsDePasse_clicked()
{
    QMessageBox::StandardButton retour;
    retour = QMessageBox::question(this, "Confirmation", "Les modifications seront abandonnées. Confirmer ?");

    if(retour != QMessageBox::Yes) return;

    ui->stackedWidget->setCurrentIndex(0);
}

// Activation des boutons de modification lors de la sélection d'un groupe
void GestionGroupes::on_listeGroupes_currentRowChanged(int currentRow)
{
    bool actif = (currentRow != -1);
    ui->modifier->setEnabled(actif);
    ui->supprimer->setEnabled(actif);
    ui->gererUtilisateurs->setEnabled(actif);
    ui->gererMotsDePasse->setEnabled(actif);
}

// Simulation du bouton modifier lors du double-clic sur un groupe
void GestionGroupes::on_listeGroupes_doubleClicked(const QModelIndex &index)
{
    on_modifier_clicked();
}

// Actualisation de la liste des groupes
void GestionGroupes::on_actualiser_clicked()
{
    rafraichirListeGroupes();
}

// Réimplémenté : exécuté lors de la fermeture de la fenêtre (croix)
void GestionGroupes::closeEvent(QCloseEvent * e) {
    // Message de confirmation si on est sur une fenêtre d'édition
    if(ui->stackedWidget->currentIndex() != 0) {
        QMessageBox::StandardButton retour;
        retour = QMessageBox::question(this, "Confirmation", "Les modifications seront abandonnées. Confirmer ?");

        if(retour != QMessageBox::Yes) e->ignore();
        else {
            ui->stackedWidget->setCurrentIndex(0);
            groupe.id = -1;
            e->accept();
        }
    }
    else e->accept();
}

// Réimplémenté : exécuté lorsqu'une touche est appuyée
void GestionGroupes::keyPressEvent(QKeyEvent * e) {
    // On demande la fermeture de la fenêtre si la touche est Echap
    if(e->key() == Qt::Key_Escape) {
        this->close();
        e->accept();
    }
    else e->ignore();
}
