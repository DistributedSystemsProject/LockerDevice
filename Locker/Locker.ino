#include <AESLib.h>
#include <ArduinoJson.h>
#include <Base64.h>
#include <EEPROM.h>
#include <sha256.h>
#include <SoftwareSerial.h>

#define  BT_PWR A2           // PIN bluetooth power
#define  BT_ST A3            // PIN bluetooth state
#define  BT_RX A4            // PIN bluetooth reception
#define  BT_TX A5            // PIN bluetooth trasmission

SoftwareSerial btSerial(BT_TX, BT_RX);    // Bluetooth serial
boolean connected = false;                // Bluetooth state
int countdown = 0;                        // Bluetooth time

boolean led = false;
const uint8_t key[] PROGMEM = {0x35, 0x74, 0xdf, 0x05, 0xd2, 0xbb, 0x88, 0x19, 0xdc, 0x25, 0xb8, 0x35, 0xd7, 0x24, 0xa3, 0x1f};
uint8_t iv[] = {'l', 'l', 'l', 'l', 'l', 'l', 'l', 'l', 'l', 'l', 'l', 'l', 'l', 'l', 'l', 'l'};

typedef struct message {
  const char *IDc;
  const char *OP;
  const char *N1;
  const char *N3;
};


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

  Serial.println("STARTING");
}


/*
 *  MAIN
 */
void loop() {
  Serial.println();
  while(stateBT()) readBT();

  // PACKET TO CLIENT
  toClient("{\"IDc\":\"123456789012345\",\"OP\":\"unlock\",\"N1\":\"123456789012345\",\"N3\":\"3456789012462462462452456234567\"}");
  delay(500);
  // PACKET FROM CLIENT
  fromJson((char *)(fromClient((char *)"bGxsbGxsbGxsbGxsbGxsbFp1bKjJC1g4XZ/nc8yi5+V9KvjvPQrMyQAHm8vQV+csZ9JT/pmAmd1ODQ5/KCn3LSrQviB7dEkKuulxAeeG7CNuE+6I8XavUmedQ8AtdbUxizv/qiI7hFaw7wLkfZRRoQS5T/bi4bLBAXSHZosbIjjWCbw3JhJF3kxlYtzWMySruSmKqdOgvOKjnkYCAETyBg==", 216)).c_str());
  
  delay(500);
}


/*
 *  GET MESSAGE FROM CLIENT
 */
String fromClient(char * input, int msgSize) {
  Serial.println("// From");
  
  int decSize = Base64.decodedLength(input, msgSize);
  char * decoded = decodeMsg(input, msgSize);
  char message[decSize-31];
  uint8_t hmac[32];
  
  memcpy(message, decoded, decSize-32);
  memcpy(hmac, decoded + (decSize-32), 32);
  message[decSize-32] = '\0';
  delete decoded;

  if(memcmp(hmac, hash(message), 32) == 0) {
    Serial.println(F("Equals!"));
    int block = cbcLength(decSize-32-16);
    char cipher[block+1];
    
    memcpy(iv, message, 16);
    memcpy(cipher, message+16, block);
    cipher[block] = '\0';
    decrypt(cipher, block);
    
    String out = String(cipher);
    return out;
  }

  return "";
}


/*
 *  SEND MESSAGE TO CLIENT
 */
void toClient(String message) {
  Serial.println("// To");
  
  int block = cbcLength(message.length());
  for(int i=0; i<block-message.length(); i++) message += ' ';
  
  char cipher[block+1];
  message.toCharArray(cipher, block+1);
  cipher[block] = '\0';
  encrypt(cipher, block);
  
  char full[16+block];
  for(int i=0; i<16; i++) full[i] = iv[i];
  for(int i=16; i<16+block; i++) full[i] = cipher[i-16];

  char packet[16+block+33];
  memcpy(packet, full, 16+block);
  memcpy(packet + (16+block), hash(full), 32);
  packet[16+block+32] = '\0';

  char * enc = encodeMsg(packet, sizeof(packet)-1);
  delete enc;
}


/*
 *  MESSAGE DESERIALIZATION (from JSON)
 */
void fromJson(char * json) {
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, json);

  if(error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
  
  message first = { doc["IDc"], doc["OP"], doc["N1"], doc["N3"] };
  Serial.println(first.IDc);
  Serial.println(first.OP);
  Serial.println(first.N1);
  Serial.println(first.N3);
}


/*
 *  NOTES:
 *  StaticJsonDocument per messaggi < 1kb
 */
