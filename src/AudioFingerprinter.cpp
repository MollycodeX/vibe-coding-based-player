// AudioFingerprinter.cpp
// Spawns the Chromaprint fpcalc tool to compute an audio fingerprint.

#include "AudioFingerprinter.h"
#include <QStandardPaths>

AudioFingerprinter::AudioFingerprinter(QObject *parent)
    : QObject(parent)
{
    connect(&m_process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            &AudioFingerprinter::onProcessFinished);
}

void AudioFingerprinter::compute(const QString &filePath)
{
    if (filePath.isEmpty()) {
        emit fingerprintFailed(QStringLiteral("Empty file path"));
        return;
    }

    if (!isAvailable()) {
        emit fingerprintFailed(QStringLiteral("fpcalc not found on system PATH"));
        return;
    }

    // fpcalc outputs DURATION=<seconds>\nFINGERPRINT=<base64>
    m_process.start(QStringLiteral("fpcalc"), {filePath});
}

bool AudioFingerprinter::isAvailable()
{
    return !QStandardPaths::findExecutable(QStringLiteral("fpcalc")).isEmpty();
}

bool AudioFingerprinter::parseFpcalcOutput(const QByteArray &output, int &duration,
                                           QString &fingerprint)
{
    duration = 0;
    fingerprint.clear();

    const QList<QByteArray> lines = output.split('\n');
    for (const QByteArray &line : lines) {
        if (line.startsWith("DURATION=")) {
            bool ok = false;
            duration = line.mid(9).trimmed().toInt(&ok);
            if (!ok)
                return false;
        } else if (line.startsWith("FINGERPRINT=")) {
            fingerprint = QString::fromUtf8(line.mid(12).trimmed());
        }
    }

    return duration > 0 && !fingerprint.isEmpty();
}

void AudioFingerprinter::onProcessFinished(int exitCode,
                                           QProcess::ExitStatus exitStatus)
{
    if (exitStatus != QProcess::NormalExit || exitCode != 0) {
        emit fingerprintFailed(
            QStringLiteral("fpcalc exited with code %1").arg(exitCode));
        return;
    }

    QByteArray output = m_process.readAllStandardOutput();
    int duration = 0;
    QString fingerprint;
    if (parseFpcalcOutput(output, duration, fingerprint)) {
        emit fingerprintReady(duration, fingerprint);
    } else {
        emit fingerprintFailed(QStringLiteral("Failed to parse fpcalc output"));
    }
}
