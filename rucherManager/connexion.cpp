#include "connexion.h"
#include "ui_connexion.h"
#include <QMessageBox>

Connexion::Connexion(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Connexion)
{
    ui->setupUi(this);
    addr = "aucune";
    connect(ui->connexion, SIGNAL(clicked()), this,SLOT(connectMe()));
    connect(ui->login, SIGNAL(returnPressed()), this,SLOT(connectMe()));
    connect(ui->pass, SIGNAL(returnPressed()), this,SLOT(connectMe()));
    connect(ui->inscription, SIGNAL(clicked()), this, SLOT(signMeup()));
    connect(ui->Adresse, SIGNAL(addressFinded(double,double,QString)), this, SLOT(finded(double,double,QString)));
}

Connexion::~Connexion()
{
    delete ui;
}


void Connexion::connectMe(){
    emit connected(ui->login->text(), ui->pass->text(), ui->stay->isChecked());
}

void Connexion::signMeup(){
    emit signup(ui->nom->text(), addr, ui->mobile->text(),ui->mail->text(), ui->login_2->text(), ui->password->text(), ui->password2->text());
}

void Connexion::finded(double lng,double lat,QString s){
    addr = s;
}
