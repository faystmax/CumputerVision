#ifndef IMAGE_H
#define IMAGE_H

#include <QImage>
#include <memory>

#include "images_global.h"
#include "Kernel.h"

using namespace std;

class IMAGESSHARED_EXPORT Image{

public:
    enum EdgeEffect{ Black, Repeat, Mirror,  Wrapping };

    Image();
    Image(Image&&) = default;
    Image& operator=(const Image&) = default;
    Image(const Image& copy);
    Image(const int width ,const int height, const EdgeEffect edgeEffect = Mirror );
    Image(const QImage &image, const EdgeEffect edgeEffect = Mirror);
    ~Image() = default;

    QImage getOutputImage() const;

    double getPixel(const int x,const int y) const;
    void setPixel(const int x,const int y, double pixel);

    int getHeight() const {return height;}
    int getWidth() const {return width;}

    void setEdgeEffect(const EdgeEffect edgeEffect) {this->edgeEffect = edgeEffect;}
    EdgeEffect getEdgeEffect() const {return this->edgeEffect;}

private:
    int height;
    int width;
    EdgeEffect edgeEffect;
    vector<double> pixels;


    double getPixelRepeat(int x ,int y) const;
    double getPixelMirror(int x ,int y) const;
    double getPixelWrapping(int x ,int y) const;
};

#endif // IMAGE_H
