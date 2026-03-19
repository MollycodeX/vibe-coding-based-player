#ifndef TRANSLATIONMANAGER_H
#define TRANSLATIONMANAGER_H

#include <QGuiApplication>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QTranslator>

class TranslationManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)

public:
    explicit TranslationManager(QGuiApplication *app, QQmlApplicationEngine *engine,
                                QObject *parent = nullptr);

    QString language() const;
    Q_INVOKABLE void setLanguage(const QString &lang);

signals:
    void languageChanged();

private:
    QGuiApplication *m_app;
    QQmlApplicationEngine *m_engine;
    QTranslator m_translator;
    QString m_language;
    bool m_translatorInstalled = false;
};

#endif // TRANSLATIONMANAGER_H
