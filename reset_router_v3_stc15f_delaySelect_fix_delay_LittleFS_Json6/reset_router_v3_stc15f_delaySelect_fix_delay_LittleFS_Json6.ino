#include "LittleFS.h" // LittleFS is declared   
#include <ESP8266Ping.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiManager.h>     //--------- https://github.com/tzapu/WiFiManager/tree/0.14 -------------
#include <ArduinoJson.h>   
#include <Ticker.h>     
Ticker ticker;
WiFiManager wifiManager;  

#define TRIGGER_PIN 0  //D3     // wifi config ( Do masy ) GIPO0
#define onboard_led 2  //D4     // Status Led  (Dioda wemosa d1 mini.) GIPO2
byte relay_on[] = {0xA0, 0x01, 0x01, 0xA2};
byte relay_off[] = {0xA0, 0x01, 0x00, 0xA1};
char host[40] = "www.google.com";
char Reset_delay[5];
int customFieldLength = 50;
WiFiManagerParameter custom_field;
bool shouldSaveConfig = false;
bool initialConfig = false;
bool net_ok = false;
int reset_delay = 5;
int DL;
unsigned long time_last = 30000;
unsigned long time_last_chek;
unsigned long Chek_delay = 15000;       //  Ping 15 sek. --- Ping Delay ---
int timeout           = 120; // Wifi manager timeout
int C_W_state = HIGH;            
int last_C_W_state = HIGH;       
unsigned long time_last_C_W_change = 0;   
long C_W_delay = 3000;               // config delay 3 seconds  ------------ opóźnienie konfiguracji 3 sekund
 
void tick(){
  int state = digitalRead(onboard_led);  
  digitalWrite(onboard_led, !state);     
}
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void ondemandwifiCallback () {
  ticker.attach(0.5, tick);

  WiFiManagerParameter custom_host("host", "host", host, 40);
  const char* custom_radio_str = "<br><div style='text-align:center; width:100%; padding:0;'><label for='customfieldid'>Reset Delay</label><input type='radio' name='customfieldid' value='5' checked> 5 minutes<input type='radio' name='customfieldid' value='10'> 10 minutes<input type='radio' name='customfieldid' value='15'> 15 minutes</div>";
  new (&custom_field) WiFiManagerParameter(custom_radio_str); // custom html input

  wifiManager.setBreakAfterConfig(true);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setSaveParamsCallback(saveParamCallback);

  wifiManager.addParameter(&custom_host);
  wifiManager.addParameter(&custom_field);

    wifiManager.setMinimumSignalQuality();
   //wifiManager.setShowStaticFields(true); // force show static ip fields
   //wifiManager.setShowDnsFields(true);    // force show dns field always
    wifiManager.setConfigPortalTimeout(timeout);

    if (!wifiManager.startConfigPortal("AutoReset")) {
      Serial.println("failed to connect and hit timeout");      
    }
    strcpy(host, custom_host.getValue());  
}
String getParam(String name){
  String value;
  if(wifiManager.server->hasArg(name)) {
    value = wifiManager.server->arg(name);
  }
  return value;
}
void saveParamCallback(){
  String valor = getParam("customfieldid");
  if (valor == "15"){DL=15;}
  else if (valor == "10"){DL=10;}
  else {DL=5;}    
  Serial.print("[CALLBACK] save Reset delay: ");
  Serial.println(DL);
  itoa(DL ,Reset_delay,10) ;
}
void check_host (){
  
  Serial.print("Pinging host ");
  Serial.println(host);

   if (digitalRead(onboard_led == HIGH)){ 
      digitalWrite(onboard_led, LOW);              
      delay(100);                                  
      digitalWrite(onboard_led, HIGH);             
    } else {
      digitalWrite(onboard_led, HIGH);
      delay(100);
      digitalWrite(onboard_led, LOW);
    }

  if(Ping.ping(host)) {
    Serial.println("Success!!");

    digitalWrite(onboard_led, HIGH);
    time_last = millis();
    net_ok = true;
  } else {
    digitalWrite(onboard_led, LOW);
    Serial.println("Error :(");
    net_ok = false;
  }
}

void setup() {
   Serial.begin(9600);
   delay(100);
   pinMode(TRIGGER_PIN, INPUT_PULLUP);
   pinMode(onboard_led, OUTPUT);
   digitalWrite(onboard_led, HIGH);
   Serial.write(relay_off, sizeof(relay_off));
   yield();

    if (WiFi.SSID()==""){   
    initialConfig = true;
  }
  if (LittleFS.begin()) {  // ------------------------- wificonfig read -----------------
    Serial.println("mounted file system");
    if (LittleFS.exists("/config.json")) {
      Serial.println("reading config file");
       File configFile = LittleFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
         size_t size = configFile.size();
         std::unique_ptr<char[]> buf(new char[size]);
         configFile.readBytes(buf.get(), size);
        DynamicJsonDocument json(1024);
        DeserializationError deserializeError = deserializeJson(json, buf.get());
        serializeJsonPretty(json, Serial);
        if (!deserializeError) {Serial.println("\nparsed json");         
          if (json.containsKey("host")) strcpy(host, json["host"]);
          if (json.containsKey("Reset_delay")) strcpy(Reset_delay, json["Reset_delay"]);
          reset_delay = (String(Reset_delay).toInt());
          reset_delay = (reset_delay * 1000 * 60);
          Serial.print("reset delay: ");
          Serial.print(reset_delay);  
          Serial.println(" MS");     
        } else {
          Serial.println("failed to load json config");
           initialConfig = true;
        }
        configFile.close(); 
      }
    }
   } else {
    Serial.println("failed to mount FS");
  }
     
  WiFi.mode(WIFI_STA);
  time_last = millis();
  reset_delay = (300000);  
}

void loop() {
    
  if (initialConfig){    //   ----------enter WiFi configuration if there is no data in memory---------------
    ondemandwifiCallback () ;
    initialConfig = false; 
  }
  
  checkButton();           

  if (shouldSaveConfig) {
    Serial.println(" config...");  
    DynamicJsonDocument json(1024);
    json["host"] = host;
    json["Reset_delay"] = Reset_delay;
    File configFile = LittleFS.open("/config.json", "w");
    if (!configFile) { Serial.println("failed to open config file for writing"); }     
    serializeJsonPretty(json, Serial);
    serializeJson(json, configFile);
    configFile.close();
    Serial.println("Config written successfully");
    shouldSaveConfig = false;
    initialConfig = false; 
    WiFi.mode(WIFI_STA);   
    delay(5000);
    ESP.restart();     
  }
  
  if (WiFi.status() != WL_CONNECTED) 
   {
    WiFi_up();
   }
  if ((millis() - time_last_chek) > Chek_delay) {    
      check_host();
      time_last_chek = millis();
    }
  if  (net_ok == false){   
   if ((millis() - time_last) > reset_delay) {
     Serial.println("Reset");
     delay(100);
     Serial.write(relay_on, sizeof(relay_on));
     Serial.print("Relay on: ");
     Serial.println(millis());
     delay(5000);                             
     Serial.write(relay_off, sizeof(relay_off));
     Serial.print("Relay off: ");
     Serial.println(millis());
     time_last = millis();
     }
    }
    yield();   
}
void checkButton(){    
  // check for button press
  int C_W_read = digitalRead(TRIGGER_PIN);{    
   if (C_W_read != last_C_W_state) {            
     time_last_C_W_change = millis();
   }
   if ((millis() - time_last_C_W_change) > C_W_delay) {     
     if (C_W_read != C_W_state) {     
       C_W_state = C_W_read;       
       if (C_W_state == LOW) {
        Serial.println("Triger config");
      ondemandwifiCallback () ;
       }
     }    
    }
   last_C_W_state = C_W_read;            
   }
}
void WiFi_up(){ 
  WiFi.begin(); 

  for (int x = 30; x > 0; x--) 
  {
    if (WiFi.status() == WL_CONNECTED) 
    {
      break;                           
    }
    else                                 
    {
      checkButton();    
      //Serial.print(".");                
      delay(200);                      
    }
  }

  if (WiFi.status() == WL_CONNECTED){
           
    Serial.println("");
    Serial.println("Connected");
    Serial.println("Adres IP: ");
    Serial.print(WiFi.localIP());
    Serial.print(" / ");
    Serial.print(WiFi.subnetMask());
    Serial.print(" / ");
    Serial.println(WiFi.gatewayIP());
    long rssi = WiFi.RSSI();
    Serial.print("(RSSI): ");
    Serial.print(rssi);
    Serial.println(" dBm");
    time_last = millis();
  }
  else    
  {
    //Serial.println("");
    //Serial.println("connection failed");
    net_ok = false;                               
  }
}
