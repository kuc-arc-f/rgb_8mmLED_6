#include "mbed.h"
#include <string>

DigitalOut mDIN_H(dp1);
DigitalOut mDIN_L(dp2);

Serial mPc(USBTX, USBRX);

//define
const int mModeNormal= 0;
const int mModeHigh   =1;
const int mModeOne   =2;
const int mModeAll    =3;
const int mMaxPlay=4;

const int mBit_MAX=8;
const int mLED_MAX=4;

const int mWait_US=5;
// const int mColor_White=0;
int mSTAT =0;
int mSTAT_RSV_START=1;
int mSTAT_DISP=2;

const unsigned char mColorMap[9][3] = {
  {0xff,0xa5,0x00 }, // #ffa500 Orange
  {0xd2,0x69,0x1e }, // #d2691e chocolate
  {0x80,0x00,0x00 }, // #800000 red 
  {0x80,0x00,0x80 }, // #800080 purple
  {0x00,0x80,0x00 }, // #008000 green
  {0xff,0xff,0x00 }, // #ffff00 Yellow
  {0x00,0x00,0xff }, // #0000ff blue
  {0x00,0xff,0xff }, // #00ffff aqua
  {0xff,0xff,0xff }, // #ffffff white
};

string mResponse="";
string mReceive="";
//
int Is_bitPosition(int src , int Position){
    int iRet=0;
        int    iLen=mBit_MAX;
        char    buff[mBit_MAX+1];
        int     bin;
        int     i1;

        bin = src;
        buff[iLen] = '\0';
        for(int j = 0; j < iLen; j++){
            buff[j]='0';
        }
        
        for(i1 = 0; i1 < iLen; i1++){
                if(i1 != 0 && bin == 0)
                        break;
                if(bin % 2 == 0)
                        buff[(iLen-1)-i1] = '0';
                else
                        buff[(iLen-1)-i1] = '1';
                bin = bin / 2;
        }
//printf(">>>%s\n", buff );
    if(strlen(buff) >= 8 ){
      if(buff[Position]=='1'){ iRet=1; }
    }
   return iRet;
}

//
void LED_Set() {
  mDIN_H=0;
  mDIN_L=1;
  wait_ms(1);
}

//
void LED_Low_Bit() {
  mDIN_H=0;
  mDIN_L=0;
  wait_us(mWait_US);
  mDIN_H=0;
  mDIN_L=1;
  wait_us(mWait_US);
}

//
void LED_Hi_Bit() {
  mDIN_H=1;
  mDIN_L=1;
  wait_us(mWait_US);
  mDIN_H=0;
  mDIN_L=1;
  wait_us(mWait_US);
}

//
void LED_Init() {
  mDIN_H=0;
  mDIN_L=1;
  wait_ms(10); 
}

//
void LED_Color_RGB(unsigned int led_r, unsigned int led_g, unsigned int led_b) {
//printf("bit_r=%d, g=%d, b=%d \n" , led_r, led_g, led_b);
  //blue
  for (int k = 0; k < mBit_MAX; k++){
      if( Is_bitPosition(led_b , k)==1){
          LED_Hi_Bit();
      }else{
          LED_Low_Bit();
      }
  }
  //green
  for (int k = 0; k < mBit_MAX; k++){
      if( Is_bitPosition(led_g , k)==1){
          LED_Hi_Bit();
      }else{
          LED_Low_Bit();
      }
  }
  //red
  for (int k = 0; k < mBit_MAX; k++){
      if( Is_bitPosition(led_r , k)==1){
          LED_Hi_Bit();
      }else{
          LED_Low_Bit();
      }
  }
}
//
void init_proc(){
    mDIN_H =0;
    mDIN_L =1;

    LED_Init();
    wait_ms(10);
}

//
void proc_display(int iCt, int iMode){
    if(iMode==mModeNormal){
        for(int n=0;n< mLED_MAX;n++){
          LED_Color_RGB(mColorMap[iCt][0], mColorMap[iCt][1], mColorMap[iCt][2] );
          wait_ms(500);
        }
        LED_Set();  
    }else if(iMode==mModeHigh){
        //HIGH
        for(int j=0; j < 3; j++){
            for(int n=0;n< mLED_MAX;n++){
              LED_Color_RGB(0x00, 0x00, 0x00);
            }
            LED_Set();
            wait_ms(200);            
            for(int n=0;n< mLED_MAX;n++){
              LED_Color_RGB(mColorMap[iCt][0], mColorMap[iCt][1], mColorMap[iCt][2] );
              wait_ms(120);
            }
            LED_Set();
        }
    }else if(iMode==mModeOne){
            //One
            for(int n=0;n< mLED_MAX;n++){
                //set
                for(int k=0;k< mLED_MAX; k++){
                    if(n==k){
                        LED_Color_RGB(mColorMap[iCt][0], mColorMap[iCt][1], mColorMap[iCt][2] );
                    }
                    else{
                        LED_Color_RGB(0x00, 0x00, 0x00);
                    }
                }
                LED_Set();
                wait_ms(500);
            }
            for(int j=0;j< mLED_MAX; j++){
                LED_Color_RGB(mColorMap[iCt][0], mColorMap[iCt][1], mColorMap[iCt][2] );
            }
            LED_Set();
            wait_ms(1000);
    }else if(iMode ==mModeAll){
            for(int n=0;n< mLED_MAX;n++){
              LED_Color_RGB(mColorMap[iCt][0], mColorMap[iCt][1], mColorMap[iCt][2] );
            }
            LED_Set();
            wait(3);
    }
}  

// 
void proc_uart(){
   if( mPc.readable()) {
        char c= mPc.getc();
        mResponse+= c;
   }else{
       if(mSTAT ==mSTAT_RSV_START){
//printf("mResponse=%s\n" , mResponse.c_str() );                   
//           if(mResponse.length() >= 6){
             if(mResponse.length() >= 10){               
               string sHead=mResponse.substr(0,4);
               if( sHead== "rgb="){
                   mReceive =mResponse.substr(4 );
printf("sTmp=%s\n" , mReceive.c_str() );                   
                   mSTAT =mSTAT_DISP;
               }
               mResponse="";
           }
       }
   }
}

//
unsigned int pow_get(int src, int iPnum){
    int ret=1;

    if(iPnum > 0){
        ret=src;
        for(int i=0; i<iPnum-1; i++){
          ret=ret * src;
        }
    }
    return ret;
}
unsigned int transUInt(unsigned char c){
    if('0'<=c && '9'>=c) return (c-0x30);//0x30は'0'の文字コード
    if('A'<=c && 'F'>=c) return (c+0x0A-0x41);//0x41は'A'の文字コード
    if('a'<=c && 'f'>=c) return (c+0x0A-0x61);//0x61は'a'の文字コード
    return 0;
}

//
unsigned int hexToUInt(char *str)
{
// printf("hexToUInt ,ptr=%s\n" , &str[0]);
    unsigned int i,j=0;
    char*str_ptr=str+strlen(str)-1;

    for(i=0;i<strlen(str);i++){
//printf("tr=%d\n" ,pow_get(16, i));
//printf("pow=%d\n" , (unsigned int)pow(16,i));
        j+=transUInt(*str_ptr--)*pow_get(16, i);
//printf("j=%d ,i=%d\n", j, i);
    }
    return j;
}

//
void display_color(string sRes){
    if(sRes.length() < 6){ return; }
    char s16_1[2+1];
    char s16_2[2+1];
    char s16_3[2+1];
    
    string cR=sRes.substr(0, 2);
    sprintf(s16_1, "%s" , cR.c_str() );
    unsigned int i_R= hexToUInt(s16_1);    
//    int i_R= atoi(sBuff.c_str() );
    string cG=sRes.substr(2, 2);
    sprintf(s16_2, "%s" , cG.c_str() );
    unsigned int i_G= hexToUInt(s16_2);    
    //int i_G=atoi(sBuff.c_str());
    string cB=sRes.substr(4, 2);
    sprintf(s16_3, "%s" , cB.c_str() );
    unsigned int i_B= hexToUInt(s16_3 );      
//    int i_B=atoi(sBuff.c_str());
    for(int n=0;n< mLED_MAX;n++){
      LED_Color_RGB(i_R, i_G, i_B );
    }
    LED_Set();
    wait(2.5);

    for(int n=0;n< mLED_MAX;n++){
      LED_Color_RGB(0x00, 0x00, 0x00);
    }
    LED_Set();
//    wait_ms(100);
    
}
//    
int main() {
    mSTAT = mSTAT_RSV_START;
    mPc.baud(9600 );
    mPc.printf("#Start-main\n");
    wait(3); //wait-Start
    int iWait=2;    
    init_proc();
    int iCt=0;
    int iCtPlay=0;
    while(1) {
       if(mSTAT != mSTAT_DISP){
           proc_uart();
           wait_ms(5);
       }else{
           display_color(mReceive);
           mSTAT =mSTAT_RSV_START;
       }
    }    
}

