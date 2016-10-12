#ifndef __FILED_FUNC_H__
#define __FILED_FUNC_H__

typedef struct{
  double pos_x;
  double pos_y;
//  double longitude; //jingdu
//  double latitude;
}ms_pos_t;

typedef struct{
    double pos_x;
    double pos_y;
//      double longitude;
//      double latitude;
    double rxlevel;
    double freq;
}__attribute__((packed)) bs_data_t;

typedef struct{
//  double true_lng;
//  double true_lat;
  double aoa;
  double toa;
  int   bs_num;
  bs_data_t bs_data[0];
}__attribute__((packed)) xml_data_t;


typedef enum{
  enum_lte,
  enum_umts,
  enum_gsm,
}ran_type_e;

//monitor.h
typedef enum{
    CONVERT_XY = 0x90,
    CONVERT_LB,
    DRAW_MAP,
}action_map_e;

struct pos_longlati_t{
    double longti;
    double lati;
};


#define CONVERT_BUFF_LEN 1024
struct action_map_t{
    int  action;
    int  len;
    char conv_buf[CONVERT_BUFF_LEN];
}__attribute__((packed));

#define XY_ARR_SIZE  30

struct pos_xy_t{
   double x;
   double y;
};
//

int get_ms_pos(void *pdata,ms_pos_t *ms_pos,int type);

#endif
