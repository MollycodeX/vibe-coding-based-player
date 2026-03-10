// MetadataWriter.h
// Writes metadata (title, artist, album) into audio files.
// When TagLib is available at build time the tags are written natively.
// Otherwise the class compiles as a stub that always returns false.

#ifndef METADATAWRITER_H
#define METADATAWRITER_H

#include <QObject>
#include <QString>

class MetadataWriter : public QObject {
    Q_OBJECT

public:
    explicit MetadataWriter(QObject *parent = nullptr);

    /// Write the given metadata into the audio file at filePath.
    /// Returns true on success.
    Q_INVOKABLE bool write(const QString &filePath, const QString &title,
                           const QString &artist, const QString &album);

    /// Returns true if metadata writing is supported in this build.
    static bool isSupported();

signals:
    void writeSucceeded(const QString &filePath);
    void writeFailed(const QString &filePath, const QString &errorMessage);
};

#endif // METADATAWRITER_H
