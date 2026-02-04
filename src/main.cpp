#include "application.h"
#include <QDebug>

int main(int argc, char* argv[])
{
    mpf::Application app(argc, argv);
    
    if (!app.initialize()) {
        qCritical() << "Failed to initialize application";
        return 1;
    }
    
    return app.run();
}
