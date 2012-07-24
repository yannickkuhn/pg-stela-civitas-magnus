#include <QtGui/QApplication>
#include "fenetre.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    fenetre w;
    if(w.TestIfDllAndExeExists() == false)
        return 0;
    w.show();
    return a.exec();
}
