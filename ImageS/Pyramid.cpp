#include "Pyramid.h"

#include "KernelCreator.h"
#include "ImageConverter.h"
#include "math.h"
#include "iostream"

Pyramid::Pyramid(const Image &image, const int scales, double sigma, double sigmaStart) {

    /* Reserve data */
    int octaveCount = min(log2(image.getWidth()),log2(image.getHeight()))-1;
    items.reserve(octaveCount * scales);

    /* First image */
    items.emplace_back(convultionSeparab(image, KernelCreator::getGauss(getDeltaSigma(sigmaStart, sigma))), 0, 0, sigma, sigma);

    double sigmaScale = sigma;
    double sigmaEffect = sigma;
    double octave = 0;
    Image tmpLastImage;

    // While image can be reduced
    while (octaveCount > 0) {
        double intervalSigma = pow(2, 1.0 / scales);

        for (int i = 0; i < scales + 3; i++) {
            double sigmaScalePrev = sigmaScale;
            sigmaScale = sigma * pow(intervalSigma, i + 1);
            double deltaSigma = getDeltaSigma(sigmaScalePrev, sigmaScale);
            sigmaEffect = sigmaScale *  pow(2, octave);

            items.emplace_back(convultionSeparab(getLastImage(), KernelCreator::getGauss(deltaSigma)), octave, i + 1,
                               sigmaScale, sigmaEffect);

            if (i == scales - 1) {
                tmpLastImage = ImageConverter::halfReduce(getLastImage());
            }
        }
        octave++;
        sigmaEffect = sigma *  pow(2, octave);
        sigmaScale = sigma;
        octaveCount--;

        items.emplace_back(tmpLastImage, octave, 0, sigmaScale, sigmaEffect);
    }

    /* Constructs DOGs */
    for (unsigned int i = 1; i < items.size(); i++) {
        if (Image::sizeEq(items[i - 1].image, items[i].image)) {
            Item &item = items[i - 1];
            dogs.emplace_back(items[i].image,items[i].image - item.image,
                              item.octave, item.scale, item.sigmaScale, item.sigmaEffect);
        }
    }
}

int Pyramid::L(int x, int y, double sigma) const {
    if (items.size() > 0) {
        int width = items[0].image.getWidth();
        int height = items[0].image.getHeight();
        for (unsigned int i = 0; i < items.size() - 1; i++) {
            if (sigma >= items[i].sigmaEffect && sigma <= items[i + 1].sigmaEffect) {
                int xCur = x * items[i].image.getWidth() * (1.0 / width);
                int yCur = y * items[i].image.getHeight() * (1.0 / height);
                return items[i].image.getPixel(xCur, yCur);
            }
        }
    }
    return 0;
}

double Pyramid::getDeltaSigma(double sigmaPrev, double sigmaNext) const {
    return sqrt(sigmaNext * sigmaNext - sigmaPrev * sigmaPrev);
}

Image Pyramid::convultionSeparab(const Image &image, Kernel &&gaussLine) {
    Image result = ImageConverter::convolution(image, gaussLine);
    gaussLine.rotate();
    return ImageConverter::convolution(result, gaussLine);
}

Image &Pyramid::getLastImage() {
    return items.at(items.size() - 1).image;
}
