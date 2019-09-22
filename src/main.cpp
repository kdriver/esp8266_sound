#include <Arduino.h>
#include <WiFiCreds_end.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <Syslog.h>


#include   "History.h"


#include <InfluxDb.h>
#define INFLUXDB_HOST "192.168.1.161"
#define INFLUXDB_PORT "1337"
#define INFLUXDB_DATABASE "boiler_sound"

#define BOILER_OFF 0
#define BOILER_ON 1
#define BOILER_NO_CHANGE 2
#define BOILER_STARTING 3

#define ON_THRESHOLD 400
#define OFF_THRESHOLD 250

#define WATCHDOG_INTERVAL 300

unsigned long epoch;
unsigned long time_now;
unsigned long previous_time;
unsigned long since_epoch;

unsigned int boiler;
unsigned int boiler_status= BOILER_OFF;
unsigned int boiler_switched_on_time;




Influxdb influx(INFLUXDB_HOST);


bool DEBUG_ON=false;

bool quiet = false;

ESP8266WebServer server(80);
void handleRoot();              // function prototypes for HTTP handlers
void handleNotFound();


void tell_influx(unsigned int status, unsigned int time_interval)
{
  InfluxData measurement("boiler_status");
  
  measurement.addValue("interval",time_interval);
  measurement.addValue("interval_mins",time_interval/60);
  measurement.addValue("interval_secs",time_interval%60);

  if ( status == BOILER_OFF )
    measurement.addValue("value",0);
  else
    measurement.addValue("value",1);
    
    influx.write(measurement);
}

void tick_influx(String text,unsigned int value)
{
  InfluxData measurement("boiler_iot");
  measurement.addTag("lifecycle",text);
  measurement.addValue("value",value);
  //measurement.addValue("uptime_hours",millis()/1000/60/60);
  influx.write(measurement);
}

void ICACHE_RAM_ATTR sound();
void ICACHE_RAM_ATTR measure();

void test(void)
{
   History *h = new History(20);

    for ( int i = 0 ; i < 30 ; i++ )
        h->add(5);
    

    Serial.println(h->average());
}
History *history = new History(30);

void setup() {

pinMode(D0, INPUT);
Serial.begin(9600);

delay(2000);

Serial.println("I'm alive");
 
  WiFi.begin(ssid, pass);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  
  String address = WiFi.localIP().toString();
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(address);

  server.on("/", handleRoot);               // Call the 'handleRoot' function when a client requests URI "/"
  server.onNotFound(handleNotFound);        // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"
  server.begin();  


  timer1_attachInterrupt(measure);
  timer1_enable(TIM_DIV256, TIM_EDGE, TIM_LOOP); // 
  timer1_write(312000*2); // 1 second

  pinMode(D2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(D2),sound,FALLING);

  influx.setDb(INFLUXDB_DATABASE);

  tick_influx(String("Initialised_1_0_"+ address),0);

  epoch = millis();
  previous_time = epoch;

}
int tick=0;
int events = 0;
int last_second_events =0;
int moving_average;

void measure()
{
  char output[50];
  
   history->add(last_second_events);
   moving_average = history->moving_average(5);

   if ( DEBUG_ON )
    {
      sprintf(output,"measure %ld events %d ps last second %d , moving ave %d",millis()/1000,events,last_second_events,moving_average);
      Serial.println(output);
    }
   last_second_events = events;
   events = 0;
}

void sound()
{
    events = events + 1;
}

void loop() {

delay(10);
noInterrupts();

time_now = millis();

moving_average = history->moving_average(5);

if ( boiler_status == BOILER_OFF  && moving_average > 150 )
{
    tell_influx(BOILER_ON,0);
    boiler_status = BOILER_ON;
    boiler_switched_on_time = time_now;
    Serial.println("");
    Serial.println(String("Boiler switched ON ") + String(moving_average ));
}
if ( boiler_status == BOILER_ON  && moving_average  < 50  )
{
    unsigned int  interval;
    char output[70];
    interval = (time_now - boiler_switched_on_time)/1000;
    tell_influx(BOILER_OFF,interval);
    boiler_status = BOILER_OFF;
    sprintf(output,"boiler was on for %d",interval);
    Serial.println(output);
}

interrupts();


server.handleClient(); 

}      

void handleRoot() {

  String document;
  document = document + "<p>Current level " + String(events,DEC) + "</p>";
  document = document + history->list();
  server.send(200, "text/html", document);   // Send HTTP status 200 (Ok) and send some text to the browser/client
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}