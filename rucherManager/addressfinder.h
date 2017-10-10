#ifndef ADDRESSFINDER_H
#define ADDRESSFINDER_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace Ui {
class AddressFinder;
}

class AddressFinder : public QWidget
{
    Q_OBJECT

public:
    explicit AddressFinder(QWidget *parent = 0);
    ~AddressFinder();
    double getLong();
    double getLat();
    QString getAddress();
    bool isValide();

signals:
    void addressFinded(double lng, double lat, QString address);

private slots:
    void chercherIsClicked();
    void replyFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager *m_pNetworkAccessManager;
    double lng;
    double lat;
    QString address;
    bool valide;
    Ui::AddressFinder *ui;
};

#endif // ADDRESSFINDER_H
