#ifndef  MR__TYPE_H_
#define  MR__TYPE_H_

#define ID_BUFFER_LEN   24

typedef unsigned long long  uint64;
typedef unsigned int        uint32;

//lte����С������ֵ
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

//lte��������ֵ
struct lte_neighbour_cell
{
    int MR_LteNcEarfcn;
    int MR_LteNcPci;
    int MR_LteNcRSRP;
    int MR_LteNcRSRQ;
    int mask;
};

//umts��������ֵ
struct umts_neighbour_cell
{
    int MR_TdsNcellUarfcn;
    int MR_TdsCellParameterId;
    int MR_TdsPccpchRSCP;
    int mask;
};

//gsm��������ֵ
struct gsm_neighbour_cell
{
    int MR_GsmNcellNcc;
    int MR_GsmNcellBcc;
    int MR_GsmNcellBcch;
    int MR_GsmNcellCarrierRSSI;
    int mask;
};

//��Ӧһ��v�е�������Ϣ������1��lte������1��umts������1��gsm�����Ĳ���ֵ
typedef struct
{
    struct lte_neighbour_cell lte_nc;
    struct umts_neighbour_cell umts_nc;
    struct gsm_neighbour_cell gsm_nc;
}meas_nc_t;

//��Ӧһ��object����������С������ֵ��v_grp_count����������ֵ
typedef struct
{
    uint32 enb_id;
    uint32 mme_code;
    uint32 mme_grp_id;
    uint32 mme_ue_s1ap_id;
    uint32 unix_time_stamp;
    char id[ID_BUFFER_LEN];
    lte_sc_t lte_sc;    //lte����С������ֵ
    int v_grp_count;    //��ʾobject���м���v
    meas_nc_t nc[];     //��������ֵ
}meas_obj_t;

#endif
