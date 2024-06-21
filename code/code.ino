#include <ESP8266WiFi.h>
#include <MQTT.h>

const char ssid[] = "***********";   //Enter your wifi SSID
const char pass[] = "***********";   //Enter your wifi password
const int sensorIn = A0;
int mVperAmp = 66; // use 185 for 5A, 100 for 20A Module and 66 for 30A Module

double Voltage = 0;
double VRMS = 0;
double AmpsRMS = 0;
WiFiClient net;
MQTTClient client;
unsigned long lastMillis = 0;

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nconnecting...");
  while (!client.connect("mqtt", "****", "******************************")) {    //enter you Adafruit.io credentials
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nconnected!");
  client.subscribe("****/errors");   //enter you Adafruit.io username inplace of ****
  client.subscribe("****/throttle");  //enter you Adafruit.io username inplace of ****
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

  // Note: Do not use the client in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `client.loop()`.
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  pinMode(A0, INPUT);

  // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported
  // by Arduino. You need to set the IP address directly.
  client.begin("io.adafruit.com", net);
  client.onMessage(messageReceived);

  connect();
}

void loop() {
  client.loop();
  delay(5000);  // <- fixes some issues with WiFi stability

  if (!client.connected()) {
    connect();
  }

  // publish a message roughly every 6 second.
  if (millis() - lastMillis > 10000) {
    lastMillis = millis();
    Voltage = getVPP();
    VRMS = (Voltage/2.0) *0.707; // sq root
    AmpsRMS = (VRMS * 1000)/mVperAmp-0.12;
    float Wattage = (220*AmpsRMS); 
    Serial.print(AmpsRMS);
    Serial.println(" Amps RMS ");
    Serial.print(Wattage); 
    Serial.println(" Watt ");
    client.publish("****/feeds/power1", String(Wattage));   //enter you Adafruit.io username inplace of ****
    client.publish("****/feeds/current1", String(AmpsRMS));  //enter you Adafruit.io username inplace of ****
  }
}
float getVPP()
{
  float result;
  
  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
  int minValue = 1024;          // store min value here
  
   uint32_t start_time = millis();

   while((millis()-start_time) < 1000) //sample for 1 Sec
   {
       readValue = analogRead(sensorIn);
       // see if you have a new maxValue
       if (readValue > maxValue) 
       {
           /*record the maximum sensor value*/
           maxValue = readValue;
       }
       if (readValue < minValue) 
       {
           /*record the maximum sensor value*/
           minValue = readValue;
       }
    }
   
   // Subtract min from max
   result = ((maxValue - minValue) * 5)/1024.0;
      
   return result;
 }
