#include "Pyramid.h"

#include "KernelCreator.h"
#include "ImageConverter.h"
#include "math.h"

void Pyramid::generate(const Image &image, const int scales, const double sigma) {

    items.clear();

    // Push orihinal
    items.push_back(Item(image, 0, 0, 0, 0));

//    unique_ptr<Image> tmpImage(new Image(image));
//    unique_ptr<Kernel> gauss =  KernelCreator::getGauss(5, 5, sigma);
//    unique_ptr<Image> result = ImageConverter::convolution(*tmpImage.get(), *gauss.get());

    double sigmaScale = sigma;
    double sigmaEffect = sigma;
    double octave = 1;
    // Push first
//    items.push_back(Item(result.release(), octave, 0, sigmaScale, sigmaEffect));

    // While image can be redeced
    while (getLastImage().getWidth() >= 2 && getLastImage().getHeight() >= 2) {
        sigmaScale = sigma;
        double intervalSigma = pow(2, 1.0 / scales);
        for (int i = 0; i < scales; i++) {
            double sigmaScalePrev = sigmaScale;
            sigmaScale *= sigma * pow(intervalSigma, i + 1);
            sigmaEffect *= sigmaScale;
            double deltaSigma = getDeltaSigma(sigmaScalePrev, sigmaScale);
            Kernel gauss = KernelCreator::getGauss(deltaSigma);
            Image result = ImageConverter::convolution(getLastImage(), gauss);
            items.emplace_back(result, octave, i + 1, sigmaScale, sigmaEffect);
        }
        octave++;
        Image half = ImageConverter::halfReduce(getLastImage());
        items.push_back(Item(half, octave, 0, sigmaScale, sigmaEffect));
    }
}

double Pyramid::getDeltaSigma(double sigmaPrev, double sigmaCur) const {
    return sqrt(sigmaCur * sigmaCur - sigmaPrev * sigmaPrev);
}

Image Pyramid::getLastImage() const {
    return items.at(items.size() - 1).image;
}
