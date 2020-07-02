/*
 *  MESSAGE ENCODING (to BASE64)
 */
char * encodeMsg(char *decString, int msgLength) {
  Serial.print("Input string is: ");
  Serial.println(decString);
  Serial.print("Size is: ");
  Serial.println(msgLength);

  int encodedLength = Base64.encodedLength(msgLength);
  char * encString = new char[encodedLength+1];
  Base64.encode(encString, decString, msgLength);
  
  Serial.print("Encoded string is: ");
  Serial.println(encString);

  return encString;
}


/*
 *  MESSAGE DECODING (from BASE64)
 */
char * decodeMsg(char *encString, int msgLength) {
  Serial.print("Input string is: ");
  Serial.println(encString);
  
  int decodedLength = Base64.decodedLength(encString, msgLength);
  char * decString = new char[decodedLength+1];
  Base64.decode(decString, encString, msgLength);
 
  Serial.print("Decoded string is: ");
  Serial.println(decString);
  Serial.print("Size is: ");
  Serial.println(decodedLength);
  
  return decString;
}
