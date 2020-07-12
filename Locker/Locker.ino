#include <AESLib.h>
#include <ArduinoJson.h>
#include <Base64.h>
#include <EEPROM.h>
#include <sha256.h>
#include <SoftwareSerial.h>

#define  FC(string_constant) (reinterpret_cast<const __FlashStringHelper *>(string_constant))
#define  BT_PWR A2           // PIN bluetooth power
#define  BT_ST A3            // PIN bluetooth state
#define  BT_RX A4            // PIN bluetooth reception
#define  BT_TX A5            // PIN bluetooth trasmission

SoftwareSerial btSerial(BT_TX, BT_RX);    // Bluetooth serial
boolean connected = false;                // Bluetooth state
int countdown = 0;                        // Bluetooth time

char const IDd[] PROGMEM = "1234567890device";     // Device id
char N[16];                     // Last sent nonce
uint8_t key[] = {0x0c, 0xc0, 0x52, 0xf6, 0x7b, 0xbd, 0x05, 0x0e, 0x75, 0xac, 0x0d, 0x43, 0xf1, 0x0a, 0x8f, 0x35};
uint8_t iv[16];

typedef struct {
  const char *IDc;
  const char *OP;
  const char *N1;
  const char *N3;
} message;


/*
 *  INIT SETUP
 */
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BT_PWR, OUTPUT);
  pinMode(BT_ST, INPUT);
  digitalWrite(BT_PWR, HIGH);
  randomSeed(analogRead(0));

  Serial.begin(9600);
  btSerial.begin(9600);
  btSerial.setTimeout(200);

  Serial.println("STARTING");
}


/*
 *  MAIN
 */
void loop() {
  while(stateBT()) readBT();
  
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
  for(int i=0; i<16; i++) full[i] = iv[i];
  for(int i=16; i<16+block; i++) full[i] = cipher[i-16];

  char packet[16+block+33];
  memcpy(packet, full, 16+block);
  memcpy(packet + (16+block), hash(full, (16+block)), 32);
  packet[16+block+32] = '\0';
  
  char * enc = encodeMsg(packet, sizeof(packet)-1);
  btSerial.println(enc);
  delete enc;
}


/*
 *  SEND REQUEST MESSAGE
 */
void reqAccess() {
  String message;
  newNonce();
  StaticJsonDocument<60> doc;
  doc["IDd"] = FC(IDd);
  doc["N1"] = String(N);
  serializeJson(doc, message);
  serializeJson(doc, Serial);

  btSerial.println(FC(IDd));
  toClient(message);
}


/*
 *  CHECK OTP ACCESS KEY
 */
boolean checkAccess(char * otp, int msgSize) {
  StaticJsonDocument<120> doc;
  DeserializationError error = deserializeJson(doc, otp);
  const char * ctr = doc["N1"];
  
  if(error || memcmp(ctr, N, 16) != 0) return false;
  
  ctr = doc["N2"];
  memcpy(N, ctr, 16);
  ctr = doc["OP"];
  Serial.println(ctr);

  return true;
}


/*
 *  GENERATE NEW NONCE
 */
void newNonce() {
  for(int i=0; i<16; i++) {
    N[i] = random(256);
  }
}


/*
 *  NOTES:
 *  StaticJsonDocument per messaggi < 1kb
 */
