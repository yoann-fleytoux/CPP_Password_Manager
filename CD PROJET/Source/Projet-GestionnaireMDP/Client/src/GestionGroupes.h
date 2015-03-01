#ifndef GESTIONGROUPES_H
#define GESTIONGROUPES_H

#include <QDialog>

class ComposantReseau;

namespace Ui {
class GestionGroupes;
}

class GestionGroupes : public QDialog
{
    Q_OBJECT

public:
    explicit GestionGroupes(ComposantReseau *p_compRes, QWidget *parent = 0);
    ~GestionGroupes();

    void rafraichirListeGroupes();

private slots:
    void on_creer_clicked();

    void on_gererUtilisateurs_clicked();

    void on_gererMotsDePasse_clicked();

    void on_modifier_clicked();

    void on_supprimer_clicked();

    void on_validerGroupe_clicked();

    void on_annulerGroupe_clicked();

    void on_ajouterUtilisateur_clicked();

    void on_enleverUtilisateur_clicked();

    void on_validerUtilisateurs_clicked();

    void on_annulerUtilisateurs_clicked();

    void on_ajouterMotDePasse_clicked();

    void on_enleverMotDePasse_clicked();

    void on_validerMotsDePasse_clicked();

    void on_annulerMotsDePasse_clicked();

    void on_listeGroupes_currentRowChanged(int currentRow);

    void on_listeGroupes_doubleClicked(const QModelIndex &index);

    void on_actualiser_clicked();

private:
    Ui::GestionGroupes *ui;
    ComposantReseau *compRes;

    struct Groupe {
        int id;
        QString nom;
    };

    struct Utilisateur {
        int id;
        QString nom;
        QString identifiant;
    };

    struct MotDePasse {
        int id;
        QString nom;
        QString identifiant;
        QString nomServeur;
        QString hoteServeur;
    };

    Groupe groupe;
    QVector<Groupe> listeGroupes;
    QVector<Utilisateur> listeUtilisateurs;
    QVector<Utilisateur> listeUtilisateursGroupe;
    QVector<MotDePasse> listeMotsDePasse;
    QVector<MotDePasse> listeMotsDePasseGroupe;

    void closeEvent(QCloseEvent *);
    void keyPressEvent(QKeyEvent *);
};

#endif // GESTIONGROUPES_H
