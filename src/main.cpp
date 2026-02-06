#include "application.h"
#include <QDebug>
#include <QQuickStyle>

int main(int argc, char* argv[])
{
    // Use Basic style to allow component customization (background, contentItem)
    // Native styles (Windows, macOS) don't support these customizations
    QQuickStyle::setStyle("Basic");
    
    mpf::Application app(argc, argv);
    
    if (!app.initialize()) {
        qCritical() << "Failed to initialize application";
        return 1;
    }
    
    return app.run();
}
