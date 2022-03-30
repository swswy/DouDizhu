#include "mylabel.h"

MyLabel::MyLabel(QWidget *parent) : QLabel(parent)
{

}
void MyLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    emit(clicked(this));
}
