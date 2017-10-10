#ifndef CONNEXION_H
#define CONNEXION_H

#include <QWidget>

namespace Ui {
class Connexion;
}

class Connexion : public QWidget
{
    Q_OBJECT

public:
    explicit Connexion(QWidget *parent = 0);
    ~Connexion();

signals:
    void connected(QString login, QString psswd, bool b);
    void signup(QString nom, QString address, QString mobile, QString mail, QString login, QString pass, QString pass2);
private slots:
    void connectMe();
    void signMeup();
    void finded(double,double,QString);
private:
    Ui::Connexion *ui;
    QString addr;
};

#endif // CONNEXION_H
