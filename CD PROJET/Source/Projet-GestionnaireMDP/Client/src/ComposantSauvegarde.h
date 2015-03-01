#ifndef COMPOSANTSAUVEGARDE_H
#define COMPOSANTSAUVEGARDE_H

#include <QDialog>

class ComposantReseau;

namespace Ui {
class ComposantSauvegarde;
}

class ComposantSauvegarde : public QDialog
{
    Q_OBJECT

public:
    explicit ComposantSauvegarde(ComposantReseau *p_compRes, QWidget *parent = 0);
    ~ComposantSauvegarde();

private slots:
    void on_demarrerSauvegarde_clicked();

private:
    Ui::ComposantSauvegarde *ui;
    ComposantReseau *compRes;

    void closeEvent(QCloseEvent *);
    void keyPressEvent(QKeyEvent *);
};

#endif // COMPOSANTSAUVEGARDE_H
