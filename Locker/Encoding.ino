/*
 *  MESSAGE ENCODING (to BASE64)
 */
char * encodeMsg(char *decString, int msgSize) {
  Serial.print("Input string is: ");
  Serial.println(decString);
  Serial.print("Size is: ");
  Serial.println(msgSize);

  int encodedLength = Base64.encodedLength(msgSize);
  char * encString = new char[encodedLength+1];
  Base64.encode(encString, decString, msgSize);
  
  Serial.print(F("Encoded string is: "));
  Serial.println(encString);

  return encString;
}


/*
 *  MESSAGE DECODING (from BASE64)
 */
char * decodeMsg(char *encString, int msgSize) {
  Serial.print("Input string is: ");
  Serial.println(encString);
  
  int decodedLength = Base64.decodedLength(encString, msgSize);
  char * decString = new char[decodedLength+1];
  Base64.decode(decString, encString, msgSize);
 
  Serial.print(F("Decoded string is: "));
  Serial.println(decString);
  Serial.print(F("Size is: "));
  Serial.println(decodedLength);
  
  return decString;
}
