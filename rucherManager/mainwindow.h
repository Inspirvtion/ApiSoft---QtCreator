#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "connexion.h"
#include <QSettings>
#include "initialisation.h"
#include <QVector>
#include <QTimer>
#include <QtSql>
#include <QMap>
#include <QStandardItemModel>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void connectMe(QString log, QString psswd, bool stay);
    void deconnectMe();
    void signMeup(QString nom, QString address, QString mobile, QString mail, QString login, QString pass, QString pass2);
    void actionfichedesuivi();
    void actionAddRucher();
    void actionAddRuche();
    void actionMonProfil();
    void lookNotif();
    void affNotif(QModelIndex i);
    void addRucher();
    void addRuche();
    void backToPrincipal();
    void pageOpen(int p);
    void carteFicheSuivie(int i);
    void addFicheSuivie();
    void carteRuche(int i);
    void modifierProfil();
    void rucherSelectionP(int i);
    void updateRucher();
    void rucheSelectionP(int i);


private:
    Ui::MainWindow *ui;
    QSettings *mySetting;
    QSqlDatabase db;
    utilisateur currentUser;
    QVector<note> notes;
    QTimer *notif;
    QStringList listNotif;
    QStringListModel *modelListNotif;
    QMap<int, rucher> listRucher;
    QMap<int, ruche> listRuche;
    QStandardItemModel *modele;
    QStringList liste;
};

#endif // MAINWINDOW_H
