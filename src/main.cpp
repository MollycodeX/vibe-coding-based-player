#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTranslator>
#include <QLocale>
#include "PlayerController.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Multi-language support: load a .qm translation file for the system locale.
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = QStringLiteral("vibe-player_") + QLocale(locale).name();
        if (translator.load(QStringLiteral(":/i18n/") + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

    QQmlApplicationEngine engine;

    // Expose the C++ player controller to QML as "playerController".
    PlayerController controller;
    engine.rootContext()->setContextProperty(QStringLiteral("playerController"), &controller);

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
