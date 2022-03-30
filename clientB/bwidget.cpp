#include "bwidget.h"
#include "ui_bwidget.h"
#include <QDebug>
#include <QMessageBox>
BWidget::BWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BWidget)
{
    //处理ui
    ui->setupUi(this);
    this->ui->stackedWidget->setCurrentIndex(0);
    setFixedSize(1000,650);
    this->ui->stackedWidget->setFixedSize(1000,650);
    ui->btnBeat->hide();ui->btnNotBeat->hide();
    ui->btnForLord->hide();ui->btnNotLord->hide();

    //绑定套接字
    aSocket=new QTcpSocket(this);
    cListen=new QTcpServer(this);
    connect(cListen,&QTcpServer::newConnection,[=](){
        cSocket=cListen->nextPendingConnection();
        if(cSocket!=nullptr){
            this->ui->stackedWidget->setCurrentIndex(1);
            connect(cSocket,&QTcpSocket::readyRead,this,&BWidget::dealWithC);
        }
    });
    connect(ui->btnForLord,&QPushButton::clicked,[=](){
        lordinfo+="B1#";
        //cSocket->write(lordinfo.toUtf8());
        bwrap.mySend(cSocket,lordinfo);
        ui->btnForLord->setVisible(false);ui->btnNotLord->setVisible(false);
    });
    connect(ui->btnNotLord,&QPushButton::clicked,[=](){
        lordinfo+="B0#";
        //cSocket->write(lordinfo.toUtf8());
        bwrap.mySend(cSocket,lordinfo);
        ui->btnForLord->setVisible(false);ui->btnNotLord->setVisible(false);
    });
    connect(aSocket,&QTcpSocket::readyRead,this,&BWidget::dealWithA);
    connect(this,&BWidget::dataLeftC,this,&BWidget::dealWithC);
    connect(this,&BWidget::dataLeftA,this,&BWidget::dealWithA);

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

BWidget::~BWidget()
{
    delete ui;
}
QString BWidget::setpath(int val)
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

void BWidget::drawCards()//显示手牌
{
    qDebug()<<"di91hang"<<goingCards.playingcards.size()<<bcards.size();
    if(bcards.size()==20){
        for(int i=0;i<17;++i){
            cardlist2[i]->deleteLater();
        }
        delete [] cardlist2;
    }
    if(goingCards.playingcards.size()!=0){
        int tempn=goingCards.playingcards.size()+bcards.size();
        for(int i=0;i<tempn;++i){
             cardlist[i]->deleteLater();
        }
        delete [] cardlist;
    }
    int n=bcards.size();
    cardlist=new MyLabel*[n];

    for(int i=0;i<n;++i){
        cardlist[i]=new MyLabel(this);
        cardlist[i]->setGeometry(210+i*25,500,100,135);
        //QString temp=setpath(ccards[i]);
        cardlist[i]->setPixmap(QPixmap(setpath(bcards[i])));
        cardlist[i]->cardval=bcards[i];
        cardlist[i]->show();
        connect(cardlist[i],&MyLabel::clicked,this,&BWidget::setUp);
    }
    cardlist2=cardlist;
    return;
}
void BWidget::setUp(MyLabel *upping)
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
void BWidget::setpreCards(QString ginfo)
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
bool BWidget::isillegal()
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

void BWidget::on_btnReady_clicked(bool checked)
{
    cListen->listen(QHostAddress::Any,7777);
    aSocket->connectToHost(QHostAddress("127.0.0.1"),9617);
    this->ui->btnReady->setVisible(false);
}

void BWidget::dealWithA()
{
    //QByteArray buf=aSocket->readAl();
    QString temp=bwrap.myRecv(aSocket);
    //获取牌
    if(temp.section("#",0,0)=="cards"){
        for(int i=1;i<=17;++i){
            bcards.append(temp.section("#",i,i).toInt());
            qDebug()<<bcards[i-1];
        }
        for(int i=18;i<21;++i){
            diPai[i-18]=temp.section("#",i,i).toInt();
        }
        std::sort(bcards.begin(),bcards.end(),[&](int x,int y){
            if(weight[x]<weight[y])
                return true;
            else if(weight[x]==weight[y])
                return ((x-1)/13) < ((y-1)/13);
            else
                return false;
        });
        drawCards();
        return;
    }
    //叫不叫地主的情况
    if(temp.section("#",0,0)=="StartLord"){
        qDebug()<<temp;
        lordinfo=temp;

        QStringList list = temp.split('#', QString::SkipEmptyParts);
        if(list.size()==4){
            //循环了一轮结束
            //处理结果，找出谁应该是地主
            //向AC广播谁是lord
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

                tolord+="B";
                setRole('B');
            }
            //aSocket->write(tolord.toUtf8());
            //cSocket->write(tolord.toUtf8());
            bwrap.mySend(aSocket,tolord);
            bwrap.mySend(cSocket,tolord);
            return;
        }
       ui->btnForLord->setVisible(true);ui->btnNotLord->setVisible(true);
       //return;
    }
    //被广播告知地主是谁的情况，这种情况下还可能是c告知的
    if(temp.section("#",0,0)=="Lord"){
        setRole(temp.back().toLatin1());
        //return;
    }
    //传牌操作开始
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
    }
    if(aSocket->bytesAvailable()) emit(dataLeftA());
    return;
}

void BWidget::dealWithC()
{
    //QByteArray buf=aSocket->readAll();
    QString temp=bwrap.myRecv(cSocket);
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
        aSocket->disconnectFromHost();

    }
    if(cSocket->bytesAvailable()) emit(dataLeftC());
    return;
}

void BWidget::setRole(char t)
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
        isPrivileged=true;
        islord=true;
        ui->btnBeat->setVisible(true);
        ui->btnNotBeat->setVisible(true);
        for (int i=0;i<3;++i) {
            bcards.append(diPai[i]);
        }
        std::sort(bcards.begin(),bcards.end(),[&](int x,int y){
            if(weight[x]<weight[y])
                return true;
            else if(weight[x]==weight[y])
                return ((x-1)/13) < ((y-1)/13);
            else
                return false;
        });
        drawCards();
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

void BWidget::on_btnBeat_clicked(bool checked)
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
         bcards.removeOne(goingCards.playingcards[i]);
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
     int tcount=bcards.size();
     toX+=("B!"+QString::number(tcount));
     toS+=("B!"+QString::number(tcount));
     bwrap.mySend(cSocket,toX);
     bwrap.mySend(aSocket,toS);
     isPrivileged=false;
     if(tcount==0){
         this->ui->stackedWidget->setCurrentIndex(2);
         this->ui->label_2->setText("YOU WIN!");
         if(islord==true){
             bwrap.mySend(aSocket,"GameOver#1");
             bwrap.mySend(cSocket,"GameOver#1");
         }else{
             bwrap.mySend(aSocket,"GameOver#0");
             bwrap.mySend(cSocket,"GameOver#0");
         }
     }
     ui->btnBeat->hide();
     ui->btnNotBeat->hide();
     //-------------把precards给隐藏起来--------------
     ui->label->setNum(0);
     drawCards();
}

void BWidget::on_btnNotBeat_clicked(bool checked)
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
    int tcount=bcards.size();
    toX+=("B!"+QString::number(tcount));
    bwrap.mySend(cSocket,toX);
    ui->btnBeat->hide();
    ui->btnNotBeat->hide();
}

void BWidget::on_pushButton_clicked(bool checked)
{
    //cListen->listen(QHostAddress::Any,7777);
    aSocket->connectToHost(QHostAddress("127.0.0.1"),9617);
    //this->ui->btnReady->setVisible(false);
}

void BWidget::on_pushButton_2_clicked(bool checked)
{
    this->close();
}
