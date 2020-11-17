#include "logic_app.h"
#include "algorithm.h"
#include "dataServer.h"

#define debug_print printf
int main(void)
{	


	u8 machine_status = 0;//, i = 0;

    initDataServer("top.txt");
	
    Get_Machine_Status();


}


u8 Get_Machine_Status(void)
{
	static u16 i = 0;
    int *dat = malloc(6*sizeof(int));
    //bool res = getData(dat);

    while (getData(dat))
    {
        short aacx = dat[0],aacy=dat[1],aacz=dat[2];		//���ٶȴ�����ԭʼ����
        short gyrox=dat[3],gyroy=dat[4],gyroz=dat[5];	//������ԭʼ����

        float value_stand, value_motion;
        u8 status = STATE_NONE;


        //	ICM20689_Get_INT_Status(&int_state);
        if(1)
        {

            //debug_print("ax is %d,ay is %d,az is %d\r\n",aacx,aacy,aacz);
            //debug_print("gx is %d,gy is %d,gz is %d\r\n",gyrox,gyroy,gyroz);
            if(i<1000)
            {
                gx_in[i] = (float)(gyrox-0.00f)*500/32768;
                gy_in[i] = (float)(gyroy-0.00f)*500/32768;
                gz_in[i] = (float)(gyroz-0.00f)*500/32768;


                ax_in[i] = (float)(aacx-0.00f)*4/32768;
                ay_in[i] = (float)(aacy-0.00f)*4/32768;
                az_in[i] = (float)(aacz-0.00f)*4/32768;

                i++;
            }
            else
            {
                i = 0;





                value_stand = optimi(ax_in, ay_in,az_in);//�õ��жϾ�ֹ״̬��ֵ
                value_motion = optimi(gx_in,gy_in,gz_in);//�õ����㷨��ֵ
                putData(value_stand, value_motion);

                //���㷨�ж�״̬
                if(value_motion>0.42f)
                {
                    status = STATE_MOTION;//�˶�״̬
                }
                else
            {
                    //debug_print("value_stand is %f.\r\n",value_stand);
                    if(value_stand>0.02f)//��ʼ��0.00194
                    {
                    status = STATE_STANDBY;//����״̬
                    }
                    else
                    {
                        status = STATE_MOTIONLESS;
                    }
                }
                //debug_print("Have get status!\r\n");
            }
        }
    }
    dump();
    return 0;
}


