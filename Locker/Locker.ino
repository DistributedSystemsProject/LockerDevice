#include <AESLib.h>
#include <ArduinoJson.h>
#include <Base64.h>
#include <sha256.h>
#include <SoftwareSerial.h>

#define  BT_PWR A2           // PIN bluetooth power
#define  BT_ST A3            // PIN bluetooth state
#define  BT_RX A4            // PIN bluetooth reception
#define  BT_TX A5            // PIN bluetooth trasmission

SoftwareSerial btSerial(BT_TX, BT_RX);    // Bluetooth serial
boolean connected = false;                // Bluetooth state
int countdown = 0;                        // Bluetooth time

struct message {
  char IDc[15];
};

boolean led = false;
uint8_t key[] = {0x35, 0x74, 0xdf, 0x05, 0xd2, 0xbb, 0x88, 0x19, 0xdc, 0x25, 0xb8, 0x35, 0xd7, 0x24, 0xa3, 0x1f};
uint8_t iv[] = "lucalucalucaluca";
char input[] = "LtqED6LEbQLJicZXjwEZmRLb0pBY8zsD0qv2ddSUkZOYJWrbAyjSOZIsDYtYdc0VxaaO/PHd0mVT6/ffaW3xQg2AkUI2ujkBtsMl/onh9fm0IYLQKE92w1oYH8UjvfzW";
String output = "{nonce: 22222222, op: unlock}";
String command;


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

  // PACKET TO CLIENT
  /*int block = cbcLength(output.length());
  for(int i=0; i<output.length()-block; i++) output += ' ';
  char cipher[block+1];
  output.toCharArray(cipher, block+1);
  cipher[block] = '\0';
  encrypt(cipher, block);
  
  char message[17+block];
  for(int i=0; i<16; i++) message[i] = iv[i];
  for(int i=16; i<17+block; i++) message[i] = cipher[i-16];

  Sha256.initHmac(key, sizeof(key));
  Sha256.print(message);
  uint8_t * reshmac = Sha256.resultHmac();

  char packet[16+block+33];
  strcpy(packet, message);
  strcat(packet, (char *)reshmac);
  packet[16+block+32] = '\0';
  
  char * enc = encodeMsg(packet, sizeof(packet)-1);
  int encodedLength = Base64.encodedLength(sizeof(packet)-1);
  decodeMsg(enc, encodedLength);*/
  
  // PACKET FROM CLIENT
  /*int decodedLength = Base64.decodedLength(input, sizeof(input));
  char * decoded = decodeMsg(input, sizeof(input));
  char message[decodedLength-31];
  uint8_t hmac[33];
  strncpy(message, decoded, decodedLength-32);
  strcpy((char *)hmac, &decoded[decodedLength-32]);
  message[decodedLength-32] = '\0';
  hmac[32] = '\0';

  Sha256.initHmac(key, sizeof(key));
  Sha256.print(message);
  uint8_t * reshmac = Sha256.resultHmac();
  reshmac[32] = '\0';
  printHash(reshmac);
  printHash(hmac);

  if(strcmp((char *)hmac, (char *)reshmac) == 0) {
    Serial.println("Equals!");
    strncpy((char *)iv, message, 16);
    int block = cbcLength(sizeof(message)-17);
    char cipher[block+1];
    strcpy(cipher, &message[16]);
    decrypt(cipher, block);
  }*/
}


/*
 *  MAIN
 */
void loop() {
  while(stateBT()) {
    while(btSerial.available()) {
      delay(10);
      char c = btSerial.read();
      command += c;
    }
    command.trim();
    if(command.length() > 0) execute(command); 
    
    delay(100);
    countdown -= 1;
  }
}


/*
 *  PRINT HASH STRING
 */
void printHash(uint8_t* hash) {
  int i;
  for (i=0; i<32; i++) {
    Serial.print("0123456789abcdef"[hash[i]>>4]);
    Serial.print("0123456789abcdef"[hash[i]&0xf]);
  }
  Serial.println();
}


/*
 *  CHOOSE OPERATION TO DO
 */
void execute(String message) {
  if(message == "T"){
    if(led) digitalWrite(LED_BUILTIN, LOW);
    else digitalWrite(LED_BUILTIN, HIGH);
    led = !led;
  }
  else {
    int msgLength = message.length();
    int block = cbcLength(msgLength);
    for(int i=0; i<msgLength-block; i++) message += ' ';
    char data[block+1];
    message.toCharArray(data, block+1);
    data[block] = '\0';
    
    if(strlen(data) > 0) {  
      Serial.print("Message: ");
      Serial.println(message);
      Serial.print("Size: ");
      Serial.println(msgLength);
      delay(200);   
      
      encrypt(data, block);
      char * encoded = encodeMsg(data, block);
      int encodedLength = Base64.encodedLength(block);
      char * decoded = decodeMsg(encoded, encodedLength);
      decrypt(decoded, block);
      randomIV();
      Serial.println();
    }
  }
  
  command = "";
  countdown = 600;
}


/*
 *  MESSAGE DESERIALIZATION (from JSON)
 */
void fromJson(char encString[]) {
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, encString);
  
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
  
  message msg;
  strcpy(msg.IDc, doc["IDc"]);
  Serial.print("Client ID is ");
  Serial.println(msg.IDc);
}


/*  
 *  NOTES:
 *  StaticJsonDocument per messaggi < 1kb
 */

