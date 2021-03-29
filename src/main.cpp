//-------- Bibliotecas -----------
#define BLYNK_PRINT Serial // Blynk Serial Print
#include <WiFi.h> // Wi-Fi
#include <WiFiClient.h> // Wi-Fi client 
#include "registros.h"
#include <LiquidCrystal.h>
#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>
#include <BlynkSimpleEsp32.h> // Blynk-ESP32
BlynkTimer timer;

#define BUTTON_1 32
#define BUTTON_2 33
#define BUTTON_3 34
#define BUTTON_4 35

#define BELL_1   13
#define BELL_2   12
#define BELL_3   14
#define BELL_4   27

#define DOOR     26

#define LCD_RS   15
#define LCD_EN   2
#define LCD_D4   4
#define LCD_D5   16
#define LCD_D6   17
#define LCD_D7   5
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
#define MSG_MAIN  "Escolha uma Casa\ne aperte o botao"
#define MSG_BUTTON_1_PRESSED "Chamando...\nPAULO"
#define MSG_BUTTON_2_PRESSED "Chamando...\nMILZA"
#define MSG_BUTTON_3_PRESSED "Chamando...\nGATA"
#define MSG_BUTTON_4_PRESSED "Chamando...\nPEDRO"
#define MSG_ACCESS_OK        "Acesso permitido"
#define MSG_ACCESS_DENIED    "Acesso negado"
#define SS_PIN  21
#define RST_PIN 22
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
boolean programMode = false;

//-------- Token de Autenticação -----------
char auth[] = "aXWHebpfVegLti7DVZRNcAxjiAGILAwY";
//-------- Configurações de Wi-Fi -----------
char ssid[] = "";
char pass[] = "";

//-------- Pino Virtual -----------
BLYNK_CONNECTED()
{         // Se conectar com Blynk
  Blynk.syncVirtual(V0);    // Sincroniza com o pino virtual V0
  Blynk.syncVirtual(V1);    // Sincroniza com o pino virtual V1
  Blynk.syncVirtual(V2);    // Sincroniza com o pino virtual V2
  Blynk.syncVirtual(V3);    // Sincroniza com o pino virtual V3
}

void check(){
  if (!digitalRead(BUTTON_1)){
    Serial.println("Button 1 is pressed.");
    digitalWrite(BELL_1, 1);
    delay(500);
    digitalWrite(BELL_1, 0);
    Blynk.notify("Alguem chamando Paulinho no portao!"); 
  }
//  else if (!digitalRead(BUTTON_2)){
//    Serial.println("Button 2 is pressed.");
//    digitalWrite(BELL_2, 1);
//    delay(500);
//    digitalWrite(BELL_2, 0);
//    Blynk.notify("Alguem chamando Milza no portao!"); 
//  }
//  else if (!digitalRead(BUTTON_3)){
//    Serial.println("Button 3 is pressed.");
//    digitalWrite(BELL_3, 1);
//    delay(500);
//    digitalWrite(BELL_3, 0);
//    Blynk.notify("Alguem chamando Gata no portao!"); 
//  }
//  else if (!digitalRead(BUTTON_4)){
//    Serial.println("Button 4 is pressed.");
//    digitalWrite(BELL_4, 1);
//    delay(500);
//    digitalWrite(BELL_4, 0);
//    Blynk.notify("Alguem chamando Pedro no portao!"); 
//  }

  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  
  //Show UID on serial monitor
  Serial.print("UID tag :");
  String content= "";
  byte readCard[4];   // Stores scanned ID read from RFID Module
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     readCard[i] = mfrc522.uid.uidByte[i];
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  // Serial.println();
  // Serial.print("Message : ");
  content.toUpperCase();
  mfrc522.PICC_HaltA(); // Stop reading
  
  // Serial.println("Deleting master");
  // deleteMaster();
  // Serial.println("Deleted");

  // check if has master card. if yes, check if the read card is the master one
  if (hasMaster()){
    readMaster(); // read the master card stored
    // check if the read card is the master
    if (checkTwo(readCard, masterCard)){ // is Master
      Serial.println("Hello Master!\n");
      Blynk.virtualWrite(V0, "Cartao Master identificado\n");
      if(programMode == false){
        Serial.println("Modo programacao\n");
        Blynk.virtualWrite(V0, "Apresente outro cartao para registra-lo ou cancele apresentando o cartao Master novamente.\n");
        programMode = true;
      }else{
        Serial.println("Modo programação cancelado\n");
        Blynk.virtualWrite(V0, "Cancelado\n");
        programMode = false;
      }

    }else if(programMode == true){
      Serial.println("salvar cartão");
      Blynk.virtualWrite(V0, "Salvando cartão...\n");
      writeID(readCard);
      
    }else{
      Serial.println("Verificar registro");
      if(findID(readCard)){ // card is stored
        Serial.println("Acesso permitido");
      }else{ //card is not stored
        Serial.println("Acesso negado");
      }
    }
  }
  else{
    Serial.println("There is no Master card stored.");
    Serial.println("Writing this card as Master!");
    writeMaster(readCard);
    Serial.println("Master card stored.");
  } 

  // defining cards here
/*  if( (content.substring(1) == "36 8B 9E AC") ) //change the UID of the card/cards that you want to give access
  {
    Serial.println(MSG_ACCESS_OK);
    lcd.print(MSG_ACCESS_OK);
    Blynk.virtualWrite(V0, "UID tag: ");
    Blynk.virtualWrite(V0, content.substring(1));
    Blynk.virtualWrite(V0, "\nAcesso permitido\n");
    Serial.println();
    delay(500);
    digitalWrite(DOOR, LOW);
//    tone(BUZZER, 500);
    delay(1000);
//    noTone(BUZZER);
//    delay(3000);
    digitalWrite(DOOR, HIGH);
  }
 else{
    Serial.println("Unregistered user");
    lcd.print(MSG_ACCESS_DENIED);
    Blynk.virtualWrite(V0, "Usuario nao registrado tentou acessar\n");
    Blynk.virtualWrite(V0, "UID tag: ");
    Blynk.virtualWrite(V0, content.substring(1));
    Blynk.virtualWrite(V0, "\nAcesso negado\n");
    Serial.println(" Access denied");
//    digitalWrite(LED_R, HIGH);
//    tone(BUZZER, 300);
//    delay(1500);
//    digitalWrite(LED_R, LOW);
//    noTone(BUZZER);
  }*/
}

void setup()
{
  Serial.begin(115200);     // Taxa de transmissão 115200
  // Initialize LCD
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print(MSG_MAIN);
  
  // Input ports configuration
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);
  pinMode(BUTTON_3, INPUT_PULLUP);
  pinMode(BUTTON_4, INPUT_PULLUP);
  // Output ports configuration
  pinMode(2, OUTPUT);           // LED Azul
  pinMode(BELL_1, OUTPUT);
  pinMode(BELL_2, OUTPUT);
  pinMode(BELL_3, OUTPUT);
  pinMode(BELL_4, OUTPUT);
  pinMode(DOOR, OUTPUT);
  digitalWrite(DOOR, HIGH);

  // EEPROM Init
  EEPROM.begin(512);

  SPI.begin();      // Init SPI bus
  // RFID Init
  mfrc522.PCD_Init();   // Initiate MFRC522 RFID
  delay(10);        // Optional delay. Some board do need more time after init to be ready, see Readme
  
  Blynk.begin(auth, ssid, pass); // TOKEN+REDE+SENHA

  timer.setInterval(100L, check); // Timer for check buttons and RFID Card
}
void loop() 
{
  timer.run();
  Blynk.run();
}