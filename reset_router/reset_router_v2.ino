#include <FS.h>       // ---- esp board manager 2.4.2 --- iwip Variant V2 higher Bandwidth
#include <ESP8266Ping.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiManager.h>     //--------- https://github.com/tzapu/WiFiManager/tree/0.14 -------------
#include <ArduinoJson.h>     //--------- https://github.com/bblanchon/ArduinoJson/tree/v5.13.2 ------
#include <Ticker.h>     
Ticker ticker;
WiFiManager wifiManager;    //   ------------------------------------ NEW Settings Reset delay 3" --------------------------------------

#define alarm_led   D1     // High if no net connection (Dodatkowa dioda LED(czerwona) przez rezystor 330 ohm do masy.) GIPO5
#define Relay       D2     // High for 5 seconds on reset ( Przekaźnik ) GIPO 4
#define TRIGGER_PIN D3     // wifi config ( Do masy ) GIPO0
#define onboard_led D4     // Status Led  (Dioda wemosa d1 mini.) GIPO2

char host[40] = "www.google.com";
bool shouldSaveConfig = false;
bool initialConfig = false;
bool net_ok = false;
unsigned long reset_delay = 900000;   // 900000 --- Reset Delay in miliseconds --- ( Czas 15 min - do resetu. )
unsigned long time_last = 30000;
unsigned long time_last_chek;
unsigned long Chek_delay = 30000;       //  Ping co 30 sek. --- Ping Delay ---
int timeout           = 120; // Wifi manager timeout

void tick()
{
  int state = digitalRead(onboard_led);  
  digitalWrite(onboard_led, !state);     
}
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void ondemandwifiCallback () {
  ticker.attach(0.2, tick);

  WiFiManagerParameter custom_host("host", "host", host, 40);

  wifiManager.setBreakAfterConfig(true);                  // new
  
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  wifiManager.addParameter(&custom_host);

  wifiManager.setMinimumSignalQuality();

    wifiManager.setConfigPortalTimeout(timeout);

    if (!wifiManager.startConfigPortal("AutoReset")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      ESP.restart();
      delay(5000);
    }
    Serial.println("connected...yeey :)");

    strcpy(host, custom_host.getValue());  
}
void check_host (){
  
  Serial.print("Pinging host ");
  Serial.println(host);

  if(Ping.ping(host)) {
    Serial.println("Success!!");
    
    digitalWrite(onboard_led, LOW);              // Mrugnięcie podczas pinga.
    delay(100);                                  //        - II -
    digitalWrite(onboard_led, HIGH);             //        - II -
   
    digitalWrite(alarm_led, LOW);
    time_last = millis();
    net_ok = true;
  } else {
    digitalWrite(alarm_led, HIGH);
    Serial.println("Error :(");
    net_ok = false;
  }
}


void setup() {
  Serial.begin(115200);

   pinMode(TRIGGER_PIN, INPUT_PULLUP);
   pinMode(onboard_led, OUTPUT);
   pinMode(Relay, OUTPUT);
   pinMode(alarm_led, OUTPUT);
   
  ticker.attach(0.8, tick);

    if (WiFi.SSID()==""){   
    initialConfig = true;
  }
  //read configuration from FS json
  Serial.println("mounting FS...");
  
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;         
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        Serial.println(jsonBuffer.size());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(host, json["host"]);

        } else {
          Serial.println("failed to load json config");
          
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  } 
  WiFi.mode(WIFI_STA);
}

void loop() {
    
  if (initialConfig){    //   ------------------------------------ NEW Settings Reset delay 3" --------------------------------------
    ondemandwifiCallback () ;
    initialConfig = false; 
  }
  
  checkButton();           //   ------------------------------------ NEW Settings Reset delay 3" --------------------------------------
  
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["host"] = host;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();
    Serial.println("config saved");
    shouldSaveConfig = false;
    WiFi.mode(WIFI_STA);
    ticker.detach();
    digitalWrite(onboard_led, HIGH);
    ESP.restart();
    delay(5000); 
    }
  
  if (WiFi.status() != WL_CONNECTED) 
   {
    ticker.attach(0.8, tick);
    WiFi_up();
   }
  if ((millis() - time_last_chek) > Chek_delay) {    
      check_host();
      time_last_chek = millis();
    }
  if  (net_ok == false){
   
   if ((millis() - time_last) > reset_delay) {
     Serial.println("Reset");
     digitalWrite(Relay, HIGH);
     delay(5000);                             // Czas włączenia przekaźnika
     digitalWrite(Relay, LOW);
     time_last = millis();
     }
    }
    yield();   
}
void checkButton(){    //   ------------------------------------ NEW Settings Reset delay 3" --------------------------------------
  // check for button press
  if ( digitalRead(TRIGGER_PIN) == LOW ) {
    delay(50);
    if( digitalRead(TRIGGER_PIN) == LOW ){
      Serial.println("Button Pressed");
      delay(3000); // reset delay hold
      if( digitalRead(TRIGGER_PIN) == LOW ){
        Serial.println("Button Held");
        Serial.println("Erasing Config, restarting");
        wifiManager.resetSettings();
        ESP.restart();
      }     
      Serial.println("Starting config portal");
      ondemandwifiCallback () ;
    }
  }
}

void WiFi_up() 

{
  WiFi.begin(); 

  for (int x = 20; x > 0; x--) 
  {
    if (WiFi.status() == WL_CONNECTED) 
    {
      break;                           
    }
    else                                 
    {
      checkButton();    //   ------------------------------------ NEW Settings Reset delay 3" --------------------------------------
      Serial.print(".");                
      delay(200);                      
    }

  }

  if (WiFi.status() == WL_CONNECTED)
  {
  //  ticker.attach(5.0, tick);  // 5.0 sek dioda ON i 5.0 sek dioda OFF podczas połączenia z WiFi
   
    ticker.detach();                                // Dioda wyłączona podczas połączenia z wifi ( źeby mrugneła podczas pingowania ).
      if (digitalRead(onboard_led == LOW)) {        //                          - II -
          digitalWrite(onboard_led, HIGH);          //                          - II -
          }                                         //                          - II -
           
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
    Serial.println("");
    Serial.println("connection failed");
    net_ok = false;                                // new
  }
}
