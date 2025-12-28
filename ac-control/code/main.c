#include <reg52.h>
#include <intrins.h>
typedef unsigned char uchar;
typedef unsigned int  uint;

sbit DHT11_DATA = P3^0;  
sbit IN1  = P2^0; 
sbit IN2  = P2^1;
sbit IN3  = P2^2; 
sbit IN4  = P2^3;
sbit ENA  = P2^4; 
sbit ENB  = P2^5;
sbit IN1_C = P1^0; 
sbit IN2_C = P1^1;
sbit ENA_C = P1^2;

uchar humi_int;  // 湿度整数位 0-99
uchar temp_int;  // 温度整数位 0-99
uchar humi_deci; // DHT11小数位固定0
uchar temp_deci; // DHT11小数位固定0
uchar check_sum; // 校验位

void Delay_1us(void)    // 12MHz
{
    _nop_();
}

void Delay_ms(uint ms)  // 12MHz
{
	uint i,j;
	for(i=ms; i>0; i--)
		for(j=120; j>0; j--);
}

uchar DHT11_Read(void)
{
	uchar i, j, buf[5] = {0};
	// 主机发送起始信号
	DHT11_DATA = 0;
	Delay_ms(20);
	DHT11_DATA = 1;
	Delay_1us();Delay_1us();Delay_1us();Delay_1us();
	
	// 等待DHT11应答
	while(DHT11_DATA == 1);
	while(DHT11_DATA == 0);
	while(DHT11_DATA == 1);
	
	// 读取40位数据 高位先出
	for(i=0; i<5; i++)
	{
		for(j=0; j<8; j++)
		{
			while(DHT11_DATA == 0);
			Delay_1us();Delay_1us();Delay_1us();Delay_1us();Delay_1us();
			buf[i] <<= 1;
			if(DHT11_DATA == 1) buf[i] |= 1;
			while(DHT11_DATA == 1);
		}
	}
	
	// 数据校验赋值
	humi_int = buf[0];
	humi_deci = buf[1];
	temp_int = buf[2];
	temp_deci = buf[3];
	check_sum = buf[4];
	
	if((humi_int + humi_deci + temp_int + temp_deci) == check_sum)
		return 1; // 校验成功，数据有效
	else
		return 0; // 校验失败，数据无效
}

void Motor_Init(void)   // 电机初始化，上电全部停止，默认低电平
{
	IN1=0;IN2=0;ENA=0;
	IN3=0;IN4=0;ENB=0;
	IN1_C=0;IN2_C=0;ENA_C=0;
}

// 电机A 正转(制冷)
void MotorA_ON(void)
{
	IN1 = 1;
	IN2 = 0;
	ENA = 1;
}
void MotorA_OFF(void){IN1=0;IN2=0;ENA=0;}

// 电机B 正转(制热)
void MotorB_ON(void)
{
	IN3 = 1;
	IN4 = 0;
	ENB = 1;
}
void MotorB_OFF(void){IN3=0;IN4=0;ENB=0;}

// 电机C 正转(除湿)
void MotorC_ON(void)
{
	IN1_C = 1;
	IN2_C = 0;
	ENA_C = 1;
}
void MotorC_OFF(void){IN1_C=0;IN2_C=0;ENA_C=0;}

void Temp_Humi_Ctrl(void)
{
	// 制冷：温度>30℃ 开电机A，否则关闭
	if(temp_int > 30)  MotorA_ON();
	else               MotorA_OFF();
	
	// 制热：温度<20℃ 开电机B，否则关闭
	if(temp_int < 20)  MotorB_ON();
	else               MotorB_OFF();
	
	// 除湿：湿度>60% 开电机C，否则关闭
	if(humi_int > 60)  MotorC_ON();
	else               MotorC_OFF();
}

void main(void)
{
	Motor_Init();  // 上电初始化所有电机停止
	while(1)      
	{
		if(DHT11_Read() == 1)  // 成功读取有效温湿度数据
		{
			Temp_Humi_Ctrl();   // 执行电机控制逻辑
		}
		Delay_ms(1000);         // 1秒采集一次
	}
}