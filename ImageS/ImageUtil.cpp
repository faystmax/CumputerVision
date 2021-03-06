#include <QImage>
#include <QPixmap>
#include <QPainter>
#include "Image.h"
#include "Pyramid.h"
#include "InterestPoints.h"
#include "Descriptor.h"
#include "Ransac.h"
#include "Hough.h"

QImage getOutputImage(const Image &image) {
    QImage resultImage(image.getWidth(), image.getHeight(), QImage::Format_ARGB32);
    Image outImage = image.getDeNormolize();
    for (int i = 0; i < outImage.getWidth(); i++) {
        for (int j = 0; j < outImage.getHeight(); j++) {
            double pixel = outImage.getPixel(i, j);
            resultImage.setPixel(i, j, qRgb(pixel, pixel, pixel));
        }
    }
    return resultImage;
}

Image constructImage(const QImage &image, const Image::EdgeEffect edgeEffect = Image::EdgeEffect::Repeat) {

    Image resultImage(image.width(), image.height(), edgeEffect);

    // Read pixels and form black and white image
    for (int i = 0; i < resultImage.getWidth(); i++) {
        for (int j = 0; j < resultImage.getHeight(); j++) {
            QRgb pixel = image.pixel(i, j);
            resultImage.setPixel(i, j, 0.213 * qRed(pixel) + 0.715 * qGreen(pixel) + 0.072 * qBlue(pixel));
        }
    }
    resultImage.normolizePixels();
    return resultImage;
}

QImage createFromL(const Image &image, const Pyramid &pyramid, const double sigma) {
    QImage resultImage(image.getWidth(), image.getHeight(), QImage::Format_ARGB32);
    for (int i = 0; i < image.getWidth(); i++) {
        for (int j = 0; j < image.getHeight(); j++) {
            double pixel = pyramid.L(i, j, sigma);
            resultImage.setPixel(i, j, qRgb(pixel, pixel, pixel));
        }
    }
    return resultImage;
}

QImage createImageWithPoints(const Image &image, const vector <Point> &points) {
    QImage resultImage = getOutputImage(image);
    for (unsigned int i = 0; i < points.size(); i++) {
        resultImage.setPixel(points[i].x - 1, points[i].y - 1, qRgb(255, 0, 0));
        resultImage.setPixel(points[i].x - 1, points[i].y, qRgb(255, 0, 0));
        resultImage.setPixel(points[i].x - 1, points[i].y + 1, qRgb(255, 0, 0));

        resultImage.setPixel(points[i].x, points[i].y - 1, qRgb(255, 0, 0));
        resultImage.setPixel(points[i].x, points[i].y, qRgb(255, 0, 0));
        resultImage.setPixel(points[i].x, points[i].y + 1, qRgb(255, 0, 0));

        resultImage.setPixel(points[i].x + 1, points[i].y - 1, qRgb(255, 0, 0));
        resultImage.setPixel(points[i].x + 1, points[i].y, qRgb(255, 0, 0));
        resultImage.setPixel(points[i].x + 1, points[i].y + 1, qRgb(255, 0, 0));
    }
    return resultImage;
}

QImage glueImages(const Image &imageLeft, const Image &imageRight) {

    // max height
    auto height = max(imageLeft.getHeight(),imageRight.getHeight());

    Image denormImageLeft = imageLeft.getDeNormolize();
    Image denormImageRight = imageRight.getDeNormolize();

    QImage resultImage(denormImageLeft.getWidth() + denormImageRight.getWidth(), height, QImage::Format_ARGB32);
    // imageLeft
    for (auto i = 0; i < denormImageLeft.getWidth(); i++) {
        for (auto j = 0; j < denormImageLeft.getHeight(); j++) {
            double pixel = denormImageLeft.getPixel(i, j);
            resultImage.setPixel(i, j, qRgb(pixel, pixel, pixel));
        }
    }

    // imageRight
    for (auto i = 0; i < denormImageRight.getWidth(); i++) {
        for (auto j = 0; j < denormImageRight.getHeight(); j++) {
            double pixel = denormImageRight.getPixel(i, j);
            resultImage.setPixel(i + denormImageLeft.getWidth(), j, qRgb(pixel, pixel, pixel));
        }
    }
    return resultImage;
}

// Строим панораму
QImage glueImagesPanoram(const Image &imageLeft, const Image &imageRight, const Matrix<9, 1>& matr) {

    QImage outputLeftImage = getOutputImage(imageLeft.getDeNormolize());
    QImage outputRightImage = getOutputImage(imageRight.getDeNormolize());

    // transform matrix
    QTransform transform(matr.at(0,0),matr.at(3,0),matr.at(6,0),
                         matr.at(1,0),matr.at(4,0),matr.at(7,0),
                         matr.at(2,0),matr.at(5,0),matr.at(8,0));

    // Создаём Новое изображение
    auto height = max(outputLeftImage.height(), outputRightImage.height() + 100); // высоту возьмём с запасом
    QImage resultImage(outputLeftImage.width() + outputRightImage.width(), height, QImage::Format_ARGB32);

    // Склеиваем панораму
    QPainter painter;
    painter.begin(&resultImage);
    painter.drawImage(0,0,outputLeftImage);
    painter.setTransform(transform);
    painter.drawImage(0,0,outputRightImage);

    return resultImage;
}

inline vector<QColor> randomColors(int count) {
    vector<QColor> colors;
    float currentHue = 0.0;
    for (int i = 0; i < count; i++) {
        colors.push_back(QColor::fromHslF(currentHue, 1.0, 0.5));
        currentHue += 0.618033988749895f;
        currentHue = std::fmod(currentHue, 1.0f);
    }
    return colors;
}

void drawLinesAndCircles(QImage &image, const int firstWidth, vector<Vector> similar) {
    QPainter painter(&image);
    QPen pen;
    pen.setWidth(1);
    vector<QColor> colors = randomColors(similar.size());
    for (unsigned int i = 0; i < similar.size(); i++) {
        pen.setColor(colors[i]);
        painter.setPen(pen);
        Point p1 = similar[i].first.getInterPoint();
        Point p2 = similar[i].second.getInterPoint();
        painter.drawLine(p1.x, p1.y, p2.x + firstWidth, p2.y);

        // Circle 1
        double radius1 = sqrt(2) * p1.sigmaEffect;
        painter.drawEllipse(QRect(p1.x - radius1, p1.y - radius1, 2 * radius1, 2 * radius1));

        // Circle 2
        double radius2 = sqrt(2) * p2.sigmaEffect;
        painter.drawEllipse(QRect(p2.x + firstWidth - radius2, p2.y - radius2, 2 * radius2, 2 * radius2));

    }
    painter.end();
}

void drawLines(QImage &image, const int firstWidth, vector<Vector> similar) {
    QPainter painter(&image);
    QPen pen;
    pen.setWidth(1);
    vector<QColor> colors = randomColors(similar.size());
    for (unsigned int i = 0; i < similar.size(); i++) {
        pen.setColor(colors[i]);
        painter.setPen(pen);
        Point p1 = similar[i].first.getInterPoint();
        Point p2 = similar[i].second.getInterPoint();
        painter.drawLine(p1.x, p1.y, p2.x + firstWidth,  p2.y);
    }
    painter.end();
}

QImage createImageWithPointsBlob(const Image &image, const vector <Point> &points) {
    QImage resultImage = getOutputImage(image);
    QPainter painter(&resultImage);
    QPen pen;
    pen.setWidth(2);
    vector<QColor> colors = randomColors(points.size());
    for (unsigned int i = 0; i < points.size(); i++) {
        pen.setColor(colors[i]);
        painter.setPen(pen);
        double radius = sqrt(2) * points[i].sigmaEffect;
        painter.drawEllipse(QRect(points[i].x - radius, points[i].y - radius, 2 * radius, 2 * radius));
        painter.drawPoint(points[i].x, points[i].y);
    }
    painter.end();
    return resultImage;
}

void drawObjects(QImage &image,vector<Transform> transforms, const int objWidth){
    QPainter painter(&image);
    QPen pen;
    pen.setWidth(2);
    pen.setColor(QColor::fromHslF(0, 1.0, 0.5));
    painter.setPen(pen);

    for(auto&t : transforms){
        //Центр
        double radius = 3;
        painter.drawEllipse(QRect(t.x + objWidth - radius, t.y - radius, 2 * radius, 2 * radius));

        //Фигура
        double cosP = cos(t.angle);
        double sinP = sin(t.angle);
        double halfWidth = 0.5 * t.width;
        double halfHeight = 0.5 * t.height;

//       Новые точки
//       painter.drawEllipse(QRect(t.x + objWidth + halfWidth - 1, t.y + halfHeight - 1, 2 , 2 ));
//       painter.drawEllipse(QRect(t.x + objWidth + halfWidth - 1, t.y - halfHeight - 1, 2 , 2 ));
//       painter.drawEllipse(QRect(t.x + objWidth - halfWidth - 1, t.y - halfHeight - 1, 2 , 2 ));
//       painter.drawEllipse(QRect(t.x + objWidth - halfWidth - 1, t.y + halfHeight - 1, 2 , 2 ));

//       Поворот вокруг точки
//       X = x0 + (x - x0) * cos(a) - (y - y0) * sin(a);
//       Y = y0 + (y - y0) * cos(a) + (x - x0) * sin(a);
        QPoint points[4] = {
            QPoint(t.x + (+ halfWidth) * cosP - (+ halfHeight) * sinP + objWidth, t.y + (+ halfWidth) * sinP + (+ halfHeight) * cosP),
            QPoint(t.x + (+ halfWidth) * cosP - (- halfHeight) * sinP + objWidth, t.y + (+ halfWidth) * sinP + (- halfHeight) * cosP),
            QPoint(t.x + (- halfWidth) * cosP - (- halfHeight) * sinP + objWidth, t.y + (- halfWidth) * sinP + (- halfHeight) * cosP),
            QPoint(t.x + (- halfWidth) * cosP - (+ halfHeight) * sinP + objWidth, t.y + (- halfWidth) * sinP + (+ halfHeight) * cosP),
        };
        painter.drawPolygon(points,4);
    }
    painter.end();
}
