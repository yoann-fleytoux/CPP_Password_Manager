#include "GestionMotsDePasse.h"
#include "ComposantReseau.h"

#ifdef Q_OS_WIN
#include "ui_GestionMotsDePasse_win.h"
#else
#include "ui_GestionMotsDePasse_unix.h"
#endif

#include <QMessageBox>
#include <QCloseEvent>
#include <QKeyEvent>

GestionMotsDePasse::GestionMotsDePasse(ComposantReseau *p_compRes, QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint|Qt::WindowCloseButtonHint|Qt::WindowMinimizeButtonHint),
    ui(new Ui::GestionMotsDePasse),
    compRes(p_compRes)
{
    ui->setupUi(this);
}

GestionMotsDePasse::~GestionMotsDePasse()
{
    delete ui;
}

// Récupère la liste des mots de passe du système
void GestionMotsDePasse::rafraichirListeMotsDePasse() {

    // Envoi de la requête et réception de la réponse
    QByteArray message = "admin_listemotsdepasse";
    QByteArray reponse = compRes->envoyerMessage(message);

    QList<QByteArray> lignes = reponse.split(30);

    listeMotsDePasse.clear();
    ui->listeMotsDePasse->clear();

    // Affichage des résultats
    if(lignes.at(0) == "admin_listemotsdepasse" && lignes.at(1).size() > 0) {
        QList<QByteArray> champs;

        for(int m = 1; m < lignes.size(); m++) {
            champs = lignes.at(m).split(31);
            motDePasse.id = champs.at(0).trimmed().toInt();
            motDePasse.nom = champs.at(1).trimmed();
            motDePasse.identifiant = champs.at(2).trimmed();
            motDePasse.motDePasse = champs.at(3).trimmed();
            motDePasse.idServeur = champs.at(4).trimmed().toInt();
            motDePasse.nomServeur = champs.at(5).trimmed();
            motDePasse.hoteServeur = champs.at(6).trimmed();
            listeMotsDePasse.append(motDePasse);
            ui->listeMotsDePasse->addItem(motDePasse.nomServeur + " : " +
                                          motDePasse.nom + " @ " + motDePasse.hoteServeur);
        }
    }

    ui->listeMotsDePasse->setCurrentRow(-1);

}

// Récupère la liste des serveurs du système
void GestionMotsDePasse::rafraichirListeServeurs(const int idServeur) {

    // Envoi de la requête et réception de la réponse
    QByteArray message = "admin_listeserveurs";
    QByteArray reponse = compRes->envoyerMessage(message);

    QList<QByteArray> lignes = reponse.split(30);

    int ligneServeur = 0;

    listeServeurs.clear();
    ui->serveur->clear();

    // Affichage des résultats
    if(lignes.at(0) == "admin_listeserveurs" && lignes.at(1).size() > 0) {
        QList<QByteArray> champs;

        Serveur serveur;
        for(int m = 1; m < lignes.size(); m++) {
            champs = lignes.at(m).split(31);
            serveur.id = champs.at(0).trimmed().toInt();
            serveur.nom = champs.at(1).trimmed();
            serveur.hote = champs.at(2).trimmed();
            listeServeurs.append(serveur);
            ui->serveur->addItem(serveur.nom + " (" + serveur.hote + ")");
            if(serveur.id == idServeur) ligneServeur = m - 1;
        }
    }

    ui->serveur->setCurrentIndex(ligneServeur);
}

// Entre en mode création d'un mot de passe
void GestionMotsDePasse::on_creer_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->titreCreerModifier->setText("Créer un mot de passe");

    motDePasse.id = -1;
    ui->nom->clear();
    ui->identifiant->clear();
    ui->motdepasse->clear();

    rafraichirListeServeurs();
}

// Entre en mode modification d'un mot de passe
void GestionMotsDePasse::on_modifier_clicked()
{
    int i = ui->listeMotsDePasse->currentRow();
    if(i < 0 || i >= listeMotsDePasse.size()) return;

    ui->stackedWidget->setCurrentIndex(1);
    ui->titreCreerModifier->setText("Modifier un mot de passe");

    motDePasse = listeMotsDePasse.at(i);
    ui->nom->setText(motDePasse.nom);
    ui->identifiant->setText(motDePasse.identifiant);
    ui->motdepasse->setText(motDePasse.motDePasse);

    rafraichirListeServeurs(motDePasse.idServeur);
}

// Suppression d'un mot de passe
void GestionMotsDePasse::on_supprimer_clicked()
{
    int i = ui->listeMotsDePasse->currentRow();
    if(i < 0 || i >= listeMotsDePasse.size()) return;

    // Message de confirmation
    QMessageBox::StandardButton retour;
    retour = QMessageBox::question(this, "Confirmation", "La suppression du mot de passe sera définitive. Confirmer ?");

    if(retour != QMessageBox::Yes) return;

    // Envoi de la requête et réception de la réponse
    QByteArray message = "admin_gerermotdepasse";
    message.append(30);
    message.append("suppression");
    message.append(30);
    message.append(QString::number(listeMotsDePasse.at(i).id));

    QByteArray reponse = compRes->envoyerMessage(message);

    // Affichage de l'état de la requête
    if(reponse == "suppressionok") {
        QMessageBox::information(this, "Suppression effectuée", "Le mot de passe a été supprimé");
        rafraichirListeMotsDePasse();
    }

    else if(reponse == "suppressionerreurinexistant") {
        QMessageBox::critical(this, "Suppression échouée", "Le mot de passe n'existe pas (id_motdepasse introuvable).");
        rafraichirListeMotsDePasse();
    }

    else if(reponse.startsWith("suppressionerreurtechnique")) {
        QMessageBox::critical(this, "Suppression échouée", "Une erreur technique a eu lieu :\n\n" + reponse.split(30).at(1));
    }
    else {
        QMessageBox::critical(this, "Erreur serveur", "Pas de réponse ou réponse malformée du serveur. Veuillez réessayer.");
    }
}

// Validation de la création / modification du mot de passe
void GestionMotsDePasse::on_validerMotDePasse_clicked()
{
    QString mode = motDePasse.id == -1 ? "creation" : "modification";

    // Envoi de la requête et réception de la réponse
    QByteArray message = "admin_gerermotdepasse";
    message.append(30);
    message.append(mode);
    message.append(30);
    message.append(QString::number(motDePasse.id));
    message.append(30);
    message.append(ui->nom->text());
    message.append(30);
    message.append(ui->identifiant->text());
    message.append(30);
    message.append(ui->motdepasse->text());
    message.append(30);
    message.append(QString::number(listeServeurs.at(ui->serveur->currentIndex()).id));

    QByteArray reponse = compRes->envoyerMessage(message);

    // Affichage de l'état de la requête
    if(mode == "creation") {
        if(reponse == "creationok") {
            ui->stackedWidget->setCurrentIndex(0);
            QMessageBox::information(this, "Création réussie", "Le mot de passe a bien été créé");
            rafraichirListeMotsDePasse();
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
            QMessageBox::information(this, "Modification réussie", "Le mot de passe a bien été modifié");
            rafraichirListeMotsDePasse();
        }
        else if(reponse == "modificationerreurinexistant") {
            ui->stackedWidget->setCurrentIndex(0);
            QMessageBox::critical(this, "Modification échouée", "Le mot de passe n'existe pas (id_motdepasse introuvable).");
            rafraichirListeMotsDePasse();
        }
        else if(reponse.startsWith("modificationerreurtechnique")) {
            QMessageBox::critical(this, "Modification échouée", "Une erreur technique a eu lieu :\n\n" + reponse.split(30).at(1));
        }
        else {
            QMessageBox::critical(this, "Erreur serveur", "Pas de réponse ou réponse malformée du serveur. Veuillez réessayer.");
        }
    }
}

// Annulation de la création / modification du mot de passe
void GestionMotsDePasse::on_annulerMotDePasse_clicked()
{
    QMessageBox::StandardButton retour;
    retour = QMessageBox::question(this, "Confirmation", "Les modifications seront abandonnées. Confirmer ?");

    if(retour != QMessageBox::Yes) return;

    ui->stackedWidget->setCurrentIndex(0);
    ui->motdepasse->clear();
}

// Ajout d'un groupe du système aux groupes du mot de passe
void GestionMotsDePasse::on_ajouterGroupe_clicked()
{
    int i = ui->groupes->currentRow();
    if(i < 0 || i >= listeGroupes.size()) return;

    Groupe groupe = listeGroupes.at(i);
    listeGroupes.remove(i);
    delete ui->groupes->takeItem(i);
    listeGroupesMotDePasse.append(groupe);
    ui->groupesMotDePasse->addItem(groupe.nom);
}

// Retrait d'un groupe des groupes du mot de passe
void GestionMotsDePasse::on_enleverGroupe_clicked()
{
    int i = ui->groupesMotDePasse->currentRow();
    if(i < 0 || i >= listeGroupesMotDePasse.size()) return;

    Groupe groupe = listeGroupesMotDePasse.at(i);
    listeGroupesMotDePasse.remove(i);
    delete ui->groupesMotDePasse->takeItem(i);
    listeGroupes.append(groupe);
    ui->groupes->addItem(groupe.nom);
}

// Validation de la liste des groupes du mot de passe
void GestionMotsDePasse::on_validerGroupes_clicked()
{

    // Envoi de la requête et réception de la réponse
    QByteArray message = "admin_gerermotdepasse";
    message.append(30);
    message.append("groupes");
    message.append(30);
    message.append(QString::number(motDePasse.id));
    message.append(30);

    for(int g = 0; g < listeGroupesMotDePasse.size(); g++) {
        message.append(QString::number(listeGroupesMotDePasse.at(g).id));
        if(g != listeGroupesMotDePasse.size() - 1) message.append(31);
    }

    QByteArray reponse = compRes->envoyerMessage(message);

    // Affichage de l'état de la requête
    if(reponse == "groupesmotdepasseok") {
        ui->stackedWidget->setCurrentIndex(0);
        QMessageBox::information(this, "Modification réussie", "Les groupes du mot de passe ont été mis à jour.");
        rafraichirListeMotsDePasse();
    }
    else if(reponse == "groupesmotdepasseerreurinexistant") {
        ui->stackedWidget->setCurrentIndex(0);
        QMessageBox::critical(this, "Modification échouée", "Le mot de passe n'existe pas (id_motdepasse introuvable).");
        rafraichirListeMotsDePasse();
    }
    else if(reponse.startsWith("groupesmotdepasseerreurtechnique")) {
        QMessageBox::critical(this, "Modification échouée", "Une erreur technique a eu lieu :\n\n" + reponse.split(30).at(1));
    }
    else {
        QMessageBox::critical(this, "Erreur serveur", "Pas de réponse ou réponse malformée du serveur. Veuillez réessayer.");
    }
}

// Annulation de la modification de la liste des groupes du mot de passe
void GestionMotsDePasse::on_annulerGroupes_clicked()
{
    QMessageBox::StandardButton retour;
    retour = QMessageBox::question(this, "Confirmation", "Les modifications seront abandonnées. Confirmer ?");

    if(retour != QMessageBox::Yes) return;

    ui->stackedWidget->setCurrentIndex(0);
}

// Activation des boutons de modification lors de la sélection d'un mot de passe
void GestionMotsDePasse::on_listeMotsDePasse_currentRowChanged(int currentRow)
{
    bool actif = (currentRow != -1);
    ui->modifier->setEnabled(actif);
    ui->supprimer->setEnabled(actif);
    ui->gererGroupes->setEnabled(actif);
}

// Simulation du bouton modifier lors du double-clic sur un mot de passe
void GestionMotsDePasse::on_listeMotsDePasse_doubleClicked(const QModelIndex &index)
{
    on_modifier_clicked();
}


// Actualisation de la liste des mots de passe
void GestionMotsDePasse::on_actualiser_clicked()
{
    rafraichirListeMotsDePasse();
}

// Réimplémenté : exécuté lors de la fermeture de la fenêtre (croix)
void GestionMotsDePasse::closeEvent(QCloseEvent * e) {
    // Message de confirmation si on est sur une fenêtre d'édition
    if(ui->stackedWidget->currentIndex() != 0) {
        QMessageBox::StandardButton retour;
        retour = QMessageBox::question(this, "Confirmation", "Les modifications seront abandonnées. Confirmer ?");

        if(retour != QMessageBox::Yes) e->ignore();
        else {
            ui->stackedWidget->setCurrentIndex(0);
            motDePasse.id = -1;
            ui->motdepasse->clear();
            e->accept();
        }
    }
    else e->accept();
}

// Réimplémenté : exécuté lorsqu'une touche est appuyée
void GestionMotsDePasse::keyPressEvent(QKeyEvent * e) {
    // On demande la fermeture de la fenêtre si la touche est Echap
    if(e->key() == Qt::Key_Escape) {
        this->close();
        e->accept();
    }
    else e->ignore();
}

void GestionMotsDePasse::on_gererGroupes_clicked()
{
    int i = ui->listeMotsDePasse->currentRow();
    if(i < 0 || i >= listeMotsDePasse.size()) return;

    listeGroupesMotDePasse.clear();
    ui->groupesMotDePasse->clear();
    listeGroupes.clear();
    ui->groupes->clear();

    motDePasse = listeMotsDePasse.at(i);

    ui->labelMotDePasse->setText(motDePasse.nomServeur + " : " +
                                 motDePasse.nom + " @ " + motDePasse.hoteServeur);

    QByteArray message;
    QList<QByteArray> lignes;
    QList<QByteArray> champs;
    Groupe groupe;

    QByteArray groupesMotDePasse;

    message = "admin_listegroupesmotdepasse";
    message.append(30);
    message.append(QString::number(motDePasse.id));
    groupesMotDePasse = compRes->envoyerMessage(message);

    lignes = groupesMotDePasse.split(30);

    if(lignes.at(0) == "admin_listegroupesmotdepasse" && lignes.at(1).size() > 0) {
        listeGroupesMotDePasse.clear();
        ui->groupesMotDePasse->clear();
        for(int m = 1; m < lignes.size(); m++) {
            champs = lignes.at(m).split(31);
            groupe.id = champs.at(0).trimmed().toInt();
            groupe.nom = champs.at(1).trimmed();
            listeGroupesMotDePasse.append(groupe);
            ui->groupesMotDePasse->addItem(groupe.nom);
        }
    }

    lignes.clear();
    champs.clear();

    QByteArray groupes;
    groupes = compRes->envoyerMessage("admin_listegroupes");
    bool trouve;

    lignes = groupes.split(30);

    if(lignes.at(0) == "admin_listegroupes" && lignes.at(1).size() > 0) {
        listeGroupes.clear();
        ui->groupes->clear();
        for(int m = 1; m < lignes.size(); m++) {
            champs = lignes.at(m).split(31);
            groupe.id = champs.at(0).trimmed().toInt();
            groupe.nom = champs.at(1).trimmed();

            for(int g = 0; g < listeGroupesMotDePasse.size(); g++) {
                if(listeGroupesMotDePasse.at(g).id == groupe.id) {
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
