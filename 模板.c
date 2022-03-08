/*
	例程：IAP15程序模板 -- 矩阵按键状态机 + 数码管扫描
	作者：电子设计工坊
	重点：	1.用STC-ISP生成1ms的定时器中断，用于扫描按键、数码管等；
			2.用状态机来实现按键扫描；一个好的按键程序，必须有：（1）消抖；（2）按键状态的判断，包括判断按键按下，判断按键弹起；（3）不阻塞程序，不用delay消抖
				所以三行按键程序和状态机程序都是非常好的。
			3.有些变量是static静态类型，同学们要仔细体会。
			
	Copyright (c) 2020 电子设计工坊 dianshe.taobao.com
	All rights reserved
*/
#include "STC15F2K60S2.h"	 //STC15系列的头文件，添加方法参考视频：https://www.bilibili.com/video/av95391542/

#define	u8	unsigned char
#define	u16	unsigned int

u8 code smg_du[]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x00}; //0-9 
u8 code smg_wei[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

#define KEY P3
#define NO_KEY		0xff	//无按键按下
#define KEY_STATE0  0   	//判断按键按下
#define KEY_STATE1  1  		//确认按键按下
#define KEY_STATE2  2  		//释放按键
	
unsigned char Key_Scan()
{
	static unsigned char key_state=KEY_STATE0; //定义为静态变量，用于保存每次按键的状态
	u8 key_value=0,key_temp;				   //key_val:返回的键值；key_temp:读取的IO状态
	u8 key1,key2;

	P30=0;P31=0;P32=0;P33=0;P34=1;P35=1;P42=1;P44=1;
	if(P44==0)	key1=0x70;
	if(P42==0)	key1=0xb0;
	if(P35==0)	key1=0xd0;
	if(P34==0)	key1=0xe0;
	if((P34==1)&&(P35==1)&&(P42==1)&&(P44==1))	key1=0xf0;

	P30=1;P31=1;P32=1;P33=1;P34=0;P35=0;P42=0;P44=0;
	if(P30==0)	key2=0x0e;
	if(P31==0)	key2=0x0d;
	if(P32==0)	key2=0x0b;
	if(P33==0)	key2=0x07;
	if((P30==1)&&(P31==1)&&(P32==1)&&(P33==1))	key2=0x0f;
	key_temp=key1|key2;

	switch(key_state)                             
	{
		case KEY_STATE0:				//判断按键按下
			if(key_temp!=NO_KEY)
			{
				key_state=KEY_STATE1; 	//有键按下，就转到状态1    
			}
			break;

		case KEY_STATE1:				//经过10ms，再次确认按键按下，用于消抖
			if(key_temp==NO_KEY)		
			{
				key_state=KEY_STATE0;	//如果是抖动，则回到状态0
			}
			else						//如果不是抖动，则返回对应的键值
			{
				switch(key_temp)                             
				{
					case 0x77: key_value=4;break;
					case 0x7b: key_value=5;break;
					case 0x7d: key_value=6;break;
					case 0x7e: key_value=7;break;

					case 0xb7: key_value=8;break;
					case 0xbb: key_value=9;break;
					case 0xbd: key_value=10;break;
					case 0xbe: key_value=11;break;

					case 0xd7: key_value=12;break;
					case 0xdb: key_value=13;break;
					case 0xdd: key_value=14;break;
					case 0xde: key_value=15;break;

					case 0xe7: key_value=16;break;
					case 0xeb: key_value=17;break;
					case 0xed: key_value=18;break;
					case 0xee: key_value=19;break;	
				}
				key_state=KEY_STATE2;
			}
			break;

		case KEY_STATE2:			//经过10ms，判断是否释放按键
			if(key_temp==NO_KEY)
			{
				key_state=KEY_STATE0;
			}
			break;
	}
	return key_value;
}

void Timer_Init(void)	//1ms定时器，用STC-ISP生成
{
	AUXR |= 0x80;		//1T的定时器
	TMOD &= 0xF0;		//设置定时器模式
	TL0 = 0xCD;			//设置定时初值
	TH0 = 0xD4;			//设置定时初值
	TF0 = 0;			//清除TF0标志
	TR0 = 1;			//定时器0开始计时
	ET0 = 1;			//开启定时器0中断
	EA=1; 				//开启总中断
}

bit key_flag;			//按键标志位
void main(void)
{
	u8 key_val=NO_KEY;
	P2=0xa0;P0=0x00;P2=0x00; //关闭蜂鸣器和继电器
	Timer_Init(); 			//1ms定时器
	while(1)
	{
		if(key_flag) 		//10ms的轮询
		{
			key_flag=0;
			key_val=Key_Scan();
			switch(key_val)                       
			{                                              
				case 4: P2=0XA0;P0=0XFF;P2=00; break;
				case 5:  break;
				case 6:  break;
				case 7:  break;
				case 8:  break;
				case 9:  break;
				case 10: break;
				case 11: break;
				case 12: break;
				case 13: break;
				case 14: break;
				case 15: break;
				case 16: break;
				case 17: break;
				case 18: break;
				case 19:P2=0XA0;P2=0X00;P2=00; break;
			} 
		}
	}
}

void timer0() interrupt 1  using 1      //定时器0的中断编号是1        
{
	static int key_count=0,smg_count=0,i=0;
	key_count++;smg_count++;
	
	if(key_count==10)					//10ms轮询
	{
		key_count=0;
		key_flag=1;
	}
	
	if(smg_count==3)					//3ms轮询
	{
		smg_count=0;
		P2=0xc0;P0=0;P2=0;				//数码管消影
		P2=0xe0;P0=~smg_du[i];P2=0;		//数码管段选赋值
		P2=0xc0;P0=smg_wei[i];P2=0; 	//数码管位选赋值
		i++;
		if(i==8) i=0;
	}
}