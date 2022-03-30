#ifndef BWIDGET_H
#define BWIDGET_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QVector>
#include "tcpdatawrap.h"
#include "../mylabel.h"
#include "../cardnode.h"
QT_BEGIN_NAMESPACE
namespace Ui { class BWidget; }
QT_END_NAMESPACE

class BWidget : public QWidget
{
    Q_OBJECT

public:
    BWidget(QWidget *parent = nullptr);
    ~BWidget();

private slots:
    void on_btnReady_clicked(bool checked);

    void on_btnBeat_clicked(bool checked);

    void on_btnNotBeat_clicked(bool checked);

    void on_pushButton_clicked(bool checked);

    void on_pushButton_2_clicked(bool checked);

private:
    Ui::BWidget *ui;
    QTcpServer *cListen;
    QTcpSocket *cSocket=nullptr;
    QTcpSocket *aSocket=nullptr;
    QVector<int> bcards;
    int diPai[3];
    QString lordinfo;
    bool islord=false;
    bool isPrivileged=false;
    void dealWithA();
    void dealWithC();
    void setRole(char t);
    TcpDataWrap bwrap;

    QString setpath(int val);
    void drawCards();
    int weight[55]={0};
    //QVector<int> mycards;存储自己的手牌
    CardNode goingCards;//里面有个容器来存储准备打出的牌，里面还有一个容器存储的是打出的牌的转化形式
    CardNode preCards;
    MyLabel** cardlist;//使用label显示手牌
    MyLabel** cardlist2;
    MyLabel** gonelist;//在桌面上显示要出的牌
    void setUp(MyLabel*);//点击牌会上升或下降
    int goneCards=0;
    void setpreCards(QString ginfo);
    bool isillegal();
signals:
    void dataLeftC();
    void dataLeftA();
};
#endif // BWIDGET_H
