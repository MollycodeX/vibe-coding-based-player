#include "TranslationManager.h"

TranslationManager::TranslationManager(QGuiApplication *app, QQmlApplicationEngine *engine,
                                       QObject *parent)
    : QObject(parent), m_app(app), m_engine(engine), m_language(QStringLiteral("en"))
{
}

QString TranslationManager::language() const
{
    return m_language;
}

void TranslationManager::setLanguage(const QString &lang)
{
    if (m_language == lang)
        return;

    m_language = lang;

    if (m_translatorInstalled) {
        m_app->removeTranslator(&m_translator);
        m_translatorInstalled = false;
    }

    if (lang == QStringLiteral("zh")) {
        if (m_translator.load(QStringLiteral(":/i18n/vibe-player_zh_CN"))) {
            m_app->installTranslator(&m_translator);
            m_translatorInstalled = true;
        }
    }

    m_engine->retranslate();
    emit languageChanged();
}
