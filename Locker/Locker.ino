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
uint8_t iv[] = {'l', 'l', 'l', 'l', 'l', 'l', 'l', 'l', 'l', 'l', 'l', 'l', 'l', 'l', 'l', 'l'};
char in[] = "bGxsbGxsbGxsbGxsbGxsbCJPYCq2XTK/74wJlqWMQSpHrXpMoqThCN4eeA6ez6rOkL3iO+bqzyddglj92kuxqz09IbSrQQGJJ5Es1NThGek=";
String output = "{nonce: 2222222}";
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
  
}


/*
 *  MAIN
 */
void loop() {
  while (stateBT()) readBT();

  // PACKET TO CLIENT
  /*toClient(output);
  char ehi[boh.length()+1];
  boh.toCharArray(ehi, boh.length()+1);
  ehi[boh.length()] = '\0';
  delay(1000);*/
  
  // PACKET FROM CLIENT
  /*Serial.println("From client...");
  String message = fromClient(in, sizeof(in)-1);
  if (message != "error") Serial.println(message);*/

  int block = cbcLength(output.length());
  char cipher[block+1];
  for(int i=0; i<output.length()-block; i++) output += ' ';
  output.toCharArray(cipher, block+1);
  cipher[block] = '\0';
  encrypt(cipher, block);
  decrypt(cipher, block);

  Serial.println();
  delay(500);
}


/*
 *  CHOOSE OPERATION TO DO
 */
void execute(String message) {
  if (message == "T") {
    if (led) digitalWrite(LED_BUILTIN, LOW);
    else digitalWrite(LED_BUILTIN, HIGH);
    led = !led;
  }
  else {
    int msgLength = message.length();
    int block = cbcLength(msgLength);
    for (int i = 0; i < msgLength - block; i++) message += ' ';
    char data[block + 1];
    message.toCharArray(data, block + 1);
    data[block] = '\0';

    if (strlen(data) > 0) {
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
 *  GET MESSAGE FROM CLIENT
 */
String fromClient(char *input, int msgSize) {
  int decSize = Base64.decodedLength(input, msgSize);
  char * decoded = decodeMsg(input, msgSize);
  char message[decSize - 31];
  uint8_t hmac[33];
  
  memcpy(message, decoded, decSize - 32);
  memcpy((char *)hmac, decoded + (decSize - 32), 32);
  message[decSize - 32] = '\0';
  hmac[32] = '\0';
  Serial.print(message);
  Serial.println();  

  if(memcmp((char *)hmac, (char *)hash(message), 32) == 0) {
    Serial.println("Equals!");
    int block = cbcLength(sizeof(message) - 17);
    char cipher[block + 1];
    
    memcpy((char *)iv, message, 16);
    memcpy(cipher, message + 16, block);
    cipher[block] = '\0';
    decrypt(cipher, block);
    
    String output = String(cipher);
    output.trim();

    return output;
  }

  return "error";
}


/*
 *  SEND MESSAGE TO CLIENT
 */
void toClient(String message) {
  int block = cbcLength(message.length());
  char cipher[block+1];
  for(int i=0; i<message.length()-block; i++) message += ' ';
  message.toCharArray(cipher, block+1);
  cipher[block] = '\0';
  encrypt(cipher, block);
  
  char full[17+block];
  for(int i=0; i<16; i++) full[i] = iv[i];
  for(int i=16; i<17+block; i++) full[i] = cipher[i-16];

  char packet[16+block+33];
  memcpy(packet, full, 16+block);
  memcpy(packet + (16+block), (char *)hash(full), 32);
  packet[16+block+32] = '\0';

  char * enc = encodeMsg(packet, sizeof(packet)-1);
  int encodedLength = Base64.encodedLength(sizeof(packet)-1);
  decodeMsg(enc, encodedLength);
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
