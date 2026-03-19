#include <QGuiApplication>
#include <QLocale>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "PlayerController.h"
#include "TranslationManager.h"

int main(int argc, char *argv[])
{
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
