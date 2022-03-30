#include "widget.h"
#include "ui_widget.h"
#include <time.h>
#include <QDebug>
#include <QMessageBox>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    //处理ui
    ui->setupUi(this);
    //QWidget* page2=this->ui->stackedWidget->widget(1);
    this->ui->stackedWidget->setCurrentIndex(0);
    setFixedSize(1000,650);
    this->ui->stackedWidget->setFixedSize(1000,650);
    ui->btnBeat->hide();ui->btnNotBeat->hide();
    ui->btnForLord->hide();ui->btnNotLord->hide();
    //绑定监听套接字
    listenServer=new QTcpServer(this);
    connect(listenServer,&QTcpServer::newConnection,[=](){
       if(bSocket==nullptr){
           bSocket=listenServer->nextPendingConnection();
       }
       else{
           cSocket=listenServer->nextPendingConnection();
           if(cSocket!=nullptr){
               this->ui->stackedWidget->setCurrentIndex(1);
               connect(bSocket,&QTcpSocket::readyRead,this,&Widget::dealWithB);
               connect(cSocket,&QTcpSocket::readyRead,this,&Widget::dealWithC);
               sendCard();
                //显示各个牌面
                //开始叫地主与不叫功能
               firstLord();
           }
       }
    });
    connect(ui->btnForLord,&QPushButton::clicked,[=](){
        ui->btnForLord->setVisible(false);ui->btnNotLord->setVisible(false);
        lordinfo+="A1#";
        //qDebug()<<"37hang"<<lordinfo;
        //bSocket->write(lordinfo.toUtf8());
        awrap.mySend(bSocket,lordinfo);
    });
    connect(ui->btnNotLord,&QPushButton::clicked,[=](){
        ui->btnForLord->setVisible(false);ui->btnNotLord->setVisible(false);
        lordinfo+="A0#";
        //bSocket->write(lordinfo.toUtf8());
        awrap.mySend(bSocket,lordinfo);
    });
    connect(this,&Widget::dataLeftC,this,&Widget::dealWithC);
    connect(this,&Widget::dataLeftB,this,&Widget::dealWithB);
    for(int i=1,j=11;i<=13;++i,++j){
        if(j>=13){
            j=j%13;
        }
        weight[i]=j;
        weight[13+i]=j;
        weight[13*2+i]=j;
        weight[13*3+i]=j;
    }
    weight[53]=14;
    weight[54]=15;
    //对手牌进行排序之后展示
    //setpreCards("PRECARDS#DoubleCard#2#5#");
}

Widget::~Widget()
{
    delete ui;
}
QString Widget::setpath(int val)
{
    QString path="E:\\myproject\\qt\\cards\\cards\\";
    if(val==54){
        return (path+"RJOKER.png");
    }
    if(val==53){
        return (path+"BJOKER.png");
    }
    int a=(val-1)/13;
    int b=(val-1)%13;
    QString  result="";
    switch(a){
    case 0:
        result+="C";
        break;
    case 1:
        result+="D";
        break;
    case 2:
        result+="H";
        break;
    case 3:
        result+="S";
        break;
    }
    result+=QString::number(b+1);
    result+=".png";
    return (path+result);
}
void Widget::drawCards()//显示手牌
{
    qDebug()<<"di91hang"<<goingCards.playingcards.size()<<acards.size();
    if(acards.size()==20){
        for(int i=0;i<17;++i){
            cardlist2[i]->deleteLater();
        }
        delete [] cardlist2;
    }
    if(goingCards.playingcards.size()!=0){
        int tempn=goingCards.playingcards.size()+acards.size();
        for(int i=0;i<tempn;++i){
             cardlist[i]->deleteLater();
        }
        delete [] cardlist;
    }
    int n=acards.size();
    cardlist=new MyLabel*[n];
    for(int i=0;i<n;++i){
        cardlist[i]=new MyLabel(this);
        cardlist[i]->setGeometry(210+i*25,500,100,135);
        //QString temp=setpath(ccards[i]);
        cardlist[i]->setPixmap(QPixmap(setpath(acards[i])));
        cardlist[i]->cardval=acards[i];
        cardlist[i]->show();
        connect(cardlist[i],&MyLabel::clicked,this,&Widget::setUp);
    }
    cardlist2=cardlist;
    return;
}
void Widget::setUp(MyLabel *upping)
{
    goingCards.isClear();
    if(upping->isup){
        upping->move(upping->x(),upping->y()-50);
        goingCards.playingcards.append(upping->cardval);//在这儿之前要将playingcards给清空一下，不然第二次出牌的时候就也会append上去
    }else{
        upping->move(upping->x(),upping->y()+50);
        goingCards.playingcards.removeOne(upping->cardval);
    }
    upping->isup=!upping->isup;
}
void Widget::setpreCards(QString ginfo)
{
    //PRECARDS#DoubleCard#2#3#3#
    preCards.isClear();
    preCards.cardsType=ginfo.section("#",1,1);
    int count=ginfo.section("#",2,2).toInt();
    for(int i=3;i<3+count;++i){
        preCards.transcards.append(ginfo.section("#",i,i).toInt());
    }
    for(int i=count+3;i<count*2+3;++i){
        preCards.playingcards.append(ginfo.section("#",i,i).toInt());
    }
    return;
}
bool Widget::isillegal()
{
    if(goingCards.cardsType=="ErrorCard"||preCards.cardsType=="WangZha"){
        return true;
    }
    //王炸直接出
    if(goingCards.cardsType=="WangZha"){
        return false;
    }
    //false表示可以要的起
    if((goingCards.cardsType=="BombCard"&&preCards.cardsType!="BombCard")){
        return false;
    }
    if(goingCards.cardsType!=preCards.cardsType){
        return true;
    }
    int iweight[16]={0};
    for(int i=1;i<14;++i){
        iweight[i]=i-2;
        if(iweight[i]<=0)
            iweight[i]+=13;
    }
    iweight[14]=14;
    iweight[15]=15;
    //连对以及连子
    if(goingCards.cardsType=="ConnectCard"&&preCards.cardsType=="ConnectCard"){
        if(goingCards.transcards.size()!=preCards.transcards.size()){
            return true;
        }
    }
    if(goingCards.cardsType=="CompanyCard"&&preCards.cardsType=="CompanyCard"){
        if(goingCards.transcards.size()!=preCards.transcards.size()){
            return true;
        }
    }
    if(iweight[goingCards.transcards[0]]>iweight[preCards.transcards[0]]){
        return false;
    }
    return true;
}

void Widget::sendCard()
{
    //存储54张牌的数组
    int total[54];
    //存储三个玩家的牌
    int player[3][17];
    //存储当前剩余牌的数量
    int leftNum=54;
    //随机数字
    int ranNumber;
    //随机对象
    for(int i=0;i<54;i++){
        total[i]=i+1; // 像数组添加牌
    }
    //循环发牌
    for(int i=0;i<17;i++){    //为每个人发牌
        for(int j=0;j<3;j++){
            //生成随机下标
            srand((unsigned)time(NULL));
            ranNumber=rand()%leftNum;
            player[j][i]=total[ranNumber];     //发牌
            total[ranNumber]=total[leftNum-1];    //移动已经发过的牌
            leftNum--;          //可发牌的数量减少1
        }
    }
    for(int i=0;i<17;++i){
        acards.append(player[0][i]);
        qDebug()<<player[0][i];
    }
    QString bCards("cards#"),cCards("cards#");
    for(int i=0;i<17;++i){
        bCards+=(QString::number(player[1][i])+"#");
        cCards+=(QString::number(player[2][i])+"#");
    }
    for(int i=0;i<3;++i){
        diPai[i]=total[i];
        bCards+=(QString::number(total[i])+"#");
        cCards+=(QString::number(total[i])+"#");
    }
    //bSocket->write(bCards.toUtf8());
    //cSocket->write(cCards.toUtf8());
    awrap.mySend(bSocket,bCards);
    awrap.mySend(cSocket,cCards);
    std::sort(acards.begin(),acards.end(),[&](int x,int y){
        if(weight[x]<weight[y])
            return true;
        else if(weight[x]==weight[y])
            return ((x-1)/13) < ((y-1)/13);
        else
            return false;
    });
    drawCards();
}

void Widget::firstLord()
{
    int temp=rand()%3;
    qDebug()<<"地主"<<temp;
    temp=2;
    switch (temp) {
    case 0:
        ui->btnForLord->setVisible(true);
        ui->btnNotLord->setVisible(true);
        lordinfo="StartLord#";
        break;
    case 1:
        //bSocket->write("StartLord#");
        awrap.mySend(bSocket,"StartLord#");
        break;
    case 2:
        //cSocket->write("StartLord#");
        awrap.mySend(cSocket,"StartLord#");
        break;
    }
    return;
}

void Widget::dealWithB()
{
    //QByteArray buf=cSocket->readAll();
    QString temp=awrap.myRecv(bSocket);
    if(temp.section("#",0,0)=="Lord"){
        setRole(temp.back().toLatin1());
    }
    if(temp.section("#",0,0)=="NextCards"){
        setpreCards(temp);
        //就是用pre来存储，因为此时c不可以出牌的
        //---------把pre绘制出来-------
        int tnum=0;
        for(int i=0;i<preCards.transcards.size();++i){
            tnum=tnum*10+preCards.transcards[i];
        }
        ui->label->setNum(tnum);

        int tcount=temp.section("!",1,1).toInt();
        ui->bCount->setNum(tcount);
    }
    if(temp.section("#",0,0)=="GameOver"){
        bool ttt=temp.section("#",1,1).toInt();
        QString overword;
        if(islord==ttt){
            overword="YOU WIN!";
        }else
            overword="YOU LOSE.";
        ui->stackedWidget->setCurrentIndex(2);
        ui->label_2->setText(overword);
    }
    if(bSocket->bytesAvailable()) emit(dataLeftB());
    return;
}

void Widget::dealWithC()
{
    //QByteArray buf=cSocket->readAll();
    QString temp=awrap.myRecv(cSocket);
    if(temp.section("#",0,0)=="StartLord"){
        lordinfo=temp;
        QStringList list = temp.split('#', QString::SkipEmptyParts);
        if(list.size()==4){
            //一轮结束
            //处理结果，找出谁应该是地主
            //向BC广播谁是lord
            int i;
            QString tolord="Lord#";
            for(i=3;i>0;--i){
                if(list.at(i).at(1)=="1"){
                    switch(list.at(i).at(0).toLatin1()){
                        case 'A':
                            tolord+="A";
                            setRole('A');
                            break;
                        case 'C':
                            tolord+="C";
                            setRole('C');
                            break;
                        case 'B':
                            tolord+="B";
                            setRole('B');
                            break;
                    }
                    break;
                }
            }
            if(i==0){
                tolord+="A";
                setRole('A');
            }
            //bSocket->write(tolord.toUtf8());
            //cSocket->write(tolord.toUtf8());
            awrap.mySend(bSocket,tolord);
            awrap.mySend(cSocket,tolord);
            return;
        }
       ui->btnForLord->setVisible(true);ui->btnNotLord->setVisible(true);
    }
    if(temp.section("#",0,0)=="Lord"){
        setRole(temp.back().toLatin1());
    }
    if(temp.section("#",0,0)=="PreCards"){
        ui->btnBeat->show();
        ui->btnNotBeat->show();
        setpreCards(temp);
        //----------把precards绘制出来------------
        int tnum=0;
        for(int i=0;i<preCards.transcards.size();++i){
            tnum=tnum*10+preCards.transcards[i];
        }
        ui->label->setNum(tnum);
        //------判断一下pre和going里面的牌是否相等，相等的话牌权在这里-------
        if(preCards.transcards==goingCards.transcards)
            isPrivileged=true;
        int tcount=temp.section("!",1,1).toInt();
        ui->cCount->setNum(tcount);
    }
    if(temp.section("#",0,0)=="GameOver"){
        bool ttt=temp.section("#",1,1).toInt();
        QString overword;
        if(islord==ttt){
            overword="YOU WIN!";
        }else
            overword="YOU LOSE.";
        ui->stackedWidget->setCurrentIndex(2);
        ui->label_2->setText(overword);
    }
    if(cSocket->bytesAvailable()) emit(dataLeftC());
    return;
}

void Widget::setRole(char t)
{
    switch(t){
    case 'A':
        ui->aRole->setText("地主");ui->aCount->setNum(20);
        ui->bRole->setText("农民");ui->bCount->setNum(17);
        ui->cRole->setText("农民");ui->cCount->setNum(17);
        isPrivileged=true;
        islord=true;
        ui->btnBeat->setVisible(true);
        ui->btnNotBeat->setVisible(true);
        for (int i=0;i<3;++i) {
            acards.append(diPai[i]);
        }
        std::sort(acards.begin(),acards.end(),[&](int x,int y){
            if(weight[x]<weight[y])
                return true;
            else if(weight[x]==weight[y])
                return ((x-1)/13) < ((y-1)/13);
            else
                return false;
        });
        drawCards();
        break;
    case 'B':
        ui->aRole->setText("农民");ui->aCount->setNum(17);
        ui->bRole->setText("地主");ui->bCount->setNum(20);
        ui->cRole->setText("农民");ui->cCount->setNum(17);
        break;
    case 'C':
        ui->aRole->setText("农民");ui->aCount->setNum(17);
        ui->bRole->setText("农民");ui->bCount->setNum(17);
        ui->cRole->setText("地主");ui->cCount->setNum(20);
        break;
    }
    ui->bottomL->setPixmap(QPixmap(setpath(diPai[0])));
    ui->bottomM->setPixmap(QPixmap(setpath(diPai[1])));
    ui->bottomR->setPixmap(QPixmap(setpath(diPai[2])));

    return;
}

void Widget::on_btnReady_clicked(bool checked)
{
    listenServer->listen(QHostAddress::Any,9617);
    this->ui->btnReady->setVisible(false);
}

void Widget::on_btnBeat_clicked(bool checked)
{
    //需要将打出的牌展示在中间，也需要将牌传输给其他两个人，还需要将按钮给隐藏？？？ 当接收到要出牌的信息后在显示
    //如果出的牌不合法，直接提示后return，也就是无响应
    goingCards.setType();
    if(isPrivileged==false){
        if(isillegal()){
            QMessageBox::critical(this,"注意","选择的牌不合规");
            return;
        }
    }
    if(isPrivileged==true){
        if(goingCards.cardsType=="ErrorCard"){
            QMessageBox::critical(this,"注意","选择的牌不合规");
            return;
        }
    }
    std::sort(goingCards.playingcards.begin(),goingCards.playingcards.end(),[&](int x,int y){
        if(weight[x]<weight[y])
            return true;
        else if(weight[x]==weight[y])
            return ((x-1)/13) < ((y-1)/13);
        else
            return false;
    });
    if(goneCards!=0){
        for(int i=0;i<goneCards;++i){
            gonelist[i]->deleteLater();
        }
        delete [] gonelist;
    }
    //------隐藏pre----
    ui->label->setNum(0);

    goneCards=goingCards.playingcards.size();
    gonelist=new MyLabel*[goneCards];
    for(int i=0;i<goneCards;++i){
         gonelist[i]=new MyLabel(this);
         gonelist[i]->setGeometry(300+i*25,200,100,135);
         //QString temp=path+setpath(goingCards.playingcards[i]);
         gonelist[i]->setPixmap(QPixmap(setpath(goingCards.playingcards[i])));
         gonelist[i]->cardval=goingCards.playingcards[i];
         gonelist[i]->show();
         acards.removeOne(goingCards.playingcards[i]);
    }

    QString toX="PreCards#"+goingCards.cardsType+"#"+QString::number(goneCards)+"#";
    QString toS="NextCards#"+goingCards.cardsType+"#"+QString::number(goneCards)+"#";

    for(int i=0;i<goneCards;++i){
        toX+=(QString::number(goingCards.transcards[i])+"#");
        toS+=(QString::number(goingCards.transcards[i])+"#");
    }
    for(int i=0;i<goneCards;++i){
        toX+=(QString::number(goingCards.playingcards[i])+"#");
        toS+=(QString::number(goingCards.playingcards[i])+"#");
    }
    int tcount=acards.size();
    toX+=("A!"+QString::number(tcount));
    toS+=("A!"+QString::number(tcount));
    awrap.mySend(bSocket,toX);
    awrap.mySend(cSocket,toS);
    isPrivileged=false;
     //如果可以成功打出的话，就向上家发送NextCards，向下家发送PreCards
     //之后在通信套接字中写上对应的上下家处理函数就行
     //如果打完牌后size=0，在向其它两个人发送GameOver#islord的值#
     //在上下家的处理函数中，如果islord和传过来的一样。就是you won
     //否则就是lose
     //打出牌后ispre肯定变成了假的
     //还要判断一下传过来的牌和打出去的牌如果一样的话，就ispre为true，这个放到套接字里面判断，之后只要不是错牌就可以打出
    if(tcount==0){
        this->ui->stackedWidget->setCurrentIndex(2);
        this->ui->label_2->setText("YOU WIN!");
        if(islord==true){
            awrap.mySend(bSocket,"GameOver#1");
            awrap.mySend(cSocket,"GameOver#1");
        }else{
            awrap.mySend(bSocket,"GameOver#0");
            awrap.mySend(cSocket,"GameOver#0");
        }
    }
     ui->btnBeat->hide();
     ui->btnNotBeat->hide();
     drawCards();

}

void Widget::on_btnNotBeat_clicked(bool checked)
{
    //上家传过来的牌是要放进precards里面的
    //不出直接把这个牌传出去就好
    //同时有一个ispre那个为真的话就直接return了
    if(isPrivileged==true){
        return;
    }
    int tt=preCards.transcards.size();
    QString toX="PreCards#"+preCards.cardsType+"#"+QString::number(tt)+"#";
    for (int i=0;i<tt;++i) {
        toX+=(QString::number(preCards.transcards[i])+"#");
    }
    for (int i=0;i<tt;++i) {
        toX+=(QString::number(preCards.playingcards[i])+"#");
    }
    int tcount=acards.size();
    toX+=("A!"+QString::number(tcount));
    awrap.mySend(bSocket,toX);
    ui->btnBeat->hide();
    ui->btnNotBeat->hide();
}

void Widget::on_pushButton_clicked(bool checked)
{

}

void Widget::on_pushButton_2_clicked(bool checked)
{
 this->close();
}
