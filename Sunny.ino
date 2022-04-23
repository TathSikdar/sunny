#include <ESP8266WiFi.h>
#include <aREST.h>
#include <FastLED.h>

#define LED_PIN              15
#define NUM_LEDS             144
#define LED_TYPE             WS2813
#define COLOR_ORDER          GRB
#define UPDATES_PER_SECOND   0.12  //0.12 -> 10 mins
#define DISMISS_LOAD_TIME    4*60000
#define BOOTUP_SUCCESS_LIGHT_TIME 4320
CRGBArray<NUM_LEDS> leds; 

IPAddress ip(192, 168, 1, 145);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
const char* ssid = "Sikdar";
const char* password = "suvrajit";
int lightmode = 0; //0-off | 1-on | 2-presunrise | 3-sunrise | 4-snooze | 5 dismiss |
//Colour definitions
const CHSV DISMISS_COLORS = CHSV(40, 90, 150);
const CHSV OFF_COLORS = CHSV(0, 0, 0);

// aREST 
aREST rest = aREST();
WiFiServer server(80);
WiFiClient client;
int light_h = 50;
int light_s = 100;
int light_v = 100;
int previous_light_h = light_h;
int previous_light_s = light_s;
int previous_light_v = light_v;
const int increment_steps = 20;

void wificlienthandle();
void wificonnect();
void wifireconnect();
void light_off();
void light_on();
void sunrise();
void snooze();
void dismiss(long mils);

int weblightoff(String command);
int weblighton(String command);
int websunrise(String command);
int websnooze(String command);
int webdismiss(String command);
int webhue(String command);
int websaturation(String command);
int webvalue(String command);

//////////////////////////////////SUNRISE VARS//////////////////////////////////
const int hue = 35;
const int min_saturation = 255;
const int max_saturation = 120;
const int min_brightness = 100;
const int max_brightness = 255;
const int bottom_led = 8;

int value_increment = (max_brightness - min_brightness)/(NUM_LEDS/2);
int saturation_increment = (min_saturation - max_saturation)/(NUM_LEDS/2);

int leds_on = 0;
int start_led = bottom_led;
int end_led = bottom_led;
int current_brightness = min_brightness;
int current_saturation = min_saturation;

//////////////////////////////////MAIN FUNCTIONS//////////////////////////////////
void setup() {
  delay(2000);
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalSMD5050 );
  light_off();
  Serial.begin(115200);
  Serial.println("Boot...");
  rest.function("off", weblightoff);
  rest.function("on", weblighton);
  rest.function("sunrise", websunrise);
  rest.function("snooze", websnooze);
  rest.function("dismiss", webdismiss);
  rest.function("hue", webhue);
  rest.function("saturation", websaturation);
  rest.function("value", webvalue);
  rest.set_id("Sunrise Alarm Clock");
  rest.set_name("esp8266");
  wificonnect();
  server.begin();
  dismiss(BOOTUP_SUCCESS_LIGHT_TIME);
}

void loop() {
  wifireconnect();
  wificlienthandle();
  
  if(lightmode == 2){
    presunrise(); 
  }
  else if(lightmode==3){
    sunrise();
  }
  else if(lightmode==4){
    snooze();
  }
  else if(lightmode==5){
    dismiss(DISMISS_LOAD_TIME);
  }
  else{
    delay(500);
  }
  
}

//////////////////////////////////WIFI FUNCTIONS//////////////////////////////////
void wificlienthandle(){
  client = server.available();
  rest.handle(client);  
}

void wificonnect(){
  int i = 0;

  WiFi.config(ip, gateway, subnet);
  delay(500);
  WiFi.disconnect(true);
  delay(1000);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
//    Serial.println(++i);
  } 
//  Serial.println("Connection established!");  
//  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void wifireconnect(){
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("Reconnecting...");
      while (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.println("Connection Failed! Rebooting...");
      delay(5000);
      ESP.restart();
    }
  }
}

//////////////////////////////////HARDWARE CONROL FUNCTINS//////////////////////////////////
void light_off(){
  FastLED.clear();
  Serial.println("Light off");
  FastLED.show();
}

void light_on(){
  Serial.println("Light on");
  int diff_h = (light_h - previous_light_h);
  int diff_s = (light_s - previous_light_s);
  int diff_v = (light_v - previous_light_v);
  
  int runfor =  max(abs(diff_h), max(abs(diff_s), abs(diff_v) ));

  for(int i=0; i<runfor; i++){
    
    if(previous_light_h != light_h){
      if(diff_h<0 ){
        previous_light_h--;
      }else{
        previous_light_h++;
      }
    }
  

    if(previous_light_s != light_s){
      if(diff_s<0 ){
        previous_light_s--;
      }else{
        previous_light_s++;
      }
     }
    
    
    if(previous_light_v != light_v){
      if(diff_v<0 ){
        previous_light_v--;
      }else{
        previous_light_v++;
      }
    }
    
    Serial.printf("%d %d %d\n", previous_light_h, previous_light_s, previous_light_v);
    fill_solid(leds, NUM_LEDS, CHSV(previous_light_h,previous_light_s,previous_light_v));
    FastLED.delay(10);
  }

  setprevious();
  
//  if(previous_light_h < light_h){
//    while(previous_light_h < light_h){
//      for(int i = 0; i< NUM_LEDS; i++){
//        leds[i] = CHSV(previous_light_h, light_s, light_v);
//        previous_light_h++;
//        FastLED.show();
//        delay(10);
//      }
//    }
//    previous_light_h = light_h;
//  }
//  else{
//    for (int i = 0; i < NUM_LEDS; i++){
//      leds[i] = CHSV(light_h, light_s, light_v);
//    }
//  }
//  FastLED.show();
}

void presunrise(){
  Serial.println("Pre Sunrise");
  
  leds_on = 0;
  start_led = bottom_led;
  end_led = bottom_led;
  current_brightness = min_brightness;
  current_saturation = min_saturation;
  
  FastLED.clear();
  leds[bottom_led] = CHSV(hue, current_saturation, current_brightness);
  FastLED.show();
  FastLED.delay(1000 / UPDATES_PER_SECOND);

  lightmode = 3; //sunrise
}

void sunrise(){
  
  if (leds_on < NUM_LEDS){
    
    start_led--;
    end_led++;
    current_saturation -= saturation_increment;
    current_brightness += value_increment;
    for (int i = start_led; i < end_led; i++){
      if( i < 0){
        leds[NUM_LEDS + i] = CHSV(hue, current_saturation, current_brightness);
      }
      else{
        leds[i] = CHSV(hue, current_saturation, current_brightness);
      }
    }
    leds_on+=2;
    FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);
  }else{
    lightmode = 0; //off 
  }
}

void snooze(){
  setprevious();
  light_h = 0;
  light_s = 50;
  light_v = 255;
  light_on();
  lightmode = 1;
}

void dismiss(long mils){
  int led_loading = bottom_led;
  long starting_time = millis();
  FastLED.clear();
  for (int i = led_loading; i< led_loading + 1; i++){
    leds[i] = DISMISS_COLORS;
  }
  while (millis() - starting_time < mils){
    FastLED.show();
    delay(5);
    leds[led_loading] = OFF_COLORS;
    led_loading = (++led_loading)%NUM_LEDS;
    leds[(led_loading+39)%NUM_LEDS] = DISMISS_COLORS;
  }
  lightmode = 0;
  light_off();
}

void setprevious(){ 
  int previous_light_h = light_h;
  int previous_light_s = light_s;
  int previous_light_v = light_v;
}
//////////////////////////////////WEB FUNCTIONS//////////////////////////////////

int weblightoff(String command){
  lightmode = 0;
  Serial.println("web off");
  setprevious();
  light_h = 0;
  light_s = 0;
  light_v = 0;
  light_on();
  return 1;
}
int weblighton(String command){
  lightmode = 1;
  Serial.println("web on");
  setprevious();
  
  light_h = 40;
  light_s = 90;
  light_v = 150;
  
  light_on();
  return 1;
}
int websunrise(String command){
  lightmode = 2; //presunrise
  Serial.println("web sunrise");
  return 1;
}
int websnooze(String command){
  lightmode = 4;
  Serial.println("web snooze");
  return 1;
}
int webdismiss(String command){
  lightmode = 5;
  Serial.println("web dismiss");
  return 1;
}
int webhue(String command){
  setprevious();
  light_h = command.toInt();
  Serial.printf("Hue: %d", light_h);
  if(lightmode == 1){
    light_on();
  }
  return 1;
}
int websaturation(String command){
  setprevious();
  light_s = command.toInt();
  Serial.printf("Saturation: %d", light_s);
  if(lightmode == 1){
    light_on();
  }
  return 1;
}
int webvalue(String command){
  setprevious();
  light_v = command.toInt();
  Serial.printf("Value: %d", light_v);
  if(lightmode == 1){
    light_on();
  }
  return 1;
}
