#ifndef CWIDGET_H
#define CWIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QVector>
#include "tcpdatawrap.h"
#include "../mylabel.h"
#include "../cardnode.h"
QT_BEGIN_NAMESPACE
namespace Ui { class CWidget; }
QT_END_NAMESPACE

class CWidget : public QWidget
{
    Q_OBJECT

public:
    CWidget(QWidget *parent = nullptr);
    ~CWidget();
    QString setpath(int val);
    void drawCards();

private slots:
    void on_btnReady_clicked(bool checked);

    void on_btnBeat_clicked(bool checked);

    void on_btnNotBeat_clicked(bool checked);

    void on_pushButton_clicked(bool checked);

    void on_pushButton_2_clicked(bool checked);

private:
    Ui::CWidget *ui;
    QVector<int> ccards;//存储c中的手牌
    int diPai[3];
    QString lordinfo;
    QTcpSocket* aSocket=nullptr;
    QTcpSocket* bSocket=nullptr;
    bool isPrivileged=false;
    bool islord=false;//记录是否地主

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

    void dealWithA();
    void dealWithB();
    void setRole(char t);
    TcpDataWrap cwrap;
signals:
    void dataLeftB();
    void dataLeftA();
};
#endif // CWIDGET_H
