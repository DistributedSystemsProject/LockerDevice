#include <AESLib.h>
#include <ArduinoJson.h>
#include <Base64.h>
#include <EEPROM.h>
#include <sha256.h>
#include <SoftwareSerial.h>
#include <uECC.h>

#define  FC(string_constant) (reinterpret_cast<const __FlashStringHelper *>(string_constant))
#define  BT_PWR A2           // PIN bluetooth power
#define  BT_ST A3            // PIN bluetooth state
#define  BT_RX A4            // PIN bluetooth reception
#define  BT_TX A5            // PIN bluetooth trasmission

SoftwareSerial btSerial(BT_TX, BT_RX);    // Bluetooth serial
boolean connected = false;                // Bluetooth state
int countdown = 0;                        // Bluetooth time
char const IDd[] PROGMEM = "1234567890device";     // Device id

const struct uECC_Curve_t * curve = uECC_secp192r1();
uint8_t privKeyDev[] = { 0x02, 0xf2, 0x82, 0x21, 0xfb, 0x3a, 0x22, 0xa4, 0x48, 0x92, 0x8c, 0x44, 
                         0x99, 0x61, 0x20, 0xfb, 0xf7, 0xbe, 0x2d, 0xa3, 0xf6, 0xcd, 0xc2, 0xe2 };
uint8_t pubKeySer[] = { 0xdc, 0x27, 0xa5, 0x67, 0x1d, 0xcb, 0x00, 0x0d, 0xc4, 0x1b, 0x99, 0x96, 
                        0x84, 0x0b, 0xb3, 0xc0, 0x08, 0xe2, 0x91, 0x08, 0xd1, 0x59, 0x49, 0x40, 
                        0x1f, 0x05, 0x7a, 0x28, 0xe0, 0x46, 0x81, 0x7e, 0xfa, 0xcc, 0x67, 0x90, 
                        0xf0, 0x5d, 0xef, 0xfd, 0x13, 0x78, 0xf5, 0xaf, 0x2d, 0xd8, 0xa9, 0x21 };
uint8_t key[16];
uint8_t iv[16];


/*
 *  RANDOM CURVE
 */
extern "C" {
  static int RNG(uint8_t *dest, unsigned size) {
    // Use the least-significant bits from the ADC for an unconnected pin (or connected to a source of 
    // random noise). This can take a long time to generate random data if the result of analogRead(0) 
    // doesn't change very frequently.
    while (size) {
      uint8_t val = 0;
      for(unsigned i = 0; i < 8; ++i) {
        int init = analogRead(0);
        int count = 0;
        while(analogRead(0) == init)
          ++count;
        
        if(count == 0)
           val = (val << 1) | (init & 0x01);
        else
           val = (val << 1) | (count & 0x01);
      }
      *dest = val;
      ++dest;
      --size;
    }
    
    return 1;
  }
}  // extern "C"


/*
 *  INIT SETUP
 */
void setup() {
  pinMode(BT_PWR, OUTPUT);
  pinMode(BT_ST, INPUT);
  digitalWrite(BT_PWR, HIGH);
  
  unsigned long seed = analogRead(0);
  seed += analogRead(0) << 10;
  seed += analogRead(0) << 20;
  seed += ((analogRead(0) & 0x3) << 30);
  randomSeed(seed);
  uECC_set_rng(&RNG);

  Serial.begin(9600);
  btSerial.begin(4800);         // Set correct bt module's baud rate
  Serial.println("STARTED");
}


/*
 *  MAIN
 */
void loop() {
  while(stateBT()) {
    if(btSerial.available()) {
      if(readBT()) resOp();
      else reqOp();
    }

    waitCount();
  }
  
  delay(100);
}


/*
 *  GET MESSAGE FROM CLIENT
 */
int fromClient(char * input, int msgSize) {
  int decSize = Base64.decodedLength(input, msgSize);
  char * decoded = decodeMsg(input, msgSize);
  
  char message[decSize-31];
  uint8_t hmac[32];
  memcpy(message, decoded, decSize-32);
  memcpy(hmac, decoded + (decSize-32), 32);
  message[decSize-32] = '\0';
  delete decoded;

  if(memcmp(hmac, hash(message, (decSize-32)), 32) == 0) {
    int block = cbcLength(decSize-32-16);
    char cipher[block+1];
    
    memcpy(iv, message, 16);
    memcpy(cipher, message+16, block);
    cipher[block] = '\0';
    decrypt(cipher, block);
    
    memcpy(input, cipher, block+1);
    return block;
  }

  return 0;
}


/*
 *  SEND MESSAGE TO CLIENT
 */
void toClient(String message) {
  int msgSize = message.length();
  int block = cbcLength(msgSize);
  for(int i=0; i<block-msgSize; i++) message += ' ';
  
  char cipher[block+1];
  char full[16+block];
  message.toCharArray(cipher, block+1);
  cipher[block] = '\0';
  encrypt(cipher, block);
  memcpy(full, iv, 16);
  memcpy(full + 16, cipher, 16+block);

  char packet[16+block+33];
  memcpy(packet, full, 16+block);
  memcpy(packet + (16+block), hash(full, (16+block)), 32);
  packet[16+block+32] = '\0';
  
  writeBT(packet, sizeof(packet)-1);
}


/*
 *  SEND REQUEST MESSAGE
 */
void reqOp() {
  uint8_t pubKeyEph[48];
  uint8_t privKeyEph[24];
  uECC_make_key(pubKeyEph, privKeyEph, curve);
  newShared(pubKeySer, privKeyEph);
  
  char packet[49];
  memcpy(packet, pubKeyEph, 48);
  packet[48] = '\0';
  
  writeBT(packet, sizeof(packet)-1);
}


/*
 *  CHECK OP ACCESS KEY
 */
boolean checkOp(char * aop, int msgSize) {
  StaticJsonDocument<120> doc;
  DeserializationError error = deserializeJson(doc, aop);
  uint8_t pubKeyEph[48];
  const char * ctr = doc["PK"];
  memcpy(pubKeyEph, decodeMsg((char *)ctr, 64), 48);
  ctr = doc["OP"];

  if(!newShared(pubKeyEph, privKeyDev)) return false;

  Serial.println(ctr);
  Serial.println("Operation DONE!");

  return true;
}


/*
 *  ANSWER TO OP REQUEST
 */
void resOp() {
  String message;
  StaticJsonDocument<50> doc;
  doc["RES"] = true;
  serializeJson(doc, message);

  Serial.println("Op response");

  toClient(message);
}


/*
 *  GENERATE SHARED KEY
 */
boolean newShared(uint8_t * pub, uint8_t * priv) {
  uint8_t shared[24];
  if(!uECC_shared_secret(pub, priv, shared, curve)) return false;

  Sha256.init();
  Sha256.write(shared, 24);
  memcpy(key, Sha256.result(), 16);

  return true;
}
