#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QElapsedTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->pixmap_item = this->scene.addPixmap(QPixmap::fromImage(this->fractal.image));

    ui->graphicsView->setScene(&this->scene);
    ui->graphicsView->show();

    connect(&this->scene, &CustomScene::pixelClicked, this, &MainWindow::clicked);
    connect(&this->scene, &CustomScene::mouseDraggedTo, this, &MainWindow::dragged);
    connect(&this->scene, &CustomScene::mouseReleased, this, &MainWindow::released);

    this->updateImage();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateImage(){
    QElapsedTimer timer;
    timer.start();

    this->fractal.updateImage();

    QPainter painter(&this->fractal.image);

    QPen pen(QBrush(Qt::GlobalColor::white), this->fractal.ui_to_coord_scale);
    pen.setColor(Qt::GlobalColor::white);
    painter.setPen(pen);

    painter.setTransform(this->fractal.coord_to_ui_tform);

    for (complex r : this->fractal.roots){
        QPointF center = this->fractal.complexToQPointF(r);
        painter.drawEllipse(center, 0.1, 0.1);
//        printf("Drawing circle at (%f, %f)\n", center.x(), center.y());
    }

    painter.end();

    this->pixmap_item->setPixmap(QPixmap::fromImage(this->fractal.image));

    printf("update image took %lld ns\n", timer.nsecsElapsed());
}

void MainWindow::clicked(QPoint p){
//    printf("click gotten at pixel (%d, %d)\n", p.x(), p.y());
//    fflush(stdout);

    QPointF pf = p;
    QPointF coord = this->fractal.ui_to_coord_tform.map(pf);

    this->root_is_selected = false;

    for (uint i = 0; i < this->fractal.roots.size(); ++i){
        complex r = this->fractal.roots.at(i);
        QPointF root_pt = this->fractal.complexToQPointF(r);
        double x_diff = root_pt.x() - coord.x();
        double y_diff = root_pt.y() - coord.y();
        double dist = sqrt(x_diff*x_diff + y_diff*y_diff);
        if (dist < 0.1){
            this->current_root_selected = i;
            this->root_is_selected = true;
//            printf("picked root %d\n", i);
            break;
        }
    }

}

void MainWindow::dragged(QPoint p){
//    printf("drag gotten at pixel (%d, %d)\n", p.x(), p.y());
//    fflush(stdout);

    if (!root_is_selected) return;

    QPointF pf = p;
    QPointF coord = this->fractal.ui_to_coord_tform.map(pf);

    this->fractal.roots.at(this->current_root_selected) = fractal.qpointfToComplex(coord);

    this->updateImage();
}

void MainWindow::released(){
//    printf("mouse released\n");
//    fflush(stdout);
}
