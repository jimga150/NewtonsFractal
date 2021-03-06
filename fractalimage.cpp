#include "fractalimage.h"

FractalImage::FractalImage(QObject *parent)
    : QObject{parent}
{
    this->setImageSize(QSize(500, 500));

    this->generateColors();
}

void FractalImage::setImageSize(QSize size){
    this->image = QImage(size, QImage::Format_ARGB32);
    this->image.fill(Qt::GlobalColor::black);

    this->setScale(1);
    this->setCenter(complex(0, 0));
}

void FractalImage::updateImage(){

//    QElapsedTimer timer;
//    timer.start();

    this->poly_fxn.prepFunctionDerivative();

//    printf("prep function derivative took %lld ns\n", timer.nsecsElapsed());
//    timer.restart();

//    this->image.fill(Qt::GlobalColor::black);

#ifdef USE_RUN

    QList<QFuture<void>> threads;

    for (int y = 0; y < this->image.height(); y = ++y){
        QFuture<void> thread = QtConcurrent::run(&FractalImage::updateImageLine, this, y);
        threads.append(thread);
    }

    for (QFuture<void> thread : threads){
        thread.waitForFinished();
    }

#elif defined (USE_MAP)

    QtConcurrent::blockingMap(this->fractal_pixels, &FractalPixel::update);

#endif

//    printf("fractal fill took %lld ns\n", timer.nsecsElapsed());
//    timer.restart();
}

void FractalImage::setScale(double scale){
    this->coord_to_ui_scale = scale*this->coord_to_ui_scale_correction_factor;
    this->ui_to_coord_scale = 1.0/this->coord_to_ui_scale;

    this->coord_to_ui_tform.reset();

    this->coord_to_ui_tform.translate(image.width()/2, this->image.height()/2);
    this->coord_to_ui_tform.scale(coord_to_ui_scale, coord_to_ui_scale);

    QPointF center_pt = complexToQPointF(this->center);
    this->coord_to_ui_tform.translate(center_pt.x(), center_pt.y());

    Q_ASSERT(this->coord_to_ui_tform.isInvertible());
    this->ui_to_coord_tform = this->coord_to_ui_tform.inverted();

    this->generatePixelObjs();
}

void FractalImage::setCenter(complex center){
    this->coord_to_ui_tform.reset();

    this->coord_to_ui_tform.translate(image.width()/2, this->image.height()/2);
    this->coord_to_ui_tform.scale(coord_to_ui_scale, coord_to_ui_scale);

    if (!std::isfinite(center.real())){
        center.real(this->center.real());
    }
    if (!std::isfinite(center.imag())){
        center.imag(this->center.imag());
    }
    this->center = center;
    QPointF center_pt = complexToQPointF(center);

    this->coord_to_ui_tform.translate(center_pt.x(), center_pt.y());

    Q_ASSERT(this->coord_to_ui_tform.isInvertible());
    this->ui_to_coord_tform = this->coord_to_ui_tform.inverted();

    this->generatePixelObjs();
}

void FractalImage::setCenterReal(double real){
    this->setCenter(complex(real, NAN));
}

void FractalImage::setCenterImag(double imag){
    this->setCenter(complex(NAN, imag));
}

void FractalImage::generatePixelObjs(){

    this->fractal_pixels.clear();

    for (int y = 0; y < this->image.height(); ++y){

        QRgb* image_line = reinterpret_cast<QRgb*>(this->image.scanLine(y));

        for (int x = 0; x < this->image.width(); ++x){

            QPointF pixel_pt(x, y);
            QPointF coord_pt = this->ui_to_coord_tform.map(pixel_pt);
            complex coord = qpointfToComplex(coord_pt);

            this->fractal_pixels.push_back(FractalPixel(&image_line[x], coord, &this->poly_fxn, &this->colors));

        }
    }
}

void FractalImage::generateColors(){
    int h_step = 360.0/(this->poly_fxn.roots.size());
    this->colors.clear();
    for (uint i = 0; i < this->poly_fxn.roots.size(); ++i){
        this->colors.push_back(QColor::fromHsv(i*h_step, 150, 200));
    }
}

QRectF FractalImage::getRootBoundingBox(){
    QRectF ans(complexToQPointF(this->poly_fxn.roots.at(0)), QSizeF(DBL_EPSILON, DBL_EPSILON));
    for (uint i = 1; i < this->poly_fxn.roots.size(); ++i){
        QRectF this_root_rect(complexToQPointF(this->poly_fxn.roots.at(i)), QSizeF(DBL_EPSILON, DBL_EPSILON));
        ans = ans.united(this_root_rect);
    }
//    printf("Rect is (%f, %f), %f x %f\n", ans.x(), ans.y(), ans.width(), ans.height());
//    fflush(stdout);
    return ans;
}

double FractalImage::getScaleOf(QRectF rect, QSizeF image_size){
    double rect_ar = rect.height()/rect.width();
    double image_ar = image_size.height()/image_size.width();

    double rect_dist;
    double image_dist;

    if (rect_ar > image_ar){
        rect_dist = rect.width();
        image_dist = image_size.width();
    } else {
        rect_dist = rect.height();
        image_dist = image_size.height();
    }

    double actual_scale = image_dist/rect_dist;

    return actual_scale/this->coord_to_ui_scale_correction_factor;
}

complex FractalImage::getCenterOF(QRectF rect){
    return complex(-(rect.left() + rect.right())/2, (rect.top() + rect.bottom())/2);
}

void FractalImage::updateImageLine(int y){

    int first = y*this->image.width();
    int end = (y+1)*this->image.width();

    for (int i = first; i < end; ++i){
        this->fractal_pixels.at(i).update();
    }
}
