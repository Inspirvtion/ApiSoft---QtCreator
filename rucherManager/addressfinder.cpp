#include "addressfinder.h"
#include "ui_addressfinder.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QWebFrame>
#include <QWebElement>

AddressFinder::AddressFinder(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddressFinder)
{
    ui->setupUi(this);
    valide = false;
    m_pNetworkAccessManager = new QNetworkAccessManager(this);
    connect(m_pNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
}

AddressFinder::~AddressFinder()
{
    delete ui;
}

void AddressFinder::replyFinished(QNetworkReply* reply){
    QString strReply =  (QString)reply->readAll();

    QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());
    QJsonObject jsonObj = jsonResponse.object();
    if(jsonObj["status"].toString().toLower() == "ok"){
        if(jsonObj["results"].toArray().count() > 0){
            address = jsonObj["results"].toArray().toVariantList()[0].toMap()["formatted_address"].toString();
            lng =  jsonObj["results"].toArray().toVariantList()[0].toMap()["geometry"].toMap()["location"].toMap()["lng"].toDouble();
            lat = jsonObj["results"].toArray().toVariantList()[0].toMap()["geometry"].toMap()["location"].toMap()["lat"].toDouble();
            ui->resultatRech->setText(address);
            valide = true;
            emit addressFinded(lng, lat, address);
        }else{
            ui->resultatRech->setText("Aucune adresse trouvée...");
            valide = false;
            }
    }else{
        valide = false;
        ui->resultatRech->setText("Aucune adresse trouvée...");
    }
}

void AddressFinder::chercherIsClicked(){
    QString url = QString("http://maps.google.com/maps/api/geocode/json?address=%1&sensor=false&language=fr").arg(ui->adresse->text().replace(" ", "+"));
    m_pNetworkAccessManager->get(QNetworkRequest(QUrl(url)));
}



double AddressFinder::getLong(){
    return lng;
}

double AddressFinder::getLat(){
    return lat;
}

QString AddressFinder::getAddress(){
    return address;
}

bool AddressFinder::isValide(){
    return valide;
}
