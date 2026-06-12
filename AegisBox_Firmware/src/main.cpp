#include <Arduino.h>
#include <ArduinoJson.h>
#include <MPU6050_tockn.h>
#include <Wire.h>
#include <DHT.h>
#include <WiFi.h>
#include <time.h>
#include <PubSubClient.h>

#define DHT_PIN 4
#define DHT_TYPE DHT11

const int LDR_PIN = 13;
volatile bool tamper_flag = false;

const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;

const char* broker_address = "test.mosquitto.org";
const int port = 1883;
const char* topic = "YOUR_TOPIC";

unsigned long last_transmission_time;
bool alert_already_sent = false;

String client_id;

WiFiClient espclient;
PubSubClient client(espclient);

time_t now;

MPU6050 mpu(Wire);
DHT dht(DHT_PIN,DHT_TYPE);

void IRAM_ATTR tamperDetectedISR(){
  tamper_flag = true;
}

int connect_broker(){
  client_id = "ESP32_" + String(random(0,0xfffff),HEX);
  Serial.println("\n\nAttempting Connection to Server......");
    if(client.connect(client_id.c_str())){
      Serial.println("Connected to the server.");
      return 1;
    }
    else{
    Serial.println("\nError status code : ");
    Serial.println(client.state());

    return 0;
    }
}

void send_payload(){
  last_transmission_time = millis();

    JsonDocument doc;
    time(&now);
    doc["device_id"] = "Aegis-Box-001";
    doc["timestamp"] = now;
    doc["temperature"] = dht.readTemperature();
    doc["humidity"] = dht.readHumidity();
    doc["accel_x"] = mpu.getAccX();
    doc["accel_y"] = mpu.getAccY();
    doc["accel_z"] = mpu.getAccZ();
    doc["gyro_x"] = mpu.getGyroX();
    doc["gyro_y"] = mpu.getGyroY();
    doc["gyro_z"] = mpu.getGyroZ();
    doc["tamper_status"] = tamper_flag;

    String JsonOutput;
    serializeJson(doc,JsonOutput);

    client.publish(topic,JsonOutput.c_str());

    Serial.println("\n========================================Serialized JSON Output========================================");
    Serial.println(JsonOutput);
}

void setup() {

  Serial.begin(115200);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
  Serial.print("\nConnecting to WiFi");

  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nSuccessfully connected to WiFi.");
  
  Serial.print("IP address of ESP32 is : ");
  Serial.println(WiFi.localIP());

  client.setServer(broker_address,port);


  configTime(gmtOffset_sec,daylightOffset_sec,ntpServer);

  Serial.print("\nSynchronizing Time");
  now = time(nullptr);
  while(now < 100000){
    delay(500);
    Serial.print(".");
    time(&now);
  }

  Serial.print("\nTime Synchronized : ");
  Serial.println(now);

  pinMode(LDR_PIN,INPUT);
  attachInterrupt(digitalPinToInterrupt(LDR_PIN),tamperDetectedISR,RISING);
  Serial.println("\nISR established.");

  Wire.begin();
  mpu.begin();
  mpu.calcGyroOffsets(true);

  dht.begin();

  last_transmission_time = millis();

  /** Debugging MPU6050 Connection error
  for(int i=1;i<128;i++){
    Wire.beginTransmission(i);
    byte error = Wire.endTransmission();
    if(error == 0){
      Serial.println(i);
    }
  }
    */
}

void loop() {

  bool connected = true;
  if(WiFi.status() != WL_CONNECTED){
    connected = false;
  }

  else if(!client.connected()){
    connected = connect_broker();
  }

  client.loop();

  mpu.update();

  float total_g = sqrt((mpu.getAccX()*mpu.getAccX()) + (mpu.getAccY()*mpu.getAccY()) + (mpu.getAccZ()*mpu.getAccZ()));

  if(total_g >= 2.0){
    Serial.println("\nCRITICAL IMPACT DETECTED!");
    Serial.print("G-Force : ");
    Serial.println(total_g);
  }

  bool emergency = (tamper_flag) && (!alert_already_sent);

  if(emergency){
    Serial.println("\nCargo has been tampered with..");
  }

  /**Print Statements
  Serial.println("");
  Serial.print("Temp : ");
  Serial.print(mpu.getTemp());
  Serial.println(" C");

  Serial.println("Acceleration metrics : (X Y Z) ");
  Serial.println(mpu.getAccX());
  Serial.println(mpu.getAccY());
  Serial.println(mpu.getAccZ());

  Serial.println("Gyro metrics : (X Y Z) ");
  Serial.println(mpu.getGyroX());
  Serial.println(mpu.getGyroY());
  Serial.println(mpu.getGyroZ());

  Serial.print("Humidity : ");
  Serial.print(dht.readHumidity());
  Serial.println(" %");

  Serial.print("Temperature : ");
  Serial.print(dht.readTemperature());
  Serial.println(" C");
  */
  bool timer_expires = (millis() - last_transmission_time) >= 5000;

  if(connected && (timer_expires || emergency)){

    send_payload();
    if(emergency){
      alert_already_sent = true;
    }
  }
}


