#include "cwidget.h"
#include "ui_cwidget.h"
#include <QHostAddress>
#include <QDebug>
#include <QMessageBox>
CWidget::CWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CWidget)
{
    //处理ui
    ui->setupUi(this);
    this->ui->stackedWidget->setCurrentIndex(0);
    setFixedSize(1000,650);
    this->ui->stackedWidget->setFixedSize(1000,650);
    ui->btnBeat->hide();ui->btnNotBeat->hide();
    ui->btnForLord->hide();ui->btnNotLord->hide();
    //绑定对应套接字
    aSocket=new QTcpSocket(this);
    bSocket=new QTcpSocket(this);
    bool isSuccess[2];
    isSuccess[0]=false; isSuccess[1]=false;
    connect(aSocket,&QTcpSocket::connected,[&](){
        isSuccess[0]=true;
        if(isSuccess[1]){
            this->ui->stackedWidget->setCurrentIndex(1);
        }
    });
    connect(bSocket,&QTcpSocket::connected,[&](){
        isSuccess[1]=true;
        if(isSuccess[0]){
            this->ui->stackedWidget->setCurrentIndex(1);
        }
    });
    connect(aSocket,&QTcpSocket::readyRead,this,&CWidget::dealWithA);
    connect(bSocket,&QTcpSocket::readyRead,this,&CWidget::dealWithB);
    //绑定按钮叫不叫功能
    connect(ui->btnForLord,&QPushButton::clicked,[=](){
        lordinfo+="C1#";
        //aSocket->write(lordinfo.toUtf8());
        cwrap.mySend(aSocket,lordinfo);
        ui->btnForLord->setVisible(false);ui->btnNotLord->setVisible(false);
    });
    connect(ui->btnNotLord,&QPushButton::clicked,[=](){
        lordinfo+="C0#";
        //aSocket->write(lordinfo.toUtf8());
        cwrap.mySend(aSocket,lordinfo);
        ui->btnForLord->setVisible(false);ui->btnNotLord->setVisible(false);
    });
    connect(this,&CWidget::dataLeftB,this,&CWidget::dealWithB);
    connect(this,&CWidget::dataLeftA,this,&CWidget::dealWithA);

    //对权值矩阵进行展示
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
CWidget::~CWidget()
{
    delete ui;
}

QString CWidget::setpath(int val)
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

void CWidget::drawCards()//显示手牌
{
    qDebug()<<"di91hang"<<goingCards.playingcards.size()<<ccards.size();
    if(ccards.size()==20){
        for(int i=0;i<17;++i){
            cardlist2[i]->deleteLater();
        }
        delete [] cardlist2;
    }
    if(goingCards.playingcards.size()!=0){
        int tempn=goingCards.playingcards.size()+ccards.size();
        for(int i=0;i<tempn;++i){
             cardlist[i]->deleteLater();
        }
        delete [] cardlist;
    }
    int n=ccards.size();
    cardlist=new MyLabel*[n];


    for(int i=0;i<n;++i){
        cardlist[i]=new MyLabel(this);
        cardlist[i]->setGeometry(210+i*25,500,100,135);
        //QString temp=setpath(ccards[i]);
        cardlist[i]->setPixmap(QPixmap(setpath(ccards[i])));
        cardlist[i]->cardval=ccards[i];
        cardlist[i]->show();
        connect(cardlist[i],&MyLabel::clicked,this,&CWidget::setUp);
    }
    cardlist2=cardlist;
    return;
}
void CWidget::on_btnReady_clicked(bool checked)
{
    aSocket->connectToHost(QHostAddress("127.0.0.1"),9617);
    bSocket->connectToHost(QHostAddress("127.0.0.1"),7777);
    this->ui->btnReady->setVisible(false);
}

void CWidget::setUp(MyLabel *upping)
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

void CWidget::setpreCards(QString ginfo)
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

bool CWidget::isillegal()
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

void CWidget::dealWithA()
{
    //QByteArray buf=aSocket->readAll();
    QString temp=cwrap.myRecv(aSocket);
    qDebug()<<"62hang"<<temp;
    if(temp.section("#",0,0)=="cards"){
        for(int i=1;i<=17;++i){
            ccards.append(temp.section("#",i,i).toInt());
            qDebug()<<ccards[i-1];
        }
        for(int i=18;i<21;++i){
            diPai[i-18]=temp.section("#",i,i).toInt();
        }
        std::sort(ccards.begin(),ccards.end(),[&](int x,int y){
            if(weight[x]<weight[y])
                return true;
            else if(weight[x]==weight[y])
                return ((x-1)/13) < ((y-1)/13);
            else
                return false;
        });
        drawCards();
        //return;
    }
    if(temp=="StartLord#"){
        lordinfo=temp;
        ui->btnForLord->setVisible(true);ui->btnNotLord->setVisible(true);
        //return;
    }
    if(temp.section("#",0,0)=="Lord"){
        setRole(temp.back().toLatin1());
        //return;
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
        ui->aCount->setNum(tcount);
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
        aSocket->disconnectFromHost();
        bSocket->disconnectFromHost();
    }
    if(aSocket->bytesAvailable()) emit(dataLeftA());
    return;
}

void CWidget::dealWithB()
{
    //QByteArray buf=bSocket->readAll();
    QString temp=cwrap.myRecv(bSocket);
    if(temp.section("#",0,0)=="StartLord"){
        qDebug()<<temp;
        lordinfo=temp;
        QStringList list = temp.split('#', QString::SkipEmptyParts);
        if(list.size()==4){
            //循环了一轮结束
            //处理结果，找出谁应该是地主
            //向AB广播谁是lord
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
                tolord+="C";
                setRole('C');
            }
            //bSocket->write(tolord.toUtf8());
            //aSocket->write(tolord.toUtf8());
            cwrap.mySend(bSocket,tolord);
            cwrap.mySend(aSocket,tolord);
            return;
        }
       ui->btnForLord->setVisible(true);ui->btnNotLord->setVisible(true);
       //return;
    }
    if(temp.section("#",0,0)=="Lord"){
        setRole(temp.back().toLatin1());
        //return;
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

        //QChar name=temp.section("!",0,0).back();
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
        aSocket->disconnectFromHost();
        bSocket->disconnectFromHost();
    }
    if(bSocket->bytesAvailable()) emit(dataLeftB());
    return;
}

void CWidget::setRole(char t)
{
    switch(t){
    case 'A':
        ui->aRole->setText("地主");ui->aCount->setNum(20);
        ui->bRole->setText("农民");ui->bCount->setNum(17);
        ui->cRole->setText("农民");ui->cCount->setNum(17);
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
        isPrivileged=true;
        islord=true;
        ui->btnBeat->setVisible(true);
        ui->btnNotBeat->setVisible(true);
        for (int i=0;i<3;++i) {
            ccards.append(diPai[i]);
        }
        //重新显示一下手牌
        std::sort(ccards.begin(),ccards.end(),[&](int x,int y){
            if(weight[x]<weight[y])
                return true;
            else if(weight[x]==weight[y])
                return ((x-1)/13) < ((y-1)/13);
            else
                return false;
        });
        drawCards();
        break;
    }
    ui->bottomL->setPixmap(QPixmap(setpath(diPai[0])));
    ui->bottomM->setPixmap(QPixmap(setpath(diPai[1])));
    ui->bottomR->setPixmap(QPixmap(setpath(diPai[2])));
    return;
}

void CWidget::on_btnBeat_clicked(bool checked)
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
     //把pre给隐藏了------------
    ui->label->setNum(0);
    goneCards=goingCards.playingcards.size();
    qDebug()<<"di349hang"<<goneCards;
    gonelist=new MyLabel*[goneCards];
    //QString path="E:\\myproject\\qt\\cards\\cards\\";
     for(int i=0;i<goneCards;++i){
         gonelist[i]=new MyLabel(this);
         gonelist[i]->setGeometry(300+i*25,200,100,135);
         //QString temp=path+setpath(goingCards.playingcards[i]);
         gonelist[i]->setPixmap(QPixmap(setpath(goingCards.playingcards[i])));
         gonelist[i]->cardval=goingCards.playingcards[i];
         gonelist[i]->show();
         ccards.removeOne(goingCards.playingcards[i]);
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
     int tcount=ccards.size();
     toX+=("C!"+QString::number(tcount));
     toS+=("C!"+QString::number(tcount));
     cwrap.mySend(aSocket,toX);
     cwrap.mySend(bSocket,toS);
     isPrivileged=false;
     if(tcount==0){
         this->ui->stackedWidget->setCurrentIndex(2);
         this->ui->label_2->setText("YOU WIN!");
         if(islord==true){
             cwrap.mySend(aSocket,"GameOver#1");
             cwrap.mySend(bSocket,"GameOver#1");
         }else{
             cwrap.mySend(aSocket,"GameOver#0");
             cwrap.mySend(bSocket,"GameOver#0");
         }
     }
     ui->btnBeat->hide();
     ui->btnNotBeat->hide();

     drawCards();
}

void CWidget::on_btnNotBeat_clicked(bool checked)
{
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
    int tcount=ccards.size();
    toX+=("C!"+QString::number(tcount));
    cwrap.mySend(aSocket,toX);
    ui->btnBeat->hide();
    ui->btnNotBeat->hide();
}

void CWidget::on_pushButton_clicked(bool checked)
{
    aSocket->connectToHost(QHostAddress("127.0.0.1"),9617);
    bSocket->connectToHost(QHostAddress("127.0.0.1"),7777);
    //this->ui->btnReady->setVisible(false);
}

void CWidget::on_pushButton_2_clicked(bool checked)
{
    this->close();
}
