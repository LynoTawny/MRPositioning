#include <Eigen/Dense>
#include <iostream>
#include <stdio.h>
#include "aoa_toa.h"
#include "oku_hata.h"
#include "field_func.h"
//#include "monitor.h"
#include <drivetestitem.h>

extern DriveTestItem * curItem;
using namespace Eigen;
using namespace std;

typedef Matrix<int,Dynamic,Dynamic> MatrixInt;

#define MAX_BS_RECORD 6
#define MAX_INTER_POINT (MAX_BS_RECORD*(MAX_BS_RECORD-1))
#define SINGLE_POINT_JUDGE_CNT 100
int   relation_arr[MAX_BS_RECORD][MAX_BS_RECORD];
int   inter_point_flag[MAX_BS_RECORD][MAX_BS_RECORD];
int   g_row_cnt = 0;
int   filed_unit_clac(MatrixXd &mat_unit,MatrixXd &mat_res,int bsi,int bsj)
{
     
     double Xi,Xj,Yi,Yj,Di,Dj;
     double Am,Bm,Cm;
     double a,b,c;
//     cout <<"BASE  :"<<bsi<<","<<bsj<<endl;
//     cout << mat_unit<<endl;
     Xi = mat_unit(0,0);
     Yi = mat_unit(0,1);
     Di = mat_unit(0,2);

     Xj = mat_unit(1,0);
     Yj = mat_unit(1,1);
     Dj = mat_unit(1,2);
 
     if((Xj == Xi)&&(Yj == Yi))
     {
       fprintf(stdout,"%s\n","error bs pos\n");
       return -1;
     }
        
     double x_root1,x_root2;
     double y_root1,y_root2;
     Am = (Di*Di - Dj*Dj - (Xi*Xi + Yi*Yi) + (Xj*Xj + Yj*Yj))/(2*(Yj-Yi));
     Bm = (Xj-Xi)/(Yj-Yi); 
     Cm = Am - Yi;
     
     if(Xi == Xj)
     {
       y_root2 = y_root1 = Am;
       if(Di*Di > Cm*Cm)
       {
          x_root1 = Xi + sqrt(Di*Di - Cm*Cm);
          x_root2 = Xi - sqrt(Di*Di - Cm*Cm);  
       }
       else
       {
          fprintf(stderr,"%s\n","Xi == Xj  and not answer");
          return -1;
       }
       
     }
     if(Yi == Yj)
     {
       x_root2 = x_root1 = (Di*Di - Dj*Dj - (Xi*Xi + Yi*Yi) + (Xj*Xj + Yj*Yj))/(2*(Xj-Xi));
       double tmp = x_root1 - Xi;
       
       if(Di*Di > tmp*tmp)
       {
          y_root1 = Xi + sqrt(Di*Di - tmp*tmp);
          y_root2 = Xi - sqrt(Di*Di - tmp*tmp);  
       }
       else
       {
          fprintf(stderr,"%s\n","Yi == Yj  and not answer");
          return -1;
       }
     }
     if((Xi == Xj) || (Yi == Yj))
     {
      relation_arr[bsi][bsj] = g_row_cnt+1;
      mat_res(g_row_cnt,0) = x_root1;
      mat_res(g_row_cnt,1) = y_root1;
      mat_res(g_row_cnt,2) = bsi;
      mat_res(g_row_cnt,3) = bsj;
      g_row_cnt++; 
      mat_res(g_row_cnt,0) = x_root2;
      mat_res(g_row_cnt,1) = y_root2; 
      mat_res(g_row_cnt,2) = bsi;
      mat_res(g_row_cnt,3) = bsj;
      g_row_cnt++;
      return 0;
     }
     a = Bm*Bm+1;
     b = -2*(Xi+Bm*Cm);
     c = (Xi*Xi+Cm*Cm-Di*Di);
    
    double res = b*b - 4*a*c;
    if(res < 0)
    {
    //  fprintf(stderr,"%s\n","(ax2+bx+c=0) has no answer");
      return -1;
    }  
 

 
    x_root1 = (-b+sqrt(b*b-4*a*c))/(2*a);
    x_root2 = (-b-sqrt(b*b-4*a*c))/(2*a);

    y_root1 = Am - Bm*x_root1;
    y_root2 = Am - Bm*x_root2;
   
    if((res >= -0.001) && (res <= 0.001))
    {
      relation_arr[bsi][bsj] = SINGLE_POINT_JUDGE_CNT+g_row_cnt+1;
      mat_res(g_row_cnt,0) = x_root1;
      mat_res(g_row_cnt,1) = y_root1; 
      mat_res(g_row_cnt,2) = bsi;
      mat_res(g_row_cnt,3) = bsj;
      g_row_cnt++;
    }
    else
    {
      relation_arr[bsi][bsj] = g_row_cnt+1;
      mat_res(g_row_cnt,0) = x_root1;
      mat_res(g_row_cnt,1) = y_root1;
      mat_res(g_row_cnt,2) = bsi;
      mat_res(g_row_cnt,3) = bsj;
      g_row_cnt++; 
      mat_res(g_row_cnt,0) = x_root2;
      mat_res(g_row_cnt,1) = y_root2; 
      mat_res(g_row_cnt,2) = bsi;
      mat_res(g_row_cnt,3) = bsj;
      g_row_cnt++; 
    }
    return 0; 
}
//Hailun 
double calc_area(double x1,double y1,double x2,double y2,double x3,double y3)
{
   double a;
   double b;
   double c;
   double p;
   a = sqrt(pow(x2-x1,2)+pow(y2-y1,2));
   b = sqrt(pow(x3-x1,2)+pow(y3-y1,2)); 
   c = sqrt(pow(x3-x2,2)+pow(y3-y2,2)); 
   
   p = (a+b+c)/2;
  
   return sqrt(p*(p-a)*(p-b)*(p-c));
}

void  alg_avr_calc(MatrixXd &mat_res,VectorXd &ms_pos)
{
     double sum_x=0;
     double sum_y=0;

     for(int i = 0; i<g_row_cnt;i++)
     {
	sum_x += mat_res(i,0);
    	sum_y += mat_res(i,1);
     }
     if(g_row_cnt > 0 )
     {
        ms_pos(0) = sum_x /g_row_cnt;
        ms_pos(1) = sum_y /g_row_cnt;
     }
}


void  alg_mainbs_calc(MatrixXd &mat_res,VectorXd &ms_pos)
{
     int not_main_index = -1;
     double x1,y1,x2,y2,x3,y3;
     double min0_dist = 10e+10;
     double tmp_dist0 = 0;
     int x1bak,x2bak,y1bak,y2bak;
     int i,j;

     not_main_index = g_row_cnt;
     for(i=0;i<g_row_cnt;i++)
     {
      if(mat_res(i,2) != 0)
      {
        not_main_index = i;
        break;
      }
     }
     for(i=0;i<not_main_index;i++)
     {
       x1 = mat_res(i,0);
       y1 = mat_res(i,1);
 
      for(j=i+1;j<not_main_index;j++)
      {
       x2 = mat_res(j,0);
       y2 = mat_res(j,1);

       tmp_dist0 = pow(x1-x2,2)+pow(y1-y2,2);
       if(tmp_dist0 < min0_dist)
       {
 //        printf(":CALC MIN DIST:\n");
         min0_dist = tmp_dist0;
         x1bak = x1;
         x2bak = x2;
         y1bak = y1;
         y2bak = y2;
       }

      }
     }
     ms_pos(0) = (x1bak+x2bak) /2;
     ms_pos(1) = (y1bak+y2bak) /2;
}


MatrixInt mat_3bs(3,3);
MatrixInt mat_4bs(12,3);
MatrixInt mat_5bs(30,3);
MatrixInt mat_6bs(60,3);

double calc_2point_dist(double x1,double y1,double x2,double y2)
{
  return  sqrt(pow(x2-x1,2)+pow(y2-y1,2));
}

double calc_square_dist(VectorXd &p1,VectorXd &p2)
{
   return (pow(p1(0)-p2(0),2) + pow(p1(1) - p2(1),2));
}

double calc_total_ratio(MatrixXd &mat_circle,int bs_num,VectorXd &ms_pos)
{
   double sum_ratio = 0;
   int i=0;
   VectorXd p1(3);
   double sum_x = 0;
   double sum_y = 0;
   VectorXd vec_ratio(bs_num);
   double ratio;
   for(i=0;i<bs_num;i++)
   {
      p1 = mat_circle.row(i);
      ratio = 1/calc_square_dist(p1,ms_pos);
      sum_ratio += ratio;
      vec_ratio(i) = ratio;
   }
   for(i=0;i<bs_num;i++)
   {
     sum_x += mat_circle(i,0)*vec_ratio(i)/sum_ratio;
     sum_y += mat_circle(i,1)*vec_ratio(i)/sum_ratio;
   }
   ms_pos(0) = sum_x;
   ms_pos(1) = sum_y;
}

void alg_converge_calc(MatrixXd &mat_circle,VectorXd &mat_rxlevel,int bs_num,MatrixXd &mat_res,VectorXd &ms_pos)
{
  int cal_cnt = 0; //
  MatrixXd mpoint = MatrixXd::Zero(2,2);
  MatrixXd mid_point(1,2);
  MatrixXd bs(2,3);
  double sum_x = 0;
  double sum_y = 0;
  int    i     = 0;
  int    j     = 0;
  int    m,n,q;
  MatrixInt     matbs;
  double ratio1,ratio2;
  double square_val = 0;


  switch(bs_num)
  {
     case 1:
      printf("Only one bs info,return immediately!\n");
      return;
     break;
     case 2:
      printf("Only two bs info!!\n");
      bs.row(0) = mat_circle.row(0);
      bs.row(1) = mat_circle.row(1);
      calc_circles_point(bs,mpoint);

     
      sum_x += (mpoint(0,0)+mpoint(1,0));
      sum_y += (mpoint(0,1)+mpoint(1,1));
     
      if(relation_arr[0][1])
      {
         sum_x += (mat_res(0,0)+mat_res(1,0));
         sum_y += (mat_res(0,1)+mat_res(1,1));
         ms_pos(0) = sum_x/4;
         ms_pos(1) = sum_y/4;
      }
      else
      { 
         ms_pos(0) = sum_x/2;
         ms_pos(1) = sum_y/2;
      }
     
     return;
     break;
    case 3:
      matbs = mat_3bs;
      cal_cnt = 3;
     break;
    case 4:
      matbs = mat_4bs;
      cal_cnt = 12;
     break;
    case 5:
      matbs = mat_5bs;
      cal_cnt = 30;
     break;
    case 6:
      matbs  =mat_6bs;
      cal_cnt = 60;
     break;
    default: 
      printf("WRONG BS NUM, TAKE CARE!!!\n");
     break;
  }
  //printf("HHHHHHHHHHH 1\n");
  
  #define ANCHOR_COUNT 10000

  memset(inter_point_flag,0,sizeof(inter_point_flag));
  MatrixInt res_flag = MatrixInt::Zero(1,60);
  MatrixXd mid_res = MatrixXd::Zero(cal_cnt,2);
  double two_bs_dist = 0;

  for(i=0;i<cal_cnt;i++)
  {
     m = matbs(i,0);
     n = matbs(i,1);
     q = matbs(i,2);
     if(inter_point_flag[m][n] == 0)
     {
      two_bs_dist = calc_2point_dist(mat_circle(m,0),mat_circle(m,1),mat_circle(n,0),mat_circle(n,1));
      if(two_bs_dist >= mat_circle(m,2) + mat_circle(n,2) + 450)
      {
          inter_point_flag[m][n] = ANCHOR_COUNT;
          continue;
      } 
      bs.row(0) = mat_circle.row(m);
      bs.row(1) = mat_circle.row(n);
      calc_circles_point(bs,mpoint);
/*
      square_val = pow(bs(0,2),2) + pow(bs(1,2),2);
      ratio1 = pow(bs(0,2),2)/square_val;
      ratio2 = pow(bs(1,2),2)/square_val;

      mid_point(0) = mpoint(1,0)*ratio2 + mpoint(0,0)*ratio1;
      mid_point(1) = mpoint(1,1)*ratio2 + mpoint(0,1)*ratio1;
      mid_res.row(i) = mid_point.row(0);
*/

      mid_point(0) = (mpoint(0,0)+mpoint(1,0))/2;
      mid_point(1) = (mpoint(0,1)+mpoint(1,1))/2;
      mid_res.row(i) = mid_point.row(0);
      inter_point_flag[m][n] = i+1; 
     }
     else if(inter_point_flag[m][n] != ANCHOR_COUNT)
     {
   //   printf("point flag =%d,i=%d\n",inter_point_flag[m][n],i);
      mid_res.row(i) = mid_res.row(inter_point_flag[m][n]-1); 
      mid_point = mid_res.row(inter_point_flag[m][n]-1);
     }
     else
     {
        continue;
     }
     for(j=0;j<2;j++)
     {
      
    // printf("HHHHHH222,i = %d,[m,n,q]=[%d,%d,%d],relaiton[m,n]=%d\n",i,m,n,q,relation_arr[m][q]);
     if(relation_arr[m][q] != 0)
     {
       double dist1,dist2;
       int who = relation_arr[m][q]-1;
       if(who < MAX_INTER_POINT)
       {
        dist1 = calc_2point_dist(mid_point(0),mid_point(1), mat_res(who,0),mat_res(who,1));
        dist2 = calc_2point_dist(mid_point(0),mid_point(1), mat_res(who+1,0),mat_res(who+1,1));
        if(dist1 >= dist2)
        {
           res_flag(who) = relation_arr[m][q];
        }
        else
        {
           res_flag(who+1) = relation_arr[m][q]+1;
        }
       }
     }
     m = n;
    }
  }  

//   cout <<"MIDDLE POINT BETWEEN CIRCLES:"<<endl;
 //  cout <<mid_res<<endl;
 //  cout <<"RES_FLAG ARRAY:"<<endl;
 //  cout <<res_flag<<endl;
   int total_used_point = 0;
   for(i=0;i<g_row_cnt;i++)
   {
     if(res_flag(i) == 0)
     {
        total_used_point++;
        sum_x += mat_res(i,0);
        sum_y += mat_res(i,1); 
     } 
   }
   memset(inter_point_flag,0,sizeof(inter_point_flag));
   for(i=0;i<cal_cnt;i++)
   {
     m = matbs(i,0);
     n = matbs(i,1);
     if((relation_arr[m][n]==0)&& (mid_res(i,0) > 0)&&(inter_point_flag[m][n]==0))
     {
        total_used_point++;
        sum_x += mid_res(i,0);
        sum_y += mid_res(i,1);  
        inter_point_flag[m][n] = 1;
     } 
   }
   ms_pos(0) = sum_x/total_used_point;
   ms_pos(1) = sum_y/total_used_point;

   calc_total_ratio(mat_circle,bs_num,ms_pos);
}      

void  filed_calc_pos(MatrixXd &STATEA,VectorXd &mat_rxlevel,int rows,double  aoa,double toa,VectorXd &ms_pos)
{
     int i=0;
     int j=0;
#if 0
     aoa = (aoa + 90);
     if(aoa > 360)
     {
       aoa -= 360;
     }     
     if(aoa < 0)
     {
       if(rows < 3)
       {
       	fprintf(stdout,"%s\n","No enough info to calc pos"); 
	return; 
       }
     }
     else if(rows < 3)
     {
       if(toa < 0)
         aoa_toa_calc(STATEA,aoa,STATEA(0,2),ms_pos);    
       else
         aoa_toa_calc(STATEA,aoa,toa,ms_pos);
       return;
     } 
#endif
     g_row_cnt = 0;     
     MatrixXd mat_unit(2,3);
     MatrixXd mat_res=MatrixXd::Zero(MAX_INTER_POINT,4);  
     for(i=0;i<rows;i++)
     {
       for(j=i+1;j<rows;j++)
       {
        mat_unit.row(0) = STATEA.row(i);
        mat_unit.row(1) = STATEA.row(j);
        filed_unit_clac(mat_unit,mat_res,i,j);
  //      cout << "====INTERCOURSE POINT SHOW===="<<endl;
  //      cout << mat_res << endl;
       }
     }
//#define ALG_AVR_CALC 1
#if ALG_AVR_CALC
     alg_avr_calc(mat_res,ms_pos);
#elif ALG_MAINBS_CALC
     alg_mainbs_calc(mat_res,ms_pos);
#else 
     alg_converge_calc(STATEA,mat_rxlevel,rows,mat_res,ms_pos);      
 #endif  
}


void dump_bs_info(xml_data_t *pdata)
{
   int i = 0;
   printf("=============DUMP BS INFO=============\n");
   printf("AOA            :%.6f\n",pdata->aoa);
   printf("TOA            :%.6f\n",pdata->toa);
   printf("bs_num         :%d\n",pdata->bs_num);
//   printf("ms_long      :%.6f\n",pdata->true_lng);
//   printf("ms_lat      :%.6f\n",pdata->true_lat);
   for(i = 0;i<pdata->bs_num;i++)
   {
    printf("BASE STATION[%02d]\n",i);
    printf("POS_X          :%.6f\n",pdata->bs_data[i].pos_x);
    printf("POS_Y           :%.6f\n",pdata->bs_data[i].pos_y);
    printf("RXLEVEL         :%.6f\n",pdata->bs_data[i].rxlevel);
    printf("FREQ           :%.6f\n",pdata->bs_data[i].freq);
   }
}

void diff_dist(double x1,double y1,double x2,double y2)
{
  printf("======DIFF DIST : [%.6f]===\n",sqrt(pow(x1-x2,2)+pow(y1-y2,2)));
}

void bubble_sort(double *unsorted,int length,int *index)
 {
	 for (int i = 0; i < length; i++)
	{
		for (int j = i; j < length; j++)
		{
		 if (unsorted[i] > unsorted[j])
		 {
				double temp = unsorted[i];
				unsorted[i] = unsorted[j];
				unsorted[j] = temp;
				
				int temp2 = index[i];
				index[i] = index[j];
				index[j] = temp2;
		 }
		}
	}
 }

void dump_sort_res(double *pdata,int num)
{
    printf("**************DIST**************\n");
    for(int i=0;i<num;i++)
    {
      printf("%.6f\n",pdata[i]);
    }
    printf("*************END******************\n");
}
	
double get_bs_dist(MatrixXd &bs,int bs_num)
{
	 int i,j;
	 double total_record[MAX_BS_RECORD]={0,};
	 int index_record[MAX_BS_RECORD]={0,};
	 memset(total_record,0,sizeof(total_record));
	 
	 for(i=0;i<bs_num;i++)
	 {
	    index_record[i] = i;
	 }
	 for(i=0;i<bs_num;i++)
	 {
	   for(j=0;j<bs_num;j++)
	   {
		   if(i == j)
	       {
			   continue;
		   }
		   total_record[i] += calc_2point_dist(bs(i,0),bs(i,1),bs(j,0),bs(j,1));
	   }
	 }
         bubble_sort(total_record,bs_num,index_record);
         
 //        dump_sort_res(total_record,bs_num);

         return (total_record[bs_num-1] - total_record[0]);
}



double bs_pos[30][2];
char   pos_arr[1024];
int    pos_val = 0; 
double prev_x = 0;
double prev_y = 0;

static int init_flag = 0;
static int judge_flag = 0;
int get_ms_pos(void *pdata,ms_pos_t *ms_pos,int type)
{
   double dist = 0;
   xml_data_t *ptr = NULL;
   int   bs_num;
 
   bs_data_t  *pbs = NULL;
   MatrixXd bs=MatrixXd::Zero(MAX_BS_RECORD+1,3);
 
  
//   struct pos_longlati_t pos_lb[MAX_BS_RECORD];
//   struct pos_xy_t       pos_xy[MAX_BS_RECORD];

   if(init_flag == 0)
   {
     init_flag = 1;
//     pos_calc_init();

     mat_3bs<< 0,1,2,
               0,2,1,
               1,2,0;
     mat_4bs<< 0,1,2,0,1,3,
               0,2,1,0,2,3,
               0,3,1,0,3,2, 
               1,2,0,1,2,3,
               1,3,0,1,3,2,
               2,3,0,2,3,1;     
     mat_5bs<< 0,1,2,0,1,3,0,1,4,
               0,2,1,0,2,3,0,2,4,
               0,3,1,0,3,2,0,3,4,
               0,4,1,0,4,2,0,4,3,
               1,2,0,1,2,3,1,2,4,
               1,3,0,1,3,2,1,3,4,
               1,4,0,1,4,2,1,4,3,
               2,3,0,2,3,1,2,3,4,
               2,4,0,2,4,1,2,4,3,
               3,4,0,3,4,1,3,4,2;
                
     mat_6bs<< 0,1,2,0,1,3,0,1,4,0,1,5,
               0,2,1,0,2,3,0,2,4,0,2,5,
               0,3,1,0,3,2,0,3,4,0,3,5,
               0,4,1,0,4,2,0,4,3,0,4,5,
               0,5,1,0,5,2,0,5,3,0,5,4,
               1,2,0,1,2,3,1,2,4,1,2,5,
               1,3,0,1,3,2,1,3,4,1,3,5,
               1,4,0,1,4,2,1,4,3,1,4,5,
               1,5,0,1,5,2,1,5,3,1,5,4,
               2,3,0,2,3,1,2,3,4,2,3,5,
               2,4,0,2,4,1,2,4,3,2,4,5,
               2,5,0,2,5,1,2,5,3,2,5,4,
               3,4,0,3,4,1,3,4,2,3,4,5,
               3,5,0,3,5,1,3,5,2,3,5,4,
               4,5,0,4,5,1,4,5,2,4,5,3;
   }
   if((NULL == pdata) ||(NULL == ms_pos))
   {
     fprintf(stderr,"%s [%s]\n","NULL pointer in ",__FUNCTION__);
     return 0;
   }
   ptr = (xml_data_t*)pdata;
   bs_num = ptr->bs_num;
   if(bs_num < 1)
   {
    fprintf(stdout,"%s\n","bs data not good");
    return -1;
   }
 
 #if 1 
  // dump_bs_info(ptr);

   pbs = ptr->bs_data;

   bs_num = bs_num > MAX_BS_RECORD ? MAX_BS_RECORD : bs_num;

//   pos_lb[0].longti = ptr->true_lng;
//   pos_lb[0].lati   = ptr->true_lat;
//   for(int i=1;i<bs_num+1 ;i++)
//   {
//     pos_lb[i].longti = pbs[i-1].longitude;  //xiaochunag
//     pos_lb[i].lati  = pbs[i-1].latitude;
//   }
//   pos_convert_xy(pos_lb,pos_xy,bs_num+1);
   // end

	VectorXd mat_rxlevel(bs_num);
   for(int i=0;i<bs_num ;i++)
   {
//      bs(i,0) = pos_xy[i+1].x;  //xiaochunag
//      bs(i,1) = pos_xy[i+1].y;
       bs(i, 0) = pbs[i].pos_x;
       bs(i, 1) = pbs[i].pos_y;
/*      if(type == enum_gsm)
        bs(i,2) = oku_hata_simple(pbs[i].rxlevel,pbs[i].freq);   
      else
        bs(i,2) = cost_hata_simple(pbs[i].rxlevel,pbs[i].freq);
*/
        mat_rxlevel(i) = pbs[i].rxlevel;
        if(pbs[i].freq < 1800)
             bs(i,2) = oku_hata_simple(pbs[i].rxlevel,pbs[i].freq);   
        else
             bs(i,2) = cost_hata_simple(pbs[i].rxlevel,pbs[i].freq);
    }
#endif

   //保存计算出的半径到DriveItem
    for(int i = 0; i < bs_num; i++)
    {
        if(curItem == NULL) break;
        curItem->setBaseRadius(i, bs(i,2));
    }
//   cout <<"-----SHOW BS STRUCT [x y dist]-----"<<endl;
//   cout << bs <<endl;

   VectorXd ms(2);
   ms<<0,0;
   memset(relation_arr,0,sizeof(relation_arr));
   filed_calc_pos(bs,mat_rxlevel,bs_num,ptr->aoa,ptr->toa,ms); //x,y ->xiaochuang

    
   for(int i=0;i<bs_num;i++)
   {  
      double tdist = calc_2point_dist(bs(i,0),bs(i,1),ms(0),ms(1));
    //  printf("BS[%d]-[real-dist=%.6f]=[calc-dist=%.6f,R=%.6f]\n",i,calc_2point_dist(bs(i,0),bs(i,1),pos_xy[0].x,pos_xy[0].y),tdist,bs(i,2));
   }   

   double max_dist = 0;
   double r_avr = 0;
   for(int i=0;i<bs_num;i++)
   {
     r_avr += bs(i,2);
   }
   r_avr = r_avr/bs_num;

   max_dist = get_bs_dist(bs,bs_num);
   cout<<"=========max_dist ="<<max_dist <<endl;
   cout<<"=========Ravr     = "<<r_avr<<endl;
   if(judge_flag)
   {
     double sina = (ms(1) - prev_y)/calc_2point_dist(ms(0),ms(1),prev_x,prev_y);
     double cosa = (ms(0) - prev_x)/calc_2point_dist(ms(0),ms(1),prev_x,prev_y);
     prev_x = ms_pos->pos_x = 50 * cosa + prev_x;
     prev_y = ms_pos->pos_y = 50 * sina + prev_y;
   }else{
     ms_pos->pos_x = ms(0);
     ms_pos->pos_y = ms(1);
   }
   if((max_dist < 1500) && (r_avr < 200))
   {
     prev_x =ms_pos->pos_x = ms(0);
     prev_y =ms_pos->pos_y = ms(1);
     judge_flag = 1;
     cout<<"----------JUDGE POINT ="<<pos_val+1<<endl;
   }
   pos_val++;
/*   
   cout << "##MS_POSITION:[ "<<ms_pos->pos_x<<","<<ms_pos->pos_y<<" ]"<<endl;   
   
   diff_dist(ms_pos->pos_x,ms_pos->pos_y,pos_xy[0].x,pos_xy[0].y);

   bs_pos[pos_val][0]=ms_pos->pos_x;
   bs_pos[pos_val][1]=ms_pos->pos_y;
   pos_val++;
   if(pos_val >= 20)
   {
       printf("BEGIN DRAW LINE \n");
       int cnt_pos = 0;
       int tmp_pos = 0;
       for(int i =0 ;i<pos_val-1;i++){
          tmp_pos = sprintf(pos_arr+cnt_pos,"%.6f,%.6f|",bs_pos[i][0],bs_pos[i][1]);
          cnt_pos += tmp_pos;
       }
       tmp_pos = sprintf(pos_arr+cnt_pos,"%.6f,%.6f",bs_pos[pos_val-1][0],bs_pos[pos_val-1][1]);
       printf("all_pos = [%s]\n",pos_arr);
       struct pos_xy_t pos_xy[30];
       convert_xy_lb(pos_xy,pos_val,pos_arr);
       show_multi_points();
       exit(0);
   }
*/
 // show_xy_func(ms_pos->pos_x,ms_pos->pos_y);


   return 0;
}

