#include "../src/ComposantPrincipal.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ComposantPrincipal w;
    w.show();

    return a.exec();
}
