#include "map.h"
#include "ui_map.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QWebFrame>
#include <QWebElement>
#include <QDebug>

map::map(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::map)
{
    ui->setupUi(this);
    QString MyAppDirPath =  QCoreApplication::applicationDirPath();
    ui->g_map->setUrl(QUrl("file:///"+ MyAppDirPath +"/map.html"));
    marker = false;
}

map::~map()
{
    delete ui;
}

void map::updateView(double lng, double lat, QString address){
    if(marker)
    {
        QString str = QString("marker.setMap(null);");
        qDebug() << str;
        ui->g_map->page()->currentFrame()->documentElement().evaluateJavaScript(str);
    }
    QString str = QString("var newLoc = new google.maps.LatLng(%1, %2);").arg(lat).arg(lng);
    str += QString("map.setCenter(newLoc);");
    str += QString("map.setZoom(%1);").arg(12);
    ui->g_map->page()->currentFrame()->documentElement().evaluateJavaScript(str);
    QString str2 = QString("var marker = new google.maps.Marker({") +QString("position: new google.maps.LatLng(%1, %2),").arg(lat).arg(lng) +QString("map: map,")+QString("title: %1").arg("\""+address+"\"")+QString("});") + QString("markers.push(marker);");
    ui->g_map->page()->currentFrame()->documentElement().evaluateJavaScript(str2);
    marker = true;
}
