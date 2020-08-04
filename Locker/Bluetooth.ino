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
  Serial.println("Reading...");
  char input[237];
  (btSerial.readStringUntil('\n')).toCharArray(input, 237);
  int s = strlen(input);
  
  if(s>15 && s<237) {
    delay(100);
    int block = fromClient(input, s);
    
    if(block > 0) return checkOp(input, block);
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
