#ifndef MYLABEL_H
#define MYLABEL_H

#include <QLabel>

class MyLabel : public QLabel
{
    Q_OBJECT
public:
    explicit MyLabel(QWidget *parent = nullptr);
    int cardval;
    bool isup=true;
protected:
    virtual void mouseReleaseEvent(QMouseEvent * ev);

signals:
    void clicked(MyLabel* click);

};

#endif // MYLABEL_H
