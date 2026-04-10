// AudioFingerprinter.h
// Computes audio fingerprints by invoking the Chromaprint fpcalc command-line
// tool. The resulting fingerprint + duration pair can be submitted to the
// AcoustID service to identify a recording even when no metadata is present.

#ifndef AUDIOFINGERPRINTER_H
#define AUDIOFINGERPRINTER_H

#include <QObject>
#include <QProcess>
#include <QString>

class AudioFingerprinter : public QObject {
    Q_OBJECT

public:
    explicit AudioFingerprinter(QObject *parent = nullptr);

    /// Start an asynchronous fingerprint computation for the given audio file.
    void compute(const QString &filePath);

    /// Returns true if the fpcalc executable is found on the system PATH.
    static bool isAvailable();

    /// Parses the key=value output of fpcalc.
    /// Returns true on success and populates duration and fingerprint.
    static bool parseFpcalcOutput(const QByteArray &output, int &duration,
                                  QString &fingerprint);

signals:
    /// Emitted when the fingerprint has been computed successfully.
    void fingerprintReady(int duration, const QString &fingerprint);
    /// Emitted when fingerprint computation fails.
    void fingerprintFailed(const QString &errorMessage);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QProcess m_process;
};

#endif // AUDIOFINGERPRINTER_H
