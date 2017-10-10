#ifndef INITIALISATION
#define INITIALISATION
#include <QString>

typedef struct utilisateur {
    int id;
    QString nom;
    QString address;
    QString mobile;
    QString mail;
    int admin;
    QString login;
} utilisateur;

typedef struct note {
    int id;
    QString titre;
    QString date_creation;
    QString date_envoie;
    QString message;
} note;

typedef struct rucher {
    int id;
    QString nom;
    QString date_creation;
    int alert;
    double lng;
    double lat;
    QString place;
    int iduser;
} rucher;

typedef struct ruche{
    int id;
    int num;
    QString info;
    QString date_creation;
    double temperature;
    double pression;
    QString traffic;
    int idrucher;
} ruche;

typedef struct fichesuivie {
    int id;
    QString date_creation;
    int alert;
    int idruche;
    int nombre_de_hausse;
    int hausse_occ;
    QString traffic;
    bool toit;
    bool porte;
    int nombre_cadre_couv;
    double surface_occ_couv;
    int nombre_cadre_pollen;
    double surface_occ_pollen;
    int nombre_cadre_miel;
    double surface_occ_miel;
    QString attitude_des_abeilles;
    QString reine;
    QString population;
    QString abeilles_morte;
    QString symptomes;
    QString traitement_effectues;
    QString date_trait;
    QString notification;
} fichesuivie;

#endif // INITIALISATION

