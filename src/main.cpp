#include <QGuiApplication>
#include <QLocale>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include "PlayerController.h"
#include "TranslationManager.h"

int main(int argc, char *argv[])
{
    // Fix: If 'basic' render loop doesn't solve window dragging stutters, it's highly likely
    // a Windows Direct3D 11 swapchain lock issue. We force the RHI backend to use OpenGL.
    qputenv("QSG_RHI_BACKEND", "opengl");

    // Upgrade: Adopt the Material UI design engine. Material provides native dark/light theme 
    // switching and beautifully animated controls, fixing the inverted Slider backgrounds 
    // present in the fallback "Basic" or ancient OS-specific styles.
    qputenv("QT_QUICK_CONTROLS_STYLE", "Material");

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // Runtime-switchable translation manager exposed to QML.
    TranslationManager translationManager(&app, &engine);

    // Auto-detect system language: default to Chinese if the system locale is zh.
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        if (locale.startsWith(QStringLiteral("zh"))) {
            translationManager.setLanguage(QStringLiteral("zh"));
            break;
        }
    }

    // Expose the C++ player controller to QML as "playerController".
    PlayerController controller;
    engine.rootContext()->setContextProperty(QStringLiteral("playerController"), &controller);
    engine.rootContext()->setContextProperty(QStringLiteral("translationManager"),
                                            &translationManager);

    // Load the QML UI.
    const QUrl url(QStringLiteral("qrc:/VibePlayer/main.qml"));
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
