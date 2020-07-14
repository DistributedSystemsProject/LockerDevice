/*
 *  MESSAGE ENCODING (to BASE64)
 */
char * encodeMsg(char * decString, int msgSize) {
  int encodedLength = Base64.encodedLength(msgSize);
  char * encString = new char[encodedLength+1];
  Base64.encode(encString, decString, msgSize);

  return encString;
}


/*
 *  MESSAGE DECODING (from BASE64)
 */
char * decodeMsg(char * encString, int msgSize) {  
  int decodedLength = Base64.decodedLength(encString, msgSize);
  char * decString = new char[decodedLength+1];
  Base64.decode(decString, encString, msgSize);
    
  return decString;
}
