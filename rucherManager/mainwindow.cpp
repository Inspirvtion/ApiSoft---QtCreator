#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql>
#include <QDebug>
#include <QMessageBox>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->addRucherAddress, SIGNAL(addressFinded(double,double,QString)), ui->carte, SLOT(updateView(double,double,QString)));
    connect(ui->Adresse_2, SIGNAL(addressFinded(double,double,QString)), ui->carte, SLOT(updateView(double,double,QString)));
    ui->Adresse_2->setVisible(false);
    ui->Valider_5->setVisible(false);
    modele = new QStandardItemModel(0, 9);
    ui->ficheView->setModel(modele);

    liste.append("n° ruche");
    liste.append("nbr Hausses");
    liste.append("nbr Hausses après");
    liste.append("Traffic");
    liste.append("Attitude");
    liste.append("Reine");
    liste.append("population");
    liste.append("symptomes");
    liste.append("traitement");
    /** base de donnée **/
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setUserName("root");
    db.setPassword("");
    db.setDatabaseName("apisoft");
    /** **************** **/
    /** Timer notif **/
    notif = new QTimer();
    connect(notif, SIGNAL(timeout()), this, SLOT(lookNotif()));
    modelListNotif = new QStringListModel(listNotif);
    ui->listNotifView->setModel(modelListNotif);
    connect(ui->listNotifView, SIGNAL(clicked(QModelIndex)), this, SLOT(affNotif(QModelIndex)));
    /** **************** **/
    mySetting = new QSettings("apisoft.ini", QSettings::IniFormat);
    bool userIsConnected = mySetting->value("Section1/userIsConnected","false").toBool();
    if(!userIsConnected){
        ui->vue_principal->setCurrentWidget(ui->connect);
        ui->menuBar->hide();
        ui->menu_flottant->close();
        ui->mainToolBar->hide();
    }else
    {
        ui->vue_principal->setCurrentWidget(ui->acceuil);
        ui->menuBar->show();
        ui->menu_flottant->show();
        ui->mainToolBar->show();
        /** Chargement des paramètre du user **/
        currentUser.address = mySetting->value("userInfo/address","").toString();
        currentUser.admin = mySetting->value("userInfo/admin","").toInt();
        currentUser.id = mySetting->value("userInfo/id","").toInt();
        currentUser.login = mySetting->value("userInfo/login","").toString();
        currentUser.mail = mySetting->value("userInfo/mail","").toString();
        currentUser.mobile = mySetting->value("userInfo/mobile","").toString();
        currentUser.nom = mySetting->value("userInfo/nom","").toString();
        /** ******************************** **/
        ui->list_rucherP->addItem("", QVariant(-1));
        db.open();
        notif->start(1000);
        QString rqt;
        QSqlQuery *q;
        q = new QSqlQuery(db);
        rqt = QString("SELECT * FROM rucher WHERE utilisateur_id = %1").arg(currentUser.id);
        if(q->exec(rqt)){
            rucher r;
            while(q->next()){
                r.alert = q->record().value("alert").toInt();
                r.date_creation = q->record().value("date_creation").toString();
                r.id = q->record().value("ID").toInt();
                r.iduser = q->record().value("utilisateur_id").toInt();
                r.lat = q->record().value("latitude").toDouble();
                r.lng = q->record().value("longitude").toDouble();
                r.nom = q->record().value("nom").toString();
                r.place = q->record().value("place").toString();
                listRucher.insert(r.id, r);
                ui->list_rucherP->addItem(r.nom, QVariant(r.id));
            }
            ui->nom_2->setText(listRucher.value(ui->list_rucherP->currentData().toInt()).nom);
            ui->dateRucher_2->setDate(QDate::fromString(listRucher.value(ui->list_rucherP->currentData().toInt()).date_creation, "d/m/y"));
            ui->addrP->setText(listRucher.value(ui->list_rucherP->currentData().toInt()).place);
        }else{
            QMessageBox::information(NULL, "ApiSoft", "Echec lors de la tentative de connexion à la base de donnée", QMessageBox::Ok);
        }
    }
    //CONNEXION DES SIGNAUX ET SLOTS
    connect(ui->con, SIGNAL(connected(QString,QString,bool)), this, SLOT(connectMe(QString,QString,bool)));
    connect(ui->actionMe_deconnecter, SIGNAL(triggered()), this, SLOT(deconnectMe()));
    connect(ui->actionUne_fiche_de_suivi, SIGNAL(triggered()),this, SLOT(actionfichedesuivi()));
    connect(ui->actionRuche, SIGNAL(triggered()), this, SLOT(actionAddRuche()));
    connect(ui->actionRucher, SIGNAL(triggered()), this, SLOT(actionAddRucher()));
    connect(ui->actionMon_profil, SIGNAL(triggered()),this, SLOT(actionMonProfil()));
    connect(ui->con, SIGNAL(signup(QString,QString,QString,QString,QString,QString,QString)), this, SLOT(signMeup(QString,QString,QString,QString,QString,QString,QString)));
    connect(ui->Valider_2, SIGNAL(clicked()), this, SLOT(addRucher()));
    connect(ui->Valider_3, SIGNAL(clicked()), this, SLOT(addRuche()));
    connect(ui->vue_principal, SIGNAL(currentChanged(int)), this, SLOT(pageOpen(int)));
    connect(ui->liste_rucher, SIGNAL(currentIndexChanged(int)), this, SLOT(carteFicheSuivie(int)));
    connect(ui->Valider, SIGNAL(clicked()), this, SLOT(addFicheSuivie()));
    connect(ui->liste_rucher_2, SIGNAL(currentIndexChanged(int)), this, SLOT(carteRuche(int)));
    connect(ui->list_rucherP, SIGNAL(currentIndexChanged(int)), this, SLOT(rucherSelectionP(int)));
    connect(ui->list_rucheP, SIGNAL(currentIndexChanged(int)), this, SLOT(rucheSelectionP(int)));
    connect(ui->Valider_4, SIGNAL(clicked()), this, SLOT(modifierProfil()));
    connect(ui->Adresse, SIGNAL(addressFinded(double,double,QString)), ui->carte, SLOT(updateView(double,double,QString)));


}

MainWindow::~MainWindow()
{
    db.close();
    delete ui;
}

/** connectMe() **/
void MainWindow::connectMe(QString log, QString psswd, bool stay)
{
    if(db.open()){
        if(log != "" && psswd != ""){
            QString rqt = QString("SELECT * FROM utilisateur WHERE login = '%1' AND password = '%2'").arg(log).arg(psswd);
            QSqlQuery *q = new QSqlQuery(db);
            if(q->exec(rqt)){
                if(q->next())
                {
                    ui->vue_principal->setCurrentWidget(ui->acceuil);
                    ui->menuBar->show();
                    ui->menu_flottant->show();
                    currentUser.address = q->record().value("adresse").toString();
                    currentUser.admin = q->record().value("admin").toInt();
                    currentUser.id = q->record().value("ID").toInt();
                    currentUser.login = q->record().value("login").toString();
                    currentUser.mail = q->record().value("mail").toString();
                    currentUser.mobile = q->record().value("mobile").toString();
                    currentUser.nom = q->record().value("nom").toString();
                    ui->mainToolBar->show();
#ifdef DEBUG
                    QMessageBox::information(NULL, "ApiSoft", "Bonjour " + q->record().value("nom").toString(), QMessageBox::Ok);
#endif
                    if(stay){
                        mySetting->setValue("Section1/userIsConnected","true");
                        mySetting->setValue("userInfo/address",currentUser.address);
                        mySetting->setValue("userInfo/admin",currentUser.admin);
                        mySetting->setValue("userInfo/id",currentUser.id);
                        mySetting->setValue("userInfo/login",currentUser.login);
                        mySetting->setValue("userInfo/mail",currentUser.mail);
                        mySetting->setValue("userInfo/mobile",currentUser.mobile);
                        mySetting->setValue("userInfo/nom",currentUser.nom);
                    }else
                    {
                        mySetting->setValue("Section1/userIsConnected","false");
                        mySetting->setValue("userInfo/address",currentUser.address);
                        mySetting->setValue("userInfo/admin",currentUser.admin);
                        mySetting->setValue("userInfo/id",currentUser.id);
                        mySetting->setValue("userInfo/login",currentUser.login);
                        mySetting->setValue("userInfo/mail",currentUser.mail);
                        mySetting->setValue("userInfo/mobile",currentUser.mobile);
                        mySetting->setValue("userInfo/nom",currentUser.nom);
                    }
                    notif->start(1000);

                }else{
                    QMessageBox::information(NULL, "ApiSoft", "Login ou mot de passe incorrecte", QMessageBox::Ok);
                }
            }else{
                QMessageBox::information(NULL, "ApiSoft", "Impossible de se connecter à la base de donnée: Vérifier votre connexion réseau et relancer l'application. Si le problème perdure contacter le support technique au ******", QMessageBox::Ok);
            }
        }else{
            QMessageBox::information(NULL, "ApiSoft", "Veillez entrer votre login et mot de passe", QMessageBox::Ok);
        }}
}

/** signMeup **/

void MainWindow::signMeup(QString nom, QString address, QString mobile, QString mail, QString login, QString pass, QString pass2){
    if(!(nom.isEmpty() || mobile.isEmpty() || mail.isEmpty() || login.isEmpty() || pass.isEmpty() || pass2.isEmpty())){
        if(address != "aucune"){
            if(pass == pass2){
                    QString rqt = QString("SELECT count(ID) AS nombre FROM utilisateur WHERE login = '%1'").arg(login);
                    QSqlQuery *q = new QSqlQuery(db);
                    if(q->exec(rqt)){
                        if(q->next()){
                            if(q->record().value("nombre").toInt() == 0){
                                rqt = QString("INSERT INTO utilisateur(ID, nom, adresse, mobile, mail, admin, login, password) VALUES (null,'%1','%2','%3','%4',0,'%5','%6')").replace("'", "\"").arg(nom).arg(address.replace("'", "\'")).arg(mobile).arg(mail).arg(login).arg(pass);

                            }else{
                                QMessageBox::information(NULL, "ApiSoft", "Ce login est déjà utilisé par un autre utilisateur", QMessageBox::Ok);
                                rqt = "";
                            }
                        }
                        if(!rqt.isEmpty()){
                            bool ok = false;
                            //vérification par code de l'inscription
                                QString code = QInputDialog::getText(NULL, "ApiSoft", "vous avez reçu un code de validation d'inscription. à l'adresse " + address +"\nSaisir le code: ", QLineEdit::Normal, QString(), &ok);
                                QString codegen = "ApiSoft"; /** CETTE VALEUR POUR L'INSTANT EST FIXE POUR LES TESTE **/
                                if (ok && code == codegen)
                                {
                                    if(q->exec(rqt))
                                        QMessageBox::information(NULL, "ApiSoft", "Erreur lors de l'inscription, recommencer plutard", QMessageBox::Ok);
                                    else
                                        QMessageBox::information(NULL, "ApiSoft", "inscription réussite", QMessageBox::Ok);
                                }
                                else
                                {
                                    QMessageBox::critical(NULL, "ApiSoft", "Code Invorrect !");
                                    code = QInputDialog::getText(NULL, "ApiSoft", "vous avez reçu un code de validation d'inscription. à l'adresse " + address +"\nSaisir le code: ", QLineEdit::Normal, QString(), &ok);

                                    if (ok && code == codegen)
                                    {
                                        if(q->exec(rqt))
                                            QMessageBox::information(NULL, "ApiSoft", "Erreur lors de l'inscription, recommencer plutard", QMessageBox::Ok);
                                        else
                                            QMessageBox::information(NULL, "ApiSoft", "inscription réussite", QMessageBox::Ok);
                                     }
                                     else
                                     {
                                        QMessageBox::critical(NULL, "ApiSoft", "Code Invorrect ! vous devez reprendre à zero l'inscription");
                                     }
                                }

                        }
                    }
            }else{
                QMessageBox::information(NULL, "ApiSoft", "Les deux mots de passe ne sont pas identiques", QMessageBox::Ok);
            }
        }else{
            QMessageBox::information(NULL, "ApiSoft", "Il semblerait que votre adresse soit incorrecte...", QMessageBox::Ok);
        }
    }else{
        QMessageBox::information(NULL, "ApiSoft", "Veillez remplir tous les champs", QMessageBox::Ok);
    }
}

/** deconnectMe() **/


void MainWindow::deconnectMe(){
    mySetting->setValue("Section1/userIsConnected","false");
    mySetting->setValue("userInfo/address","");
    mySetting->setValue("userInfo/admin","");
    mySetting->setValue("userInfo/id","");
    mySetting->setValue("userInfo/login","");
    mySetting->setValue("userInfo/mail","");
    mySetting->setValue("userInfo/mobile","");
    mySetting->setValue("userInfo/nom","");
    ui->vue_principal->setCurrentWidget(ui->connect);
    ui->menuBar->hide();
    ui->menu_flottant->close();
    ui->mainToolBar->hide();
    notif->stop();
    notes.clear();
    listNotif.clear();
    ui->msgNotif->clear();
    modelListNotif->setStringList(listNotif);
}

/** actionfichedesuivi **/


void MainWindow::actionfichedesuivi(){
   ui->vue_principal->setCurrentWidget(ui->creerFicheSuivi);
}


/** actionAddRucher **/
void MainWindow::actionAddRucher(){
    ui->vue_principal->setCurrentWidget(ui->creerRucher);
}


/** NOTIFICATION **/

void MainWindow::lookNotif()
{
    int limite = 0;
    bool newNote = false;
    QSqlQuery *q = new QSqlQuery(db);
    QString rqt = QString("SELECT count(ID) AS nombre FROM notification WHERE date_envoie <= NOW() AND contact_id = %1 AND vue = 0 ORDER BY date_envoie DESC").arg(currentUser.id);
    if(q->exec(rqt)){
        if(q->next()){
            if((limite = q->record().value("nombre").toInt() - notes.count()) > 0)
                newNote = true;
        }
    }
    if(newNote){
        q = new QSqlQuery(db);
        QString rqt = QString("SELECT * FROM  notification WHERE date_creation <= NOW() AND contact_id = %1 AND vue = 0 ORDER BY ID DESC LIMIT %2").arg(currentUser.id).arg(limite);
        note n;
        if(q->exec(rqt)){
            while(q->next()){
                n.titre = q->record().value("titre").toString();
                n.date_creation =  q->record().value("date_creation").toString();
                n.date_envoie = q->record().value("date_envoie").toString();
                n.id = q->record().value("ID").toInt();
                n.message = q->record().value("message").toString();
                listNotif.append(n.titre);
                modelListNotif->setStringList(listNotif);
                notes.append(n);
                ui->nbrNote->setText(QString("Notification (%1)").arg(notes.count()));
                qDebug() << n.message;
            }
        }
    }
    ui->nbrNote->setText(QString("Notification (%1)").arg(notes.count()));
    notif->start(1000);
}


void MainWindow::affNotif(QModelIndex i)
{
    QString msg = notes.at(i.row()).message;
    QString date = notes.at(i.row()).date_creation;
    ui->dateNotif->setText(date);
    ui->msgNotif->setText(msg);
}


/** AddRucher **/

void MainWindow::addRucher(){
    if(ui->nomRucher->text().isEmpty()){
           QMessageBox::information(NULL, "ApiSoft", "Merci de nommer votre Rucher", QMessageBox::Ok);
    }else if(!ui->addRucherAddress->isValide()){
        QMessageBox::information(NULL, "ApiSoft", "Votre adresse est introuvable.", QMessageBox::Ok);
    }else{
        QSqlQuery *q = new QSqlQuery(db);
        QString rqt = QString("INSERT INTO rucher(ID, nom, date_creation, alert, longitude, latitude, utilisateur_id, place) VALUES (NULL,'%1',STR_TO_DATE('%2', '%d/%m/%Y'),0,%3,%4,%5,'%6')").replace("'","\"").arg(ui->nomRucher->text().replace("'", "\'")).arg(ui->dateRucher->text()).arg(ui->addRucherAddress->getLong()).arg(ui->addRucherAddress->getLat()).arg(currentUser.id).arg(ui->addRucherAddress->getAddress().replace("'", "\'"));
        qDebug() << rqt;
        if(q->exec(rqt)){
            QMessageBox::information(NULL, "ApiSoft", "Ajout réalisé avec succès", QMessageBox::Ok);
            ui->vue_principal->setCurrentIndex(0);
        }else{
            QMessageBox::information(NULL, "ApiSoft", "Erreur 1 lors de l'ajout: rucher non ajouter", QMessageBox::Ok);
        }
    }
}

/** pageOpen **/

void MainWindow::pageOpen(int p){
    listRucher.clear();
    ui->liste_rucher->clear();
    ui->liste_rucher_2->clear();
    ui->list_rucherP->clear();
    QString rqt;
    QSqlQuery *q;
    switch(p){
    case 4:
        q = new QSqlQuery(db);
        rqt = QString("SELECT * FROM rucher WHERE utilisateur_id = %1").arg(currentUser.id);
        if(q->exec(rqt)){
            rucher r;
            while(q->next()){
                r.alert = q->record().value("alert").toInt();
                r.date_creation = q->record().value("date_creation").toString();
                r.id = q->record().value("ID").toInt();
                r.iduser = q->record().value("utilisateur_id").toInt();
                r.lat = q->record().value("latitude").toDouble();
                r.lng = q->record().value("longitude").toDouble();
                r.nom = q->record().value("nom").toString();
                r.place = q->record().value("place").toString();
                listRucher.insert(r.id, r);
                ui->liste_rucher->addItem(r.nom, QVariant(r.id));
            }
            if(ui->liste_rucher->count() > 0){
                r = listRucher.value(ui->liste_rucher->currentData().toInt());
                ui->carte->updateView(r.lng, r.lat, r.place);
            }
        }else{
            QMessageBox::information(NULL, "ApiSoft", "Echec lors de la tentative de connexion à la base de donnée", QMessageBox::Ok);
        }
            break;
    case 3:
        q = new QSqlQuery(db);
        rqt = QString("SELECT * FROM rucher WHERE utilisateur_id = %1").arg(currentUser.id);
        if(q->exec(rqt)){
            rucher r;
            while(q->next()){
                r.alert = q->record().value("alert").toInt();
                r.date_creation = q->record().value("date_creation").toString();
                r.id = q->record().value("ID").toInt();
                r.iduser = q->record().value("utilisateur_id").toInt();
                r.lat = q->record().value("latitude").toDouble();
                r.lng = q->record().value("longitude").toDouble();
                r.nom = q->record().value("nom").toString();
                r.place = q->record().value("place").toString();
                listRucher.insert(r.id, r);
                ui->liste_rucher_2->addItem(r.nom, QVariant(r.id));
            }
            if(ui->liste_rucher_2->count() > 0){
                r = listRucher.value(ui->liste_rucher_2->currentData().toInt());
                ui->carte->updateView(r.lng, r.lat, r.place);
            }
        }else{
            QMessageBox::information(NULL, "ApiSoft", "Echec lors de la tentative de connexion à la base de donnée", QMessageBox::Ok);
        }
        break;
    case 5:
        ui->modifProfil->setChecked(false);
        ui->modifPass->setChecked(false);
        ui->nom->setText(currentUser.nom);
        ui->mail->setText(currentUser.mail);
        ui->mobile->setText(currentUser.mobile);
        ui->login_2->setText(currentUser.login);
        ui->addrProfil->setText(currentUser.address);
        break;
    case 0:
        ui->list_rucherP->addItem("", QVariant(-1));
        q = new QSqlQuery(db);
        rqt = QString("SELECT * FROM rucher WHERE utilisateur_id = %1").arg(currentUser.id);
        if(q->exec(rqt)){
            rucher r;
            while(q->next()){
                r.alert = q->record().value("alert").toInt();
                r.date_creation = q->record().value("date_creation").toString();
                r.id = q->record().value("ID").toInt();
                r.iduser = q->record().value("utilisateur_id").toInt();
                r.lat = q->record().value("latitude").toDouble();
                r.lng = q->record().value("longitude").toDouble();
                r.nom = q->record().value("nom").toString();
                r.place = q->record().value("place").toString();
                listRucher.insert(r.id, r);
                ui->list_rucherP->addItem(r.nom, QVariant(r.id));
            }
            if(ui->list_rucherP->count() > 0){
                r = listRucher.value(ui->list_rucherP->currentData().toInt());
                ui->carte->updateView(r.lng, r.lat, r.place);
            }
            ui->nom_2->setText(listRucher.value(ui->list_rucherP->currentData().toInt()).nom);
            ui->dateRucher_2->setDate(QDate::fromString(listRucher.value(ui->list_rucherP->currentData().toInt()).date_creation, "d/m/y"));
            ui->addrP->setText(listRucher.value(ui->list_rucherP->currentData().toInt()).place);
        }else{
            QMessageBox::information(NULL, "ApiSoft", "Echec lors de la tentative de connexion à la base de donnée", QMessageBox::Ok);
        }
        break;
    }
}

/** ajout fiche de suivi **/


void MainWindow::carteFicheSuivie(int i){

        listRuche.clear();
        ui->liste_ruche->clear();
        ui->list_rucheP->addItem("", QVariant(-1));
        rucher r = listRucher.value(ui->liste_rucher->itemData(i).toInt());
        qDebug() << r.place << "| long: " + QString("%1").arg(r.lng) << " lat: " + QString("%1").arg(r.lat);
        ui->carte->updateView(r.lng, r.lat, r.place);
        QSqlQuery *q = new QSqlQuery(db);
        QString rqt1 = QString("SET @a = 0");
        QString rqt = QString("SELECT (@a:=@a+1) as num, ID, info, date_creation, temperature, pression, traffic, rucher_id FROM ruchee WHERE rucher_id = %1").arg(r.id);
       if(q->exec(rqt1)) {
            if(q->exec(rqt)){
                ruche rche;
                while(q->next()){
                    rche.date_creation = q->record().value("date_creation").toString();
                    rche.id = q->record().value("ID").toInt();
                    rche.idrucher = q->record().value("rucher_id").toInt();
                    rche.info = q->record().value("info").toString();
                    rche.num = q->record().value("num").toInt();
                    rche.pression = q->record().value("pression").toDouble();
                    rche.temperature = q->record().value("temperature").toDouble();
                    rche.traffic = q->record().value("traffic").toString();
                    listRuche.insert(rche.id, rche);
                    ui->liste_ruche->addItem(QString("numero %1").arg(rche.num), QVariant(rche.id));
                }
            }else{
                QMessageBox::information(NULL, "ApiSoft", "Echec lors de la tentative de connexion à la base de donnée", QMessageBox::Ok);
            }
        }

}

void MainWindow::addFicheSuivie(){
    if(ui->liste_ruche->count() > 0 && ui->liste_rucher->count()>0){
        rucher r = listRucher.value(ui->liste_rucher->currentData().toInt());
        ruche rche = listRuche.value(ui->liste_ruche->currentData().toInt());
        int toit, porte;
        if(ui->toitOui->isChecked())
            toit = 1;
        else
           toit = 0;
        if(ui->porteOui->isChecked())
           porte = 1;
        else
           porte = 0;
        QSqlQuery *q = new QSqlQuery(db);
        QString rqt;
        if(ui->trait_a_eff->isChecked()){
            db.transaction();
            rqt = QString("INSERT INTO fichesuivie(ID, num, date_creation, alert, ruche_id, Nombre_de_Hausses, nombre_de_hausse_final, Traffic_entrée, Toit_en_place, Porte_en_place, nombre_cadres_Couv, surface_occ_Couv, nombre_cadres_Pollen, surface_occ_Pollen, nombre_cadres_Miel, surface_occ_Miel, Housse_occ, Attitude_des_abeilles, Reine, Population, Abeilles_mortes, Symptomes_et_anomalies, Traiements_effectues, Traiements_a_effectues, notification) VALUES (NULL,%1,STR_TO_DATE('%2', '%d/%m/%Y'),0,%3,%4,%5,'%6',%7").replace("'","\"").arg(rche.num).arg(ui->date_fiche->text()).arg(rche.id).arg(ui->nbrHausse->value()).arg(ui->nbrHausse_final->value()).arg(ui->traffic_fiche->currentText()).arg(toit) +QString(",%1,%2,%3,%4,%5,%6,%7,%8").replace("'","\"").arg(porte).arg(ui->nbrCouv->value()).arg(ui->occCou->value()).arg(ui->nbrPollen5->value()).arg(ui->occPollen->value()).arg(ui->nbrMiel->value()).arg(ui->occMiel->value()).arg(ui->occHausse->value())+QString(" ,'%1','%2','%3','%4','%5','%6',STR_TO_DATE('%7', '%d/%m/%Y'),'%8')").replace("'","\"").arg(ui->attitude->currentText()).arg(ui->reine->currentText()).arg(ui->population->currentText()).arg(ui->morte->text().replace("'","\'")).arg(ui->symptome->toPlainText().replace("'","\'")).arg(ui->traitement_eff->toPlainText().replace("'","\'")).arg(ui->date_trait->text()).arg(ui->notif->toPlainText().replace("'","\'"));
            if(q->exec(rqt)){
                q = new QSqlQuery(db);
                rqt = QString("INSERT INTO notification(ID, date_creation, date_envoie, message, contact_id, vue, titre) VALUES (NULL,NOW(),STR_TO_DATE('%1', '%d/%m/%Y'),'%2',%3,0,'(%4,Ruche %5) Traitement a effectuer le %6')").replace("'", "\"").arg(ui->datenotif->text()).arg(ui->notif->toPlainText().replace("'", "\'")).arg(currentUser.id).arg(r.nom.replace("'", "\'")).arg(rche.num).arg(ui->date_trait->text());
                qDebug() << rqt;
                if(q->exec(rqt)){
                    db.commit();
                    QMessageBox::information(NULL, "ApiSoft", "Nouvelle fiche créée avec succès.", QMessageBox::Ok);
                    ui->vue_principal->setCurrentIndex(0);
                }else{
                    qDebug() << db.lastError();
                    db.rollback();
                    QMessageBox::information(NULL, "ApiSoft", "Echec 3 lors de la tentative de connexion à la base de donnée", QMessageBox::Ok);
                }
            }else{
                db.rollback();
                QMessageBox::information(NULL, "ApiSoft", "Echec 2 lors de la tentative de connexion à la base de donnée", QMessageBox::Ok);
            }
        }else{
            rqt = QString("INSERT INTO fichesuivie(ID, num, date_creation, alert, ruche_id, Nombre_de_Hausses, nombre_de_hausse_final, Traffic_entrée, Toit_en_place, Porte_en_place, nombre_cadres_Couv, surface_occ_Couv, nombre_cadres_Pollen, surface_occ_Pollen, nombre_cadres_Miel, surface_occ_Miel, Housse_occ, Attitude_des_abeilles, Reine, Population, Abeilles_mortes, Symptomes_et_anomalies, Traiements_effectues, Traiements_a_effectues, notification) VALUES (NULL,%1,STR_TO_DATE('%2', '%d/%m/%Y'),0,%3,%4,%5,'%6',%7").replace("'","\"").arg(rche.num).arg(ui->date_fiche->text()).arg(rche.id).arg(ui->nbrHausse->value()).arg(ui->nbrHausse_final->value()).arg(ui->traffic_fiche->currentText()).arg(toit) +QString(",%1,%2,%3,%4,%5,%6,%7,%8").replace("'","\"").arg(porte).arg(ui->nbrCouv->value()).arg(ui->occCou->value()).arg(ui->nbrPollen5->value()).arg(ui->occPollen->value()).arg(ui->nbrMiel->value()).arg(ui->occMiel->value()).arg(ui->occHausse->value())+QString(" ,'%1','%2','%3','%4','%5','%6',STR_TO_DATE('%7', '%d/%m/%Y'),'%8')").replace("'","\"").arg(ui->attitude->currentText()).arg(ui->reine->currentText()).arg(ui->population->currentText()).arg(ui->morte->text().replace("'","\'")).arg(ui->symptome->toPlainText().replace("'","\'")).arg(ui->traitement_eff->toPlainText().replace("'","\'")).arg("00/00/0000").arg("AUCUN");
            if(q->exec(rqt)){
                QMessageBox::information(NULL, "ApiSoft", "Nouvelle fiche créée avec succès.", QMessageBox::Ok);
                ui->vue_principal->setCurrentIndex(0);
            }else{
                QMessageBox::information(NULL, "ApiSoft", "Echec 1 lors de la tentative de connexion à la base de donnée", QMessageBox::Ok);
            }
        }
    }
}


/** addRuche **/

void MainWindow::actionAddRuche(){
    ui->vue_principal->setCurrentWidget(ui->creerRuche);
}

void MainWindow::carteRuche(int i){
    rucher r = listRucher.value(ui->liste_rucher_2->itemData(i).toInt());
    ui->carte->updateView(r.lng, r.lat, r.place);
    QSqlQuery *q = new QSqlQuery(db);
    QString rqt = QString("SELECT COUNT(ID) + 1 as num FROM ruchee WHERE rucher_id = %1").arg(r.id);
    if(q->exec(rqt)){
        if(q->next()){
            int val = q->record().value("num").toInt();
           if(val <= 0) val = 1;
          ui->numeroDeLaRuche->setText(QString("%1").arg(val));
        }
    }else{
        QMessageBox::information(NULL, "ApiSoft", "Echec lors de la tentative de connexion à la base de donnée", QMessageBox::Ok);
    }
}

void MainWindow::addRuche(){
    if(ui->liste_rucher_2->count() > 0){
        rucher r = listRucher.value(ui->liste_rucher_2->currentData().toInt());
        QSqlQuery *q = new QSqlQuery(db);
        QString rqt = QString("INSERT INTO ruchee(ID, info, date_creation, temperature, pression, traffic, rucher_id) VALUES (NULL,'%1',NOW(),0,0,'moyen',%2)").replace("'","\"").arg(ui->information->toPlainText().replace("'", "\'")).arg(r.id);
        if(q->exec(rqt)){
            QMessageBox::information(NULL, "ApiSoft", "Ajout réalisé avec succès", QMessageBox::Ok);
            ui->vue_principal->setCurrentIndex(0);
        }else{
            QMessageBox::information(NULL, "ApiSoft", "Erreur 1 lors de l'ajout: rucher non ajouter", QMessageBox::Ok);
        }
    }

}

void MainWindow::backToPrincipal(){
    ui->vue_principal->setCurrentIndex(0);
}



/** PROFIL UTILISATEUR **/
void MainWindow::actionMonProfil()
{
    ui->vue_principal->setCurrentWidget(ui->profilUser);
}

void MainWindow::modifierProfil(){
    QSqlQuery *q = new QSqlQuery(db);
    QString nom, mobile, mail, login_2, Adresse, newPass, rqt;
    bool ok = false, ok2 = false;
//modification du profil
        if(ui->modifProfil->isChecked()){
            if(!(ui->nom->text().isEmpty() || ui->mobile->text().isEmpty() || ui->mail->text().isEmpty() || ui->login_2->text().isEmpty()))
            {
                nom = ui->nom->text();
                mobile = ui->mobile->text();
                mail = ui->mail->text();
                login_2 = ui->login_2->text();
                if(ui->Adresse->isValide()){
                    Adresse = ui->Adresse->getAddress();
                }else
                {
                    Adresse = currentUser.address;
                }
                ok = true;
            }else{
                QMessageBox::information(NULL, "ApiSoft", "Veillez remplir tous les champs!", QMessageBox::Ok);
                ok = false;
            }


        }else
        {
            nom = currentUser.nom;
            mobile = currentUser.mobile;
            mail = currentUser.mail;
            login_2 = currentUser.login;
            Adresse = currentUser.address;
            ok = true;
        }
//modification du mot de passe
        if(ui->modifPass->isChecked()){
            if(ui->oldPass->text().isEmpty() || ui->newPass->text().isEmpty() || ui->confPass->text().isEmpty()){
                QMessageBox::information(NULL, "ApiSoft", "veillez remplir tous les champs!", QMessageBox::Ok);
                ok2 = false;
            }else if( ui->newPass->text() != ui->confPass->text()){
             QMessageBox::information(NULL, "ApiSoft", "Le nouveau mot de passe et sa confirmation ne correspondent pas!", QMessageBox::Ok);
             ok2 = false;
            }else{
                rqt = QString("SELECT * FROM utilisateur WHERE ID = %1").arg(currentUser.id);
                if(q->exec(rqt)){
                    if(q->next()){
                        if(ui->oldPass->text() == q->record().value("password").toString()){
                            newPass = ui->newPass->text();
                            ok2 = true;
                            q->clear();
                        }else{
                            QMessageBox::information(NULL, "ApiSoft", "Erreur 3: mauvais mot de passe!", QMessageBox::Ok);
                            ok2 = false;
                        }
                    }else{
                        QMessageBox::information(NULL, "ApiSoft", "Erreur 2", QMessageBox::Ok);
                        ok2 = false;
                    }
                }else{
                    QMessageBox::information(NULL, "ApiSoft", "Erreur 1 ", QMessageBox::Ok);
                    ok2 = false;
                }
            }
        }else{
            ok2 = false;
        }

//requette
        bool uptodate = false;
        if(ok && ok2){
            uptodate = true;
            rqt = QString("UPDATE utilisateur SET nom='%1',adresse='%2',mobile='%3',mail='%4',password='%5' WHERE ID = %6").replace("'", "\'").arg(nom).arg(Adresse.replace("'", "\'")).arg(mobile).arg(mail).arg(newPass).arg(currentUser.id);
        }else if(ok && !ui->modifPass->isChecked()){
            uptodate = true;
            rqt = QString("UPDATE utilisateur SET nom='%1',adresse='%2',mobile='%3',mail='%4' WHERE ID = %5").replace("'", "\'").arg(nom).arg(Adresse.replace("'", "\'")).arg(mobile).arg(mail).arg(currentUser.id);
        }else if(ok2 && !ui->modifProfil->isChecked()){
            uptodate = true;
            rqt = QString("UPDATE utilisateur SET password='%1' WHERE ID = %2").replace("'", "\'").arg(newPass).arg(currentUser.id);
        }
     if(ui->modifPass->isChecked() || ui->modifProfil->isChecked())
     {
         if(uptodate){
             if(q->exec(rqt)){
                 QMessageBox::information(NULL, "ApiSoft", "modifications réalisées avec succès", QMessageBox::Ok);
                 currentUser.nom = nom;
                 currentUser.address = Adresse;
                 currentUser.mail = mail;
                 currentUser.mobile = mobile;
                 mySetting->setValue("userInfo/address",currentUser.address);
                 mySetting->setValue("userInfo/admin",currentUser.admin);
                 mySetting->setValue("userInfo/id",currentUser.id);
                 mySetting->setValue("userInfo/login",currentUser.login);
                 mySetting->setValue("userInfo/mail",currentUser.mail);
                 mySetting->setValue("userInfo/mobile",currentUser.mobile);
                 mySetting->setValue("userInfo/nom",currentUser.nom);
                 ui->vue_principal->setCurrentIndex(0);
             }else{
                 QMessageBox::information(NULL, "ApiSoft", "Erreur lors de la connexion", QMessageBox::Ok);
             }
         }
     }else{
         QMessageBox::information(NULL, "ApiSoft", "Merci d'activer au moins une modification!", QMessageBox::Ok);
     }
}



/** VUE PRINCIPAL **/
void MainWindow::rucherSelectionP(int i){
    ui->nom_2->setText(listRucher.value(ui->list_rucherP->currentData().toInt()).nom);
    ui->dateaff->setText(QString("Date de création: %1").arg(listRucher.value(ui->list_rucherP->currentData().toInt()).date_creation));
    ui->addrP->setText(listRucher.value(ui->list_rucherP->currentData().toInt()).place);
    listRuche.clear();
    ui->list_rucheP->clear();
    ui->list_rucheP->addItem("", QVariant(-1));
    rucher r = listRucher.value(ui->list_rucherP->itemData(i).toInt());
    ui->carte->updateView(r.lng, r.lat, r.place);
    QSqlQuery *q = new QSqlQuery(db);
    QString rqt1 = QString("SET @a = 0");
    QString rqt = QString("SELECT (@a:=@a+1) as num, ID, info, date_creation, temperature, pression, traffic, rucher_id FROM ruchee WHERE rucher_id = %1").arg(r.id);
   if(q->exec(rqt1)) {
        if(q->exec(rqt)){
            ruche rche;
            while(q->next()){
                rche.date_creation = q->record().value("date_creation").toString();
                rche.id = q->record().value("ID").toInt();
                rche.idrucher = q->record().value("rucher_id").toInt();
                rche.info = q->record().value("info").toString();
                rche.num = q->record().value("num").toInt();
                rche.pression = q->record().value("pression").toDouble();
                rche.temperature = q->record().value("temperature").toDouble();
                rche.traffic = q->record().value("traffic").toString();
                listRuche.insert(rche.id, rche);
                ui->list_rucheP->addItem(QString("numero %1").arg(rche.num), QVariant(rche.id));
            }
        }else{
            QMessageBox::information(NULL, "ApiSoft", "Echec lors de la tentative de connexion à la base de donnée", QMessageBox::Ok);
        }
    }

}

//UPDATE RUCHER
void MainWindow::updateRucher(){
    if(ui->list_rucherP->currentData().toInt() > 0)
    {
        QString nom, date, adresse;
        int ID = ui->list_rucherP->currentData().toInt();
        double lng, lat;
        if(!ui->nom_2->text().isEmpty()){
            if(ui->Adresse_2->isValide()){
                adresse = ui->Adresse_2->getAddress();
                lng = ui->Adresse_2->getLong();
                lat = ui->Adresse_2->getLat();

            }else{
                adresse = listRucher.value(ui->list_rucherP->currentData().toInt()).place;
                lng = listRucher.value(ui->list_rucherP->currentData().toInt()).lng;
                lat = listRucher.value(ui->list_rucherP->currentData().toInt()).lat;
            }
            date = ui->dateRucher_2->text();
            nom = ui->nom_2->text();
            QSqlQuery *q = new QSqlQuery(db);
            QString rqt = QString("UPDATE rucher SET nom='%1',date_creation=STR_TO_DATE('%2', '%d/%m/%Y'),longitude=%3,latitude=%4,place='%5' WHERE ID = %6").replace("'", "\"").arg(nom).arg(date).arg(lng).arg(lat).arg(adresse).arg(ID);
            if(q->exec(rqt)){
                QMessageBox::information(NULL, "ApiSoft", "modifications réalisées avec succès", QMessageBox::Ok);
                this->pageOpen(0);
                ui->modifProfil_2->setChecked(false);
            }else{
                QMessageBox::information(NULL, "ApiSoft", "Erreur 1", QMessageBox::Ok);
            }
        }else{
            QMessageBox::information(NULL, "ApiSoft", "Merci de remplir tous les champs!", QMessageBox::Ok);
        }
    }else{
        ui->modifProfil_2->setChecked(false);
    }
}

void MainWindow::rucheSelectionP(int i){
    modele->clear();
     ruche rche = listRuche.value(ui->list_rucheP->itemData(i).toInt());
     rucher r = listRucher.value(ui->list_rucherP->currentData().toInt());
    ui->numRuche->setText(QString("Ruche numero %1").arg(rche.num));
    ui->dateRuche->setText(QString("Date de création: %1").arg(rche.date_creation));
    ui->infoRuche->setPlainText(rche.info);
    QSqlQuery *q = new QSqlQuery(db);
    QString rqt;
    QList<QStandardItem*> newRow;
    QStandardItem* itm;
    if(rche.id > 0)
        rqt = QString("SELECT * FROM fichesuivie WHERE ruche_id = %1").arg(rche.id);
    else
        rqt = QString("SELECT f.* FROM fichesuivie f, ruchee r WHERE f.ruche_id = r.ID && r.rucher_id  = %1").arg(r.id);

    if(q->exec(rqt)){
        modele->setHorizontalHeaderLabels(this->liste);
        while(q->next()){
           itm = new QStandardItem(QString("%1").arg(q->record().value("num").toInt()));
           newRow.append(itm);
           itm = new QStandardItem(QString("%1").arg(q->record().value("Nombre_de_Hausses").toInt()));
           newRow.append(itm);
           itm = new QStandardItem(QString("%1").arg(q->record().value("nombre_de_hausse_final").toInt()));
           newRow.append(itm);
           itm = new QStandardItem(QString("%1").arg(q->record().value("Traffic_entrée").toString()));
           newRow.append(itm);
           itm = new QStandardItem(QString("%1").arg(q->record().value("Attitude_des_abeilles").toString()));
           newRow.append(itm);
           itm = new QStandardItem(QString("%1").arg(q->record().value("Reine").toString()));
           newRow.append(itm);
           itm = new QStandardItem(QString("%1").arg(q->record().value("Population").toString()));
           newRow.append(itm);
           itm = new QStandardItem(QString("%1").arg(q->record().value("Symptomes_et_anomalies").toString()));
           newRow.append(itm);
           itm = new QStandardItem(QString("%1").arg(q->record().value("Traiements_effectues").toString()));
           newRow.append(itm);
            modele->appendRow(newRow);
            newRow.clear();
        }
    }
}
