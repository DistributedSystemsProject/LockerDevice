/*
 *  MESSAGE ENCRYPTION (with AES-128)
 */
void encrypt(char *plain, int msgLength) {
  aes128_cbc_enc(key, iv, plain, msgLength);
  
  Serial.print("Block size: ");
  Serial.println(msgLength);
  btSerial.print("Encrypted: ");
  Serial.print("Encrypted: ");
  btSerial.println(plain);
  Serial.println(plain);
}


/*
 *  MESSAGE DECRYPTION (with AES-128)
 */
void decrypt(char *cipher, int msgLength) {
  aes128_cbc_dec(key, iv, cipher, msgLength);
  cipher[msgLength] = '\0';
  
  Serial.print("Size: ");
  Serial.println(strlen(cipher));
  Serial.print("Decrypted: ");
  Serial.println(cipher);
}


/*
 *  RANDOM IV ARRAY GENERATION
 */
void randomIV() {
  for(int i=0; i<16; i++) {
    iv[i] = random(256);
    Serial.print(iv[i]);
  }

  Serial.println();
}


/*
 *  CBC ENCRYPTION BLOCK SIZE
 */
int cbcLength(int len) {
  if(len % 16 == 0) return len;
  return (16 - (len % 16) + len);
}
