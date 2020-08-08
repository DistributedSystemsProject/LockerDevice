/*
 *  BLUETOOTH STATE CHANGES
 *  true = connected
 *  false = not connected
 */
boolean stateBT() {
  if(digitalRead(BT_ST) != connected) {
    if(digitalRead(BT_ST)) connectBT();
    else disconnectBT();
  }
  else if(connected && overCount()) disconnectBT();
  return connected;
}


/* 
 *  CONNECTION TO CLIENT
 */
void connectBT() {
  Serial.println("Pairing...");
  btSerial.println(FC(IDd));
  reqOp();
  resetCount();
  connected = true;
  Serial.println("Paired");
}


/* 
 *  DISCONNECTION FROM CLIENT
 */
void disconnectBT() {
  if(overCount()) {
    digitalWrite(BT_PWR, LOW);
    delay(2000);
    digitalWrite(BT_PWR, HIGH);
    delay(2000);
  }
  btSerial.flush();
  connected = false;
}


/*
 *  READ FROM BLUETOOTH
 *  true = successfull read
 *  false = parse or message error
 */
boolean readBT() {
  char input[200];
  (btSerial.readStringUntil('\n')).toCharArray(input, 200);
  int s = strlen(input);
  
  if(s>15 && s<201) {
    Serial.println("Reading...");
    delay(100);
    int block = fromClient(input, s);
    if (block > 0) {
      StaticJsonDocument<120> doc;
      DeserializationError error = deserializeJson(doc, input);
      uint8_t pubKeyEph[48];
      const char * ctr = doc["PK"];
      memcpy(pubKeyEph, decodeMsg((char *)ctr, 64), 48);
      ctr = doc["OP"];
    
      if(!newShared(pubKeyEph, privKeyDev)) return false;
    
      Serial.println("Operation DONE");
    
      return true;
    }
  }
  
  return false;
}


/*
 *  DECREMENT COUNTDOWN
 */
void waitCount() {
  delay(100);
  countdown -= 1;
}


/*
 *  CHECK IF COUNTDOWN IS OVER
 */
boolean overCount() {
  return (countdown < 1);
}


/*
 *  RESET COUNTDOWN
 */
void resetCount() {
  countdown = 600;
}
