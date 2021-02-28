#include "smartHome.h"
#include "sapi.h"
#include "hal_led.h"
#include"onBoard.h"
#include "hal_uart.h"
#include "stdio.h"

#define uchar unsigned char 
#define uint unsigned int
#define  DHT11_DATA  P0_4
#if 1
  #define NUM_LED_1 1
  #define NUM_LED_2 2
#elif 1
  #define NUM_LED_1 3
  #define NUM_LED_2 4
#else
  #define NUM_LED_1 5
  #define NUM_LED_2 6
#endif

#define LEDNUM  2
uint8 ledIdList[LEDNUM]={NUM_LED_1,NUM_LED_2};

#define NUM_IN_CMD_LEDDEVICE 1
#define NUM_OUT_CMD_LEDDEVICE 2

/*****************ȫ�ֱ����Ķ���******************/
uchar  Overtime_counter;  //�жϵȴ��Ƿ�ʱ�ļ�����������uchar�͵���ֵ��Χ�����Զ���ʱ���ƣ�ʱ���ɳ�ֵ�����������ж��Ƿ�ʱ
uchar  bit_value;          //��DATA�����϶�����λֵ
uchar  T_data_H, T_data_L, RH_data_H, RH_data_L, checkdata;//У������¶ȸ�8λ,�¶ȵ�8λ,ʪ�ȸ�8λ,ʪ�ȵ�8λ,У���8λ
uchar  T_data_H_temp, T_data_L_temp, RH_data_H_temp, RH_data_L_temp, checkdata_temp;//δ��У�������
uchar  comdata;            //��DHT11��ȡ��һ���ֽڵ�����

char  str[16];


const cId_t ledDeviceInputCommandList[NUM_IN_CMD_LEDDEVICE]=
                                {TOGGLE_LED_CMD_ID};
const cId_t ledDeviceOutputCommandList[NUM_OUT_CMD_LEDDEVICE]=
                                {LEDJOINNET_CMD_ID,HEART_BEAT_CMD_ID};
const SimpleDescriptionFormat_t zb_SimpleDesc=
{
  ENDPOINT_ID_SMARTHOME,
  PROFILE_ID_SMARTHOME,
  DEVICE_ID_LEDDEVICE,
  DEVIDE_VERSION_ID,
  0,
  NUM_IN_CMD_LEDDEVICE,
  (cId_t*)ledDeviceInputCommandList,
  NUM_OUT_CMD_LEDDEVICE,
  (cId_t*)ledDeviceOutputCommandList  
};

void Read_Byte(void);

/*****************��DHT11��ȡһ���ֽں���******************/        
void  Read_Byte(void)
{
  uchar i;
  for (i = 0; i < 8; i++)                     //ѭ��8�Σ���ȡ8bit������
  {
    Overtime_counter = 2;                          //��ȡ���ȴ�DHT11������12-14us�͵�ƽ��ʼ�ź�
    P0DIR &= ~0x10;
    while ((!DHT11_DATA) && Overtime_counter++);
    //Delay_10us(80);                   //26-28us�ĵ͵�ƽ�ж�����
    MicroWait(27);
    bit_value = 0;                          //�������޺��ж������Ǹ߻��ǵͣ���Ϊ1����Ϊ0
    if(DHT11_DATA)
    bit_value = 1;
    Overtime_counter=2;                          //�ȴ�1bit�ĵ�ƽ�źŽ�����������0��1��118us�󶼱�Ϊ�͵�ƽ���������ʱ
    while (DHT11_DATA && Overtime_counter++);  //��U8FLAG�ӵ�255�����Ϊ0������ѭ��������Ӽ�Ϊ1
    if (Overtime_counter == 1)
      break;                           //��ʱ������forѭ��        
    comdata <<= 1;                      //����1λ��LSB��0
    comdata |= bit_value;                  //LSB��ֵ
  }
}


int Read_DHT11(unsigned char *temp,unsigned char *humid);
/*****************DHT11��ȡ����ֽں���******************/
/*
������������ȡdht11����ʪ��
����˵����
    *temp�������¶ȵ���������
    *humid������ʪ�ȵ���������
����ֵ��
  0���ɹ���ȡ
  <0����ȡʧ��
*/
int Read_DHT11(unsigned char *temp,unsigned char *humid)
{
    int result=-1;
    uchar checksum;
    P0DIR |= 0x10;
    DHT11_DATA = 0;                //��������18ms
    MicroWait(18000);
    DHT11_DATA = 1;                //������������������ ������ʱ20us-40us    
    MicroWait(35);
    DHT11_DATA = 1;                //����תΪ�����������ߵ�ƽ��DATA���������������ߣ�׼���ж�DHT11����Ӧ�ź�
    P0DIR &= ~0x10;
    if (!DHT11_DATA)                //�жϴӻ��Ƿ��е͵�ƽ��Ӧ�ź� �粻��Ӧ����������Ӧ����������        
    {
      Overtime_counter = 2;   //�ж�DHT11������80us�ĵ͵�ƽ��Ӧ�ź��Ƿ����
      while ((!DHT11_DATA)&&Overtime_counter++);
      Overtime_counter=2;   //�ж�DHT11�Ƿ񷢳�80us�ĸߵ�ƽ���緢����������ݽ���״̬
      while ((DHT11_DATA)&&Overtime_counter++);
      Read_Byte();                //��ȡʪ��ֵ�������ֵĸ�8bit
      RH_data_H_temp = comdata;
      Read_Byte();                //��ȡʪ��ֵС�����ֵĵ�8bit
      RH_data_L_temp = comdata;
      Read_Byte();                //��ȡ�¶�ֵ�������ֵĸ�8bit
      T_data_H_temp = comdata;
      Read_Byte();                //��ȡ�¶�ֵС�����ֵĵ�8bit
      T_data_L_temp = comdata;
      Read_Byte();                //��ȡУ��͵�8bit
      checkdata_temp = comdata;
      P0DIR |= 0x10;
      DHT11_DATA = 1;                //�������ݽ���������
      checksum = (T_data_H_temp + T_data_L_temp + RH_data_H_temp + RH_data_L_temp);//��������У��
      if (checksum == checkdata_temp)
        {
          RH_data_H = RH_data_H_temp;//����ʪ�ȵ���������
          RH_data_L = RH_data_L_temp;//����ʪ�ȵ�С������
          T_data_H  = T_data_H_temp;//�����¶ȵ���������
          T_data_L  = T_data_L_temp;//�����¶ȵ�С������
          checkdata = checkdata_temp;
          *temp = T_data_H;
          *humid = RH_data_H;
          result=0;
        }
      
    }
    return result;
}

/***********
ִ��ʱ�������͵����ݰ������շ��յ�ʱ������
handle:���ı�ţ�
status:ZSUCCESS��ʾ�ɹ�����
************/
void zb_SendDataConfirm( uint8 handle, uint8 status )
{
  
}

/***********
ִ��ʱ�������յ������ݰ�������
************/
void zb_ReceiveDataIndication( uint16 source, uint16 command, 
                              uint16 len, uint8 *pData  )
{
  
}


void zb_AllowBindConfirm( uint16 source )
{
}

void zb_HandleKeys( uint8 shift, uint8 keys )
{
  
}

void zb_BindConfirm( uint16 commandId, uint8 status )
{
}


//void zb_SendDataRequest ( uint16 destination, uint16 commandId, uint8 len,
//                          uint8 *pData, uint8 handle, uint8 ack, uint8 radius );
void zb_StartConfirm( uint8 status )
{
  if(status==ZSUCCESS)
  {   
    halUARTCfg_t uartcfg;
    uartcfg.baudRate=HAL_UART_BR_115200;
    uartcfg.flowControl=FALSE;
    uartcfg.callBackFunc=NULL;
    HalUARTOpen(0,&uartcfg);
    HalUARTWrite(0,"join success\r\n",osal_strlen("join success\r\n"));
    //�ɰѽڵ���������led�Ƶ�ID�ŷ��͹�ȥ
    zb_SendDataRequest(0X0,LEDJOINNET_CMD_ID,LEDNUM,ledIdList,0,FALSE,AF_DEFAULT_RADIUS);
    osal_start_timerEx(sapi_TaskID,TIMER_TIMEOUT_EVT,2000);   
    osal_start_timerEx(sapi_TaskID,READ_DHT11_EVENT,5000);
  }
}

void zb_HandleOsalEvent( uint16 event )
{
  if(event&READ_DHT11_EVENT){//��ȡDHT11�����¼�
    unsigned  char temphumi[2];
    Read_DHT11(temphumi,temphumi+1);
    if(temphumi!=NULL)
    {
      osal_start_timerEx(sapi_TaskID,READ_DHT11_EVENT,5000);
      //char buffer[50];
      //sprintf(buffer,"temp=%d,humid=%d\r\n",temp,humid);
      //sprintf(buffer,"{\"temp\":\"%d\"}",temp);
      //HalUARTWrite(0,(uint8*)buffer,osal_strlen(buffer)); 
      zb_SendDataRequest(0X0,TEMP_HUMI_CMD_ID,osal_strlen(temphumi),temphumi,0,FALSE,AF_DEFAULT_RADIUS);
      //sprintf(buffer,"{\"humi\":\"%d\"}",humid);
      HalUARTWrite(0,temphumi,osal_strlen(temphumi));
      //zb_SendDataRequest(0X0,TEMP_HUMI_CMD_ID,osal_strlen(buffer),buffer,0,FALSE,AF_DEFAULT_RADIUS);
      
     }
   }
  
  if(event&TIMER_TIMEOUT_EVT){
      osal_start_timerEx(sapi_TaskID,TIMER_TIMEOUT_EVT,2000);
      zb_SendDataRequest(0X0,HEART_BEAT_CMD_ID,0,NULL,0,FALSE,AF_DEFAULT_RADIUS); //���������� 
   }
}

void zb_FindDeviceConfirm( uint8 searchType, 
                          uint8 *searchKey, uint8 *result )
{
  
}