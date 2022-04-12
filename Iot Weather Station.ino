// V1.4 : Ajout fonctionalite SetDate
//Affichage Heure Minute et Secodne avec un 0 au debut si inferieur a 0
// V1.5 : Photoresitance
// V1.6 Memory Management
// V1.7 NTP Management
// V1.8 Default date/time values

// TODO : 
// handleBlink et bouton associe
// Verifier affichage annee avec heure
// gestion trace des actions
// trouver bug affichage eau
// gerer watchdog
// service NTP

// managed URLS
// Board NodeMCU 0.9 -12MODULE
#include <ESP8266WiFi.h> // default with board 8266
#include <ESP8266WebServer.h> // default with board 8266
#include <DHT.h> // DHT sensor library from AdaFruit
#include <OneWire.h> // OneWire from Jim Studt [2.3.5]
#include <Wire.h> //
#include <DallasTemperature.h> // DallasTemperature from
#include <ds3231.h> // from Peter Rodan
#include <stdio.h>  // Standard Library // Utility ?
#include "user_interface.h"

// NTP
#include <NTPClient.h>
#include <WiFiUdp.h>

// =======================================================================================================
//                 -------
//   lumiere TOUT ADC0   A0-     -D0  GPIO16 USER WAKE             
//                      RSV-     -D1  GPIO05 SCL              
//                      RSV-     -D2  GPIO04 SDA
//          SDD3 GPIO10 SD3-     -D3  GPIO00 FLASH            DHT22 TEMP & HUMIDITY
//          SDD2 GPIO09 SD2-     -D4  GPIO02 TXD1             
//          SDD1   MOSI SD1-     -3V3 3.3V
//          SDCMD    CS CMD-     -GND 
//          SDD0   MISO SD0-     -D5  GPIO14 HSCLK            WATER TEMP
//          SDCLK  SCLK CLK-     -D6  GPIO12 HMISO            LIGHT
//                      GND-     -D7  GPIO13 RXD2 HMOSI       PUMP
//                 3.3V 3V3-     -D8  GPIO15 TXD2 HCS         LUNINOSITY  (should be free for reboot)
//                   EN  EN-     -RX  GPIO03 RXD0 (COMPUTER)
//                  RST RST-     -TX  GPIO01 TXD0 (COMPUTER)
//                      GND-     -GND
//                  Vin Vin-     -3V3 3.3V
//                         -------
//
// =======================================================================================================

#define HEADER "ESP8266 - Temperature and Humidite V1.8"

#define ONE_WIRE_BUS 14 // eau
#define RelaisLed 12
#define RelaisPompe 13 
//#define CapteurLight 15
#define DHTPin 0 // DHT 22 
#define Verbose 1 // Control des println : 0 off 1 on each sec, 2 on each min, on each hour elsewhere 
#define DHTTYPE DHT22   // DHT 22 
#define LightThreshold 500


// =======================================================================================================// definition des variables globales
  int light;
  static int min_fin = 0;
  static int pompe = 0;
  static int pompe_done = 0;
  static int PompeForced = 0;
  static int LightForced = 0;
  float AirHumidity; // humidite air
  float AirTemp; // temp air
  float WaterTemp; // sonde eau
  int Print_time=0;
  int Print_Active = 1;
  int LightAna;
  int LightValue = 0;
  int LightOn = 0;
  int BlinkThreshold=10;
// variable du dernier reboot
  unsigned int BootYear;
  unsigned int BootMonth;
  unsigned int BootDay;
  unsigned int BootHour;
  unsigned int BootMinute;
  unsigned int BootSecond;
  ESP8266WebServer MyWebServer(80);
  
  OneWire oneWire(ONE_WIRE_BUS);//temp eau sonde dsb28
  DallasTemperature sensors(&oneWire);
// la sonde est de type dht22
 
// Definition Reseau
  const char* ssid = "Your WIFI";
  const char* password = "Your Password";
  
// definition d'un serveur web ecoutant sur le port 80
  WiFiServer server(80);
  WiFiClient client;
  ts rtc; //ts is a struct findable in ds3231.h

  DHT dht(DHTPin, DHTTYPE); //Init DHT
    static char StrAirTemp[15]; // variables de travail
    static char StrAirHumidity[15]; // variables de travail
// NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

//
  //static char * sptr=0;
  //sptr = (char *)malloc(1000*sizeof(char));
  // 31 Ko available
  
void MyPrintInit()
{
  int Print_val=0;

  if (Verbose == 1) Print_val = rtc.sec;
  else if (Verbose == 2) Print_val = rtc.min;
  else Print_val = rtc.hour;
  
  if (Print_time != Print_val)
  {
    Print_time = Print_val;
    Print_Active = 1;  
  }
  else
  {
    Print_Active = 0;
  }
}
void MyPrint(const char *str)
{
  if (Verbose) 
  {
    if (Print_Active) Serial.print(str);
  }
}

void MyPrintln(const char *str)
{
  if (Verbose) 
  {
    if (Print_Active) Serial.println(str);
  }
}
void MyPrint(float a)
{
  char str[10];
  sprintf(str,"%f",a);
  MyPrint(str);
}

/////////////////////////////////////////////
void handle_OnConnect() {
  MyPrintln("Connection");
  MyWebServer.send(200, "text/html", SendHTML(PompeForced,LightForced));
}


////////////////////////////////////////////////
void Gestion()
{
  DS3231_get(&rtc);// get Time
  static int light_on = 0;
  MyPrintInit();
  static int IsAlive = 0; // permet un clignotement toutes les secondes
  static int Led2_Status = HIGH;

  if (IsAlive != rtc.sec)
  {
    IsAlive = rtc.sec;
    if (Led2_Status == HIGH)
    {
      Led2_Status = LOW;
    }
    else
    {
      Led2_Status = HIGH;      
    }
      digitalWrite(2, Led2_Status); // Blue led ON
  } // Led2 blinks every seconds
  
  sensors.requestTemperatures();// sonde eau
  WaterTemp = sensors.getTempCByIndex(0); // sonde eau en degres
  min_fin = (WaterTemp*30)/24 ;
  if (min_fin<5) {
    min_fin = 5;
  }
  else if (min_fin>59) {
    min_fin=59;
  }
  MyPrint("Temperature "); //print on serial monitor using ""
      MyPrint(WaterTemp);
      MyPrint(" duree de pompage ");
      MyPrint(min_fin);
      MyPrint(" etat ");
      MyPrint(pompe);
      MyPrint(" pompe_done ");
      MyPrint(pompe_done);
      MyPrint(" heure ");
      MyPrint(rtc.min);
      MyPrintln("");
   if (rtc.min == 0) pompe_done = 0;
   
   if (min_fin > rtc.min) 
   {
    if ((pompe==0) && (pompe_done==0))
    {
      MyPrintln("POMPE ON"); //print on serial monitor using ""
      if (! PompeForced) 
      {
        digitalWrite(RelaisPompe, LOW);
        pompe = 1;
      }
    }  
   }
   else 
   {
     if (pompe==1)
     {
      if (! PompeForced) 
      {
      
       MyPrintln("POMPE OFF"); //print on serial monitor using ""
       digitalWrite(RelaisPompe, HIGH);
       pompe = 0;
       pompe_done = 1;
      }
     }
   }
   
  // lecture sonde humidite air du DHT 22
  AirHumidity = dht.readHumidity();
  // lecture temperature air du DHT 22
  AirTemp = dht.readTemperature();

  // verif si lecture dht22 ok
  if (isnan(AirHumidity) || isnan(AirTemp)) {
    MyPrintln("!impossible de lire les donnees du dht22 !");
    strcpy(StrAirTemp, "probleme");
    strcpy(StrAirHumidity, "probleme");
  }
  else {
    // lecture de la sonde ok, envoi des elements sur le port serie
    float hic = dht.computeHeatIndex(AirTemp, AirHumidity, false);
//  dtostrf(hic, 6, 2, StrAirTemp);
    dtostrf(AirTemp, 6, 2, StrAirTemp);
    dtostrf(AirHumidity, 6, 2, StrAirHumidity);
    // envoi des message sur la console serie
    MyPrint("Humidite : ");
    MyPrint(StrAirHumidity);
    MyPrint(" %\t Temperature :");
    MyPrint(StrAirTemp);
    MyPrint(" *C ");
  }

  // Luminosite
  //light = digitalRead(CapteurLight);    //assign value of LDR sensor to a temporary variable
  LightAna = analogRead(A0);
  
  if (light_on != rtc.sec)
  {
    light_on = rtc.sec;
    
    if (LightAna > LightThreshold) {
      MyPrintln("Intensity= Jour"); //print on serial monitor using ""
      if (! LightForced)
      {
        if (! LightOn) // if light is not on then switch it on
        {
          if (abs(LightAna-LightValue)>BlinkThreshold) // light really differs to avoid blinking
          {
            digitalWrite(RelaisLed, HIGH);//Relais OFF
            LightOn = 1;
            LightValue = LightAna;
          }
        }
      }
    } else {
      MyPrintln("Intensity= Nuit"); //print on serial monitor using ""
      if (! LightForced) 
      {
        if (LightOn) // if light is on then switch it off
        {
          if (abs(LightAna-LightValue)> BlinkThreshold) // light really differs to avoid blinking
          {
            digitalWrite(RelaisLed, LOW);        //Relais ON
            LightOn=0;
            LightValue = LightAna;
          }
        }
      }
    }
  }
} // void Gestion()

String WebSendTime()
{
  String ptr;
  char str[6];
  
  ptr ="<h3>";
  ptr += "Time ";
  sprintf(str,"%02d",rtc.hour);
  ptr += str;
  ptr +=":";
  sprintf(str,"%02d",rtc.min);
  ptr += str;
  ptr += ":";
  sprintf(str,"%02d",rtc.sec);
  ptr += str;
  ptr += " (";
  sprintf(str,"%02d",rtc.mday);
  ptr += str;
  sprintf(str," %02d",rtc.mon);
  ptr += str;
  sprintf(str," %04d)",rtc.year+100);
  ptr += str;
  
  ptr +="</h3>\n";

  return (ptr);
}

/////////////////////////////////////////////
String WebSendLight()
{
  String ptr;
  char str[6];
  
  ptr = "<h3>Luminosite: ";
  sprintf(str,"%4d",LightAna);
  ptr += str;
  ptr += " ";
  if (LightAna > LightThreshold) ptr += "Jour";
  else if (light <= LightThreshold) ptr += "Nuit";
  else ptr += "Etat inconnu";
  ptr += " State ";
  sprintf(str,"%1d",LightOn);
  ptr += str;
  ptr += " Decision Value ";
  sprintf(str,"%1d",LightValue);
  ptr += str;
  ptr += " Delta Value ";
  sprintf(str,"%4d",BlinkThreshold);
  ptr += str;
  
  ptr += "</h3>\n";

  return (ptr);        
}


/////////////////////////////////////////////
String WebSendTable()
{
  String ptr;

  int i = 0;
  int j = 0;
  int pos = 0;
  char strR[4];
  char strG[4];
  char strB[4];
  
  ptr = "<table style=\"height: 53px; width= 500px; border: 1px \">";
  ptr += "<tbody>";
  ptr += "<tr>";
#define STEP 8
pos = 256*3/STEP*(WaterTemp-10)/(40-10);
if (pos < 0) pos = 0;
if (pos > 256*3/STEP) pos = 256*3/STEP;

// degrade de bleu
for (i=0;i<256;i=i+STEP)
{
  sprintf(strR,"%02X",0);
  sprintf(strG,"%02X",i);
  sprintf(strB,"%02X",255);
  ptr += "<td style=\"height= 47px; background-color: #";
  ptr += strR;
  ptr += strG;
  ptr += strB;
  ptr += ";";
  if (j==pos) ptr += "border: 2px solid #000000;";
  ptr += "\"></td>";
  j++;
}
ptr += "\n";

// degrade de vert
for (i=255;i>-1;i=i-STEP)
{
  sprintf(strR,"%02X",00);
  sprintf(strG,"%02X",255);
  sprintf(strB,"%02X",i);
  ptr += "<td style=\"background-color: #";
  ptr += strR;
  ptr += strG;
  ptr += strB;
  ptr += ";";
  if (j==pos) ptr += "border: 2px solid #000000;";
  ptr += "\"></td>";
  j++;
  
}
ptr += "\n";

// degrade de rouge
for (i=255;i>-1;i=i-STEP)
{
  sprintf(strR,"%02X",255-i);
  sprintf(strG,"%02X",i);
  sprintf(strB,"%02X",0);
  
  ptr += "<td style=\"background-color: #";
  ptr += strR;
  ptr += strG;
  ptr += strB;
  ptr += ";";
  if (j==pos) ptr += "border: 3px solid #000000;";
  ptr += "\"></td>";
  j++;
  
}
ptr += "\n";
ptr += "</tr>";

//// Deuxieme ligne
ptr += "<tr>";
j=0;
int b=10;
for (i=0;i<256;i=i+STEP)
{
  ptr += "<td style=\"height: 5px; background-color: #FFFFFF";
  ptr += ";";
  if (j+1>=256*3/STEP*(b-10)/(40-10)) 
  {
    ptr += " border-left: 2px solid #000000;";
    b++;
  }
  ptr += " \"></td>";
  j++;
}
for (i=0;i<256;i=i+STEP)
{
  ptr += "<td style=\"height: 5px; background-color: #FFFFFF";
  ptr += "; ";
  if (j+1>=256*3/STEP*(b-10)/(40-10)) 
  {
    ptr += "border-left: 2px solid #000000;";
    b++;
  }
  ptr += " \"></td>";
  j++;
}
for (i=0;i<256;i=i+STEP)
{
  ptr += "<td style=\"height: 5px; background-color: #FFFFFF";
  ptr += "; ";
  if (j+1>=256*3/STEP*(b-10)/(40-10)) 
  {
    ptr += "border-left: 2px solid #000000;";
    b++;
  }
  ptr += " \"></td>";
  j++;
}
ptr += "</tr>";
ptr += "</tbody>";
ptr += "</table>";
ptr += "\n";

return (ptr);
}

/////////////////////////////////////////////
String WebSendWater()
{
  String ptr;
  char str[16];

  ptr = "<h3>";
  ptr += "Temp Piscine : ";
  sprintf(str,"%4.2f",WaterTemp);
  ptr += str;
  ptr += " C ";
  ptr += "delai : ";
  sprintf(str,"%2d",min_fin);
  ptr += str;
  ptr += " on : ";
  sprintf(str,"%2d",pompe);
  ptr += str;
  ptr += " done : ";
  sprintf(str,"%2d",pompe_done);
  ptr += str;
  ptr += "</h3>";
  ptr += WebSendTable();

  return (ptr);
}

//////////////////////////////////////////////
  // boucle de traitement
  //
  //================================
void loop() {
  MyWebServer.handleClient();
  Gestion();
  yield();
}


/////////////////////////////////////////////
void handle_PompeOn() {
  PompeForced = 1;
  digitalWrite(RelaisPompe, LOW);
  MyPrintln("Pompe Status: ON");
  MyWebServer.send(200, "text/html", SendHTML(PompeForced,LightForced));
}

/////////////////////////////////////////////
void handle_PompeOff() {
  PompeForced = 0;
  digitalWrite(RelaisPompe, HIGH);
  MyPrintln("Pompe Status: OFF");
  MyWebServer.send(200, "text/html", SendHTML(PompeForced,LightForced));
}

/////////////////////////////////////////////
void handle_LightOn() {
  LightForced= 1;
  if (~LightOn) digitalWrite(RelaisLed, LOW);
  MyPrintln("Light Status: ON");
  MyWebServer.send(200, "text/html", SendHTML(PompeForced,LightForced));
}

void handle_LightOff() {
  LightForced= 0;
  
    if (LightAna <= LightThreshold)
    {
      LightValue = LightAna;
      LightOn = 0;
    }
    else
      digitalWrite(RelaisLed, HIGH);
 
  MyPrintln("Light Status: OFF");
  MyWebServer.send(200, "text/html", SendHTML(PompeForced,LightForced));
}

/////////////////////////////////////////////
void handle_NotFound(){
  String ptr;
  char str[16];

  ptr = "Not found";
  ///
  if (MyWebServer.method() != HTTP_POST) {
    ptr += "Method Not Allowed";
  } else {
    ptr += "POST form was:\n";
    for (uint8_t i = 0; i < MyWebServer.args(); i++) {
      ptr += " " + MyWebServer.argName(i) + ": " + MyWebServer.arg(i) + "\n";
    }
  }

  ///
  MyWebServer.send(404, "text/plain", ptr);
}

/////////////////////////////////////////////
String WebSendHeader ()
{
  String ptr = "<!DOCTYPE html> <html>\n";
  char string[128];
  
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>BOBET's Piscine Control</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: left;}\n";
//ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +="body{margin-top: 0px;} h1 {color: #4444FF;margin: 0px auto 0px;} h3 {color: #444444;margin-bottom: 0px;}\n";
//ptr +=".button {display: block;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 15px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
//ptr +=".button {display: block;background-color: #3498db;border: none;color: white;padding: 2px 2px;text-decoration: none;font-size: 15px;margin: 0px auto 0px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button {display: block;background-color: #3498db;border: none;color: padding: 2px 2px; border-radius: 4px; border: 2px solid dark-blue; white;text-decoration: none;font-size: 15px;display: inline-block; cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #00FF00;}\n";
  ptr +=".button-on: active {background-color: #2980b9;}\n";
  ptr +=".button-off {background-color: #FF0000;}\n";
  ptr +=".button-off: active {background-color: #2c3e50;}\n";
  ptr +=".button-3 {background-color: #4444FF;}\n";
  ptr +=".button-3: active {background-color: #8888FF;}\n";
  
//ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="p {font-size: 14px;color: #888;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";

 ptr +="<h1><center>";
 ptr += HEADER;
 ptr +="</center></h1>\n";

  ptr +="<p>Last boot :";
  sprintf(string,"% 2d ",BootDay);
  ptr += string;
  sprintf(string,"% 2d ",BootMonth);
  ptr += string;
  sprintf(string,"% 4d  ",BootYear);
  ptr += string;
  sprintf(string,"%02d",BootHour);
  ptr +=string;
  ptr +=":";
  sprintf(string,"%02d",BootMinute);
  ptr +=string;
  ptr +=":";
  sprintf(string,"%02d",BootSecond);
  ptr +=string;
  ptr +="</p>\n\n";

  return(ptr);
}

/////////////////////////////////////////////
String WebSendEnd ()
{
  String ptr = "";
  char string[16];
  // Ajout de donnees concernant la gestion de la memoire

  ptr +="<h4 align=\"right\"> Memory ";
  sprintf(string,"Used %d of ", ESP.getFreeHeap()); //available heap
  ptr += string;
  //sprintf(string,"%d", ESP.getHeapSize()); //total heap size
  ptr +="</h4>\n";
  
  ptr +="<h4 align=\"right\"> Compilation ";
  ptr +=F(__DATE__);
  ptr +=" ";
  ptr +=F(__TIME__);
  ptr +="</h4>\n";

  ptr +="<h4 align=\"right\"> Last Reboot ";
  ptr += ESP.getResetReason(); //
  ptr +="</h4>\n";
  
  ptr +="<h4 align=\"right\"> Vcc ";
  ptr += ESP.getVcc(); //
  ptr +="</h4>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";

  return(ptr);
}

/////////////////////////////////////////////
void handle_SetDate()
{
  String ptr;
  char str[16];
  
  unsigned int Year=2020;
  unsigned int Month=01;
  unsigned int Day=01;
  unsigned int Hour=0;
  unsigned int Minute=0;
  unsigned int Seconde=0;
  uint8_t i;
  int DateModified = 0;
  
  DS3231_get(&rtc);// get Time
  Year = rtc.year+100;
  Month = rtc.mon;
  Day = rtc.mday;
  Hour = rtc.hour;
  Minute = rtc.min;
  Seconde = rtc.sec;

  ptr = WebSendHeader();
  ptr += "<p>";
  ptr += "SetDate  ";
  ///
  if (MyWebServer.method() == HTTP_POST) 
  {
    ptr += "POST Method";
  } 
  else if (MyWebServer.method() == HTTP_GET) 
  {
    ptr += "GET Method";
  }
  else
  {
    ptr += "form was:\n";
    
  }
  ptr += "</p>";
  ptr += "<p>";
  
  for (i = 0; i < MyWebServer.args(); i++) 
    {
      ptr += "#" +  MyWebServer.argName(i) + ": " + MyWebServer.arg(i) + "\n";
    }
  ptr += "</p>";
  
    for (i = 0; i < MyWebServer.args(); i++) 
    {
      if (MyWebServer.argName(i) == "Year") 
      {
        DateModified = 1;
        ptr += " argv is Year";
        MyWebServer.arg(i).toCharArray(str,5);
        sscanf(str,"%d",&Year);
        ptr += "=== Year === ";
        sprintf(str,"%4d ",Year);
        ptr += str;
      }
      if (MyWebServer.argName(i) == "Month")
      {
        DateModified = 1;
        ptr += " argv is Month";
        MyWebServer.arg(i).toCharArray(str,3);
        sscanf(str,"%d",&Month);
        ptr += "=== Month === ";
        sprintf(str,"%02d ",Month);
        ptr += str;
      }
      if (MyWebServer.argName(i) == "Day")
      {
        DateModified = 1;
        ptr += " argv is Day";
        MyWebServer.arg(i).toCharArray(str,3);
        sscanf(str,"%d",&Day);
        ptr += "=== Day === ";
        sprintf(str,"%02d ",Day);
        ptr += str;
      }
      if (MyWebServer.argName(i) == "Hour")
      {
        DateModified = 1;
        ptr += " argv is Hour";
        MyWebServer.arg(i).toCharArray(str,3);
        sscanf(str,"%d",&Hour);
        ptr += "=== Hour === ";
        sprintf(str,"%4d ",Hour);
        ptr += str;        
      }
      if (MyWebServer.argName(i) == "Minute")
      {
        DateModified = 1;
        ptr += " argv is Minute";
        MyWebServer.arg(i).toCharArray(str,3);
        sscanf(str,"%d",&Minute);
        ptr += "=== Minute === ";
        sprintf(str,"%4d ",Minute);
        ptr += str;        
      }
      if (MyWebServer.argName(i) == "Seconde")
      {
        DateModified = 1;
        ptr += " argv is Seconde";
        MyWebServer.arg(i).toCharArray(str,3);
        sscanf(str,"%d",&Seconde);
        ptr += "=== Seconde === ";
        sprintf(str,"%4d ",Seconde);
        ptr += str;
      }
    }  

  if (DateModified)
  {
    rtc.year = Year-100;
    rtc.mon = Month ;
    rtc.mday = Day;
    rtc.hour = Hour;
    rtc.min = Minute;
    rtc.sec = Seconde;
    DS3231_set(rtc);
  }
  else
  {
    ptr += "Incorrect Call !!! /SetDate?SetDate?Year=2020&Month=05&Day=01&Hour=23&Minute=12&Seconde=08\n";
  }
  ptr += WebSendTime();
  
/// ??????????????????????
  ptr +="<h3>Temperature en Celsius : ";
  ptr += StrAirTemp;
  ptr += "*C";
  ptr +="</h3>";
  
  ptr +="<h3>Humidite : ";
  ptr += StrAirHumidity;
  ptr += "%";  
  ptr +="</h3>";

  ptr += WebSendWater();
  ptr += WebSendLight();


  ptr += WebSendForcage();
  ptr += WebSendEnd();
  MyWebServer.send(200, "text/html", ptr);
}

/////////////////////////////////////////////
void handle_Date()
{
  String ptr;
  char str[16];
  
  unsigned int Year=2020;
  unsigned int Month=01;
  unsigned int Day=01;
  unsigned int Hour=0;
  unsigned int Minute=0;
  unsigned int Seconde=0;
  uint8_t i;
  int DateModified = 0;

#if 1
  DS3231_get(&rtc);// get Time
  Year = rtc.year+100;
  Month = rtc.mon;
  Day = rtc.mday;
  Hour = rtc.hour;
  Minute = rtc.min;
  Seconde = rtc.sec;
////
  ptr ="";
  ptr +="<!DOCTYPE html>";
  ptr +="<html>";
  
  ////
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: left;}\n";
  ptr +="body{margin-top: 0px;} h1 {color: #4444FF;margin: 0px auto 0px;} h3 {color: #444444;margin-bottom: 0px;}\n";
  ptr +=".button {display: block;background-color: #3498db;border: none;color: padding: 2px 2px; border-radius: 4px; border: 2px solid dark-blue; white;text-decoration: none;font-size: 15px;display: inline-block; cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #00FF00;}\n";
  ptr +=".button-on: active {background-color: #2980b9;}\n";
  ptr +=".button-off {background-color: #FF0000;}\n";
  ptr +=".button-off: active {background-color: #2c3e50;}\n";
  ptr +=".button-3 {background-color: #4444FF;}\n";
  ptr +=".button-3: active {background-color: #8888FF;}\n";
  ptr +="p {font-size: 14px;color: #888;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";

  ////
  ptr +="<body>";
  ptr +="<h2>Definition de la date</h2>";

  ptr +="<form action=\"/SetDate?\">";
  
  ptr +="<label for=\"Year\">Year:</label><br>";
  ptr +="<input type=\"text\" id=\"Year\" name=\"Year\" value=\"2020\"><br>";
  
  ptr +="<label for=\"Month\">Month:</label><br>";
  ptr +="<input type=\"text\" id=\"Month\" name=\"Month\" value=\"05\"><br><br>";
  
  ptr +="<label for=\"Day\">Day:</label><br>";
  ptr +="<input type=\"text\" id=\"Day\" name=\"Day\" value=\"05\"><br><br>";
  ptr +="<label for=\"Hour\">Hour:</label><br>";
  ptr +="<input type=\"text\" id=\"Hour\" name=\"Hour\" value=\"05\"><br><br>";
  ptr +="<label for=\"Minute\">Minute:</label><br>";
  ptr +="<input type=\"text\" id=\"Minute\" name=\"Minute\" value=\"05\"><br><br>";
  ptr +="<label for=\"Seconde\">Seconde:</label><br>";
  ptr +="<input type=\"text\" id=\"Seconde\" name=\"Seconde\" value=\"05\"><br><br>";

  ptr +="<input type=\"submit\" value=\"SetDate\">";
  ptr +="<a class=\"button button-3\" href=\"/\">Status</a>";

  ptr +="</form>";

  ptr +="<p>If you click the \"Submit\" button, the form-data will be sent to a page called \"/action_page.php\".</p>";

ptr += "<script>";
ptr += "var dt = new Date();";
ptr += "document.getElementById(\"Year\").value = dt.getFullYear();";
ptr += "document.getElementById(\"Month\").value = ((\"0\"+(dt.getMonth()+1)).slice(-2));";
ptr += "document.getElementById(\"Day\").value = ((\"0\"+(dt.getDate())).slice(-2));";
ptr += "document.getElementById(\"Hour\").value = ((\"0\"+(dt.getHours())).slice(-2));";
ptr += "document.getElementById(\"Minute\").value = ((\"0\"+(dt.getMinutes())).slice(-2));";
ptr += "document.getElementById(\"Seconde\").value = ((\"0\"+(dt.getSeconds())).slice(-2));";
ptr += "</script>";

  ptr +="</body>";
  ptr +="</html>";
#endif
  MyWebServer.send(200, "text/html", ptr);
} // handle_Date

/////////////////////////////////////////////
void handle_Blink()
{
  String ptr;
  
#if 1
  ptr ="";
  ptr +="<!DOCTYPE html>";
  ptr +="<html>";
  ptr +="<body>";
  ptr +="<h2>Definition du seuil de lumiere</h2>";

  ptr +="<form action=\"/SetBlink?\">";
  
  ptr +="<label for=\"fname\">Blink:</label><br>";
  ptr +="<input type=\"text\" id=\"fname\" name=\"fname\" value=\"20\"><br>";
  
  ptr +="<input type=\"submit\" value=\"SetBlink\">";
  ptr +="</form>";

  ptr +="<p>If you click the \"SetBlink\" button, the form-data will be sent to a page called \"/SetBlink\".</p>";

  ptr +="</body>";
  ptr +="</html>";
#endif
  MyWebServer.send(200, "text/html", ptr);
} // handle_Blink

/////////////////////////////////////////////
void handle_SetBlink(){
  String ptr;
  char str[16];
  
  unsigned int Year=2020;
  unsigned int Month=01;
  unsigned int Day=01;
  unsigned int Hour=0;
  unsigned int Minute=0;
  unsigned int Seconde=0;
  uint8_t i;
  int DateModified = 0;
  
  DS3231_get(&rtc);// get Time
  Year = rtc.year+100;
  Month = rtc.mon;
  Day = rtc.mday;
  Hour = rtc.hour;
  Minute = rtc.min;
  Seconde = rtc.sec;

  ptr = WebSendHeader();
  ptr += "<p>";
  ptr += "SetDate  ";
  ///
  if (MyWebServer.method() == HTTP_POST) 
  {
    ptr += "POST Method";
  } 
  else if (MyWebServer.method() == HTTP_GET) 
  {
    ptr += "GET Method";
  }
  else
  {
    ptr += "form was:\n";
    
  }
  ptr += "</p>";
  ptr += "<p>";
  
  for (i = 0; i < MyWebServer.args(); i++) 
    {
      ptr += "#" +  MyWebServer.argName(i) + ": " + MyWebServer.arg(i) + "\n";
    }
  ptr += "</p>";
  
    for (i = 0; i < MyWebServer.args(); i++) 
    {
      if (MyWebServer.argName(i) == "Blink") 
      {
        ptr += " argv is Blink";
        MyWebServer.arg(i).toCharArray(str,5);
        sscanf(str,"%d",&BlinkThreshold);
        ptr += "=== Blink === ";
        sprintf(str,"%4d ",BlinkThreshold);
        ptr += str;
      }
    }  

  ptr += WebSendTime();
  
  ptr +="<h3>Temperature en Celsius : ";
  ptr += StrAirTemp;
  ptr += "*C";
  ptr +="</h3>";
  
  ptr +="<h3>Humidite : ";
  ptr += StrAirHumidity;
  ptr += "%";  
  ptr +="</h3>";

  ptr += WebSendWater();
  ptr += WebSendLight();

///


  ptr += WebSendForcage();

  ptr += WebSendEnd();
  MyWebServer.send(200, "text/html", ptr);
} // SetBlink

/////////////////////////////////////////////////////////////////////////////////
String WebSendForcage()
{
  String ptr;

  ptr ="";
if(PompeForced)
{
  ptr += "<p>Pompe Status: ON";
}
else
{ptr +="<p>Pompe Status: OFF";}

if(LightForced)
{ptr +=" Light Status: ON</p>";}
else
{ptr +=" Light Status: OFF</p>";}
ptr += "\n";
  ptr += "<p>";

if(PompeForced)
  {ptr +="<a class=\"button button-off\" href=\"/PompeOff\">Arreter forcage</a>";}
else
  {ptr +="<a class=\"button button-on\" href=\"/PompeOn\">Forcer Pompe</a>";}

ptr +="<a class=\"button button-3\" href=\"/\">Status</a>";
ptr +="<a class=\"button button-3\" href=\"/Date\">Date</a>";

if(LightForced)
  {ptr +="<a class=\"button button-off\" href=\"/LightOff\">Arreter forcage</a>";}
else
  {ptr +="<a class=\"button button-on\" href=\"/LightOn\">Forcer Lumiere</a>";}

ptr += "</p>";
ptr += "\n";
return(ptr);
}

/////////////////////////////////////////////
String SendHTML(uint8_t PompeStat,uint8_t LightStat)
{
///
char string[25];
  String ptr = "";

  ptr += WebSendHeader();
  ptr += WebSendTime();
  
  ptr +="<h3>Temperature en Celsius : ";
  ptr += StrAirTemp;
  ptr += "*C";
  ptr +="</h3>";
  
  ptr +="<h3>Humidite : ";
  ptr += StrAirHumidity;
  ptr += "%";  
  ptr +="</h3>";

  ptr += WebSendWater();
  ptr += WebSendLight();

///

  ptr += WebSendForcage();
  ptr += WebSendEnd();

return (ptr);
}

///////////////////////////////////////////////////
///////////////////////////////////////////////////
///////////////////////////////////////////////////
#if 0
#include <WiFiUdp.h>
#include <WiFi.h>
#include "time.h"
#endif
/////////////////////////////////////////////
void handle_NTP()
{
#if 0
String ptr;
  char str[16];
  
  unsigned int Year=2020;
  unsigned int Month=01;
  unsigned int Day=01;
  unsigned int Hour=0;
  unsigned int Minute=0;
  unsigned int Seconde=0;
  uint8_t i;
  int DateModified = 0;
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\



  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
  DS3231_get(&rtc);// get Time
  Year = rtc.year+100;
  Month = rtc.mon;
  Day = rtc.mday;
  Hour = rtc.hour;
  Minute = rtc.min;
  Seconde = rtc.sec;
////
  ptr ="";
  ptr +="<!DOCTYPE html>";
  ptr +="<html>";
  
  ////
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: left;}\n";
  ptr +="body{margin-top: 0px;} h1 {color: #4444FF;margin: 0px auto 0px;} h3 {color: #444444;margin-bottom: 0px;}\n";
  ptr +=".button {display: block;background-color: #3498db;border: none;color: padding: 2px 2px; border-radius: 4px; border: 2px solid dark-blue; white;text-decoration: none;font-size: 15px;display: inline-block; cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #00FF00;}\n";
  ptr +=".button-on: active {background-color: #2980b9;}\n";
  ptr +=".button-off {background-color: #FF0000;}\n";
  ptr +=".button-off: active {background-color: #2c3e50;}\n";
  ptr +=".button-3 {background-color: #4444FF;}\n";
  ptr +=".button-3: active {background-color: #8888FF;}\n";
  ptr +="p {font-size: 14px;color: #888;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";

  ////
  ptr +="<body>";
  ptr +="<h2>Definition de la date</h2>";

  ptr +="<form action=\"/SetDate?\">";
  
  ptr +="<label for=\"Year\">Year:</label><br>";
  ptr +="<input type=\"text\" id=\"Year\" name=\"Year\" value=\"2020\"><br>";
  
  ptr +="<label for=\"Month\">Month:</label><br>";
  ptr +="<input type=\"text\" id=\"Month\" name=\"Month\" value=\"05\"><br><br>";
  
  ptr +="<label for=\"Day\">Day:</label><br>";
  ptr +="<input type=\"text\" id=\"Day\" name=\"Day\" value=\"05\"><br><br>";
  ptr +="<label for=\"Hour\">Hour:</label><br>";
  ptr +="<input type=\"text\" id=\"Hour\" name=\"Hour\" value=\"05\"><br><br>";
  ptr +="<label for=\"Minute\">Minute:</label><br>";
  ptr +="<input type=\"text\" id=\"Minute\" name=\"Minute\" value=\"05\"><br><br>";
  ptr +="<label for=\"Seconde\">Seconde:</label><br>";
  ptr +="<input type=\"text\" id=\"Seconde\" name=\"Seconde\" value=\"05\"><br><br>";

  ptr +="<input type=\"submit\" value=\"SetDate\">";
  ptr +="<a class=\"button button-3\" href=\"/\">Status</a>";

  ptr +="</form>";

  ptr +="<p>If you click the \"Submit\" button, the form-data will be sent to a page called \"/action_page.php\".</p>";

  ptr +="</body>";
  ptr +="</html>";

  MyWebServer.send(200, "text/html", ptr);
#endif
} // handle_NTP

void printLocalTime(){
 #if 0
 struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  Serial.print("Day of week: ");
  Serial.println(&timeinfo, "%A");
  Serial.print("Month: ");
  Serial.println(&timeinfo, "%B");
  Serial.print("Day of Month: ");
  Serial.println(&timeinfo, "%d");
  Serial.print("Year: ");
  Serial.println(&timeinfo, "%Y");
  Serial.print("Hour: ");
  Serial.println(&timeinfo, "%H");
  Serial.print("Hour (12 hour format): ");
  Serial.println(&timeinfo, "%I");
  Serial.print("Minute: ");
  Serial.println(&timeinfo, "%M");
  Serial.print("Second: ");
  Serial.println(&timeinfo, "%S");

  Serial.println("Time variables");
  char timeHour[3];
  strftime(timeHour,3, "%H", &timeinfo);
  Serial.println(timeHour);
  char timeWeekDay[10];
  strftime(timeWeekDay,10, "%A", &timeinfo);
  Serial.println(timeWeekDay);
  Serial.println();
#endif
}

//////////////////////////////////////
void setup() {
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW); // Blue led ON
  Wire.begin(); //clock : start i2c (required for connection)
  DS3231_init(DS3231_INTCN); //register the ds3231 (DS3231_INTCN is the default address of ds3231,
  //this is set by macro for no performance loss)
  DS3231_get(&rtc);// get Time
  BootYear = rtc.year+100;
  BootMonth = rtc.mon;
  BootDay = rtc.mday;
  BootHour = rtc.hour;
  BootMinute = rtc.min;
  BootSecond = rtc.sec;
  // init liaison serie
  Serial.begin(115200);
  delay(7);
  Serial.println("----- SETUP -----");
  dht.begin(); // demarage du DHT 22

  //informations serial Wifi
  Serial.println("");
  Serial.print("lancement connexion au reseau wifi :  ");
  Serial.println(ssid);

  WiFi.begin(ssid, password); //connexion au wifi

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("connexion au WiFi OK");

  // demarrage serveur web
  //server.begin();
  Serial.println("le serveur web est lancé, attente fourniture ip locale de l esp ...");
  delay(1000);

  // affichage adresse ip locale
  Serial.println(WiFi.localIP());
//  pinMode(CapteurLight, INPUT); // Input capteur de luminosite
  pinMode(RelaisPompe, OUTPUT); //Outpout Relais Pompe Relais 2
  pinMode(RelaisLed, OUTPUT); // Outpout Relais Lumiere exterieure Relais 1
  Serial.println(" --- SETUP ENDED --- ");
  MyWebServer.on("/", handle_OnConnect);
  MyWebServer.on("/PompeOn", handle_PompeOn);
  MyWebServer.on("/PompeOff", handle_PompeOff);
  MyWebServer.on("/LightOn", handle_LightOn);
  MyWebServer.on("/LightOff", handle_LightOff);
  MyWebServer.on("/SetDate", handle_SetDate);
  MyWebServer.on("/Date", handle_Date);
  MyWebServer.on("/SetBlink", handle_SetBlink);
  MyWebServer.on("/NTP", handle_NTP);
  MyWebServer.on("/Date", handle_Date);MyWebServer.onNotFound(handle_NotFound);
  MyWebServer.begin();
  MyPrintln("HTTP server started");

// NTP
  timeClient.setPoolServerName("pool.ntp.org");
  timeClient.begin();
  timeClient.update();
  Serial.println(timeClient.getFormattedTime()); // String format !!!
  Serial.println("Day : " + timeClient.getDay());
  Serial.println(timeClient.getHours());
  Serial.println(timeClient.getMinutes());
  Serial.println(timeClient.getSeconds());

  DS3231_get(&rtc);// get Time
  
  Serial.println("year : ");
  Serial.println(rtc.year);
  if (rtc.year < 1920)
  {
    Serial.println("Update Time");
    rtc.year = 2020-100;
    rtc.mon = 6 ;
    rtc.mday = 1;
    rtc.hour = timeClient.getHours()+2;
    rtc.min = timeClient.getMinutes();
    rtc.sec = timeClient.getSeconds();
    DS3231_set(rtc);

  }
    
  digitalWrite(2, HIGH); // Blue led off
} /// Setup
