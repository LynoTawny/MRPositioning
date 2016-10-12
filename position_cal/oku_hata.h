#ifndef __OKU_HATA_H__
#define __OKU_HATA_H__
extern double oku_hata_dist(double rxlevel,double frq_car,double hgh_rx,double hgt_tx,int zone);
extern double oku_hata_simple(double rxlevel,double frq);
extern double cost_hata_dist(double rxlevel,double frq_car,double hgh_rx,double hgt_tx,int zone);
extern double cost_hata_simple(double rxlevel,double frq);



#endif
