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
  else if(connected && countdown < 1) disconnectBT();
  return connected;
}


/* 
 *  CONNECTION TO CLIENT
 */
void connectBT() {
  Serial.println("----- CONNECTED -----");
  connected = true;
  countdown = 600;
}


/* 
 *  DISCONNECTION FROM CLIENT
 */
void disconnectBT() {
  Serial.println("----- DISCONNECTED -----");
  Serial.println();
  connected = false;

  if(countdown < 1) {
    digitalWrite(BT_PWR, LOW);
    delay(2000);
    digitalWrite(BT_PWR, HIGH);
    delay(2000);
    btSerial.flush();
  }
  countdown = 0;
}
