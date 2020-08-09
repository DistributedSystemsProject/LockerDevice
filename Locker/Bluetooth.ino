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
  Serial.println("Paired!");
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
int readBT(char * input, int size) {
  (btSerial.readStringUntil('\n')).toCharArray(input, size);
  return strlen(input);
}


/*
 *  WRITE TO BLUETOOTH
 */
void writeBT(char * output, int size) {
  char * enc = encodeMsg(output, size);
  btSerial.println(enc);
  delete enc;
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
