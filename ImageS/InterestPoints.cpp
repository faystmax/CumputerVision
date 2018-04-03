#include "InterestPoints.h"
#include "KernelCreator.h"
#include "ImageConverter.h"
#include "Pyramid.h"
#include "Image.h"
#include "iostream"

vector<Point> InterestPoints::moravek(const Image &image, const double threshold, const int radius, const int pointsCount) {

    Image image_S(image.getWidth(),image.getHeight());  // Веса

    for (auto x = 0; x < image.getWidth(); x++) {
        for (auto y = 0; y < image.getHeight(); y++) {
            array<double, 8> local_S = { 0 };                    // 8 directions
            for (auto u = -radius; u < radius; u++) {
                for (auto v = -radius; v < radius; v++) {
                    double directDiff[8];
                    auto pixel = image.getPixel(x + u, y + v);
                    directDiff[0] = pixel - image.getPixel(x + u, y + v - 1);
                    directDiff[1] = pixel - image.getPixel(x + u, y + v + 1);
                    directDiff[2] = pixel - image.getPixel(x + u + 1, y + v);
                    directDiff[3] = pixel - image.getPixel(x + u + 1, y + v - 1);
                    directDiff[4] = pixel - image.getPixel(x + u + 1, y + v + 1);
                    directDiff[5] = pixel - image.getPixel(x + u - 1, y + v);
                    directDiff[6] = pixel - image.getPixel(x + u - 1, y + v - 1);
                    directDiff[7] = pixel - image.getPixel(x + u - 1, y + v + 1);

                    for (auto i = 0; i < 8; i++) {
                        local_S[i] += directDiff[i] * directDiff[i];
                    }
                }
            }
            image_S.setPixelNoValidation(x, y, *std::min_element(local_S.begin(), local_S.end()));
        }
    }

    vector <Point> points = thresholdFilter(image_S, threshold);
    return anmsFilter(points, pointsCount);
}

vector<Point>  InterestPoints::harris(const Image &image, const double threshold, const int radius, const int pointsCount) {

    Image image_dx = ImageConverter::convolution(image, KernelCreator::getSobelX());
    Image image_dy = ImageConverter::convolution(image, KernelCreator::getSobelY());

    Image image_S(image.getWidth(),image.getHeight());  // Веса
    for (int x = 0; x < image.getWidth(); x++) {
        for (int y = 0; y < image.getHeight(); y++) {
           image_S.setPixelNoValidation(x, y, lambda(image_dx, image_dy, x, y, radius));
        }
    }

    vector<Point> points = thresholdFilter(image_S, threshold);
    vector<Point> localMaximumPoints = localMaximum(points, image_S);
    return anmsFilter(localMaximumPoints, pointsCount);
}

vector <Point> InterestPoints::blob(const Image &image, const double threshold, const int radius, const int pointsCount) {
    Pyramid pyramid(image);
    vector<Point> points;

    Kernel kernel_x = KernelCreator::getSobelX();
    Kernel kernel_y = KernelCreator::getSobelY();
    for (int z = 1; z < pyramid.getDogsSize() - 3; z++) {
        Image imageDOG = pyramid.getDog(z).image;
        Image image_dx = ImageConverter::convolution(imageDOG, kernel_x);
        Image image_dy = ImageConverter::convolution(imageDOG, kernel_y);

        for (int i = 1; i < imageDOG.getWidth(); i++) {
            for (int j = 1; j < imageDOG.getHeight(); j++) {
                if (isExtremum(pyramid, i, j, z)) {

                    // check harris
                    double lambdaMin = lambda(image_dx, image_dy, i, j, radius);
                    if (lambdaMin < threshold)
                        continue; // skip - haris to low

                    double step_W = double(image.getWidth()) / imageDOG.getWidth();
                    double step_H = double(image.getHeight()) / imageDOG.getHeight();
                    double radius = sqrt(2) * pyramid.getDog(z).sigmaEffect;
                    points.emplace_back(round(i * step_W), round(j * step_H), z, lambdaMin, radius);
                }
            }
        }
    }

    // Сортируем и оборезаем если нужно
    std::sort(points.begin(), points.end(), [](auto &p1, auto &p2) { return p1.s > p2.s; });
    if(points.size()>pointsCount)
        points.resize(pointsCount);
    return points;
}

bool InterestPoints::isExtremum(Pyramid &pyramid, const int x, const int y, const int z){

    if(pyramid.getDog(z-1).octave == pyramid.getDog(z+1).octave){
        bool min = true, max = true;
        double center = pyramid.getDog(z).image.getPixel(x, y);

        // ищем в 3D
        for (int i = -1;i <= 1;i++) {
            for (int j = -1;j <= 1;j++) {
                for (int k = -1;k <= 1;k++) {
                    if (i == 0 && j == 0 && k == 0) {
                        continue;   //skip center
                    }
                    double value = pyramid.getDog(z + k).image.getPixel(x + i, y + j);
                    if (value >= center) max = false;
                    if (value <= center) min = false;
                }
            }
        }

        return max || min;
    }
    return false;
}



// Adaptive Non-Maximum Suppression
vector<Point> InterestPoints::anmsFilter(vector<Point> points, const int pointsCount) {

    vector<bool> flagUsedPoints(points.size(), true);
    auto radius = 3;
    int usedPointsCount = points.size();
    while (usedPointsCount > pointsCount) {
        for (unsigned int i = 0; i < points.size(); i++) {

            if (!flagUsedPoints[i]) {
                continue;
            }

            auto &p1 = points[i];
            for (unsigned int j = i + 1; j < points.size(); j++) {
                if (flagUsedPoints[j]) {
                    Point &p2 = points[j];
                    if (p1.s * 0.9 > p2.s && sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y)) <= radius) {
                        flagUsedPoints[j] = false;
                        usedPointsCount--;
                        if (usedPointsCount <= pointsCount) {
                            break;
                        }
                    }
                }
            }
        }
        radius++;
    }

    vector <Point> resultPoints;
    for (unsigned int i = 0; i < points.size(); i++) {
        if (flagUsedPoints[i]) {
            resultPoints.push_back(points[i]);
        }
    }
    return resultPoints;
}

double InterestPoints::lambda(const Image &image_dx, const Image &image_dy, const int x, const int y, const int radius) {
    double A = 0 , B = 0, C = 0;
    for (auto i = x - radius; i < x + radius; i++) {
        for (auto j = y - radius; j < y + radius; j++) {
            auto curA = image_dx.getPixel(i, j);
            auto curB = image_dy.getPixel(i, j);
            A += curA * curA;
            B += curA * curB;
            C += curB * curB;
        }
    }
    auto descreminant = sqrt((A - C) * (A - C) + 4 * B * B);
    return min(abs((A + C - descreminant) / 2), abs((A + C + descreminant) / 2));
}

vector <Point> InterestPoints::thresholdFilter(const Image &image_S, const double threshold) {

    vector <Point> points;
    for (auto i = 0; i < image_S.getWidth(); i++) {
        for (auto j = 0; j < image_S.getHeight(); j++) {
            if (image_S.getPixel(i, j) >= threshold) {
                points.emplace_back(i, j, image_S.getPixel(i, j));
            }
        }
    }
    std::sort(points.begin(), points.end(), [](auto &p1, auto &p2) { return p1.s > p2.s; });
    return points;
}

vector<Point> InterestPoints::localMaximum(const vector<Point> points, const Image &image_S){

    vector <Point> result;
    const int radius = 2;

    for(unsigned int i = 0; i < points.size(); i ++){
        auto p1 = points[i];
        bool flagMaximum = true;

        for (auto j = -radius; j <= radius; ++j) {
            for (auto k = -radius; k <= radius; ++k) {
                if(j ==0 && k == 0) continue;

                if (image_S.getPixel(p1.x + j, p1.y + k) >= p1.s) {
                    flagMaximum = false;
                    break;
                }
            }
        }

        if(flagMaximum == true){
            result.push_back(p1);
        }
    }
    return result;
}

