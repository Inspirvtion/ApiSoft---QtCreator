#ifndef MAP_H
#define MAP_H

#include <QWidget>


namespace Ui {
class map;
}

class map : public QWidget
{
    Q_OBJECT

public:
    explicit map(QWidget *parent = 0);
    ~map();
public slots:
    void updateView(double lng, double lat, QString address);
private:
    Ui::map *ui;
    bool marker;
};

#endif // MAP_H
