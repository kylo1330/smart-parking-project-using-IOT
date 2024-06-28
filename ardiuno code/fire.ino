#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <FirebaseESP8266.h> 
#include <ESP8266WiFi.h>
#include <Servo.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <NTPClient.h>;
#include <WiFiUdp.h>
#define FIREBASE_HOST "smart-parking-1330-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "t12pH6nTpVLE4xVFVGHIiVuNzaGHa2DzeFGnPRP3"
#define WIFI_SSID "Muhammee"
#define WIFI_PASSWORD "hallo4321"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

FirebaseData firebaseData;

FirebaseJson json;

#define SCREEN_WIDTH 128 // OLED display width,  in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define MQTT_SERV "io.adafruit.com"
#define MQTT_PORT 1883
#define MQTT_NAME "alan"
#define MQTT_PASS "1ac95cb8580b4271bbb6d9f75d0668f1"
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800,60000);
Servo myservo;                          //servo as gate
Servo myservos;   

int carEnter = D0;                      // entry sensor 
int carExited = D7;                    //exit sensor              
int slot2 = D6;
int slot1  = D3;
int count = 0; 
int CLOSE_ANGLE = 80;  // The closing angle of the servo motor arm
int OPEN_ANGLE = 0;  // The opening angle of the servo motor arm              
int  hh, mm, ss;
int pos;
int pos1;

String h, m,EntryTimeSlot1,ExitTimeSlot1, EntryTimeSlot2,ExitTimeSlot2, EntryTimeSlot3,ExitTimeSlot3;
boolean entrysensor, exitsensor,s1,s2,s3;

boolean s1_occupied = false;
boolean s2_occupied = false;

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, MQTT_SERV, MQTT_PORT, MQTT_NAME, MQTT_PASS);

//Set up the feed you're subscribing to
Adafruit_MQTT_Subscribe EntryGate = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/f/EntryGate");
Adafruit_MQTT_Subscribe ExitGate = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/f/ExitGate");

//Set up the feed you're publishing to
Adafruit_MQTT_Publish CarsParked = Adafruit_MQTT_Publish(&mqtt,MQTT_NAME "/f/CarsParked");
Adafruit_MQTT_Publish EntrySlot1 = Adafruit_MQTT_Publish(&mqtt,MQTT_NAME "/f/EntrySlot1");
Adafruit_MQTT_Publish ExitSlot1 = Adafruit_MQTT_Publish(&mqtt,MQTT_NAME "/f/ExitSlot1");
Adafruit_MQTT_Publish EntrySlot2 = Adafruit_MQTT_Publish(&mqtt,MQTT_NAME "/f/EntrySlot2");
Adafruit_MQTT_Publish ExitSlot2 = Adafruit_MQTT_Publish(&mqtt,MQTT_NAME "/f/ExitSlot2");
Adafruit_MQTT_Publish EntrySlot3 = Adafruit_MQTT_Publish(&mqtt,MQTT_NAME "/f/EntrySlot3");
Adafruit_MQTT_Publish ExitSlot3 = Adafruit_MQTT_Publish(&mqtt,MQTT_NAME "/f/ExitSlot3");



void wifiConnect()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}
void firebaseReconnect()
{
  Serial.println("Trying to reconnect");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void print_ok()
{
    Serial.println("------------------------------------");
    Serial.println("OK");
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.println("ETag: " + firebaseData.ETag());
    Serial.println("------------------------------------");
    Serial.println();
}


void print_fail()
{
    Serial.println("------------------------------------");
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
}
void setup() 
{
  mqtt.subscribe(&EntryGate);
  mqtt.subscribe(&ExitGate);
  timeClient.begin(); 
  Serial.begin (9600); 
  myservo.attach(D5);      // servo pin to D5
  myservos.attach(D4);       // servo pin to D4
  pinMode(carExited, INPUT);    // ir as input
  pinMode(carEnter, INPUT);     // ir as input
  pinMode(slot1, INPUT);
  pinMode(slot2, INPUT);
  wifiConnect();
 Serial.println("Connecting Firebase.....");
 Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
 Firebase.reconnectWiFi(true);
 Serial.println("Firebase OK.");

  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

  delay(2000);         // wait for initializing
  oled.clearDisplay(); // clear display

  oled.setTextSize(2);          // text size
  oled.setTextColor(WHITE);     // text color
  oled.setCursor(0, 10);        // position to display


}

 void oledDisplayCenter(String text) 
{
  int16_t x1;
  int16_t y1;
  uint16_t width;
  uint16_t height;

  oled.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);

  // display on horizontal and vertical center
  oled.clearDisplay(); // clear display
  oled.setCursor((SCREEN_WIDTH - width) / 2, (SCREEN_HEIGHT - height) / 2);
  oled.println(text); // text to display
  oled.display();
}

void loop() 
{
  entrysensor= !digitalRead(carEnter);
  exitsensor = !digitalRead(carExited);
    s1 = digitalRead(slot1);
    s2 = digitalRead(slot2);


    if(entrysensor == 0) // D0
    {
      count= count+1;
      myservo.write(120); // gate open entry  // D5
      //delay(1500);
      Serial.println("Gate opened at entrance");
      //delay(15);
      String eng = "Entry Close";
      oledDisplayCenter(eng);
      delay(1000);
    }
    else
    {
     myservo.write(0);
     //delay(150);
     Serial.println("Gate closed at entrance");
     //delay(150);
     String eng = "Entry Open";
      oledDisplayCenter(eng);
      delay(1000);
    }

  if(exitsensor == 0) // D7
    {
      count= count-1;
      myservos.write(180); // gate open entry D4
      //delay(100);
      Serial.println("Gate opened at exit");
      //delay(150);
      String exg = "Exit Close";
      oledDisplayCenter(exg);
      delay(1000);
    }
    else
    {
     myservos.write(0);
    // delay(150);
     Serial.println("Gate closed at exit");
     //delay(150);
     String exg = "Exit Open";
      oledDisplayCenter(exg);
      delay(1000);
    }

////////////////////////////////////////////////////////////////////

if (! CarsParked.publish(count)) {}


      if (s1 == 1 && s1_occupied == false) 
      {                     
         if(Firebase.setBool(firebaseData, "/Spark/s1_occupied", false))
      {print_ok();}
    else
      {print_fail();}
 }
        Serial.println("Slot1 is Available!");
        EntryTimeSlot1 =  h +" :" + m;
        Serial.print("EntryTimeSlot1");
        Serial.print(EntryTimeSlot1);

      String s1o = "Slot1 Available!";
      oledDisplayCenter(s1o);
      delay(6000);
  
        s1_occupied = true;
        if (! EntrySlot1.publish((char*) EntryTimeSlot1.c_str())){}
      }

      
    if(s1 == 0 && s1_occupied == true) 
      {
        if(Firebase.setBool(firebaseData, "/Spark/s1_occupied", true))
      {print_ok();}
    else
      {print_fail();}
       Serial.println("Slot1 is Occupied");
       ExitTimeSlot1 =  h +" :" + m;


      String s1o = "Slot1 Occupied!";
      oledDisplayCenter(s1o);
      delay(6000);
      
       s1_occupied = false;
        if (! ExitSlot1.publish((char*) ExitTimeSlot1.c_str())){} 
      }


//////////////////////////////////////////////////////////////////////

  if (s2 == 1&& s2_occupied == false) 
    {     
      if(Firebase.setBool(firebaseData, "/Spark/s2_occupied", false))
      {print_ok();}
    else
      {print_fail();}
      String s2o = "Slot2 Available!";
      oledDisplayCenter(s2o);
      delay(6000);
                      
      Serial.println("Slot2 is Available! ");
      EntryTimeSlot2 =  h +" :" + m;
      s2_occupied = true;
      if (! EntrySlot2.publish((char*) EntryTimeSlot2.c_str())){}   
    }


    
  if(s2 == 0 && s2_occupied == true) 
     {
      if(Firebase.setBool(firebaseData, "/Spark/s2_occupied", true))
      {print_ok();}
    else
      {print_fail();}
      String s2o = "Slot2 Occupied!";
      oledDisplayCenter(s2o);
      delay(6000);
      
       Serial.println("Slot2 is Occupied ");
       ExitTimeSlot2 =  h +" :" + m;

      
       s2_occupied = false;
       if (! ExitSlot2.publish((char*) ExitTimeSlot2.c_str())){}
       
     }

///////////////////////////////////////////////////////////////////////



 




    
}
