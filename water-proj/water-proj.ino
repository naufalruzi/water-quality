#include <OneWire.h>
#include <DallasTemperature.h>

// GPIO where the DS18B20 is connected to
const int oneWireBus = 4;     

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

const int PHPin=4;
float ph;
float Value=0;

int turbidityPin = 34;
 
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  sensors.begin();
  pinMode(PHPin,INPUT);
  delay(1000);
}
 void loop(){
 
 //ph sensor
    Value= analogRead(PHPin);
    Serial.print(Value);
    Serial.print(" | ");
    float voltage=Value*(3.3/4095.0);
    ph=(3.3*voltage);
    Serial.println(ph);
    delay(500);

    //temp sensor
    sensors.requestTemperatures(); 
    float temperatureC = sensors.getTempCByIndex(0);
    float temperatureF = sensors.getTempFByIndex(0);
    Serial.print(temperatureC);
    Serial.println("ºC");
    Serial.print(temperatureF);
    Serial.println("ºF");
    delay(5000);

    //turbidity sensor
    int turbiValue = analogRead(turbidityPin);
    
    int turbidity = map(turbiValue, 0, 750, 100, 0);
    Serial.println(turbidity);
    // lcd.setCursor(0, 0);
    // lcd.print("Turbidity:");
    // lcd.print("   ");
    // lcd.setCursor(10, 0);
    //lcd.print(turbidity);
    
    delay(1000);
    if (turbidity < 20) {
      // lcd.setCursor(0, 1);
      // lcd.print(" its CLEAR ");
      Serial.print(" its CLEAR ");
    }
    if ((turbidity > 20) && (turbidity < 50)) {
      // lcd.setCursor(0, 1);
      // lcd.print(" its CLOUDY ");
      Serial.print(" its CLOUDY ");
    }
    if (turbidity > 50) {
      // lcd.setCursor(0, 1);
      // lcd.print(" its DIRTY ");
      Serial.print(" its DIRTY ");
    }
 }