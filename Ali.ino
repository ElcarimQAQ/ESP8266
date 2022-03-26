#include <dht11.h>
#include <ESP8266WiFi.h>
/* 依赖 PubSubClient 2.4.0 */
#include <PubSubClient.h>
/* 依赖 ArduinoJson 5.13.4 */
#include <ArduinoJson.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "draw.h"
#ifndef U8G2
#define U8G2
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 14, /* data=*/ 2);
#endif
dht11 DHT11;//定义传感器类型


#define DHT11PIN 5//定义传感器连接引脚。此处的PIN2在NodeMcu8266开发板上对应的引脚是D4
#define SENSOR_PIN    13
/* 连接您的WIFI SSID和密码 */
#define WIFI_SSID         "YLB"
#define WIFI_PASSWD       "YANGLIBING"


/* 设备证书信息*/
#define PRODUCT_KEY       "ga72IpHCDa7"
#define DEVICE_NAME       "Device"
#define DEVICE_SECRET     "b5a2a43626875aa01694f814a9046c67"
#define REGION_ID         "cn-shanghai"

/* 线上环境域名和端口号，不需要改 */
#define MQTT_SERVER       "ga72IpHCDa7.iot-as-mqtt.cn-shanghai.aliyuncs.com"
#define MQTT_PORT         1883
#define MQTT_USRNAME      "Device&ga72IpHCDa7"

#define CLIENT_ID         "123456|securemode=3,signmethod=hmacsha1,timestamp=789|"
// 请使用以上说明中的加密工具或参见MQTT-TCP连接通信文档加密生成password。
// 加密明文是参数和对应的值（clientIdesp8266deviceName${deviceName}productKey${productKey}timestamp1234567890）按字典顺序拼接
// 密钥是设备的DeviceSecret
#define MQTT_PASSWD       "314b1b7fd7f83116a94ac4e111967b7dadf17958"

#define ALINK_BODY_FORMAT         "{\"id\":\"123\",\"version\":\"1.0\",\"method\":\"thing.event.property.post\",\"params\":%s}"
#define ALINK_TOPIC_PROP_POST     "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/event/property/post"
#define ALINK_TOPIC_PROP_SET     "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/event/property/set"


unsigned long lastMs = 0;
WiFiClient espClient;
PubSubClient  client(espClient);
int state;
int warn;

void callback(char *topic, byte *payload, unsigned int length)
{
    StaticJsonBuffer<200> jsonBuffer;
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    payload[length] = '\0';
    Serial.println((char *)payload);
    JsonObject& root = jsonBuffer.parseObject(payload);      
    if (!root.success()) {  
        Serial.println("parseObject() failed");
        return; 
    }
    const char* status = root["params"]["LightSwitch"];
    Serial.println(status);
    if(status[0] == '0'){
        //Serial.println("off"); 
        digitalWrite(LED_BUILTIN, HIGH);
        state = 0;
    }else{
        //Serial.println("on"); 
        digitalWrite(LED_BUILTIN, LOW); 
        state = 1;
    }
}


void wifiInit()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("WiFi not Connect");
    }

    Serial.println("Connected to AP");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    
    Serial.print("espClient [");
    client.setServer(MQTT_SERVER, MQTT_PORT);   /* 连接WiFi之后，连接MQTT服务器 */
    client.setCallback(callback);
}


void mqttCheckConnect()
{
    while (!client.connected())
    {
        Serial.println("Connecting to MQTT Server ...");
        if (client.connect(CLIENT_ID, MQTT_USRNAME, MQTT_PASSWD))
        {
            Serial.println("MQTT Connected!");
            client.subscribe(ALINK_TOPIC_PROP_SET); 
        }
        else
        {
            Serial.print("MQTT Connect err:");
            Serial.println(client.state());
            delay(5000);
        }
    }
}


void mqttIntervalPost()
{
    char param[32];
    char jsonBuf[128];
    float t = (float)DHT11.temperature;
    float h = (float)DHT11.humidity;
    sprintf(param, "{\"CurrentTemperature\":%f}",(float)DHT11.temperature);
    sprintf(jsonBuf, ALINK_BODY_FORMAT, param);
    Serial.println(jsonBuf);
    boolean d = client.publish(ALINK_TOPIC_PROP_POST, jsonBuf);
    if(d){
      Serial.println("publish Temperature success"); 
    }else{
      Serial.println("publish Temperature fail"); 
    } 
    sprintf(param, "{\"CurrentHumidity\":%f}",(float)DHT11.humidity);
    sprintf(jsonBuf, ALINK_BODY_FORMAT, param);
    Serial.println(jsonBuf);
    d = client.publish(ALINK_TOPIC_PROP_POST, jsonBuf);
    if(d){
      Serial.println("publish Humidity success"); 
    }else{
      Serial.println("publish Humidity fail"); 
    }
    sprintf(param, "{\"LightSwitch\":%d}",state);
      sprintf(jsonBuf, ALINK_BODY_FORMAT, param);
      d = client.publish(ALINK_TOPIC_PROP_POST, jsonBuf);
      if(d){
        Serial.println("publish Switch  success"); 
    }else{
      Serial.println("publish Switch fail"); 
    } 
    sprintf(param, "{\"Warn\":%d}",warn);
      sprintf(jsonBuf, ALINK_BODY_FORMAT, param);
      d = client.publish(ALINK_TOPIC_PROP_POST, jsonBuf);
      if(d){
        Serial.println("publish WARN  success"); 
    }else{
      Serial.println("publish WARN fail"); 
    } 
    //显示
     display((float)DHT11.temperature,(float)DHT11.humidity);
     if(t >= 33.0 || h>= 75.0) {
        digitalWrite(LED_BUILTIN,LOW);
        warn = 1;
     }
     else warn = 0;
   
}

void setup() 
{
    pinMode(SENSOR_PIN,  INPUT);
    /* initialize serial for debugging */
    Serial.begin(115200);
    Serial.println("Demo Start");
    pinMode(LED_BUILTIN, OUTPUT);
    wifiInit();
    u8g2.begin();
    u8g2.enableUTF8Print();
}


// the loop function runs over and over again forever
void loop()
{
     DHT11.read(DHT11PIN); //更新传感器所有信息
    if (millis() - lastMs >= 5000)
    {
        lastMs = millis();
        mqttCheckConnect(); 
        mqttIntervalPost();
    }
    client.loop();
}
