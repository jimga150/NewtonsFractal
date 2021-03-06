#ifndef FRACTALIMAGE_H
#define FRACTALIMAGE_H

#include <cfloat>

#include <QObject>
#include <QImage>
#include <QtConcurrent>

#include "common.h"
#include "fractalpixel.h"
#include "polynomial.h"

#define USE_MAP

#ifndef USE_MAP

#define USE_RUN

#endif

class FractalImage : public QObject
{
    Q_OBJECT
public:
    explicit FractalImage(QObject *parent = nullptr);

    void setImageSize(QSize size);

    void updateImage();

    void updateImageLine(int y);

    inline std::vector<complex>* getRoots(){
        return &this->poly_fxn.roots;
    }

    inline int getNumIters(){
        return this->poly_fxn.num_iterations;
    }

    inline void setNumRoots(int nr){
        this->poly_fxn.changeNumRoots(nr);
        this->generateColors();
    }

    inline void setNumIters(int ni){
        this->poly_fxn.changeNumIters(ni);
    }

    void setScale(double scale);

    void setCenter(complex center);

    void setCenterReal(double real);

    void setCenterImag(double imag);

    inline complex getCenter(){
        return this->center;
    }

    void generatePixelObjs();

    void generateColors();

    QRectF getRootBoundingBox();

    double getScaleOf(QRectF rect, QSizeF image_size);

    complex getCenterOF(QRectF rect);


    QImage image;

    const double coord_to_ui_scale_correction_factor = 150.0;

    double coord_to_ui_scale = coord_to_ui_scale_correction_factor;
    double ui_to_coord_scale = 1.0/coord_to_ui_scale;

    QTransform coord_to_ui_tform;
    QTransform ui_to_coord_tform;

    std::vector<QColor> colors;

    std::vector<FractalPixel> fractal_pixels;

private:

    Polynomial poly_fxn;

    complex center;

signals:

};

#endif // FRACTALIMAGE_H
