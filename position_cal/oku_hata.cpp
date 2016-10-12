#include <Eigen/Dense>

#define  BASE_GSM_POWER  33 //dBm
#define  BASE_LTE_POWER  33
#define  BS_ANNE_HIGH 30
#define  MS_ANNE_HIGH   1.5

typedef enum
{
  MID_CITY=1,
  LARGE_CITY,
  URBAN_AREA,
  TOWN_AREA,
}hata_area_e;

using namespace Eigen;
double oku_hata_dist(double rxlevel,double frq_car,double hgt_rx,double hgt_tx,int zone)
{
  double L = BASE_GSM_POWER - rxlevel;
  double alpha = 0;
  double tmp  = 0;
  double dist = 0;
  switch(zone)
  {
   case MID_CITY:
    alpha = (1.1*log10(frq_car)-0.7)*hgt_rx - (1.56*log10(frq_car)-0.8);
   break;
   case LARGE_CITY:
    tmp = log10(11.75*hgt_rx);
    alpha = 3.20*(tmp*tmp)-4.97;
   break;
   case URBAN_AREA:
    tmp = log10(frq_car/28);
    alpha = 2*tmp*tmp+5.4;
   break;
   case TOWN_AREA:
    tmp = log10(frq_car);
    alpha = 4.78*tmp*tmp+18.33*log10(frq_car)+40.98;
   break;
  } 
  tmp = (L-(69.55+26.16*log10(frq_car)-13.82*log10(hgt_tx)-alpha))/(44.9-6.55*log10(hgt_tx));
  dist = pow(10,tmp)*1000;
  return dist;  
}

double oku_hata_simple(double rxlevel,double frq)
{
  if((frq > 2000) && (frq < 300))
     return 0;
  return oku_hata_dist(rxlevel,frq,MS_ANNE_HIGH,BS_ANNE_HIGH,LARGE_CITY);
}

typedef enum{
   CM0=0,
   CM1=3,
}cm_value_e;

double cost_hata_dist(double rxlevel,double frq_car,double hgt_rx,double hgt_tx,int zone)
{
  double L = BASE_LTE_POWER - rxlevel;
  double alpha = 0;
  double tmp  = 0;
  double dist = 0;
  int cm = CM0;
  switch(zone)
  {
   case MID_CITY:
    alpha = (1.1*log10(frq_car)-0.7)*hgt_rx - (1.56*log10(frq_car)-0.8);
   break;
   case LARGE_CITY:
    tmp = log10(11.75*hgt_rx);
    alpha = 3.20*(tmp*tmp)-4.97;
    cm = CM1;
   break;
   case URBAN_AREA:
    tmp = log10(frq_car/28);
    alpha = 2*tmp*tmp+5.4;
   break;
   case TOWN_AREA:
    tmp = log10(frq_car);
    alpha = 4.78*tmp*tmp+18.33*log10(frq_car)+40.98;
   break;
  } 
  tmp = (L-(46.3+33.9*log10(frq_car)-13.82*log10(hgt_tx)-alpha + cm))/(44.9-6.55*log10(hgt_tx));
  dist = pow(10,tmp)*1000;
  return dist;  
}

double cost_hata_simple(double rxlevel,double frq)
{
//  if(frq <= 1000)
 //    return 0;
  return cost_hata_dist(rxlevel,frq,MS_ANNE_HIGH,BS_ANNE_HIGH,MID_CITY);
}


