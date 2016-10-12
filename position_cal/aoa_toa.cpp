#include <Eigen/Dense>
using namespace Eigen;

VectorXd& aoa_toa_calc(MatrixXd &STATEA,double aoa,double toa,VectorXd &ms_pos)
{
    double x1 = STATEA(0,0);
    double y1 = STATEA(0,1);

    if(toa == 90)
    {
      ms_pos(0) = x1;
      ms_pos(1) = y1 + toa;
    }
    else if(toa == 270)
    {
      ms_pos(0) = x1;
      ms_pos(1) = y1 - toa;
    }
   
    double pi = 3.1415926;
    double tv = tan(aoa*pi/180);
  
    double x_root1,x_root2;
    double a,b;
    int choose=-1;
    a = tv*tv+1;
    b = toa*toa;//toa means distance here
    x_root1 = sqrt(b/a) + x1;
    x_root2 = -sqrt(b/a) + x1;
    
    double y_root1 = y1 + tv*sqrt(b/a);
    double y_root2 = y1 - tv*sqrt(b/a);  


    if(tv < 0)
    {
        if(aoa > 180)
        {
	   choose = 0;
        } 
        else
        {
           choose = 1;  
        }
    }
    else
    {
         if(aoa > 180)
        {
	   choose = 1;
        } 
        else
        {
           choose = 0;  
        }
      
    }
    if(choose == 0)
    {
        ms_pos(0) = x_root1;
        ms_pos(1) = y_root1;
    }
    else
    {
        ms_pos(0) = x_root2;
        ms_pos(1) = y_root2;
    }
    return ms_pos;
}

void  calc_circles_point(MatrixXd &mat_circle,MatrixXd &mpoint)
{
    double x1,y1,R1;  
    double x2,y2,R2;
    double tan_info;

    x1 = mat_circle(0,0);
    y1 = mat_circle(0,1); 
    R1 = mat_circle(0,2);
   
    x2 = mat_circle(1,0);
    y2 = mat_circle(1,1);
    R2 = mat_circle(1,2);  
 
    tan_info = (y2-y1)/(x2-x1); 
    if(x1 == x2)
    { 
      mpoint(0,0) = mpoint(1,0) = x1;
      if(y1 < y2)
      {
          mpoint(0,1) = y2 - R2;
          mpoint(1,1) = y1 + R1;
      }
      else
      {
          mpoint(0,1) = y2 + R2;
          mpoint(1,1) = y1 - R1;
      }
      return;  
    }
     
    double a = tan_info*tan_info + 1;
    double b = R1*R1;
    double tmp = sqrt(b/a);
   
    double x_root1 = tmp + x1;
    double x_root2 = -tmp + x1;
    double y_root1 = y1 + tan_info*tmp;
    double y_root2 = y1 - tan_info*tmp;
    if(x2 > x1)
    {
      mpoint(0,0) = x_root1;
      mpoint(0,1) = y_root1;
    }
    else
    {
      mpoint(0,0) = x_root2;
      mpoint(0,1) = y_root2;     
    }


    b = R2*R2;
    tmp = sqrt(b/a);
   
    x_root1 = tmp + x2;
    x_root2 = -tmp + x2;
    y_root1 = y2 + tan_info*tmp;
    y_root2 = y2 - tan_info*tmp;
    if(x1 > x2)
    {
      mpoint(1,0) = x_root1;
      mpoint(1,1) = y_root1;
    }
    else
    {
      mpoint(1,0) = x_root2;
      mpoint(1,1) = y_root2;     
    }

} 

