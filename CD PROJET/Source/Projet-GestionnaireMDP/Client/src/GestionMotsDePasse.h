#ifndef GESTIONMOTSDEPASSE_H
#define GESTIONMOTSDEPASSE_H

#include <QDialog>

class ComposantReseau;

namespace Ui {
class GestionMotsDePasse;
}

class GestionMotsDePasse : public QDialog
{
    Q_OBJECT

public:
    explicit GestionMotsDePasse(ComposantReseau *p_compRes, QWidget *parent = 0);
    ~GestionMotsDePasse();

    void rafraichirListeMotsDePasse();

private slots:
    void on_creer_clicked();

    void on_modifier_clicked();

    void on_supprimer_clicked();

    void on_validerMotDePasse_clicked();

    void on_annulerMotDePasse_clicked();

    void on_ajouterGroupe_clicked();

    void on_enleverGroupe_clicked();

    void on_validerGroupes_clicked();

    void on_annulerGroupes_clicked();

    void on_listeMotsDePasse_currentRowChanged(int currentRow);

    void on_listeMotsDePasse_doubleClicked(const QModelIndex &index);

    void on_actualiser_clicked();

    void on_gererGroupes_clicked();

private:
    Ui::GestionMotsDePasse *ui;
    ComposantReseau *compRes;

    struct MotDePasse {
        int id;
        QString nom;
        QString identifiant;
        QString motDePasse;
        int idServeur;
        QString nomServeur;
        QString hoteServeur;
    };

    struct Serveur {
        int id;
        QString nom;
        QString hote;
    };

    struct Groupe {
        int id;
        QString nom;
    };

    MotDePasse motDePasse;
    QVector<MotDePasse> listeMotsDePasse;
    QVector<Serveur> listeServeurs;
    QVector<Groupe> listeGroupes;
    QVector<Groupe> listeGroupesMotDePasse;

    void rafraichirListeServeurs(const int idServeur = -1);

    void closeEvent(QCloseEvent *);
    void keyPressEvent(QKeyEvent *);
};

#endif // GESTIONMOTSDEPASSE_H
