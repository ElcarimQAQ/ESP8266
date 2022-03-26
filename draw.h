#include <dht11.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

#ifndef U8G2
#define U8G2
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 14, /* data=*/ 2);
#endif
/********************###定义###********************/
#define DHT11PIN 5//定义传感器连接引脚。此处的PIN2在NodeMcu8266开发板上对应的引脚是D4
#define LED_BUILTIN 4

/********************###子函数###********************/
double Fahrenheit(double celsius)
{
  return 1.8 * celsius + 32; //摄氏温度度转化为华氏温度
}
 
double Kelvin(double celsius)
{
  return celsius + 273.15; //摄氏温度转化为开氏温度
}

void blink(){
      digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
      delay(500);                      // Wait for a second
      digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
      delay(500);     }

void drawT(float m){
    u8g2.drawStr(0,55,"T:");
    u8g2.setCursor(20,55);
    u8g2.print(m);

}

void drawH(float m){
    //Serial.println(m_str);
    //u8g2.drawStr(0,20, "Tem:");
    //u8g2.drawStr(0,20,m_str);
   // u8g2.drawStr(50,63,str3);
    
    u8g2.drawStr(64,55,"H:");
    u8g2.setCursor(84,55);
    u8g2.print(m);
    
}

void display(float t, float h){
  u8g2.firstPage();
      do {
         u8g2.clearBuffer();
         if(t >= 33.0 || h >= 75.0){    
            u8g2.setFont(u8g2_font_ncenB12_tr);
            u8g2.drawUTF8(10,40,"WARINNING！");
         
        }else{
           u8g2.setFont(u8g2_font_wqy12_t_gb2312);
           u8g2.drawUTF8(20,30,"当前实验室环境：");
           u8g2.setFont(u8g2_font_ncenB10_tr);
           drawT(t);
           drawH(h);
        }
        u8g2.sendBuffer();
    } while (u8g2.nextPage() );
}
