#include <QImage>
#include "Image.h"
#include "Pyramid.h"
#include "InterestPoints.h"

QImage getOutputImage(const Image &image) {
    QImage resultImage(image.getWidth(), image.getHeight(), QImage::Format_ARGB32);
    for (int i = 0; i < image.getWidth(); i++) {
        for (int j = 0; j < image.getHeight(); j++) {
            double pixel = image.getPixel(i, j);
            resultImage.setPixel(i, j, qRgb(pixel, pixel, pixel));
        }
    }
    return resultImage;
}

Image constructImage(const QImage &image, const Image::EdgeEffect edgeEffect = Image::EdgeEffect::Mirror) {

    Image resultImage(image.width(), image.height(), edgeEffect);

    // Read pixels and form black and white image
    for (int i = 0; i < resultImage.getWidth(); i++) {
        for (int j = 0; j < resultImage.getHeight(); j++) {
            QRgb pixel = image.pixel(i, j);
            resultImage.setPixel(i, j, 0.213 * qRed(pixel) + 0.715 * qGreen(pixel) + 0.072 * qBlue(pixel));
        }
    }
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