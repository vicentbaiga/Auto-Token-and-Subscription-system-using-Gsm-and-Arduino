void SendATcommand(String ATCMD) {
  while (Serial.available()) {
    char x = Serial.read();
  }
  modem.println(ATCMD);
}


String getResponse_Multiplelines(String ATCMD , uint8_t x_max, unsigned long timeout) {


  String content = "";
  char y;
  uint8_t x = 0; // number of new lines
  bool seenEOL = false;
  bool seenEOL_max = false;


  SendATcommand(ATCMD); // Send AT command from here
  unsigned long startTime2 = millis();
  //  Serial.println(F("Command Sent"));

  while (!modem.available() && ( millis() - startTime2 < 2000UL ));

  unsigned long startTime = millis(); // timestarted to check loop

  //Serial.println(F("spooling response"));

  while (((millis() - startTime) < timeout)) {


    while (1) {
      while (modem.available()  ) {

        char c = modem.read();
        content = content + String (c);

        if ( c == '\r') {
          seenEOL = true;

        } else if ( (c == '\n') && seenEOL == true) {

          x = x + 1; // increase the number of newlines.

          if (x == x_max) {
            //          Serial.println("x @ max");
            seenEOL_max = true;
            break;
          }
        } else {
          seenEOL = false;
        }
      }
      if (seenEOL_max == true) {
        break;
      }
    }
  }

  //  Serial.println(content);
  delay(500);

  return content;
}

String midString(String str, String start, String finish) {
  int locStart = str.indexOf(start);
  if (locStart == -1) return "";
  locStart += start.length();
  int locFinish = str.indexOf(finish, locStart);
  if (locFinish == -1) return "";
  return str.substring(locStart, locFinish);
}


uint8_t networkRegistration(unsigned timeout) {

  unsigned long startTime = millis();

  while (millis() - startTime < timeout) {

    while ( getResponse_Multiplelines("AT\r\n", 2, 5000UL) == "AT\r\nOK\r\n");

    Serial.println(F("Modem detected"));

    int x = midString(getResponse_Multiplelines("AT+CSQ\r\n", 4, 5000UL), "+CSQ: ", ",").toInt();
    while ( x < 15) {
      x = midString(getResponse_Multiplelines("AT+CSQ\r\n", 4, 5000UL), "+CSQ: ", ",").toInt();
      delay(2000);
    }

    Serial.print(F("NetStrength "));
    Serial.println(x);

    while ( getResponse_Multiplelines("AT+CREG=1\r\n", 2, 5000UL) == "AT+CREG=1\r\nOK\r\n");

    Serial.println(F("Registered on Network "));
    delay(2000);

  }
}


void setTotextMode() {
  getResponse_Multiplelines("AT\r\n", 2, 2000UL);
  getResponse_Multiplelines("AT+CMGF=1\r\n", 2, 2000UL);
  getResponse_Multiplelines("AT+CNMI=1,2,0,0,0\r\n", 2, 2000UL);

}
void chat_gsm() {
  if (modem.available())  {
    Serial.write(modem.read());
  }
  if (Serial.available()) {

    while (Serial.available()) {

      modem.write(Serial.read());
    }
    modem.println();
  }
}

void powerOffGsm() {
  digitalWrite(gsmSwitch, LOW);

}
void powerOnGsm() {
  digitalWrite(gsmSwitch, HIGH);
  modem.begin(9600);

  delay(8000);
}

String getResponse() {
  String content = "";
  char y;

  //while (!modem.available());

  while (modem.available()) {

    char c = modem.read();
    content = content + String (c);

  }
  Serial.println(".");
  return content;
}
void deleteMessage() {
  getResponse_Multiplelines("AT\r\n", 2, 2000UL);
  getResponse_Multiplelines("AT+CMGD=?\r\n", 2, 2000UL);
  getResponse_Multiplelines("AT+CMGD=1\r\n", 2, 2000UL);
  getResponse_Multiplelines("AT+CMGD=2\r\n", 2, 2000UL);
  getResponse_Multiplelines("AT+CMGD=3\r\n", 2, 2000UL);

  Serial.println("Deleted all messages");

}


void SendMessage(String Message, String number) {
  Serial.print(F("Number TO:")); Serial.println(number);
  lcd.clear();
  lcd.print(F("Sending Alert"));
  getResponse_Multiplelines("AT\r\n" , 2, 2000);
  Serial.println("Sending Message");
  getResponse_Multiplelines("AT+CMGF=1\r\n" , 2, 2000);

  String newNum = String("AT+CMGS=\"" + number + "\"");

  modem.println(newNum);
  delay(2000);
  modem.println(Message);
  delay(3000);
  modem.write(26);

  modem.flush();
  Serial.flush();
}
float listen4Units() {

  String Received = getResponse();

  if (Received != "" ) Serial.println(Received);

  String Receivedx = midString(Received, "<units:\"", "\">");

  if (Receivedx != "") {
    Serial.println("New units loaded");
  }
  
  return Receivedx.toInt();
}
