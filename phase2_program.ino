#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <Stepper.h>
#include <ESP32Servo.h> 

// Firebase Helper files
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

// ==== Pin Definitions ====
#define RST_PIN    4  
#define SS_PIN     5
#define IR_PIN     34
#define SLOT_IR    15

// ========to store the slot information ======
#define Queue_Size 10
int queue[Queue_Size];
int head=0;
int tail=0;
bool queueEmpty =true;
bool queueFull =false;

//variables to store slot state and slot no.
bool myVar=false;
int slot=0; 

//fnction to add a element into the queue
void enqueue(int data){
  if(queueEmpty){
    queueEmpty = false;
  }
  queue[tail] = data;
  tail = (tail+1) % Queue_Size;
  if(tail == head){
    queueFull = true;
  }
}

//function to retain the first added item from the queue
int dequeue(){
  if (!queueEmpty){
    int data = queue[head];
    head = (head+1) % Queue_Size; 
    if(tail == head){
    queueFull == true;
  }
  return data;
  }else{
    return -1;
  }
}

//=========end of queue operation======

//initializing the lcd display , RC5222 RFID reader and the stepper motor(run the convayer belt)
LiquidCrystal_I2C lcd(0x27,16,2); 
MFRC522 mfrc522(SS_PIN, RST_PIN); //RC5222 using the SPI communication protocol
Stepper myStepper(200,25, 26, 27,14);

//initialize the slot servos currently using 3 slots( 1,2=>2 servo and invalid )
Servo servo1;   //slot 1
Servo servo2;   //slot 2

// RFID Reading
byte lastUID[10];
byte lastUIDLength = 0;
byte block = 4;
byte data[18];
byte size = sizeof(data);
char code[5] = "";  // To store RFID value
int read_count = 0;
int slot_count = 0;

// ==== WiFi Credentials ====
#define WIFI_SSID "Pixel_2"
#define WIFI_PASSWORD "@111112345"

// ==== Firebase credentials ====
#define API_KEY "AIzaSyC2K2jBFJgxSxOeAGUFLqE6S_DSbCxUT6c"
#define DATABASE_URL "https://warehouse-details-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define USER_EMAIL "nnm22ec135@nmamit.in"
#define USER_PASSWORD "warehouse"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// ==== Setup ====
void setup() {
  Serial.begin(115200);
  
  pinMode(IR_PIN, INPUT);
  pinMode(SLOT_IR, INPUT);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Initializing... ");

  servo1.setPeriodHertz(50); 
  servo2.setPeriodHertz(50); 

  //configuring the servo pins in esp32 and initializing the current state
  servo1.attach(13); 
  servo2.attach(33); 

  // slot gates initial state 
  servo1.write(0);
  servo2.write(90);

  // Initialize SPI and RFID
  SPI.begin(); // SCK=18, MISO=19, MOSI=23
  mfrc522.PCD_Init();

  // Connecting to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    // waiting for the wifi connectivity
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nConnected to Wi-Fi");

  // Firebase Configuration
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectNetwork(true);
  fbdo.setBSSLBufferSize(4096, 1024);
  Firebase.setDoubleDigits(5);

  Serial.println("System ready: IR + RC522 + Firebase");
  lcd.clear();
  lcd.print("Mode : Sleep");
  delay(2000);
  lcd.noBacklight();

}

// ==== Check for new RFID card ==== ==== Do not read the same product multiple times ====
bool isNewTag(byte *uid, byte length) {
  if (length != lastUIDLength) return true;
  for (int i = 0; i < length; i++) {
    if (uid[i] != lastUID[i]) return true;
  }
  return false;
}

// ==== Main Loop ====
void loop() {
  //==== convayer and servo controlling functions ====
  ConvayerOn();
  handleServo();

  // ==== detects when a new product kept on the convayer ====
  if (digitalRead(IR_PIN) == LOW) {
    read_count++;     //increment the product left for verification
    slot_count++;     //increment the product left for slot distribution
    
    while (digitalRead(IR_PIN) == LOW) {
      ConvayerOn();
      handleServo();
      delay(1);
    }
    Serial.println("Object detected!");
  }

  // ==== for display the no. of products in the convayer ====
  if (max(read_count,slot_count)!=0)
  {
  lcd.clear();
  lcd.backlight();
  lcd.print("Product present:");
  lcd.setCursor(0,1);
  lcd.print(max(read_count,slot_count));
  }
  else
  {
    // ==== sleep mode ====
    lcd.clear();
    lcd.noBacklight();
  }
  
  
  while (read_count != 0) {
    // loop runs until the products left for the verification \
    
     ConvayerOn();
     handleServo();

    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) return;

    byte *uid = mfrc522.uid.uidByte;
    byte uidLength = mfrc522.uid.size;
    memset(code, 0, sizeof(code)); // Clear previous code

    if (isNewTag(uid, uidLength)) {
      memcpy(lastUID, uid, uidLength);
      lastUIDLength = uidLength;

      Serial.print("New card UID: ");
      for (byte i = 0; i < uidLength; i++) {
        Serial.print(uid[i] < 0x10 ? " 0" : " ");
        Serial.print(uid[i], HEX);
      }
      Serial.println();

      // ==== Read data from the nfc tag ====
      MFRC522::StatusCode status = mfrc522.MIFARE_Read(block, data, &size);

      // ==== Read first 4 characters from the block 4 ====
      for (int i = 0; i < 4; i++) {
        code[i] = data[i];
      }

      // ==== product code(code) sent to the firebase for the verification ====
      if (Firebase.ready()) {
        getProductSlot(String(code));
      }

      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
    }

    read_count--;
    delay(500);
    mfrc522.PCD_Init();  // Reinitialize after halt
  }
}

// ==== Firebase Handling ====
void getProductSlot(String productName) {
  // ==== store the slot no. received from the firebase ====
  int slotNumber;

  // ==== Creates a String 'slotPath' which is the path in the Firebase Realtime Database. ====
   String slotPath = "/temporary_entries/" + productName + "/slot";
   
  String statusPath = "/temporary_entries/" + productName + "/status";
  // ==== Creates a String 'statusPath' which is the path in the Firebase Realtime Database. ====
  

  Serial.printf("Checking for product: %s\n", productName.c_str());

  // ==== Attempts to retrieve the integer value from the Firebase Realtime Database at the specified 'slotPath' using the 'fbdo' object ====
  // ==== The 'if' condition is true if the retrieval is successful. ====
  if (Firebase.getInt(fbdo, slotPath)) {
    slotNumber = fbdo.intData();
    Serial.printf("Product %s found. Slot number: %d\n", productName.c_str(), slotNumber);
    
       // Display on LCD
      lcd.clear();
      lcd.print("Product:");
      lcd.setCursor(9,0);
      lcd.print(code);
      lcd.setCursor(0,1);
      lcd.print("Slot:");
      lcd.setCursor(6,1);
      lcd.print(slotNumber);
      Serial.print("Slot : ");
      Serial.println(slotNumber);

    // ==== update that product entered to the warehouse ====
    if (Firebase.setString(fbdo, statusPath, "arrived")) {
      Serial.printf("Status updated to 'arrived'\n");
    } else {
      Serial.printf("Failed to update status: %s\n", fbdo.errorReason().c_str());
    }
  } else {
    // ==== if the product not present int the firebase list [unverified product] ====
    slotNumber=0;
    Serial.printf("Product not found or error: %s\n", fbdo.errorReason().c_str());
      lcd.clear();
      lcd.print("Product:");
      lcd.setCursor(9,0);
      lcd.print("invalid");
      lcd.setCursor(0,1);
      lcd.print("Slot:");
      lcd.setCursor(6,1);
      lcd.print("NULL");
      Serial.print("Slot : ");
      Serial.println("NULL");
  }
  //feed the slot number to the queue
  enqueue(slotNumber);
}

// ==== end of firebase management ====

// ==== convayer control function ====
void ConvayerOn()
{
  if(slot_count !=0)
  {
    // convayer is ON only when a product present in the convayer
    myStepper.setSpeed(90); // speed of stepper motor
    myStepper.step(200); // no steps per cycle for stepper motor
    Serial.println("stepper step");
    delay(1);
  }
}

// ==== slot gates control function ====
void handleServo() {
  //detect the incomming object
  if (digitalRead(SLOT_IR) == LOW && slot_count!=0 && myVar==false) {
    // ==== select the slot from the queue ====
    slot=dequeue();
    switch(slot){
      // ==== blocks the path of product using respective slot gate ====
      case 1: {
                Serial.println("IR1 LOW - opening servo1");
                myVar = true;
                servo1.write(90);
                break;
              }
      case 2: {
                Serial.println("IR1 LOW - opening servo2");
                myVar = true;
                servo2.write(0);
                break;
              }
      case 0: {
                // ==== when the product is unverified ====
                Serial.println("IR1 LOW - end slot");
                myVar = true;
              }
      default:  break;
    }
  }

  // close opened slot gate by pushing the product into the slot
  if(digitalRead(SLOT_IR) == HIGH && myVar == true && slot_count !=0) {
    switch(slot){
      case 1: {
                Serial.println("IR1 LOW - closing servo1");
                myVar = true;
                servo1.write(0);
                slot_count--;
                myVar=false;
                break;
              }
      case 2: {
                Serial.println("IR1 LOW - closing servo2");
                myVar = true;
                servo2.write(90);
                slot_count--;
                myVar=false;
                break;
              }
      case 0: {
                Serial.println("IR1 LOW - closing servo2");
                slot_count--;
                myVar=false;
                break;
              }
      default: break;
    }
  }
}
