#include <QApplication>
#include <QSvgRenderer>
#include <QPainter>
#include <QImage>
#include <QIcon>
#include <QDebug>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv); // Required for GUI and font rendering

    QImage image(256, 256, QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    QSvgRenderer renderer(QString("icon.svg"));
    if (!renderer.isValid()) {
        qDebug() << "Failed to load SVG from icon.svg";
        return 1;
    }

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    renderer.render(&painter);
    painter.end();

    QIcon icon;
    icon.addPixmap(QPixmap::fromImage(image)); // Adds the 256x256 image

    // Optional: generate a few downscaled versions for the ICO wrapper to be nice
    icon.addPixmap(QPixmap::fromImage(image.scaled(128, 128, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    icon.addPixmap(QPixmap::fromImage(image.scaled(64, 64, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    icon.addPixmap(QPixmap::fromImage(image.scaled(32, 32, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    icon.addPixmap(QPixmap::fromImage(image.scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));

    // Actually, save the scaled pixmap directly requires QImage implementation.
    // QImage::save("app.ico", "ICO") supports standard image bounds.
    if (image.save("app.ico", "ICO")) {
        qDebug() << "Saved app.ico successfully.";
        return 0;
    }

    qDebug() << "Failed to save app.ico. Ensure the imageformats plugin is available.";
    return 1;
}
