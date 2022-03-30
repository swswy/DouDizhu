#include "cwidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CWidget w;
    w.show();
    return a.exec();
}
