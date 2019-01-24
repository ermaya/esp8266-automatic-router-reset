Led alarma  D1 :  LOW = conexion OK.  HIGH = no conexion.
Status Led  D4 :  flach rapido 0,2 = configuracion. flach mediano 0,8 = no Wifi. flach lento 5,0 =  normal.
TRIGGER PIN D3:   configuracion.
Relay       D2:   normal LOW. Reset 5 segundos HIGH

sistema hace Ping cada 30 segundos a www.google.com (puede cambiar en Wifi Config) si esta bien Led alarma se apaga.
si no hay conexion Led alarma enciende y cuenta tiempo para Reset.
unsigned long reset_delay = 900000; 15 minutos = 15x60x1000

[IMG]http://i65.tinypic.com/dhcvue.png[/IMG]