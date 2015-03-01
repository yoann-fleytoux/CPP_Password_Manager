#ifndef GESTIONSERVEURS_H
#define GESTIONSERVEURS_H

#include <QDialog>

class ComposantReseau;

namespace Ui {
class GestionServeurs;
}

class GestionServeurs : public QDialog
{
    Q_OBJECT

public:
    explicit GestionServeurs(ComposantReseau *p_compRes, QWidget *parent = 0);
    ~GestionServeurs();

    void rafraichirListeServeurs();

private slots:
    void on_creer_clicked();

    void on_modifier_clicked();

    void on_supprimer_clicked();

    void on_validerServeur_clicked();

    void on_annulerServeur_clicked();

    void on_listeServeurs_currentRowChanged(int currentRow);

    void on_listeServeurs_doubleClicked(const QModelIndex &index);

    void on_actualiser_clicked();

private:
    Ui::GestionServeurs *ui;
    ComposantReseau *compRes;

    struct Serveur {
        int id;
        QString nom;
        QString hote;
    };

    Serveur serveur;
    QVector<Serveur> listeServeurs;

    void closeEvent(QCloseEvent *);
    void keyPressEvent(QKeyEvent *);
};

#endif // GESTIONSERVEURS_H
