// SPDX-License-Identifier: GPL-2.0-or-later
#include "notificationslistener.h"
#include "plugin_sendnotifications_debug.h"
#include "pw_libnotificationmanager/notification.h"
#include <QImage>
#include <QTemporaryDir>
#include <QtTest>

Q_LOGGING_CATEGORY(KDECONNECT_PLUGIN_SENDNOTIFICATIONS, "kdeconnect.plugin.sendnotifications")

class ImageEncoder : public NotificationsListener
{
public:
    using NotificationsListener::iconFromQImage;
};

class NotificationImageTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void payloadContainsReadablePng()
    {
        QImage original(96, 64, QImage::Format_ARGB32);
        original.fill(Qt::blue);
        auto payload = ImageEncoder::iconFromQImage(original);
        QVERIFY(payload);
        QVERIFY(payload->size() > 0);
        // Match UploadJob: it opens the returned device for reading.
        QVERIFY(payload->open(QIODevice::ReadOnly));
        const auto png = payload->readAll();
        QVERIFY(png.startsWith("\x89PNG\r\n\x1a\n"));
        QCOMPARE(QImage::fromData(png, "PNG"), original);
    }
    void fileThumbnailIsRetained()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        QImage original(96, 64, QImage::Format_ARGB32);
        original.fill(Qt::blue);
        const auto path = directory.filePath(QStringLiteral("thumbnail.png"));
        QVERIFY(original.save(path, "PNG"));
        NotificationManager::Notification notification(0);
        notification.processHints({{QStringLiteral("image-path"), path}});
        QCOMPARE(notification.image(), original);
        QVERIFY(ImageEncoder::iconFromQImage(notification.image()));
    }
    void notificationsOwnTheirImages()
    {
        QImage red(96, 64, QImage::Format_ARGB32);
        red.fill(Qt::red);
        QImage blue(96, 64, QImage::Format_ARGB32);
        blue.fill(Qt::blue);
        NotificationManager::Notification first(0);
        first.setImage(red);
        const auto queuedCopy = first;
        NotificationManager::Notification second(0);
        second.setImage(blue);
        QCOMPARE(first.image(), red);
        QCOMPARE(queuedCopy.image(), red);
        QCOMPARE(second.image(), blue);
        NotificationManager::Notification withoutImage(0);
        QVERIFY(withoutImage.image().isNull());
        NotificationManager::Notification replacement(42);
        replacement.setImage(red);
        NotificationManager::Notification updated(42);
        updated.setImage(blue);
        QCOMPARE(replacement.image(), red);
        QCOMPARE(updated.image(), blue);
    }
    void screenshotInApplicationIconPath()
    {
        QTemporaryDir directory;
        QImage original(96, 64, QImage::Format_ARGB32);
        original.fill(Qt::green);
        const auto path = directory.filePath(QStringLiteral("screenshot.png"));
        QVERIFY(original.save(path, "PNG"));
        NotificationManager::Notification notification(0);
        notification.setApplicationIconName(path);
        notification.processHints({});
        QCOMPARE(notification.image(), original);
    }
    void nullImageHasNoPayload()
    {
        QVERIFY(!ImageEncoder::iconFromQImage(QImage()));
    }
};
QTEST_GUILESS_MAIN(NotificationImageTest)
#include "notificationimagetest.moc"
