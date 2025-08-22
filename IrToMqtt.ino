#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

// Wi-Fi 設定
const char* ssid = "your home AP ssid ";
const char* password = "your home ap password";

// MQTT 設定
const char* mqtt_server = "you_home_assistant_ip";
const int mqtt_port = 1883;  // 預設MQTT port
const char* mqtt_client_id = "ESP8266_IR_Client";
const char* mqtt_user = "you_home_assistant_user_name";   // 新增
const char* mqtt_pass = "you_home_assistant_password";  // 新增
const char* mqtt_ir_topic = "/webduino/irrecv";
const char* mqtt_heartbeat_topic = "/webduino/hb";

// 紅外線接收器設定
const int IR_RECV_PIN = 5;  // 連接紅外線接收器的GPIO腳位 (D2)
IRrecv irrecv(IR_RECV_PIN);
decode_results results;
unsigned long lastIrData = 0;
unsigned long newIrData =0;

// 宣告全域變數
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;           // 用於計時，發送心跳
const int heartbeatInterval = 10000;  // 心跳間隔 (5秒)

// 函式宣告
void setup_wifi();
void reconnect_mqtt();
void callback(char* topic, byte* payload, unsigned int length);

void setup() {
  Serial.begin(115200);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // 初始化紅外線接收器
  irrecv.enableIRIn();
  Serial.println("IR Receiver enabled");
}

void loop() {
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop();
  // 心跳發送處理
  unsigned long now = millis();
  if (now - lastMsg > heartbeatInterval) {
    lastMsg = now;
    client.publish(mqtt_heartbeat_topic, "online");
    //Serial.println("Heartbeat sent.");
  }
  // 紅外線訊號處理
  if (irrecv.decode(&results)) {
    newIrData = results.value;
    Serial.print("New code: ");
    Serial.println(newIrData, HEX);
    
    if (newIrData == 0xFFFFFFFF || newIrData == lastIrData) {
      Serial.print("IR Data: ");
      Serial.println(lastIrData, HEX);
      // 將紅外線訊號轉換為字串並發送至MQTT
      String ir_data_string = String(lastIrData, HEX);
      client.publish(mqtt_ir_topic, ir_data_string.c_str());
    } 
    lastIrData = newIrData;
    irrecv.resume();  // 繼續接收下一個訊號
  }else{
      delay(50);
  }
}

// Wi-Fi 連接函式
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// MQTT 重連函式
void reconnect_mqtt() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(mqtt_client_id, mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      // 可以在這裡訂閱主題
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// MQTT 接收回調函式 (本程式未使用，但為PubSubClient必要)
void callback(char* topic, byte* payload, unsigned int length) {
  // 您可以在此處理從MQTT Broker收到的訊息
  // 範例：將收到的訊息印出來
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
