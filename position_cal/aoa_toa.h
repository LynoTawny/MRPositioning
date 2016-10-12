#ifndef __AOA_TOA_H__
#define __AOA_TOA_H__

#include <Eigen/Dense>

using namespace Eigen;
extern VectorXd& aoa_toa_calc(MatrixXd &STATEA,double aoa,double toa,VectorXd &ms_pos);
extern void  calc_circles_point(MatrixXd &mat_circle,MatrixXd &point);
#endif



