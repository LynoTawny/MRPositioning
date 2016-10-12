#ifndef  MR__TYPE_H_
#define  MR__TYPE_H_

#define ID_BUFFER_LEN   24

typedef unsigned long long  uint64;
typedef unsigned int        uint32;

//lte服务小区测量值
typedef struct
{
    int MR_LteScEarfcn;
    int MR_LteScPci;
    int MR_LteScRSRP;
    int MR_LteScTadv;
    int MR_LteScAOA;
    int MR_LteSceNBRxTxTimeDiff;
    int mask;
}lte_sc_t;

//lte邻区测量值
struct lte_neighbour_cell
{
    int MR_LteNcEarfcn;
    int MR_LteNcPci;
    int MR_LteNcRSRP;
    int MR_LteNcRSRQ;
    int mask;
};

//umts邻区测量值
struct umts_neighbour_cell
{
    int MR_TdsNcellUarfcn;
    int MR_TdsCellParameterId;
    int MR_TdsPccpchRSCP;
    int mask;
};

//gsm邻区测量值
struct gsm_neighbour_cell
{
    int MR_GsmNcellNcc;
    int MR_GsmNcellBcc;
    int MR_GsmNcellBcch;
    int MR_GsmNcellCarrierRSSI;
    int mask;
};

//对应一个v中的邻区信息，包含1个lte邻区，1个umts邻区，1个gsm邻区的测量值
typedef struct
{
    struct lte_neighbour_cell lte_nc;
    struct umts_neighbour_cell umts_nc;
    struct gsm_neighbour_cell gsm_nc;
}meas_nc_t;

//对应一个object，包含服务小区测量值和v_grp_count组邻区测量值
typedef struct
{
    uint32 enb_id;
    uint32 mme_code;
    uint32 mme_grp_id;
    uint32 mme_ue_s1ap_id;
    uint32 unix_time_stamp;
    char id[ID_BUFFER_LEN];
    lte_sc_t lte_sc;    //lte服务小区测量值
    int v_grp_count;    //表示object下有几个v
    meas_nc_t nc[];     //邻区测量值
}meas_obj_t;

#endif
