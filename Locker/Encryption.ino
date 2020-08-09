/*
 *  MESSAGE ENCRYPTION (with AES-128)
 */
void encrypt(char * plain, int msgSize) {  
  randomIV();
  aes128_cbc_enc(key, iv, plain, msgSize);
}


/*
 *  MESSAGE DECRYPTION (with AES-128)
 */
void decrypt(char * cipher, int msgSize) {
  aes128_cbc_dec(key, iv, cipher, msgSize);
  cipher[msgSize] = '\0';
}


/*
 *  RANDOM IV ARRAY GENERATION
 */
void randomIV() {
  for(int i=0; i<16; i++) iv[i] = random(256);
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
uint8_t * hash(char * message, int msgSize) {
  Sha256.initHmac(key, sizeof(key));
  Sha256.write(message, msgSize);
  uint8_t * hmac = Sha256.resultHmac();

  return hmac;
}
