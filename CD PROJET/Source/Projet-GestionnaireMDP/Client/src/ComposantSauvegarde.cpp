#include "ComposantSauvegarde.h"
#include "ComposantReseau.h"

#ifdef Q_OS_WIN
#include "ui_ComposantSauvegarde_win.h"
#else
#include "ui_ComposantSauvegarde_unix.h"
#endif

#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QKeyEvent>

ComposantSauvegarde::ComposantSauvegarde(ComposantReseau *p_compRes, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ComposantSauvegarde),
    compRes(p_compRes)
{
    ui->setupUi(this);
}

ComposantSauvegarde::~ComposantSauvegarde()
{
    delete ui;
}

// Lance le processus de sauvegarde
void ComposantSauvegarde::on_demarrerSauvegarde_clicked()
{
    //Envoi du message et réception de la réponse
    QByteArray reponse = compRes->envoyerMessage("admin_sauvegarder");

    // Vérification de la réponse
    if(reponse.startsWith("sauvegardeok")) {

        // Demande de l'emplacement de la sauvegarde à écrire
        QString nomFichier = QFileDialog::getSaveFileName(this, "Emplacement de la sauvegarde", "",
                                                       "Sauvegarde SPM (*.spm)");

        QFile fichier(nomFichier);
        if(fichier.open(QFile::WriteOnly | QFile::Truncate)) {
            // On doit lire la sauvegarde à partir de la position 28
            // 12 caractères pour "sauvegardeok", 16 caractètes pour le mot de passe
            fichier.write(reponse.mid(28));
            fichier.close();

            QMessageBox::information(this, "Sauvegarde réussie",
                                     "La sauvegarde a été effectuée avec succès.\n"
                                     "Veuillez noter le mot de passe puis fermer la fenêtre.");

            ui->motDePasse->setText(reponse.mid(12, 16));
        }
        else {
            QMessageBox::warning(this, "Sauvegarde échouée", "Impossible d'écrire dans le fichier.");
        }
    }
    else QMessageBox::warning(this, "Sauvegarde échouée", "La sauvegarde a échoué. Veuillez réessayer.");
}

// Réimplémenté : exécuté lors de la fermeture de la fenêtre (croix)
void ComposantSauvegarde::closeEvent(QCloseEvent * e) {
    ui->motDePasse->clear();
    e->accept();
}

// Réimplémenté : exécuté lorsqu'une touche est appuyée
void ComposantSauvegarde::keyPressEvent(QKeyEvent * e) {
    // On demande la fermeture de la fenêtre si la touche est Echap
    if(e->key() == Qt::Key_Escape) {
        this->close();
        e->accept();
    }
    else e->ignore();
}
