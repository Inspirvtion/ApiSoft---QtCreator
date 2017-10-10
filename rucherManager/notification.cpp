#include "notification.h"
#include <QDate>
#include <QtSql>
Notification::Notification(QSqlDatabase db2,QObject *parent, int lastId, int contactId): QThread(parent)
{
    db = db2;
    db.open();
    this->lastNotifId = lastId;
    this->contact = contactId;
}

Notification::~Notification()
{

}

void Notification::run(){
    int max = this->lastNotifId, a;
    QString rqt;
        if(db.isOpen()){
            rqt = QString("SELECT * FROM notification WHERE ID >= %1 AND date_envoie <= NOW() AND contact_id = %2 ORDER BY `date_envoie`").arg(max).arg(this->contact);
            QSqlQuery *q = new QSqlQuery(db);
            if(q->exec(rqt)){
                while(q->next()){
                    a = q->record().value("ID").toInt();
                    emit newNotification(a, q->record().value("date_creation").toString(), q->record().value("date_envoie").toString(), q->record().value("message").toString());
                qDebug() << q->record().value("message").toString();
                    if(a>= max) max = a;
                }
            }

        }
        QObject().thread()->sleep(10);
    }
