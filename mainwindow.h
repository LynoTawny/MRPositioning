#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <basedbhandler.h>
#include "drivetestitem.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void on_clearBtn_clicked();

    void on_drTstRsltBtn_clicked();

    void on_showBasePosBtn_clicked();

    void on_apiPosBtn_clicked();

    void on_ourPosBtn_clicked();

    void on_truePosBtn_clicked();

    void on_apiVsTrueBtn_clicked();

    void on_outVsTrueBtn_clicked();

    void on_preprocessBtn_clicked();

private:
    Ui::MainWindow *ui;
    QList<DriveTestItem *> itemList;
    double GetDistance(double lng1, double lat1, double lng2, double lat2);
    double GetDistanceByXY(double x1, double y1, double x2, double y2);
    void convertLonLat2XY(double lng, double lat, double *pos_x, double *pox_y);
    void convertXY2LonLat(double pos_x, double pos_y, double *lng, double *lat);
    void rawDataRead(void);
    void rawDataFiltrate(void);
    void rawDataFill(void);
    void rawDataFinalyCheck(void);
    void rawDataOutputNewFile(void);
    QString getNetIP(void);

    QList<meas_point_raw_data_t *> measPointList;
};

#endif // MAINWINDOW_H
