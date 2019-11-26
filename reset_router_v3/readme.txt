reset image ulr
[IMG]http://i65.tinypic.com/dhcvue.png[/IMG]

Led alarma  D1 :  LOW = conexion OK.  HIGH = no conexion.
Status Led  D4 :  flach rapido 0,2 = configuracion. flach mediano 0,8 = no Wifi. flach lento normal.
TRIGGER PIN D3:   configuracion.
Relay       D2:   normal LOW. Reset 5 segundos HIGH

#define alarm_led   5  //D1     // High if no net connection (Dodatkowa dioda LED(czerwona) przez rezystor 330 ohm do masy.) GIPO5
#define Relay       4  //D2     // High for 5 seconds on reset ( Przekaźnik ) GIPO 4
#define TRIGGER_PIN 0  //D3     // wifi config ( Do masy ) GIPO0
#define onboard_led 2  //D4     // Status Led  (Dioda wemosa d1 mini.) GIPO2


sistema hace Ping cada 30 segundos a www.google.com (puede cambiar en Wifi Config) si esta bien Led alarma se apaga.
si no hay conexion Led alarma enciende y cuenta tiempo para Reset.
unsigned long reset_delay = 900000; 15 minutos = 15x60x1000

Dioda LED alarmu D1: LOW = połączenie OK. HIGH = brak połączenia.
Dioda stanu D4: flash 0,2 = konfiguracja.flash 0.8 = brak WiFi.flash 5.0 = Normalny
TRIGGER PIN D3: konfiguracja.
Relay D2: normalny NISKI. Zresetuj 5 sekund WYSOKI

System ping co 30 sekund do www.google.com (można zmienić w konfiguracji Wifi), jeśli jest w porządku. Alarm Led gaśnie.
Jeśli nie ma połączenia, dioda Led alarmu włącza się i liczy czas na resetowanie.
unsigned long reset_delay = 900000; 15 minut = 15 x 60 x 1000

i Zresetuj, jeśli nie ma wifi