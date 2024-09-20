ESP32 ha un sistema nativo per la gestione del tempo 
tramite protocollo NTP, agganciandosi ad esempio
al server: "pool.ntp.org"


Oppure mediante un client web è possibile ottenere
l'ora esatta (e non solo) in formato JSON, mediante
un webservice molto comodo:

curl http://worldtimeapi.org/api/timezone/Europe/Rome

{
	"utc_offset":"+02:00",
	"timezone":"Europe/Rome",
	"day_of_week":1,
	"day_of_year":260,
	"datetime":"2024-09-16T21:20:30.969787+02:00",
	"utc_datetime":"2024-09-16T19:20:30.969787+00:00",
	"unixtime":1726514430,
	"raw_offset":3600,
	"week_number":38,
	"dst":true,
	"abbreviation":"CEST",
	"dst_offset":3600,
	"dst_from":"2024-03-31T01:00:00+00:00",
	"dst_until":"2024-10-27T01:00:00+00:00",
	"client_ip":"x.x.x.x"
}