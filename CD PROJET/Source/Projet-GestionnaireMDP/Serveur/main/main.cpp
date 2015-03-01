#include <QCoreApplication>
#include "ComposantPrincipal.h"
#include "ComposantCryptographie.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ComposantPrincipal cp(argc, argv);

    return a.exec();
}
