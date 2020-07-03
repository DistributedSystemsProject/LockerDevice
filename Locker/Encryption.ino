/*
 *  MESSAGE ENCRYPTION (with AES-128)
 */
void encrypt(char *plain, int msgSize) {
  aes128_cbc_enc(key, iv, plain, msgSize);
  
  Serial.print("Block size: ");
  Serial.println(msgSize);
  btSerial.print("Encrypted: ");
  Serial.print("Encrypted: ");
  btSerial.println(plain);
  Serial.println(plain);
}


/*
 *  MESSAGE DECRYPTION (with AES-128)
 */
void decrypt(char *cipher, int msgSize) {
  aes128_cbc_dec(key, iv, cipher, msgSize);
  cipher[msgSize] = '\0';
  
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


/*
 *  MESSAGE HASHING (with SHA-256)
 */
 uint8_t * hash(char *message) {
  Sha256.initHmac(key, sizeof(key));
  Sha256.print(message);
  uint8_t * hmac = Sha256.resultHmac();
  hmac[32] = '\0';

  return hmac;
 }


/*
 *   PRINT HASH STRING
 */
void printHash(uint8_t* hash) {
  for (int i = 0; i < 32; i++) {
    Serial.print("0123456789abcdef"[hash[i] >> 4]);
    Serial.print("0123456789abcdef"[hash[i] & 0xf]);
  }
  Serial.println();
}
