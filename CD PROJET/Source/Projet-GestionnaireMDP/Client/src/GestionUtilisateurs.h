#ifndef GESTIONUTILISATEURS_H
#define GESTIONUTILISATEURS_H

#include <QDialog>

class ComposantReseau;

namespace Ui {
class GestionUtilisateurs;
}

class GestionUtilisateurs : public QDialog
{
    Q_OBJECT

public:
    explicit GestionUtilisateurs(ComposantReseau *p_compRes, QWidget *parent = 0);
    ~GestionUtilisateurs();

    void rafraichirListeUtilisateurs();

private slots:
    void on_creer_clicked();

    void on_modifier_clicked();

    void on_supprimer_clicked();

    void on_listeUtilisateurs_currentRowChanged(int currentRow);

    void on_listeUtilisateurs_doubleClicked(const QModelIndex &index);

    void on_gererGroupes_clicked();

    void on_validerUtilisateur_clicked();

    void on_annulerUtilisateur_clicked();

    void on_ajouterGroupe_clicked();

    void on_enleverGroupe_clicked();

    void on_validerGroupes_clicked();

    void on_annulerGroupes_clicked();

    void on_actualiser_clicked();

private:

    Ui::GestionUtilisateurs *ui;
    ComposantReseau *compRes;

    struct Utilisateur {
        int id;
        QString nom;
        QString identifiant;
        QString motDePasse;
        bool administrateur;
    };

    struct Groupe {
        int id;
        QString nom;
    };

    Utilisateur utilisateur;
    QVector<Utilisateur> listeUtilisateurs;
    QVector<Groupe> listeGroupes;
    QVector<Groupe> listeGroupesUtilisateur;

    void closeEvent(QCloseEvent *);
    void keyPressEvent(QKeyEvent *);
};

#endif // GESTIONUTILISATEURS_H
