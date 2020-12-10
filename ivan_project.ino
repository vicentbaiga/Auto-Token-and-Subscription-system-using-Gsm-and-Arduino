#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>


#define gsmRX 7
#define gsmTx 8
#define gsmSwitch 6
#define appliances 13
#define pot A1
int sent = 0;
int sent2 = 0;
float upperLimitUnits = 100.00;

//String telNumber = "+256778266649";
String telNumber = "+256703606084";
int yakaUnits = 0;

SoftwareSerial modem(gsmRX, gsmTx);
LiquidCrystal_I2C lcd(0x27, 16, 2);

float upperLimi = 100.00;
float yakaLoadedNow = 100.00;

void setup() {
  Serial.begin(9600);
  modem.begin(9600);
  lcd.begin();

  pinMode(pot, INPUT);

  pinMode(gsmSwitch, OUTPUT);
  pinMode(appliances,OUTPUT);
  powerOnGsm();

  lcd.clear();
  lcd.print(F("Time Synched "));

  lcd.clear();
  lcd.print(F("Cleared Inbox "));
  setTotextMode();

  lcd.clear();
  lcd.print(F("Set to tex Mode"));


}


void loop() {

  // display number of units.
  // reduce them as pot is varied
  //  chat_gsm();

  float bal = getUnits(yakaLoadedNow, upperLimi);
  Serial.println(bal);

  dipslayUnits(bal);

  // when done switch led/ relay off
  controlSwitch(bal);

  // when less than 0 units, send a reminder

  if ((bal <= 5.00) && (sent == 0) ) {
    SendMessage(String("Your balance is " + String(bal) + String(" KWH")), telNumber);
   
    Serial.println("Sent Message");
     sent = 1;
  }

  while (bal == 0.00) {
 
    Serial.println("Waiting for units");
    if (sent2 == 0) {
      SendMessage(String("Your balance is " + String(bal) + String(" KWH")), telNumber);
      Serial.println("Sent Message");
      sent2 = 1;
    }
    lcd.clear();
    delay(100);
    lcd.print("Load Units");

    while (!modem.available()) {

    }

    float Loaded =  listen4Units();

    Serial.print("New Units = "); Serial.println(Loaded);
    if ( Loaded > 0.00) {
      Serial.println("adjust Potentiometer");

      while ( analogRead(pot) > 20 ) {
        lcd.clear();
        lcd.print("Reset");
        delay(500);
      }
      bal = Loaded;
      yakaLoadedNow = Loaded;
      upperLimi = Loaded;
      sent2 = 0;
      sent = 0;
    }
  }

  // update display with number of units and switch back relay.

}

void dipslayUnits(float units) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("UNITS  ");
  lcd.print(units);
  lcd.print(" KWH");
}

float getUnits(float xc, float upperLimit ) {

  long int sensorValue = 0;
  int samplesNum = 101;
  float yaka = xc ;

  for (int xz = 0; xz < samplesNum; xz++) {
    sensorValue = sensorValue + (analogRead(pot));
    //    Serial.println(xz);
    delay(10);
  }

  //  Serial.print("Final Sensor Value :="); Serial.println(sensorValue);
  int sensorValueAverage = sensorValue / (samplesNum - 1 );

  if (sensorValueAverage < 0) sensorValueAverage = (sensorValueAverage * -1);
  //  Serial.print("Average = "); Serial.println(sensorValueAverage);

  float yakaBalance = yaka + (-1 * mapf(sensorValueAverage, 0, 910, 0, upperLimit));
  if (yakaBalance < 0) {
    yakaBalance = 0.00;
  }
  //  Serial.print ("Yaka balance ="); Serial.println(yakaBalance);
  return yakaBalance;
}

float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void controlSwitch(float x1x) {
  if (x1x < 0.05) {
    digitalWrite(appliances, HIGH);
    Serial.println(F("Applicances OFF"));
  } else {
    digitalWrite(appliances, LOW);
    Serial.println(F("Applicances ON"));
  }
}
