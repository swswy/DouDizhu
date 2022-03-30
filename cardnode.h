#ifndef CARDNODE_H
#define CARDNODE_H
#include<QVector>

/*
WangZha 王炸    SingleCard 单牌
DoubleCard 对子    ThreeCard 三不带
BombCard 炸弹    ThreeSingleCard 三带单
ThreeDoubleCard 三带对    BombTwoCard 四带二
BombTwoDoubleCard 四带二双    ConnectCard 连子
CompanyCard 连队    PlaneCard 飞机
PlaneSingleCard 飞机带单牌    PlaneDoubelCard 飞机带对子
ErrorCard 错误
*/
class CardNode
{
public:
    QVector<int> playingcards;
    QVector<int> transcards;
    QString cardsType="";
    CardNode();
    void setType();
    void isClear();
    int getCardValue(int v);
    //写一个可以判断类型的函数
    //重载一下比较运算符
    //
};

#endif // CARDNODE_H
