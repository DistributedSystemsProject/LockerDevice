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
  reqAccess();
  connected = true;
  resetCount();
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
    btSerial.flush();
  }
  connected = false;
  countdown = 0;
}


/*
 *  READ FROM BLUETOOTH
 */
void readBT() {
  if(btSerial.available()) {
    char input[237];
    int s = 0;
    
    while(btSerial.available() && s<255) {
      char c = btSerial.read();
      if(c != ' ' && c != '\n' && c != '\r' && c != '\0') {
        input[s] = c;
        s++;
      }
    }
    
    if(s > 15 && s < 237) {
      s = 172;
      memcpy(input, "AWnMlXdVlVGi26w5TPD2BquppMIdRQkbJjVv+aa5t/xadEHxeiDaMRO6p0tXcoEeAly87TYEnAp8t55IhHlmz4+HOLN9HhqQ71ARZ2x6XG92Wd8NY6oP5qC9J/s+E5qAWjcXMgOF1GxF3ID++KtqzYpBIR2ZRNwRh5ELk86oRy4=", 172);
      input[s] = '\0';
      int block = fromClient(input, s);
      
      if(block > 0) checkAccess(input, block);
    }
  }
  
  waitCount();
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
