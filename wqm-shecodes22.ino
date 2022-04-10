#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include "DFRobot_ESP_EC.h"
#include "DFRobot_ESP_PH_WITH_ADC.h"
#include "OneWire.h"
#include "DallasTemperature.h"
#include "EEPROM.h"
#include <WiFiManager.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include <addons/TokenHelper.h>

//Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>


//For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
#define API_KEY "AIzaSyAoAAgwIq60E43uuPzrsiYTnTotg-nRZyY"

/* 3. Define the RTDB URL */
#define DATABASE_URL "wqmtest-f7ebb-default-rtdb.firebaseio.com" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "test@test.com"
#define USER_PASSWORD "test12345"

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

unsigned long count = 0;

//////////////////////////////////

const long utcOffsetInSeconds = 19800;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

// Variables to save date and time


unsigned long starttime=0, nowtime=0;
const unsigned long period = 2000;


DFRobot_ESP_EC ec;
#define ONE_WIRE_BUS 18
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DFRobot_ESP_PH_WITH_ADC ph;
Adafruit_ADS1115 ads;

#define VREF 5.0    
#define SCOUNT  30 

float tdsAvg = 0.0;
float phAvg = 0.0;
float tempAvg = 0.0;

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;

// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

void setup(){
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    // it is a good practice to make sure your code sets wifi mode how you want it.

    // put your setup code here, to run once:
    Serial.begin(115200);
    
    //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wm;

    // reset settings - wipe stored credentials for testing
    // these are stored by the esp library
    //wm.resetSettings();

    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
    // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
    // then goes into a blocking loop awaiting configuration and will return success result

    bool res;
    // res = wm.autoConnect(); // auto generated AP name from chipid
     res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    //res = wm.autoConnect("AutoConnectAP","password"); // password protected ap

    if(!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
    }
  // initialize LCD
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();

  
  pinMode(13,OUTPUT);
  pinMode(14,OUTPUT);
  digitalWrite(13,LOW);
  digitalWrite(14, LOW);

  ph.begin();
  sensors.begin();
  Serial.println("Getting single-ended readings from AIN0..3");
  Serial.println("ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)"); 
  ads.setGain(GAIN_TWOTHIRDS); 
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }

  timeClient.begin();
  starttime = millis();
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  //Or use legacy authenticate method
  //config.database_url = DATABASE_URL;
  //config.signer.tokens.legacy_token = "<database secret>";

  //To connect without auth in Test Mode, see Authentications/TestMode/TestMode.ino

  //////////////////////////////////////////////////////////////////////////////////////////////
  //Please make sure the device free Heap is not lower than 80 k for ESP32 and 10 k for ESP8266,
  //otherwise the SSL connection will fail.
  //////////////////////////////////////////////////////////////////////////////////////////////

  Firebase.begin(&config, &auth);

  //Comment or pass false value when WiFi reconnection will control by your code or third party library
  Firebase.reconnectWiFi(true);

  Firebase.setDoubleDigits(5);

  /** Timeout options.

  //WiFi reconnect timeout (interval) in ms (10 sec - 5 min) when WiFi disconnected.
  config.timeout.wifiReconnect = 10 * 1000;

  //Socket connection and SSL handshake timeout in ms (1 sec - 1 min).
  config.timeout.socketConnection = 10 * 1000;

  //Server response read timeout in ms (1 sec - 1 min).
  config.timeout.serverResponse = 10 * 1000;

  //RTDB Stream keep-alive timeout in ms (20 sec - 2 min) when no server's keep-alive event data received.
  config.timeout.rtdbKeepAlive = 45 * 1000;

  //RTDB Stream reconnect timeout (interval) in ms (1 sec - 1 min) when RTDB Stream closed and want to resume.
  config.timeout.rtdbStreamReconnect = 1 * 1000;

  //RTDB Stream error notification timeout (interval) in ms (3 sec - 30 sec). It determines how often the readStream
  //will return false (error) when it called repeatedly in loop.
  config.timeout.rtdbStreamError = 3 * 1000;

  Note:
  The function that starting the new TCP session i.e. first time server connection or previous session was closed, the function won't exit until the 
  time of config.timeout.socketConnection.

  You can also set the TCP data sending retry with
  config.tcp_data_sending_retry = 1;

  */
}

void loop(){
  nowtime = millis();
  lcd.clear();
  // set cursor to first column, first row
  lcd.setCursor(0, 0);
  // print message
  if(Firebase.ready() && nowtime - starttime > period){
      lcd.print("Processing Data");
      processBegin();
      delay(1000);
      timeClient.update();
      unsigned long epochTime = timeClient.getEpochTime();
      Serial.printf("Set tds... %s\n", Firebase.RTDB.pushFloat(&fbdo, F("/wqm01/tds"), tdsAvg) ? "ok" : fbdo.errorReason().c_str());
      Serial.printf("Set ph... %s\n", Firebase.RTDB.pushFloat(&fbdo, F("/wqm01/ph"), phAvg) ? "ok" : fbdo.errorReason().c_str());
      Serial.printf("Set temp... %s\n", Firebase.RTDB.pushFloat(&fbdo, F("/wqm01/temp"), tempAvg) ? "ok" : fbdo.errorReason().c_str());
//      Serial.printf("Set timestamp... %s\n", Firebase.RTDB.setString(&fbdo, F("/test/string"), F(")) ? "ok" : fbdo.errorReason().c_str());
      Serial.printf("Set timestamp... %s\n", Firebase.RTDB.pushFloat(&fbdo, F("/wqm01/timestamp"), epochTime) ? "ok" : fbdo.errorReason().c_str());
      timeClient.update();
      lcd.clear();
      starttime = nowtime; 
      FirebaseJson json; 

  }
  
  lcd.setCursor(0, 0);
  // print message
  lcd.print("TDS");
  lcd.setCursor(0, 1);
  // print message
  lcd.print(int(tdsAvg));

  lcd.setCursor(10, 0);
  // print message
  lcd.print("|");
  lcd.setCursor(10, 1);
  // print message
  lcd.print("|");
  
  lcd.setCursor(6, 0);
  // print message
  lcd.print("pH");
  lcd.setCursor(6, 1);
  // print message
  lcd.print(int(phAvg));
  
  lcd.setCursor(5, 0);
  // print message
  lcd.print("|");
  lcd.setCursor(5, 1);
  // print message
  lcd.print("|");
  
  lcd.setCursor(11, 0);
  // print message
  lcd.print("Temp");
  lcd.setCursor(11, 1);
  // print message
  lcd.print(int(tempAvg));
  delay(10000);
}

int getMedianNum(int bArray[], int iFilterLen) 
{
      int bTab[iFilterLen];
      for (byte i = 0; i<iFilterLen; i++)
      bTab[i] = bArray[i];
      int i, j, bTemp;
      for (j = 0; j < iFilterLen - 1; j++) 
      {
      for (i = 0; i < iFilterLen - j - 1; i++) 
          {
        if (bTab[i] > bTab[i + 1]) 
            {
        bTemp = bTab[i];
            bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
         }
      }
      }
      if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
      else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
      return bTemp;
}
void processBegin(){
  digitalWrite(13, HIGH);
  digitalWrite(14, HIGH);
  tempAvg = tempCal();
  delay(1000);
  tdsAvg = tdsCal(tempAvg);
  digitalWrite(13, LOW);
  digitalWrite(14, LOW);
  // mqtt.loop();
  delay(10000);
  //mqtt.loop();
  digitalWrite(13, LOW);
  digitalWrite(14, LOW);
  
  phAvg = phCal(tempAvg);
  digitalWrite(13, LOW);
  digitalWrite(14, LOW);
}

float tempCal(){
  sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);
  return temperatureC;
}
float tdsCal(float temperature){
  
  int analogBuffer[SCOUNT];   
  int analogBufferTemp[SCOUNT];
  int analogBufferIndex = 0,copyIndex = 0;
  
  float averageVoltage = 0,tdsValue = 0;
  int16_t adc;
  int Exit = 0;
  
  while(Exit != 1){
    Exit = 0;
    adc = ads.readADC_SingleEnded(1);
    analogBuffer[analogBufferIndex] = adc;    //read the analog value and store into the buffer
    analogBufferIndex++;
    delay(40);
    if(analogBufferIndex == SCOUNT){
      analogBufferIndex = 0;
      Exit = 1;
    }
  }
  for(copyIndex=0;copyIndex<SCOUNT;copyIndex++){
    analogBufferTemp[copyIndex]= analogBuffer[copyIndex];
  }
  averageVoltage = (getMedianNum(analogBufferTemp,SCOUNT) * 0.1875) / 1000.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
  float compensationCoefficient=1.0+0.02*(temperature-25.0);    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
  float compensationVolatge=averageVoltage/compensationCoefficient;  //temperature compensation
  tdsValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; //convert voltage value to tds value
  return tdsValue; 
}

float phCal(float temperature){
  int16_t adc;
  float phValue, voltage;
  for(int i=0;i<20;i++){
    voltage = ads.readADC_SingleEnded(2) / 10; // read the voltage
    phValue = ph.readPH(voltage, temperature); // convert voltage to pH with temperature compensation
    // phValue -=1.2218;
    ph.calibration(voltage, temperature);
  }
  return phValue;
}
String getTimestamp(){
  
  //Week Days
  String weekDays[7]={"Sun.", "Mon.", "Tue.", "Wed.", "Thu.", "Fri.", "Sat."};
  //Month names
  String months[12]={"Jan.", "Feb.", "Mar.", "Apr.", "May", "Jun.", "Jul.", "Aug.", "Sep.", "Oct.", "Nov.", "Dec."};

  String formattedTime = timeClient.getFormattedTime();
  
  unsigned long epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime); 
  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon+1;
  int currentYear = ptm->tm_year+1900;
  String currentMonthName = months[currentMonth-1];
  String weekDay = weekDays[timeClient.getDay()];
  // String currentDate = weekDay + " " + currentMonthName + " " + String(monthDay) + " " + String(currentYear);
  String currentDate = String(monthDay) + "/" + String(currentMonth)+ "/" + String(currentYear);
  String timestamp =currentDate  + ", " + formattedTime;
  return timestamp;
}
