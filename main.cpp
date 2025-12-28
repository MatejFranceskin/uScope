#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Application metadata
    QApplication::setOrganizationName("uScope");
    QApplication::setOrganizationDomain("uscope.org");
    QApplication::setApplicationName("uScope");
    QApplication::setApplicationVersion("0.1.0");
    
    MainWindow window;
    window.show();
    
    return app.exec();
}
