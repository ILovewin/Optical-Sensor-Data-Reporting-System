#include <WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include <ArduinoJson.h>

#define led 2

volatile bool newDataAvailable = false;
char receivedData[50];
int len;
int num = 0;

// 设置wifi接入信息(请根据您的WiFi信息进行修改)
const char* ssid = "Redmi K40";
const char* password = "19832712692";
const char* mqttServer = "ProductKey.iot-as-mqtt.cn-shanghai.aliyuncs.com";  //*MQTT连接参数你的设备登陆网址*//
 
// 如以上MQTT服务器无法正常连接，请前往以下页面寻找解决方案
// http://www.taichi-maker.com/public-mqtt-broker/
 
Ticker ticker;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
 
int count;    // Ticker计数用变量

// 串口2中断处理函数
void serial2ISR() {
  // 检查是否有可用数据从 ESP32 的串口2 接收
//  if (Serial2.available()) {
//    newDataAvailable = true;  // 设置标志位为true
//    num = num + 1;
//  }
  newDataAvailable = true;  // 设置标志位为true
  num = num + 1;
}
 
void setup() {
  Serial.begin(115200);

  // 初始化 ESP32 的串口2
  Serial2.begin(115200);
  
  //设置ESP8266工作模式为无线终端模式
  WiFi.mode(WIFI_STA);

  pinMode(led, OUTPUT); //将指定的led引脚设置为输出模式

  digitalWrite(led, LOW); // 设置LED灯的初始状态为关闭
  
  // 连接WiFi
  connectWifi();
  
  // 设置MQTT服务器和端口号
  mqttClient.setServer(mqttServer, 1883);
 
  // 连接MQTT服务器
  connectMQTTServer();

  // 订阅主题
  mqttClient.subscribe("/sys/k0lox7iYdk5/dev-esp32/thing/service/property/set");
 
  // Ticker定时对象
  ticker.attach(1, tickerCount); 

  // 配置串口2的RX引脚为中断模式
  pinMode(16, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(16), serial2ISR, FALLING);//中断服务例程,第一个参数为将数字引脚16转换为相应的中断号，第二个参数为当引脚状态改变时应该被调用的函数，第三个参数为Arduino常量，表示当引脚状态从高电平变为低电平时，应该触发中断

}
 
void loop() { 
  if (mqttClient.connected()) { // 如果开发板成功连接服务器
//    // 每隔3秒钟发布一次信息
//    if (count >= 3){
//      pubMQTTmsg();
//      count = 0;
//    }    
    // 保持心跳
    mqttClient.loop();
    
    if (newDataAvailable) {
      // 读取数据到 receivedData 数组中
      len = Serial2.readBytes(receivedData, 50);
      Serial.print("len:     ");
      Serial.println(len);
      Serial.println();
      Serial.println(num);
      Serial.println();
      processData();
      newDataAvailable = false; // 重置标志位
    }
    
  } else {                  // 如果开发板未能成功连接服务器
    connectMQTTServer();    // 则尝试连接服务器
  }
}
 
void tickerCount(){
  count++;
}
 
void connectMQTTServer(){
  // 根据ESP8266的MAC地址生成客户端ID（避免与其它ESP8266的客户端ID重名）
  String clientId = "dev-esp32|securemode=2,signmethod=hmacsha1,timestamp=1702572363280|";   //*这里的字符串要与自己的设备MQTT参数相同*//
  String user = "dev-esp32&k0lox7iYdk5";
  String password = "AB1A50021FEDB1B36EE751900D317EA058E204BC";
 
  // 连接MQTT服务器，原代码仅传clientid,通过查阅pubsubclient.cpp可发现多个connect重载
  if (mqttClient.connect(clientId.c_str(),user.c_str(),password.c_str())) { 
    Serial.println("MQTT Server Connected.");
    Serial.println("Server Address: ");
    Serial.println(mqttServer);
    Serial.println("ClientId:");
    Serial.println(clientId);

    // 设置回调函数
    mqttClient.setCallback(your_callback);
  } else {
    Serial.print("MQTT Server Connect Failed. Client State:");
    Serial.println(mqttClient.state());
    delay(3000);
  }   
}
 
// 发布信息
void pubMQTTmsg(const char* json) {
  //static int value; // 客户端发布信息用数字
 
  // 确保不同用户进行MQTT信息发布时，ESP8266客户端名称各不相同，
  String topicString = "/sys/k0lox7iYdk5/dev-esp32/thing/event/property/post"; // 更改自己的devicename
  char publishTopic[topicString.length() + 1];
  strcpy(publishTopic, topicString.c_str());
 
  // 实现ESP8266向主题发布信息
  if (mqttClient.publish(publishTopic, json)) {
    Serial.println("Publish Topic:");
    Serial.println(publishTopic);
    Serial.println("Publish message:");
    Serial.println(json);
  } else {
    Serial.println("Message Publish Failed.");
  }
}

// 订阅回调函数
void your_callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received on topic: ");
  Serial.println(topic);

  Serial.print("Message: ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // 创建一个足够大的JsonDocument对象来存储payload中的数据
  StaticJsonDocument<200> doc;

  // 将payload转换为字符串
  String payloadStr;
  for (unsigned int i = 0; i < length; i++) {
    payloadStr += (char)payload[i];
  }

  // 解析payload
  DeserializationError error = deserializeJson(doc, payloadStr);
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  // 获取led的值
  int ledStatus = doc["params"]["led"];
  
  // 根据led的值来控制LED灯
  if (ledStatus == 1) {
    // 打开LED灯
    digitalWrite(led, HIGH);
  } else if (ledStatus == 0) {
    // 关闭LED灯
    digitalWrite(led, LOW);
  }
}

 
// ESP8266连接wifi
void connectWifi(){
 
  WiFi.begin(ssid, password);
 
  //等待WiFi连接,成功连接后输出成功信息
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi Connected!");  
  Serial.println(""); 
}

void processData() {
  num = 0;
  // 在这里进行数据处理，例如打印接收到的数据
  Serial.print("Received data: ");
  for (int i = 0; i < len; i++) {
    Serial.print(receivedData[i]);
  }
  Serial.println();
  uint16_t ir, als, ps;
  sscanf(receivedData, "IR: %hu, ALS: %hu, PS: %hu", &ir, &als, &ps);
  memset(receivedData, 0, sizeof(receivedData));
  // 发送信息处理代码
  char json[100];
  sprintf(json, "{\"params\":{\"ir\":%hu,\"als\":%hu,\"ps\":%hu},\"version\":\"1.0.0\"}", ir, als, ps);//使用sprintf()函数将这些值格式化为一个JSON格式的字符串，并将结果存储在json数组中
  pubMQTTmsg(json);
  
}
