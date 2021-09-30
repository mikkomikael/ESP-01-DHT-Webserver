#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#include <WifiUDP.h>
#include <String.h>
#include <NTPClient.h>
#include <Time.h>
#include <TimeLib.h>
//#include <Timezone.h> //original library
#include <Timezone_Generic.h>

#define DHTPin 2 //D2-GPIO4 (D1 Mini), D4-GPIO2 (GPIO2 ESP-01S) D4(D1 Mini), D6-GPIO12 (D1 Mini)
#define DHTTYPE DHT11 //DHT11
//#define DHTTYPE DHT22 //DHT22 (AM2302)
DHT dht(DHTPin, DHTTYPE);

#define NTP_OFFSET 60 * 60
#define NTP_INTERVAL 60 * 1000
#define NTP_ADDRESS "ntp.unice.fr" //change to closest pool (see ntp.org)

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

const char* ssid = "SSID"; //change SSID
const char* password = "PASSWORD"; //change PASSWORD

float Temperature = 0;
float Humidity = 0;

String date;
String wd;
String t;
String y;

const char * days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"} ;
const char * months[] = {"Jan", "Feb", "Mar", "Apr", "Maj", "Jun", "Jul", "Aug", "Sept", "Oct", "Nov", "Dec"} ;
const char * ampm[] = {"AM", "PM"} ;

ESP8266WebServer server(80);

void setup () {
  Serial.begin(115200);
  pinMode(DHTPin, INPUT);
  //Wire.pins(0, 2);
  //Wire.begin(0, 2); // 0=sda, 2=scl
  delay(100);
  dht.begin();
  timeClient.begin();
  Serial.println("");
  Serial.print("Connecting...");
  Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connecting to WiFi...");
  Serial.println("");
  Serial.print("IP address: ");
  Serial.print(WiFi.localIP());
  Serial.println("");
  Serial.println("");
  delay(5000);
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  if (WiFi.status() == WL_CONNECTED)
  {
    date = "";
    t = "";
    wd = "";
    y = "";
    timeClient.update();
    unsigned long epochTime =  timeClient.getEpochTime();
    time_t local, utc;
    utc = epochTime;
    TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, 60};
    TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, 0};
    Timezone usEastern(usEDT, usEST);
    local = usEastern.toLocal(utc);
    
    wd +=days[weekday(local) - 1];
    date +=day(local);
    date +=" ";
    date +=months[month(local) - 1];
    date +=" ";
    y +=year(local);
    t +=hour(local);
    t +=":";
    if (minute(local) < 10)
      t +="0";
    t +=minute(local);
    t +=":";
    if (second(local) < 10)
      t +="0";
    t +=second(local);

    Serial.print("Local date: ");
    Serial.print(date);
    Serial.print(y);
    Serial.println("");
    Serial.print("Local time: ");
    Serial.print(t);
    Serial.println("");
    Serial.print("Temperature: ");
    Serial.print(Temperature);
    Serial.println("°C");
    Serial.print("Humidity: ");
    Serial.print(Humidity);
    Serial.println("%");
    Serial.println();
  }
  else
  {
    Serial.print("Reconnecting");
    WiFi.begin(ssid, password);
    Serial.print("Connected!");
    delay(1000);
  }
  delay(1000);
}

void handle_OnConnect() {
  Temperature = dht.readTemperature();
  Humidity = dht.readHumidity();
  server.send(200, "text/html", SendHTML(t, date, wd, y, Temperature, Humidity));
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

String SendHTML(String t, String date, String wd, String y, float Temperaturestat, float Humiditystat) {
  String ptr = "<!DOCTYPE html>";
  ptr +="<html>";
  ptr +="<head>";
  ptr +="<title>Smart Home Starter set</title>";
  ptr +="<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  //ptr +="<meta http-equiv='refresh' content='5'>"; //refresh web page every '1' seconds
  ptr +="<link href='https://fonts.googleapis.com/css?family=Open+Sans:300,400,600' rel='stylesheet'>";
  ptr +="<style>";
  ptr +="html { font-family: 'Open Sans', sans-serif; display: block; margin: 0px auto; text-align: center;color: #444444;}";
  ptr +="body{margin: 0px;}";
  ptr +="h1 {margin: 10px auto 10px; color: #b8282eff;}";
  ptr +=".side-by-side{display: table-cell; vertical-align: middle; position: relative;}";
  ptr +=".text{font-weight: 400; font-size: 20px; width: 200px;}";
  ptr +=".reading{font-weight: 500; font-size: 22px; padding-right: 20px;}";
  ptr +=".time .reading{color: #945ba5;}";
  ptr +=".date .reading{color: #26B99A;}";
  ptr +=".wd .reading{color: #b8282eff;}";
  ptr +=".y .reading{color: #dd55ff;}";
  ptr +=".temperature .reading{color: #F29C1F;}";
  ptr +=".humidity .reading{color: #3C97D3;}";
  ptr +=".superscript{font-size: 12px;font-weight: 600;position: absolute;top: 10px;}";
  ptr +=".data{padding: 10px;}";
  ptr +=".container{display: table;margin: 0 auto;}";
  ptr +=".icon{width: 50px}";
  ptr +="</style>";
  //AJAX REFRESH START
  ptr +="<script>\n";
  ptr +="setInterval(loadDoc,200);\n";
  ptr +="function loadDoc() {\n";
  ptr +="var xhttp = new XMLHttpRequest();\n";
  ptr +="xhttp.onreadystatechange = function() {\n";
  ptr +="if (this.readyState == 4 && this.status == 200) {\n";
  ptr +="document.getElementById(\"webpage\").innerHTML =this.responseText}\n";
  ptr +="};\n";
  ptr +="xhttp.open(\"GET\", \"/\", true);\n";
  ptr +="xhttp.send();\n";
  ptr +="}\n";
  ptr +="</script>\n";
  //AJAX REFRESH END
  ptr +="</head>";
  ptr +="<body>";  
  ptr +="<h1>AZ-Delivery</h1>";
  ptr +="<h3>Weather Station - Time Server</h3>";
  ptr +="<h3>ESP8266-01S/DHT22</h3>";
  ptr +="<div class='container'>";
  ptr +="<div class='data time'>";
  ptr +="<div class='side-by-side icon'>";
  ptr +="<svg width='40' height='40' enable-background='new 0 0 19.438 54.003' version='1.1' xmlns='http://www.w3.org/2000/svg'>";
  ptr +="<path d='m20 0a20 20 0 0 0-20 20 20 20 0 0 0 20 20 20 20 0 0 0 20-20 20 20 0 0 0-20-20zm-0.007812 3.8203c0.9695 0 1.75 0.7805 1.75 1.75v12.75h11.469c0.9695 0 1.75 0.7805 1.75 1.75 0 0.9695-0.7805 1.75-1.75 1.75h-13.219c-0.9695 0-1.75-0.7805-1.75-1.75v-14.5c0-0.9695 0.7805-1.75 1.75-1.75z' fill='#945ba5'/></svg>";
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>Time</div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=(t);
  ptr +="<span class='superscript'></span></div>";
  ptr +="</div>";
  ptr +="<div class='data date'>";
  ptr +="<div class='side-by-side icon'>";
  ptr +="<svg width='40' height='40' enable-background='new 0 0 19.438 54.003' version='1.1' xmlns='http://www.w3.org/2000/svg'>";
  ptr +="<path d='m20 0a20 20 0 0 0-20 20 20 20 0 0 0 20 20 20 20 0 0 0 20-20 20 20 0 0 0-20-20zm-7.2871 9.3652h16.113v3.1094l-8.3359 18.275h-5.3711l7.8906-17.33h-10.297v-4.0547z' fill='#26b99a'/></svg>";
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>Date</div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=(date);
  ptr +="<span class='superscript'></span></div>";
  ptr +="</div>";
  ptr +="<div class='data wd'>";
  ptr +="<div class='side-by-side icon'>";
  ptr +="<svg width='40' height='40' enable-background='new 0 0 19.438 54.003' version='1.1' xmlns='http://www.w3.org/2000/svg'>";
  ptr +="<path d='m20 0a20 20 0 0 0-20 20 20 20 0 0 0 20 20 20 20 0 0 0 20-20 20 20 0 0 0-20-20zm-15.189 10.428h5.2852l3.6953 15.541 3.668-15.541h5.3125l3.668 15.541 3.6953-15.541h5.2422l-5.043 21.385h-6.3594l-3.8809-16.256-3.8379 16.256h-6.3594l-5.0859-21.385z' fill='#b8282e'/></svg>";
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>Weekday</div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=(wd);
  ptr +="<span class='superscript'></span></div>";
  ptr +="</div>";
  ptr +="<div class='data y'>";
  ptr +="<div class='side-by-side icon'>";
  ptr +="<svg width='40' height='40' enable-background='new 0 0 19.438 54.003' version='1.1' xmlns='http://www.w3.org/2000/svg'>";
  ptr +="<path d='m20 0a20 20 0 0 0-20 20 20 20 0 0 0 20 20 20 20 0 0 0 20-20 20 20 0 0 0-20-20zm-9.7949 11.525h5.4805l4.4277 6.9258 4.4277-6.9258h5.4941l-7.4082 11.25v8.1895h-5.0137v-8.1895l-7.4082-11.25z' fill='#d5f'/></svg>";
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>Year</div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=(y);
  ptr +="<span class='superscript'></span></div>";
  ptr +="</div>";
  ptr +="<div class='data temperature'>";
  ptr +="<div class='side-by-side icon'>";
  ptr +="<svg width='40' height='40' enable-background='new 0 0 19.438 54.003' version='1.1' xmlns='http://www.w3.org/2000/svg'>";
  ptr +="<path d='m20 0a20 20 0 0 0-20 20 20 20 0 0 0 20 20 20 20 0 0 0 20-20 20 20 0 0 0-20-20zm-0.14599 3.3562h0.26673c1.4693 0 2.6523 1.183 2.6523 2.6523v15.777c0 0.05936-0.0049 0.11762-0.0087 0.17598a7.0707 7.0707 0 0 1 4.3063 6.5073 7.0707 7.0707 0 0 1-7.0707 7.0707 7.0707 7.0707 0 0 1-7.0707-7.0707 7.0707 7.0707 0 0 1 4.2827-6.4891c-0.0046-0.06427-0.01026-0.12865-0.01026-0.19413v-15.777c0-1.4693 1.183-2.6523 2.6523-2.6523z' fill='#f39c12'/></svg>";
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>Temperature</div> ";
  ptr +="<div class='side-by-side reading'>";
  ptr +=(int)Temperaturestat;
  ptr +="<span class='superscript'>°C</span></div>";
  ptr +="</div>";
  ptr +="<div class='data humidity'>";
  ptr +="<div class='side-by-side icon'>";
  ptr +="<svg width='40' height='40' enable-background='new 0 0 19.438 54.003' version='1.1' xmlns='http://www.w3.org/2000/svg'>";
  ptr +="<path d='m20 0a20 20 0 0 0-20 20 20 20 0 0 0 20 20 20 20 0 0 0 20-20 20 20 0 0 0-20-20zm-0.10938 10.752 5.8965 10.439a7.5 7.5 0 0 1 1.2832 2.2715l0.087891 0.15625h-0.035156a7.5 7.5 0 0 1 0.38476 2.3438 7.5 7.5 0 0 1-7.5 7.5 7.5 7.5 0 0 1-7.5-7.5 7.5 7.5 0 0 1 0.33789-2.2227h-0.021484l0.064453-0.12109a7.5 7.5 0 0 1 1.2852-2.3594l5.7168-10.508z' fill='#3c97d3'/></svg>";
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>Humidity</div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=(int)Humiditystat;
  ptr +="<span class='superscript'>%</span></div>";
  ptr +="</div>";
  ptr +="</div>";
  ptr +="</body>";
  ptr +="</html>";
  return ptr;
}
