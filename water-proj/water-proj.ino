#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <ESP32httpUpdate.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#define TdsSensorPin 27
#define VREF 3.3              // analog reference voltage(Volt) of the ADC
#define SCOUNT  30            // sum of sample point

String ssid = "UUMWiFi_Guest", pass = "";
WiFiClient wifiClient;

// GPIO where the DS18B20 is connected to
const int oneWireBus = 2;     

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

const int PHPin=4;
float ph;
float Value=0;

int turbidityPin = 34;

int analogBuffer[SCOUNT];     // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;
float temperatureC = 0;
int turbidity = 0;

float averageVoltage = 0;
float tdsValue = 0;
float temperature = 25;       // current temperature for compensation

int getMedianNum(int bArray[], int iFilterLen){//tds fun function generator
  int bTab[iFilterLen];
  for (byte i = 0; i<iFilterLen; i++)
  bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0){
    bTemp = bTab[(iFilterLen - 1) / 2];
  }
  else {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  return bTemp;
}

void uploadData() {
  String request = "http://naufal3003.000webhostapp.com/insertbpm.php?bpm="  + String(ph) + String(temperatureC) + String(turbidity) + String(tdsValue);
  Serial.println(request);
  
  HTTPClient http;
  http.begin(wifiClient, request);
  int httpCode = http.GET();
  if (httpCode == 200) {
    String responseBody = http.getString();
    Serial.println(responseBody);

  } else {
    Serial.println("Error: " + String(httpCode));
  }
  http.end();
      
}
 
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  sensors.begin();
  pinMode(PHPin,INPUT);
  pinMode(TdsSensorPin,INPUT);
  delay(1000);

  //wifi
  WiFi.begin(ssid, pass);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}
 void loop(){
 
 //ph sensor
    Value= analogRead(PHPin);
    Serial.print(Value);
    Serial.print(" | ");
    float voltage=Value*(3.3/4095.0);
    ph=(3.3*voltage);
    Serial.println(ph);
    uploadData(ph);
    delay(1000);

    //temp sensor
    sensors.requestTemperatures(); 
    float temperatureC = sensors.getTempCByIndex(0);
    float temperatureF = sensors.getTempFByIndex(0);
    Serial.print(temperatureC);
    Serial.print("ºC ");
    uploadData(temperatureC);
    Serial.print(temperatureF);
    Serial.println("ºF");
    delay(1000);

    //turbidity sensor
    int turbiValue = analogRead(turbidityPin);
    
    int turbidity = map(turbiValue, 0, 750, 100, 0);
    Serial.print(turbidity);
    uploadData(turbidity);
    // lcd.setCursor(0, 0);
    // lcd.print("Turbidity:");
    // lcd.print("   ");
    // lcd.setCursor(10, 0);
    //lcd.print(turbidity);
    
    delay(1000);
    if ((turbidity > 0) && (turbidity < 50)) {
      // lcd.setCursor(0, 1);
      // lcd.print(" its CLEAR ");
      Serial.println(" its CLEAR ");
    }
    if ((turbidity > 20) && (turbidity < 50)) {
      // lcd.setCursor(0, 1);
      // lcd.print(" its CLOUDY ");
      Serial.println(" its CLOUDY ");
    }
    if (turbidity > 50) {
      // lcd.setCursor(0, 1);
      // lcd.print(" its DIRTY ");
      Serial.println(" its DIRTY ");
    }

    //tds sensor
  for (i = 0; i < 10; i++) {
    static unsigned long analogSampleTimepoint = millis();
  if(millis()-analogSampleTimepoint > 40U){     //every 40 milliseconds,read the analog value from the ADC
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
    analogBufferIndex++;
    if(analogBufferIndex == SCOUNT){ 
      analogBufferIndex = 0;
    }
  }   
  
  static unsigned long printTimepoint = millis();
  if(millis()-printTimepoint > 800U){
    printTimepoint = millis();
    for(copyIndex=0; copyIndex<SCOUNT; copyIndex++){
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
      
      // read the analog value more stable by the median filtering algorithm, and convert to voltage value
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 4096.0;
      
      //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0)); 
      float compensationCoefficient = 1.0+0.02*(temperature-25.0);
      //temperature compensation
      float compensationVoltage=averageVoltage/compensationCoefficient;
      
      //convert voltage value to tds value
      tdsValue=(133.42*compensationVoltage*compensationVoltage*compensationVoltage - 255.86*compensationVoltage*compensationVoltage + 857.39*compensationVoltage)*0.5;
      
      //Serial.print("voltage:");
      //Serial.print(averageVoltage,2);
      //Serial.print("V   ");
      
      }
      Serial.print("TDS Value:");
      Serial.print(tdsValue,0);
      Serial.println("ppm");
      uploadData(tdsValue);
    }
  }


 }