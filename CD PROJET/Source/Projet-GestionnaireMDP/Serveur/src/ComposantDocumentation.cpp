#include "ComposantDocumentation.h"

#include <QTextStream>
#include <QDir>

ComposantDocumentation::ComposantDocumentation()
{
}

void ComposantDocumentation::aide() {
    QTextStream out(stdout);

    out << endl << "Usage :" << endl << endl;

    out << "  spmd" << endl;

    out << "    Lance le serveur avec comme fichier de configuration par defaut : " << QDir::homePath() << "/spmd/spmd.conf" << endl << endl;

    out << "  spmd --config <fichier>" << endl;
    out << "    Lance le serveur avec le fichier de configuration fourni en parametre." << endl << endl;

    out << "  spmd --install" << endl;
    out << "      [--config <fichier>]" << endl;
    out << "      [--priv-key <fichier>]" << endl;
    out << "      [--pub-key <fichier>]" << endl;
    out << "      [--database <fichier>]" << endl;
    out << "      [--port <port>]" << endl;
    out << "      [--use-existing-keys]" << endl << endl;

    out << "    Installe le serveur avec les parametres fournis." << endl;
    out << "    Le fichier de configuration est renseigne avec --config, ou est " << QDir::homePath() << "/spmd/spmd.conf par defaut." << endl;
    out << "    Le fichier de cle privee RSA est renseigne avec --priv-key, ou est " << QDir::homePath() << "/spmd/private_key.pem par defaut." << endl;
    out << "    Le fichier de cle publique RSA est renseigne avec --pub-key, ou est " << QDir::homePath() << "/spmd/public_key.pem par defaut." << endl;
    out << "    Le fichier de base de donnees est renseigne avec --database, ou est " << QDir::homePath() << "/spmd/database.sqlite3 par defaut." << endl;
    out << "    Le port d'ecoute du serveur est renseigne avec --port, ou est 3665 par defaut" << endl;
    out << "    Les cles RSA seront generees aleatoirement sauf si presence du parametre --use-existing-keys." << endl;
    out << "    Le processus d'installation demandera confirmation en cas de fichiers existants." << endl << endl;

    out << "  spmd --backup" << endl;
    out << "       --file <fichier>" << endl;
    out << "      [--config <fichier>]" << endl << endl;

    out << "    Effectue une sauvegarde des cles et base de donnees dans un fichier crypte." << endl;
    out << "    Le fichier de configuration est renseigne avec --config, ou est " << QDir::homePath() << "/spmd/spmd.conf par defaut." << endl;
    out << "    Le fichier est crypte avec un mot de passe genere aleatoirement qui sera affiche." << endl << endl;

    out << "  spmd --restore" << endl;
    out << "       --file <fichier>" << endl;
    out << "       --password <motdepasse>" << endl;
    out << "      [--config <fichier>]" << endl << endl;

    out << "    Restaure la sauvegarde a partir du fichier crypte." << endl;
    out << "    Le fichier de configuration a ecrire est renseigne avec --config, ou est " << QDir::homePath() << "/spmd/spmd.conf par defaut." << endl << endl;

    out << "  spmd --help" << endl << endl;
    out << "    Affiche cette aide." << endl << endl;
}
