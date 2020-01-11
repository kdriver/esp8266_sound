#include <Arduino.h>
#include <WiFiCreds.h>
#include <WiFi.h>
#include <Wire.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include "History.h"
#include "UDPLogger.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <driver/adc.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


#define BOILER_OFF 0
#define BOILER_ON 1
#define BOILER_NO_CHANGE 2
#define BOILER_STARTING 3
#define ON_THRESHOLD 900
#define OFF_THRESHOLD 300
#define WATCHDOG_INTERVAL 300

unsigned long epoch;
unsigned long time_now;
unsigned long previous_time;
unsigned long since_epoch;

unsigned int boiler;
unsigned int boiler_status= BOILER_OFF;
unsigned int boiler_switched_on_time=0;

unsigned long last_time=millis();
unsigned long on_for=0;
unsigned int threshold = 1300;
unsigned int boiler_on = 120;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

String address = "0.0.0.0";

bool DEBUG_ON=false;
bool quiet = false;

WebServer server(80);
void handleRoot();              // function prototypes for HTTP handlers
void handleTest();
void handleNotFound();


UDPLogger loggit("192.168.1.161",8787);


int post_it(String payload )
{
    HTTPClient http;
    int response;
    //Serial.println("post to influx\n");
    http.begin("http://192.168.1.161:8086/write?db=boiler_sound");\
    http.addHeader("Content-Type","text/plain");
    response = http.POST(payload);

    if ( response > 250 )
      loggit.send("InfluxDb POST error response " + String(response) + "\n");

    http.end();
    return response;
}
void tell_influx(unsigned int status, unsigned int time_interval)
{
  String payload;
  //int response;

  payload = "boiler_status interval=" + String(time_interval);
  payload = payload + ",interval_mins=" + String(time_interval/60);
  payload = payload + ",interval_secs=" + String(time_interval%60);
  payload = payload + ",value=" + String(status);
  post_it(payload);
  //Serial.print(response);
  
}

void tick_influx(String text,unsigned int value)
{
    String payload;
    //int response;
    payload = "boiler_iot,lifecycle=" + text + " value=" + String(value);
    post_it(payload);
    //Serial.print(response); 
 
}

void IRAM_ATTR sound();
void IRAM_ATTR measure();

History *history = new History(30);

hw_timer_t *timer=NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void p_lcd(String s,unsigned int x,unsigned int y)
{
  display.setTextSize(1); 
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(x, y);
  display.println(s);
  display.display();
}
void setup() {
char prog[] = {'|','/','-','\\'};
int rot=0;
String c;

Serial.begin(9600);

if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

Serial.println("I'm alive");
display.clearDisplay();
p_lcd("Searching for WiFi",0,0);
 
  WiFi.begin(ssid, pass);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
    c=prog[rot];
    rot = rot + 1 ;
    if ( rot == sizeof(prog))
      rot = 0;
    display.writeFillRect(0,8,5,8,SSD1306_BLACK);
    p_lcd(c,0,8);
  }
  
  
  p_lcd("i'm alive",0,0);   

  address = WiFi.localIP().toString();
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(address);

  server.on("/", handleRoot);               // Call the 'handleRoot' function when a client requests URI "/"
  server.on("/test", handleTest);               // Call the 'handleRoot' function when a client requests URI "/"
  server.onNotFound(handleNotFound);        // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"
  server.begin();  

  loggit.init();
  loggit.send("up and running\n");
  // sgart a timer to count the number of events each second.
/* 8266
  timer1_attachInterrupt(measure);
  timer1_enable(TIM_DIV256, TIM_EDGE, TIM_LOOP); // 
  timer1_write(312000*2); // 1 second

  pinMode(D2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(D2),sound,FALLING);

*/
/*
  timer = timerBegin(0,80,true);
  timerAttachInterrupt(timer, &measure, true);
  timerAlarmWrite(timer, 1000000, true);
  timerAlarmEnable(timer);
*/
  
  p_lcd("start influx",0,0);

  tick_influx(String("Initialised_1_0_"+ address),0);

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  epoch = millis();
  previous_time = epoch;
//  Just set the threshold to the first reading. We'll average it out later.
  threshold = analogRead(A0);

  p_lcd("end of setup",0,0);
  Serial.println("setup is finnished\n");

  display.clearDisplay();
}

unsigned int events = 0;
//int last_second_events =0;
float moving_average;

unsigned int choose_scale(unsigned int max)
{
  unsigned int scale=500;

  scale = max * 1.5;

  if  ( scale < 100 )
    scale = 100;
       return scale;
}
void drawHistory()
{
    unsigned int h[51];
    unsigned int events;
    unsigned int max=0;
    unsigned int scale=0;
    String s;

    
    events = history->getHistory(40,(unsigned int *)&h,&max);
    
    scale = choose_scale(max);
    s = "values :(" + String(events) + ") :";
    display.writeFillRect(127-40,0,40,31,SSD1306_BLACK);
    // 32 pixels high
    for ( int i =0 ; i < events ; i++ )
    {
      int v;
      v = h[i]*32/scale;
      if ( v > 31 )
        v = 31;
       s = s + String(h[i]) + ',';
       display.drawFastVLine(127-i,31-v,v,SSD1306_WHITE);
    }
    /* now draw a short line indication of the 'on' threshold */
    unsigned int threshold;
    threshold = (ON_THRESHOLD*32)/scale;
    if ( threshold > 31 ) // Clamp threshold at top of screen . Will happen at low scale values
      threshold = 31;
    display.drawFastHLine(127-50,31-threshold,10,SSD1306_WHITE);
    loggit.send(s + String(" scale=") + String(scale) + '\n');
    display.display();
}


bool read_analogue(void)
{
    int the_diff,abs_value;
    unsigned int sensor_value;
    static unsigned int the_total=0,abs_total=0;
    static unsigned int num_samples=0,abs_samples=0,the_average=0,abs_average=0;
    static unsigned const int max_samples=250,abs_max_samples=250;
    bool detected_on = false;
   
    sensor_value = analogRead(A0);

    the_diff = sensor_value - threshold;
    abs_value = abs(the_diff);

    the_total = the_total + sensor_value;
    abs_total = abs_total + abs_value;

    if ( num_samples > max_samples )
        the_total = the_total - the_average;
    else
        num_samples = num_samples + 1;

    if ( abs_samples > abs_max_samples )
        abs_total = abs_total - abs_average;
    else
        abs_samples = abs_samples + 1 ;

    the_average = the_total/num_samples;
    // set the threshold to measure the variation to the long term average.
    // Basically the audio signal modulates the threshold value above and below the threshold. Threshold is essntially a bias figure
    //  Overall, the  modulated value should be above the threshold as much as below.
    threshold = the_average;
    // The abs average is a measure of how much the audio has mogulated the threshold value
    abs_average = abs_total / abs_samples;

    if ( abs_average > boiler_on )
      detected_on = true;
    else
      detected_on = false;

      Serial.print(boiler_on);
      Serial.print(" ");
      Serial.print(abs_average);
      Serial.print(" ");
      Serial.print(threshold);
      Serial.print(" ");
      
      if ( detected_on == true )
        Serial.println(50);
      else
        Serial.println(0);
      
    return  detected_on ;

}
void loop() {
bool boiler_on = false;

        delay(10);
        display.clearDisplay();
        String s;
        time_now = millis();
        boiler_on = read_analogue();
        
        if ( time_now - last_time  >  1000 )
        {
          p_lcd(address + " " + String(time_now/1000),0,8);
          if ( boiler_status == true )
          {
            p_lcd("BOILER ON ",0,0);
            p_lcd("ON for " + String((time_now - boiler_switched_on_time)/1000),0,16);
          }
          else
          {
            p_lcd("BOILER OFF",0,0);
            p_lcd("was ON for " + String(on_for),0,16);
          }
           p_lcd("threshold " + String(threshold),0,24);
          
          last_time = time_now;
          //adc1_config_width(ADC_WIDTH_BIT_10);
          //adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_DB_11);
          //int value = adc1_get_raw(ADC1_CHANNEL_0);
          //int value = analogRead(GPIO_NUM_36);
          //Serial.print("input pin is " + String(value )+ "\n" );
          //s="events " + String(history->last()) + "\n";
          //Serial.println(s);
          //loggit.send(s);
        }


        if ( (boiler_status == BOILER_OFF) && (boiler_on == true) )
        {
            String text;
            text = String("Boiler switched ON \n ") ;
            loggit.send(text);
            tell_influx(BOILER_ON,0);
            boiler_status = BOILER_ON;
            boiler_switched_on_time = time_now;
            //Serial.println("");
            //Serial.println(text);
          
        }
        if ( (boiler_status == BOILER_ON)  && ( boiler_on == false ) )
        {
            unsigned int  interval;
            char output[70];
            interval = (time_now - boiler_switched_on_time)/1000;
            on_for = interval;
            tell_influx(BOILER_OFF,interval);
            boiler_status = BOILER_OFF;
            sprintf(output,"boiler was on for %d seconds \n",interval);
            loggit.send(output);
            //Serial.println(output);
        }

        server.handleClient(); 

}      

void handleRoot() {

  String document;
  //Serial.println("webroot");
  document = document + "<p>Current level " + String(events,DEC) + "</p>";
  document = document + history->list(ON_THRESHOLD);
  server.send(200, "text/html", document);   // Send HTTP status 200 (Ok) and send some text to the browser/client
}

void handleTest() {
  String document;
  document = "<p>Test URL - sending UDP logger message</p>";
  loggit.send("Testing the logger\n");
  server.send(200,"text/html",document);
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}
