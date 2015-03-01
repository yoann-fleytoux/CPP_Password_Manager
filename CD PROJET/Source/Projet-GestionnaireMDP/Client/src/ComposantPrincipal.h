#ifndef COMPOSANTPRINCIPAL_H
#define COMPOSANTPRINCIPAL_H

#include <QMainWindow>
#include <QFile>

namespace Ui {
class ComposantPrincipal;
}

class ComposantReseau;
class ComposantSauvegarde;
class GestionGroupes;
class GestionMotsDePasse;
class GestionServeurs;
class GestionUtilisateurs;
class ComposantCryptographie;

class ComposantPrincipal : public QMainWindow
{
    Q_OBJECT

public:
    ComposantPrincipal(QWidget *parent = 0);
    ~ComposantPrincipal();

private slots:

    void on_enregistrerPageConfiguration_clicked();

    void on_connexionPageConnexion_clicked();

    void on_actionDeconnexion_triggered();

    void on_actionQuitter_triggered();

    void on_actionGestionUtilisateurs_triggered();

    void on_actionGestionServeurs_triggered();

    void on_actionGestionMDP_triggered();

    void on_actionGestionGroupes_triggered();

    void on_actionSauvegarder_triggered();

    void on_actualiser_clicked();

    void on_afficherMasquer_clicked();

    void on_listeMotsDePasse_clicked(const QModelIndex &index);

    void on_identifiantPageConnexion_returnPressed();

    void on_mdpPageConnexion_returnPressed();

private:

    struct MotDePasse {
        QString nomServeur;
        QString hoteServeur;
        QString nom;
        QString identifiant;
        QString motDePasse;
    };

    Ui::ComposantPrincipal *ui;
    ComposantReseau *compRes;
    ComposantSauvegarde *compSave;
    GestionGroupes *gestGroupe;
    GestionMotsDePasse *gestMDP;
    GestionServeurs *gestServ;
    GestionUtilisateurs *gestUtil;
    ComposantCryptographie *compCrypt;
    QString ipServ;
    int portServ;
    QString cleServ;
    QFile fichierConfig;
    int idUtilisateur;
    QString nomUtilisateur;
    bool estAdministrateur;

    QVector<MotDePasse> listeMotsDePasse;

    void reinitialiser();
    void rafraichirListeMotsDePasse();
};

#endif // COMPOSANTPRINCIPAL_H
