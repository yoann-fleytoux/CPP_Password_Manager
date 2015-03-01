#include "GestionUtilisateurs.h"
#include "ComposantReseau.h"

#ifdef Q_OS_WIN
#include "ui_GestionUtilisateurs_win.h"
#else
#include "ui_GestionUtilisateurs_unix.h"
#endif

#include <QMessageBox>
#include <QCloseEvent>
#include <QKeyEvent>

GestionUtilisateurs::GestionUtilisateurs(ComposantReseau *p_compRes, QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint|Qt::WindowCloseButtonHint|Qt::WindowMinimizeButtonHint),
    ui(new Ui::GestionUtilisateurs),
    compRes(p_compRes)
{
    ui->setupUi(this);
}

GestionUtilisateurs::~GestionUtilisateurs()
{
    delete ui;
}

// Récupère la liste des utilisateurs du système
void GestionUtilisateurs::rafraichirListeUtilisateurs() {

    // Envoi de la requête et réception de la réponse
    QByteArray message = "admin_listeutilisateurs";
    QByteArray reponse = compRes->envoyerMessage(message);

    QList<QByteArray> lignes = reponse.split(30);

    listeUtilisateurs.clear();
    ui->listeUtilisateurs->clear();

    // Affichage des résultats
    if(lignes.at(0) == "admin_listeutilisateurs" && lignes.at(1).size() > 0) {
        QList<QByteArray> champs;

        for(int m = 1; m < lignes.size(); m++) {
            champs = lignes.at(m).split(31);
            utilisateur.id = champs.at(0).trimmed().toInt();
            utilisateur.nom = champs.at(1).trimmed();
            utilisateur.identifiant = champs.at(2).trimmed();
            utilisateur.motDePasse = champs.at(3).trimmed();
            utilisateur.administrateur = champs.at(4).trimmed() == "admin";
            listeUtilisateurs.append(utilisateur);
            ui->listeUtilisateurs->addItem(utilisateur.nom + " (" + utilisateur.identifiant + ")"
                                           + (utilisateur.administrateur ? ", administrateur" : ""));
        }
    }

    utilisateur.motDePasse.clear();
    ui->listeUtilisateurs->setCurrentRow(-1);

}

// Entre en mode création d'un utilisateur
void GestionUtilisateurs::on_creer_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->titreCreerModifier->setText("Créer un utilisateur");

    utilisateur.id = -1;
    ui->nom->clear();
    ui->identifiant->clear();
    ui->motdepasse->clear();
    ui->administrateur->setChecked(false);
}

// Entre en mode modification d'un utilisateur
void GestionUtilisateurs::on_modifier_clicked()
{
    int i = ui->listeUtilisateurs->currentRow();
    if(i < 0 || i >= listeUtilisateurs.size()) return;

    ui->stackedWidget->setCurrentIndex(1);
    ui->titreCreerModifier->setText("Modifier un utilisateur");

    utilisateur = listeUtilisateurs.at(i);
    ui->nom->setText(utilisateur.nom);
    ui->identifiant->setText(utilisateur.identifiant);
    ui->motdepasse->setText(utilisateur.motDePasse);
    ui->administrateur->setChecked(utilisateur.administrateur);
}

// Suppression d'un utilisateur
void GestionUtilisateurs::on_supprimer_clicked()
{
    int i = ui->listeUtilisateurs->currentRow();
    if(i < 0 || i >= listeUtilisateurs.size()) return;

    // Message de confirmation
    QMessageBox::StandardButton retour;
    retour = QMessageBox::question(this, "Confirmation", "La suppression de l'utilisateur sera définitive. Confirmer ?");

    if(retour != QMessageBox::Yes) return;

    // Envoi de la requête et réception de la réponse
    QByteArray message = "admin_gererutilisateur";
    message.append(30);
    message.append("suppression");
    message.append(30);
    message.append(QString::number(listeUtilisateurs.at(i).id));

    QByteArray reponse = compRes->envoyerMessage(message);

    // Affichage de l'état de la requête
    if(reponse == "suppressionok") {
        QMessageBox::information(this, "Suppression effectuée", "L'utilisateur a été supprimé");
        rafraichirListeUtilisateurs();
    }

    else if(reponse == "suppressionerreurinexistant") {
        QMessageBox::critical(this, "Suppression échouée", "L'utilisateur n'existe pas (id_utilisateur introuvable).");
        rafraichirListeUtilisateurs();
    }

    else if(reponse.startsWith("suppressionerreurtechnique")) {
        QMessageBox::critical(this, "Suppression échouée", "Une erreur technique a eu lieu :\n\n" + reponse.split(30).at(1));
    }
    else {
        QMessageBox::critical(this, "Erreur serveur", "Pas de réponse ou réponse malformée du serveur. Veuillez réessayer.");
    }
}

// Entre en mode gestion des groupe de l'utilisateur
void GestionUtilisateurs::on_gererGroupes_clicked()
{
    int i = ui->listeUtilisateurs->currentRow();
    if(i < 0 || i >= listeUtilisateurs.size()) return;

    listeGroupesUtilisateur.clear();
    ui->groupesUtilisateur->clear();
    listeGroupes.clear();
    ui->groupes->clear();

    utilisateur = listeUtilisateurs.at(i);

    ui->labelUtilisateur->setText(utilisateur.nom + " (" + utilisateur.identifiant + ")");

    QByteArray message;
    QList<QByteArray> lignes;
    QList<QByteArray> champs;
    Groupe groupe;

    // Récupération des groupes actuels de l'utilisateur
    QByteArray groupesUtilisateur;
    message = "admin_listegroupesutilisateur";
    message.append(30);
    message.append(QString::number(utilisateur.id));
    groupesUtilisateur = compRes->envoyerMessage(message);

    lignes = groupesUtilisateur.split(30);

    // Affichage des résultats
    if(lignes.at(0) == "admin_listegroupesutilisateur" && lignes.at(1).size() > 0) {
        listeGroupesUtilisateur.clear();
        ui->groupesUtilisateur->clear();
        for(int m = 1; m < lignes.size(); m++) {
            champs = lignes.at(m).split(31);
            groupe.id = champs.at(0).trimmed().toInt();
            groupe.nom = champs.at(1).trimmed();
            listeGroupesUtilisateur.append(groupe);
            ui->groupesUtilisateur->addItem(groupe.nom);
        }
    }

    lignes.clear();
    champs.clear();

    // Récupération des groupes du système
    QByteArray groupes;
    groupes = compRes->envoyerMessage("admin_listegroupes");
    bool trouve;

    lignes = groupes.split(30);

    // Affichage des résultats
    if(lignes.at(0) == "admin_listegroupes" && lignes.at(1).size() > 0) {
        listeGroupes.clear();
        ui->groupes->clear();
        for(int m = 1; m < lignes.size(); m++) {
            champs = lignes.at(m).split(31);
            groupe.id = champs.at(0).trimmed().toInt();
            groupe.nom = champs.at(1).trimmed();

            // On exclue les groupes qui sont dans les groupes de l'utilisateur
            for(int g = 0; g < listeGroupesUtilisateur.size(); g++) {
                if(listeGroupesUtilisateur.at(g).id == groupe.id) {
                    trouve = true;
                    break;
                }
            }
            if(!trouve) {
                listeGroupes.append(groupe);
                ui->groupes->addItem(groupe.nom);
            }
            trouve = false;
        }
    }

    ui->stackedWidget->setCurrentIndex(2);
}

// Validation de la création / modification de l'utilisateur
void GestionUtilisateurs::on_validerUtilisateur_clicked()
{
    QString mode = utilisateur.id == -1 ? "creation" : "modification";

    // Envoi de la requête et réception de la réponse
    QByteArray message = "admin_gererutilisateur";
    message.append(30);
    message.append(mode);
    message.append(30);
    message.append(QString::number(utilisateur.id));
    message.append(30);
    message.append(ui->nom->text());
    message.append(30);
    message.append(ui->identifiant->text());
    message.append(30);
    message.append(ui->motdepasse->text());
    message.append(30);
    message.append(ui->administrateur->isChecked() ? "true" : "false");

    QByteArray reponse = compRes->envoyerMessage(message);

    // Affichage de l'état de la requête
    if(mode == "creation") {
        if(reponse == "creationok") {
            ui->stackedWidget->setCurrentIndex(0);
            ui->motdepasse->clear();
            QMessageBox::information(this, "Création réussie", "L'utilisateur a bien été créé");
            rafraichirListeUtilisateurs();
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
            ui->motdepasse->clear();
            QMessageBox::information(this, "Modification réussie", "L'utilisateur a bien été modifié");
            rafraichirListeUtilisateurs();
        }
        else if(reponse == "modificationerreurinexistant") {
            ui->stackedWidget->setCurrentIndex(0);
            ui->motdepasse->clear();
            QMessageBox::critical(this, "Modification échouée", "L'utilisateur n'existe pas (id_utilisateur introuvable).");
            rafraichirListeUtilisateurs();
        }
        else if(reponse.startsWith("modificationerreurtechnique")) {
            QMessageBox::critical(this, "Modification échouée", "Une erreur technique a eu lieu :\n\n" + reponse.split(30).at(1));
        }
        else {
            QMessageBox::critical(this, "Erreur serveur", "Pas de réponse ou réponse malformée du serveur. Veuillez réessayer.");
        }
    }
}

// Annulation de la création / modification de l'utilisateur
void GestionUtilisateurs::on_annulerUtilisateur_clicked()
{
    QMessageBox::StandardButton retour;
    retour = QMessageBox::question(this, "Confirmation", "Les modifications seront abandonnées. Confirmer ?");

    if(retour != QMessageBox::Yes) return;

    ui->stackedWidget->setCurrentIndex(0);
    ui->motdepasse->clear();
}

// Ajout d'un groupe du système aux groupes de l'utilisateur
void GestionUtilisateurs::on_ajouterGroupe_clicked()
{
    int i = ui->groupes->currentRow();
    if(i < 0 || i >= listeGroupes.size()) return;

    Groupe groupe = listeGroupes.at(i);
    listeGroupes.remove(i);
    delete ui->groupes->takeItem(i);
    listeGroupesUtilisateur.append(groupe);
    ui->groupesUtilisateur->addItem(groupe.nom);
}

// Retrait d'un groupes des groupes de l'utilisateur
void GestionUtilisateurs::on_enleverGroupe_clicked()
{
    int i = ui->groupesUtilisateur->currentRow();
    if(i < 0 || i >= listeGroupesUtilisateur.size()) return;

    Groupe groupe = listeGroupesUtilisateur.at(i);
    listeGroupesUtilisateur.remove(i);
    delete ui->groupesUtilisateur->takeItem(i);
    listeGroupes.append(groupe);
    ui->groupes->addItem(groupe.nom);
}

// Validation de la liste des groupes de l'utilisateur
void GestionUtilisateurs::on_validerGroupes_clicked()
{
    // Envoi de la requête et réception de la réponse
    QByteArray message = "admin_gererutilisateur";
    message.append(30);
    message.append("groupes");
    message.append(30);
    message.append(QString::number(utilisateur.id));
    message.append(30);

    for(int g = 0; g < listeGroupesUtilisateur.size(); g++) {
        message.append(QString::number(listeGroupesUtilisateur.at(g).id));
        if(g != listeGroupesUtilisateur.size() - 1) message.append(31);
    }

    QByteArray reponse = compRes->envoyerMessage(message);

    // Affichage de l'état de la requête
    if(reponse == "groupesutilisateurok") {
        ui->stackedWidget->setCurrentIndex(0);
        QMessageBox::information(this, "Modification réussie", "Les groupes de l'utilisateur ont été mis à jour.");
        rafraichirListeUtilisateurs();
    }
    else if(reponse == "groupesutilisateurerreurinexistant") {
        ui->stackedWidget->setCurrentIndex(0);
        QMessageBox::critical(this, "Modification échouée", "L'utilisateur n'existe pas (id_utilisateur introuvable).");
        rafraichirListeUtilisateurs();
    }
    else if(reponse.startsWith("groupesutilisateurerreurtechnique")) {
        QMessageBox::critical(this, "Modification échouée", "Une erreur technique a eu lieu :\n\n" + reponse.split(30).at(1));
    }
    else {
        QMessageBox::critical(this, "Erreur serveur", "Pas de réponse ou réponse malformée du serveur. Veuillez réessayer.");
    }
}

// Annulation de la modification de la liste des groupes de l'utilisateur
void GestionUtilisateurs::on_annulerGroupes_clicked()
{
    QMessageBox::StandardButton retour;
    retour = QMessageBox::question(this, "Confirmation", "Les modifications seront abandonnées. Confirmer ?");

    if(retour != QMessageBox::Yes) return;

    ui->stackedWidget->setCurrentIndex(0);
}

// Activation des boutons de modification lors de la sélection d'un utilisateur
void GestionUtilisateurs::on_listeUtilisateurs_currentRowChanged(int currentRow)
{
    bool actif = (currentRow != -1);
    ui->modifier->setEnabled(actif);
    ui->supprimer->setEnabled(actif);
    ui->gererGroupes->setEnabled(actif);
}

// Simulation du bouton modifier lors du double-clic sur un utilisateurs
void GestionUtilisateurs::on_listeUtilisateurs_doubleClicked(const QModelIndex &index)
{
    on_modifier_clicked();
}

// Actualisation de la liste des utilisateurs
void GestionUtilisateurs::on_actualiser_clicked()
{
    rafraichirListeUtilisateurs();
}

// Réimplémenté : exécuté lors de la fermeture de la fenêtre (croix)
void GestionUtilisateurs::closeEvent(QCloseEvent * e) {
    // Message de confirmation si on est sur une fenêtre d'édition
    if(ui->stackedWidget->currentIndex() != 0) {
        QMessageBox::StandardButton retour;
        retour = QMessageBox::question(this, "Confirmation", "Les modifications seront abandonnées. Confirmer ?");

        if(retour != QMessageBox::Yes) e->ignore();
        else {
            ui->stackedWidget->setCurrentIndex(0);
            utilisateur.id = -1;
            ui->motdepasse->clear();
            e->accept();
        }
    }
    else e->accept();
}

// Réimplémenté : exécuté lorsqu'une touche est appuyée
void GestionUtilisateurs::keyPressEvent(QKeyEvent * e) {
    // On demande la fermeture de la fenêtre si la touche est Echap
    if(e->key() == Qt::Key_Escape) {
        this->close();
        e->accept();
    }
    else e->ignore();
}
