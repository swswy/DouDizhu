#include "cardnode.h"
#include <algorithm>
CardNode::CardNode()
{

}
void CardNode::setType()
{
    int frequency[16]={0};
    for(int i=0;i<playingcards.size();++i){
        transcards.append(getCardValue(playingcards[i]));
        ++frequency[transcards[i]];
    }
    int weight[16]={0};
    for(int i=1;i<14;++i){
        weight[i]=i-2;
        if(weight[i]<=0)
            weight[i]+=13;
    }
    weight[14]=14;
    weight[15]=15;
    std::sort(transcards.begin(),transcards.end(),[&](int a,int b){
        if(frequency[a]>frequency[b]){
            return  true;
        }else if(frequency[a]==frequency[b]) {
            return weight[a]<weight[b];
        }
        else
            return false;
    });
    int t=transcards.size();
    if(t==2&&transcards[0]==14&&transcards[1]==15){
        cardsType="WangZha";
        return;
    }
    if(t<5){
        if(transcards[0]==transcards[t-1]){
            switch (t) {
            case 1:
                cardsType="SingleCard";
                break;
            case 2:
                cardsType="DoubleCard";
                break;
            case 3:
                cardsType="ThreeCard";
                break;
            case 4:
                cardsType="BombCard";
                break;
            }
            return;
        }
        if(transcards[0]==transcards[t-2]&&t==4){
            cardsType="ThreeSingleCard";
            return;
        }
    }
    QVector<int> vone;
    QVector<int> vtwo;
    QVector<int> vthree;
    QVector<int> vfour;
    for(int i=0;i<t;++i){
        int ttem=transcards[i];
        if(frequency[ttem]==1&& !vone.contains(ttem))
            vone.append(ttem);
        else if(frequency[ttem]==2&& !vtwo.contains(ttem))
            vtwo.append(ttem);
        else if(frequency[ttem]==3&& !vthree.contains(ttem))
            vthree.append(ttem);
        else if(!vfour.contains(ttem))
            vfour.append(ttem);
    }
    if(t==5&& vthree.size()==1 && vtwo.size()==1){
        cardsType="ThreeDoubleCard";
        return;
    }
    if(t==6&&vfour.size()==1){
        cardsType="BombTwoCard";
        return;
    }
    if(t==8&&vfour.size()==1&&vtwo.size()==2){
        cardsType="BombTwoDoubleCard";
        return;
    }
    if(t>=5&&vone.size()==t&&(weight[transcards[t-1]]-weight[transcards[0]])==(t-1)
            &&!vone.contains(2)&&!vone.contains(14)&&!vone.contains(15)){
        cardsType="ConnectCard";
        return;
    }
    if((t/2>=3)&&(t%2==0)&&(vtwo.size()==t/2)&&(weight[transcards[t-1]]-weight[transcards[0]])==(t/2-1)
            &&!vtwo.contains(2)&&!vone.contains(14)&&!vone.contains(15)){
        cardsType="CompanyCard";
        return;
    }
    if(vthree.size()==t/3&&(t%3==0)&&(weight[transcards[t-1]]-weight[transcards[0]])==(t/3-1)
             &&!vtwo.contains(2)&&!vone.contains(14)&&!vone.contains(15)){
        cardsType="PlaneCard";
        return;
    }
    if(vthree.size()>=2 && vthree.size()==t/4&& (vthree.back()-vthree[0])==(t/4-1)){
        cardsType="PlaneSingleCard";
        return;
    }
    if(vthree.size()>=2&&vthree.size()==t/5&&(vthree.back()-vthree[0])==(t/5-1)){
        cardsType="PlaneDoubelCard";
        return;
    }
    cardsType="ErrorCard";
    return;
}

void CardNode::isClear()
{
    if(cardsType!=""){
        playingcards.clear();
        transcards.clear();
        cardsType="";
    }
    return;
}

int CardNode::getCardValue(int v)
{
    if(v>=53) return v-39;
    int t = (v-1) % 13;
    //小于3为 A  2
    return t+1;
}
