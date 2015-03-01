#include "GestionServeurs.h"
#include "ComposantReseau.h"

#ifdef Q_OS_WIN
#include "ui_GestionServeurs_win.h"
#else
#include "ui_GestionServeurs_unix.h"
#endif

#include <QMessageBox>
#include <QCloseEvent>
#include <QKeyEvent>

GestionServeurs::GestionServeurs(ComposantReseau *p_compRes, QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint|Qt::WindowCloseButtonHint|Qt::WindowMinimizeButtonHint),
    ui(new Ui::GestionServeurs),
    compRes(p_compRes)
{
    ui->setupUi(this);
}

GestionServeurs::~GestionServeurs()
{
    delete ui;
}

// Récupère la liste des serveurs du système
void GestionServeurs::rafraichirListeServeurs() {

    // Envoi de la requête et réception de la réponse
    QByteArray message = "admin_listeserveurs";
    QByteArray reponse = compRes->envoyerMessage(message);

    QList<QByteArray> lignes = reponse.split(30);

    listeServeurs.clear();
    ui->listeServeurs->clear();

    // Affichage des résultats
    if(lignes.at(0) == "admin_listeserveurs" && lignes.at(1).size() > 0) {
        QList<QByteArray> champs;

        for(int m = 1; m < lignes.size(); m++) {
            champs = lignes.at(m).split(31);
            serveur.id = champs.at(0).trimmed().toInt();
            serveur.nom = champs.at(1).trimmed();
            serveur.hote = champs.at(2).trimmed();
            listeServeurs.append(serveur);
            ui->listeServeurs->addItem(serveur.nom + " (" + serveur.hote + ")");
        }
    }

    ui->listeServeurs->setCurrentRow(-1);

}

// Entre en mode création d'un serveur
void GestionServeurs::on_creer_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->titreCreerModifier->setText("Créer un serveur");

    serveur.id = -1;
    ui->nom->clear();
    ui->hote->clear();
}

// Entre en mode modification d'un serveur
void GestionServeurs::on_modifier_clicked()
{
    int i = ui->listeServeurs->currentRow();
    if(i < 0 || i >= listeServeurs.size()) return;

    ui->stackedWidget->setCurrentIndex(1);
    ui->titreCreerModifier->setText("Modifier un serveur");

    serveur = listeServeurs.at(i);
    ui->nom->setText(serveur.nom);
    ui->hote->setText(serveur.hote);
}

// Suppression d'un serveur
void GestionServeurs::on_supprimer_clicked()
{
    int i = ui->listeServeurs->currentRow();
    if(i < 0 || i >= listeServeurs.size()) return;

    // Messages de confirmation
    QMessageBox::StandardButton retour;

    retour = QMessageBox::question(this, "Confirmation", "La suppression du serveur sera définitive. Confirmer ?");
    if(retour != QMessageBox::Yes) return;

    retour = QMessageBox::question(this, "Confirmation", "Tous les mots de passe associés au serveur seront également supprimés. Confirmer ?");
    if(retour != QMessageBox::Yes) return;

    // Envoi de la requête et réception de la réponse
    QByteArray message = "admin_gererserveur";
    message.append(30);
    message.append("suppression");
    message.append(30);
    message.append(QString::number(listeServeurs.at(i).id));

    QByteArray reponse = compRes->envoyerMessage(message);

    // Affichage de l'état de la requête
    if(reponse == "suppressionok") {
        QMessageBox::information(this, "Suppression effectuée", "Le serveur a été supprimé");
        rafraichirListeServeurs();
    }

    else if(reponse == "suppressionerreurinexistant") {
        QMessageBox::critical(this, "Suppression échouée", "Le serveur n'existe pas (id_serveur introuvable).");
        rafraichirListeServeurs();
    }

    else if(reponse.startsWith("suppressionerreurtechnique")) {
        QMessageBox::critical(this, "Suppression échouée", "Une erreur technique a eu lieu :\n\n" + reponse.split(30).at(1));
    }
    else {
        QMessageBox::critical(this, "Erreur serveur", "Pas de réponse ou réponse malformée du serveur. Veuillez réessayer.");
    }
}

// Validation de la création / modification du serveur
void GestionServeurs::on_validerServeur_clicked()
{
    QString mode = serveur.id == -1 ? "creation" : "modification";

    // Envoi de la requête et réception de la réponse
    QByteArray message = "admin_gererserveur";
    message.append(30);
    message.append(mode);
    message.append(30);
    message.append(QString::number(serveur.id));
    message.append(30);
    message.append(ui->nom->text());
    message.append(30);
    message.append(ui->hote->text());

    QByteArray reponse = compRes->envoyerMessage(message);

    // Affichage de l'état de la requête
    if(mode == "creation") {
        if(reponse == "creationok") {
            ui->stackedWidget->setCurrentIndex(0);
            QMessageBox::information(this, "Création réussie", "Le serveur a bien été créé");
            rafraichirListeServeurs();
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
            QMessageBox::information(this, "Modification réussie", "Le serveur a bien été modifié");
            rafraichirListeServeurs();
        }
        else if(reponse == "modificationerreurinexistant") {
            ui->stackedWidget->setCurrentIndex(0);
            QMessageBox::critical(this, "Modification échouée", "Le serveur n'existe pas (id_serveur introuvable).");
            rafraichirListeServeurs();
        }
        else if(reponse.startsWith("modificationerreurtechnique")) {
            QMessageBox::critical(this, "Modification échouée", "Une erreur technique a eu lieu :\n\n" + reponse.split(30).at(1));
        }
        else {
            QMessageBox::critical(this, "Erreur serveur", "Pas de réponse ou réponse malformée du serveur. Veuillez réessayer.");
        }
    }
}

// Annulation de la création / modification du serveur
void GestionServeurs::on_annulerServeur_clicked()
{
    QMessageBox::StandardButton retour;
    retour = QMessageBox::question(this, "Confirmation", "Les modifications seront abandonnées. Confirmer ?");

    if(retour != QMessageBox::Yes) return;

	ui->stackedWidget->setCurrentIndex(0);
}

// Activation des boutons de modification lors de la sélection d'un serveur
void GestionServeurs::on_listeServeurs_currentRowChanged(int currentRow)
{
    bool actif = (currentRow != -1);
    ui->modifier->setEnabled(actif);
    ui->supprimer->setEnabled(actif);
}

// Simulation du bouton modifier lors du double-clic sur un serveur
void GestionServeurs::on_listeServeurs_doubleClicked(const QModelIndex &index)
{
    on_modifier_clicked();
}

// Actualisation de la liste des serveurs
void GestionServeurs::on_actualiser_clicked()
{
    rafraichirListeServeurs();
}

// Réimplémenté : exécuté lors de la fermeture de la fenêtre (croix)
void GestionServeurs::closeEvent(QCloseEvent * e) {
    // Message de confirmation si on est sur une fenêtre d'édition
    if(ui->stackedWidget->currentIndex() != 0) {
        QMessageBox::StandardButton retour;
        retour = QMessageBox::question(this, "Confirmation", "Les modifications seront abandonnées. Confirmer ?");

        if(retour != QMessageBox::Yes) e->ignore();
        else {
            ui->stackedWidget->setCurrentIndex(0);
            serveur.id = -1;
            e->accept();
        }
    }
    else e->accept();
}

// Réimplémenté : exécuté lorsqu'une touche est appuyée
void GestionServeurs::keyPressEvent(QKeyEvent * e) {
    // On demande la fermeture de la fenêtre si la touche est Echap
    if(e->key() == Qt::Key_Escape) {
        this->close();
        e->accept();
    }
    else e->ignore();
}
