// MetadataWriter.cpp
// Writes metadata tags into audio files using TagLib when available.

#include "MetadataWriter.h"

#ifdef MSCPLAYER_HAS_TAGLIB
#include <fileref.h>
#include <tag.h>
#include <tpropertymap.h>
#include <mpegfile.h>
#include <id3v2tag.h>
#include <attachedpictureframe.h>
#include <flacfile.h>
#include <flacpicture.h>
#endif

MetadataWriter::MetadataWriter(QObject *parent)
    : QObject(parent)
{
}

bool MetadataWriter::isSupported()
{
#ifdef MSCPLAYER_HAS_TAGLIB
    return true;
#else
    return false;
#endif
}

bool MetadataWriter::write(const QString &filePath, const QString &title,
                           const QString &artist, const QString &album,
                           const QByteArray &coverArt)
{
#ifdef MSCPLAYER_HAS_TAGLIB
    // TagLib::FileRef can open most common audio formats automatically.
#if defined(_WIN32)
    TagLib::FileRef f(reinterpret_cast<const wchar_t *>(filePath.utf16()));
#else
    TagLib::FileRef f(filePath.toUtf8().constData());
#endif

    if (f.isNull() || !f.tag()) {
        emit writeFailed(filePath, QStringLiteral("Cannot open file for tagging"));
        return false;
    }

    TagLib::Tag *tag = f.tag();
    if (!title.isEmpty())
        tag->setTitle(TagLib::String(title.toUtf8().constData(), TagLib::String::UTF8));
    if (!artist.isEmpty())
        tag->setArtist(
            TagLib::String(artist.toUtf8().constData(), TagLib::String::UTF8));
    if (!album.isEmpty())
        tag->setAlbum(
            TagLib::String(album.toUtf8().constData(), TagLib::String::UTF8));

    // Embed cover art if provided, using format-specific APIs.
    if (!coverArt.isEmpty()) {
        TagLib::ByteVector imageData(coverArt.constData(), coverArt.size());

        if (auto *mpegFile = dynamic_cast<TagLib::MPEG::File *>(f.file())) {
            auto *id3v2 = mpegFile->ID3v2Tag(true);
            if (id3v2) {
                // Remove existing cover art frames.
                id3v2->removeFrames("APIC");
                auto *picFrame = new TagLib::ID3v2::AttachedPictureFrame();
                picFrame->setMimeType("image/jpeg");
                picFrame->setPicture(imageData);
                picFrame->setType(
                    TagLib::ID3v2::AttachedPictureFrame::FrontCover);
                id3v2->addFrame(picFrame);
            }
        } else if (auto *flacFile =
                       dynamic_cast<TagLib::FLAC::File *>(f.file())) {
            flacFile->removePictures();
            auto *pic = new TagLib::FLAC::Picture();
            pic->setMimeType("image/jpeg");
            pic->setData(imageData);
            pic->setType(TagLib::FLAC::Picture::FrontCover);
            flacFile->addPicture(pic);
        }
    }

    if (!f.save()) {
        emit writeFailed(filePath, QStringLiteral("Failed to save tags"));
        return false;
    }

    emit writeSucceeded(filePath);
    return true;
#else
    Q_UNUSED(filePath)
    Q_UNUSED(title)
    Q_UNUSED(artist)
    Q_UNUSED(album)
    Q_UNUSED(coverArt)
    emit writeFailed(filePath,
                     QStringLiteral("TagLib not available in this build"));
    return false;
#endif
}
