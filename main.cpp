#include "src/ui/imagecontrolpanel.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setWindowIcon(QIcon(":/config/icon.png"));


    ImageControlPanel w;
    w.show();
    return a.exec();
}
