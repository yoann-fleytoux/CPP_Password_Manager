#include "ComposantCryptographie.h"

#include <QFile>

#ifdef Q_OS_WIN
#include <openssl/applink.c>
#endif

ComposantCryptographie::ComposantCryptographie()
{
    clePubliqueRSA = NULL;
    clePriveeRSA = NULL;
    encAES = NULL;
    decAES = NULL;
    cleAES = NULL;
    fichierCleRSA = "";
}

// Destructeur : libère toutes les clés RSA et AES
ComposantCryptographie::~ComposantCryptographie()
{
    RSA_free(clePubliqueRSA);

    RSA_free(clePriveeRSA);

    if(encAES) {
        EVP_CIPHER_CTX_cleanup(encAES);
        free(encAES);
        encAES = NULL;
    }

    if(decAES) {
        EVP_CIPHER_CTX_cleanup(decAES);
        free(decAES);
        decAES = NULL;
    }

    free(cleAES);
    cleAES = NULL;
}

/******************************* RSA *******************************/

// Charge une clé publique RSA à partir d'un fichier
// Renvoie true si le chargement a réussi, false sinon
bool ComposantCryptographie::loadClePubliqueRSA(const QString fichier) {

    // Vérification de l'existence du fichier
    if(!QFile::exists(fichier)) return false;

    // Ouverture du fichier
    FILE * fp = fopen(fichier.toUtf8().data(), "r");
    if(fp == NULL) return false;

    // Chargement de la clé
    PEM_read_RSA_PUBKEY(fp, &clePubliqueRSA, NULL, NULL);
    fclose(fp);
    if(clePubliqueRSA == NULL) return false;

    fichierCleRSA = fichier;

    return true;

}

// Charge une clé privée RSA à partir d'un fichier
// Renvoie true si le chargement a réussi, false sinon
bool ComposantCryptographie::loadClePriveeRSA(const QString fichier) {

    // Vérification de l'existence du fichier
    if(!QFile::exists(fichier)) return false;

    // Ouverture du fichier
    FILE * fp = fopen(fichier.toUtf8().data(), "r");
    if(fp == NULL) return false;

    // Chargement de la clé
    PEM_read_RSAPrivateKey(fp, &clePriveeRSA, NULL, NULL);
    fclose(fp);
    if(clePriveeRSA == NULL) return false;

    return true;
}

// Retourne la clé publique RSA au format PEM
QByteArray ComposantCryptographie::getClePubliqueRSA() {
    if(fichierCleRSA == "") return QByteArray();
    QFile fichier(fichierCleRSA);
    if(!fichier.open(QFile::ReadOnly)) return QByteArray();
    QByteArray cleRSA = fichier.readAll();
    fichier.close();
    return cleRSA;
}

// Crypte un message avec RSA avec la clé publique actuellement chargée
QByteArray ComposantCryptographie::encrypterRSA(const QByteArray messageClair) {

    // Initialisations
    QByteArray messageCrypte, aux;
    int num;
    int size = RSA_size(clePubliqueRSA);
    unsigned char * from;
    unsigned char * to = (unsigned char *)malloc(size);
    size -= 11;

    // Le message doit être découpé en plusieurs morceaux pour être crypté
    for(int c = 0; c < messageClair.size(); c += size) {
        aux = messageClair.mid(c, size);
        from = (unsigned char *)aux.data();
        num = RSA_public_encrypt(aux.size(), from, to, clePubliqueRSA, RSA_PKCS1_PADDING);
        aux = QByteArray((char *)to, num);
        messageCrypte.append(aux);
    }

    free(to);

    return messageCrypte;
}

// Décrypte un message crypté avec RSA avec la clé privée actuellement chargée
QByteArray ComposantCryptographie::decrypterRSA(const QByteArray messageCrypte) {

    // Initialisations
    QByteArray messageClair, aux;
    int num;
    int size = RSA_size(clePriveeRSA);
    unsigned char * from;
    unsigned char * to = (unsigned char *)malloc(size);

    // Le message doit être découpé en plusieurs morceaux pour être décrypté
    for(int c = 0; c < messageCrypte.size(); c += size) {
        aux = messageCrypte.mid(c, size);
        from = (unsigned char *)aux.data();
        num = RSA_private_decrypt(aux.size(), from, to, clePriveeRSA, RSA_PKCS1_PADDING);
        messageClair.append(QByteArray((char *)to, num));
    }

    free(to);

    return messageClair;
}

// Génère un couple de clés RSA privée/publique et les stocke dans des fichiers
bool ComposantCryptographie::genererClesRSA(const QString fichierClePriveeRSA, const QString fichierClePubliqueRSA) {

    // Initialisations
    RSA * clesRSA = RSA_new();
    BIGNUM * e = BN_new();

    // Génération de l'exposant et de la paire de clés RSA
    BN_set_word(e, 65537);
    if(!RSA_generate_key_ex(clesRSA, 2048, e, NULL)) return false;

    // Ouverture et écriture du fichier de la clé privée
    FILE * fp = fopen(fichierClePriveeRSA.toUtf8().data(), "w");
    if(fp == NULL) return false;

    if(!PEM_write_RSAPrivateKey(fp, clesRSA, NULL, NULL, 0, NULL, NULL)) return false;
    fclose(fp);

    // Ouverture et écriture du fichier de la clé publique
    FILE * fp2 = fopen(fichierClePubliqueRSA.toUtf8().data(), "w");
    if(fp2 == NULL) return false;

    if(!PEM_write_RSA_PUBKEY(fp2, clesRSA)) {
        QFile::remove(fichierClePriveeRSA);
        return false;
    }
    fclose(fp2);

    RSA_free(clesRSA);

    return true;
}

/******************************* AES *******************************/

// Génère une clé AES
// Renvoie true si la génération a réussi, false sinon
bool ComposantCryptographie::genererCleAES() {

    // Allocation de mémoire pour la clé AES
    if(cleAES) free(cleAES);
    cleAES = (unsigned char *)malloc(32);
    if(cleAES == NULL) return false;

    // Génération aléatoire de la clé
    int i = RAND_bytes(cleAES, 32);
    if(i == 0) {
        free(cleAES);
        cleAES = NULL;
        return false;
    }

    // Chargement de la clé
    if(!setCleAES()) {
        free(cleAES);
        cleAES = NULL;
        return false;
    }

    return true;
}

// Renvoie la clé AES actuelle
QByteArray ComposantCryptographie::getCleAES() {

    // Vérification de l'existence de la clé
    if(cleAES == NULL) return QByteArray();

    return QByteArray((char *)cleAES, 32);
}

// Charge une clé AES
// Renvoie true si le chargement a réussi, false sinon
bool ComposantCryptographie::setCleAES(const QByteArray cle) {

    // Vérification de la taille de clé
    if(cle.size() != 32) return false;

    // Allocation de mémoire pour la clé AES
    cleAES = (unsigned char *)malloc(32);
    if(cleAES == NULL) return false;

    // Copie de la clé
    memcpy((void*)cleAES, (void *)cle.data(), 32);

    // Chargement de la clé
    if(!setCleAES()) {
        free(cleAES);
        cleAES = NULL;
        return false;
    }

    return true;
}

// Génère les contextes encAES et decAES à partir de la cleAES actuelle
// Renvoie true si la génération a réussi, false sinon
bool ComposantCryptographie::setCleAES() {

    // Vérification de l'existence de la clé
    if(cleAES == NULL) return false;

    // Initialisations key et iv
    unsigned char key[32], iv[32];
    memset(key, 0, 32);
    memset(iv, 0, 32);

    // Génération des key et iv
    int i = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha1(), NULL, cleAES, 32, 5, key, iv);
    if (i == 0) return false;

    // Génération du contexte encAES
    encAES = (EVP_CIPHER_CTX*)malloc(sizeof(EVP_CIPHER_CTX));
    EVP_CIPHER_CTX_init(encAES);
    i = EVP_EncryptInit_ex(encAES, EVP_aes_256_cbc(), NULL, key, iv);
    if(i == 0) {
        EVP_CIPHER_CTX_cleanup(encAES);
        free(encAES);
        encAES = NULL;
        return false;
    }

    // Génération du contexte decAES
    decAES = (EVP_CIPHER_CTX*)malloc(sizeof(EVP_CIPHER_CTX));
    EVP_CIPHER_CTX_init(decAES);
    EVP_DecryptInit_ex(decAES, EVP_aes_256_cbc(), NULL, key, iv);
    if(i == 0) {
        EVP_CIPHER_CTX_cleanup(encAES);
        free(encAES);
        encAES = NULL;
        EVP_CIPHER_CTX_cleanup(decAES);
        free(decAES);
        decAES = NULL;
        return false;
    }

    return true;
}

// Décrypte un message crypté avec AES avec la clé actuellement chargée
QByteArray ComposantCryptographie::decrypterAES(const QByteArray messageCrypte) {

    // Vérifications du contexte de décryptage et du message crypté
    if(decAES == NULL || messageCrypte.size() == 0) return QByteArray();

    // Initialisations
    unsigned char * ciphertext = (unsigned char *)messageCrypte.data();
    int p_len = messageCrypte.size(), f_len = 0;
    unsigned char *plaintext = (unsigned char *)malloc(p_len);

    // Décryptage
    if(!EVP_DecryptInit_ex(decAES, NULL, NULL, NULL, NULL))
        return QByteArray();

    if(!EVP_DecryptUpdate(decAES, plaintext, &p_len, ciphertext, messageCrypte.size()))
        return QByteArray();

    if(!EVP_DecryptFinal_ex(decAES, plaintext+p_len, &f_len))
        return QByteArray();

    QByteArray messageClair((char *)plaintext, p_len + f_len);

    free(plaintext);

    return messageClair;
}

// Crypte un message avec AES avec la clé actuellement chargée
QByteArray ComposantCryptographie::encrypterAES(const QByteArray messageClair) {

    // Vérification du contexte d'encryptage
    if(encAES == NULL) return QByteArray();

    // Initialisations
    unsigned char * plaintext = (unsigned char *)messageClair.data();
    int c_len = messageClair.size() + AES_BLOCK_SIZE - 1, f_len = 0;
    unsigned char *ciphertext = (unsigned char *)malloc(c_len);

    // Encryptage
    if(!EVP_EncryptInit_ex(encAES, NULL, NULL, NULL, NULL))
        return QByteArray();

    if(!EVP_EncryptUpdate(encAES, ciphertext, &c_len, plaintext, messageClair.size()))
        return QByteArray();

    if(!EVP_EncryptFinal_ex(encAES, ciphertext+c_len, &f_len))
        return QByteArray();

    QByteArray messageCrypte((char *)ciphertext, c_len + f_len);

    free(ciphertext);

    return messageCrypte;
}
