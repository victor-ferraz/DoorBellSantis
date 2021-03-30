//-------- Bibliotecas -----------
#define BLYNK_PRINT Serial // Blynk Serial Print
#define EEPROM_SIZE 512
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
#define BUTTON_3 01
#define BUTTON_4 03

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
#define MSG_MAIN_1  "Escolha uma Casa"
#define MSG_MAIN_2  "e aperte o botao"
#define MSG_BUTTON_PRESSED   "Chamando..."
#define MSG_BUTTON_1_PRESSED "PAULO"
#define MSG_BUTTON_2_PRESSED "MILZA"
#define MSG_BUTTON_3_PRESSED "GATA"
#define MSG_BUTTON_4_PRESSED "PEDRO"
#define MSG_DOOR_REMOTE_1    "Portao Acionado"
#define MSG_DOOR_REMOTE_2    "remotamente..."
#define MSG_ACCESS_OK_1      "Acesso permitido"
#define MSG_ACCESS_OK_2      "Portao Acionado!"
#define MSG_ACCESS_DENIED    " Acesso negado"
#define MSG_DEFINE_MASTER_1  "Apresente a nova"
#define MSG_DEFINE_MASTER_2  "tag Mestre..."
#define MSG_DEFINED_MASTER_1 "Nova tag Mestre"
#define MSG_DEFINED_MASTER_2 "foi definida!"
#define MSG_ADD_REMOVE_TAG_1 "Apresente a tag"
#define MSG_ADD_REMOVE_TAG_2 "Incluir/Excluir"
#define MSG_MODE_CANCEL_1    "    Operacao"
#define MSG_MODE_CANCEL_2    "   cancelada!"
#define MSG_REMOVE_ALL_1     "Removendo todos"
#define MSG_REMOVE_ALL_2     "os registros..."
#define MSG_REMOVED_ALL_1    "    Registros"
#define MSG_REMOVED_ALL_2    "    removidos!"
#define MSG_REMOVED_TAG_1    "    Registro"
#define MSG_REMOVED_TAG_2    "    removido!"
#define MSG_ADDED_TAG_1      "    Registro"
#define MSG_ADDED_TAG_2      "   adicionado!"
#define SS_PIN  21
#define RST_PIN 22
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
boolean programMode = false;

//-------- Token de Autenticação -----------
char auth[] = "";
//-------- Configurações de Wi-Fi -----------
char ssid[] = "";
char pass[] = "";

//-------- Main message on LCD -----------
void lcd_msg_main(){
  lcd.clear();
  lcd.print(MSG_MAIN_1);
  lcd.setCursor(0,1);
  lcd.print(MSG_MAIN_2);
}
//-------- Pino Virtual -----------
BLYNK_CONNECTED()
{         // Se conectar com Blynk
  Blynk.syncVirtual(V0);    // Sincroniza com o pino virtual V0
  Blynk.syncVirtual(V1);    // Sincroniza com o pino virtual V1
  Blynk.syncVirtual(V2);    // Sincroniza com o pino virtual V2
  Blynk.syncVirtual(V3);    // Sincroniza com o pino virtual V3
}
// in Blynk app writes values to the Virtual Pin 1
// Botão: Acionar Portão
BLYNK_WRITE(V1)
{
  uint8_t flag = param.asInt(); // assigning incoming value from pin V1 to a variable
  if(!flag){
    Serial.println(F("DOOR output"));
    Serial.println(F("-----------------------------"));
    Blynk.virtualWrite(V0, "Acionando o portão...\n");
    lcd.clear();
    lcd.print(MSG_DOOR_REMOTE_1);
    lcd.setCursor(0,1);
    lcd.print(MSG_DOOR_REMOTE_2);
    digitalWrite(DOOR, LOW);
    delay(1000);
    digitalWrite(DOOR, HIGH);
    Blynk.virtualWrite(V0, "Portão acionado!\n");
    lcd_msg_main();
  }
}
// Botão: Adicionar tag RFID
BLYNK_WRITE(V2)
{
  uint8_t flag = param.asInt(); // assigning incoming value from pin V2 to a variable
  if(flag){
    programMode = true;
    lcd.clear();
    lcd.print(MSG_ADD_REMOVE_TAG_1);
    lcd.setCursor(0,1);
    lcd.print(MSG_ADD_REMOVE_TAG_2);
    Serial.println(F("Scan a PICC to ADD or REMOVE to EEPROM"));
    Serial.println(F("Scan Master Card again to Exit Program Mode"));
    Serial.println(F("-----------------------------"));
    Blynk.virtualWrite(V0, "Apresente outro cartao para registra-lo ou cancele apresentando o cartao Master novamente.\n");
  }
}
// Botão: Definir cartão Master
BLYNK_WRITE(V3)
{
  uint8_t flag = param.asInt(); // assigning incoming value from pin V3 to a variable
  if(flag){
    lcd.clear();
    lcd.print(MSG_DEFINE_MASTER_1);
    lcd.setCursor(0,1);
    lcd.print(MSG_DEFINE_MASTER_2);
    Serial.println(F("Master Card removed"));
    Serial.println(F("Scan a PICC to be the Master"));
    Serial.println(F("-----------------------------"));
    Blynk.virtualWrite(V0, "Apresente um cartao para registra-lo como cartao Master.\n");
    deleteMaster();
  }
}
// Botão: Apagar todos os registros
BLYNK_WRITE(V4)
{
  uint8_t flag = param.asInt(); // assigning incoming value from pin V4 to a variable
  if(flag){
    lcd.clear();
    lcd.print(MSG_REMOVE_ALL_1);
    lcd.setCursor(0,1);
    lcd.print(MSG_REMOVE_ALL_2);
    Serial.println(F("Removing all cards..."));
    Blynk.virtualWrite(V0, "Removendo todos os cartões registrados...\n");
    EEPROM.write(0, 0x00);
    EEPROM.commit();
    delay(500);
    lcd.clear();
    lcd.print(MSG_REMOVED_ALL_1);
    lcd.setCursor(0,1);
    lcd.print(MSG_REMOVED_ALL_2);
    delay(500);
    Serial.println(F("All cards removed"));
    Serial.println(F("-----------------------------"));
    Blynk.virtualWrite(V0, "Todos os registros de cartões foram apagados.\n");
    lcd_msg_main();
  }
}

void check(){
  
  if (!digitalRead(BUTTON_1)){
    lcd.clear();
    lcd.print(MSG_BUTTON_PRESSED);
    lcd.setCursor(0,1);
    lcd.print(MSG_BUTTON_1_PRESSED);
    Serial.println("Button 1 is pressed.");
    digitalWrite(BELL_1, 1);
    delay(500);
    digitalWrite(BELL_1, 0);
    Blynk.notify("Alguem chamando Paulinho no portao!"); 
    delay(5000);
    lcd_msg_main();
  }
 else if (!digitalRead(BUTTON_2)){
    lcd.clear();
    lcd.print(MSG_BUTTON_PRESSED);
    lcd.setCursor(0,1);
    lcd.print(MSG_BUTTON_2_PRESSED);
    Serial.println("Button 2 is pressed.");
    digitalWrite(BELL_2, 1);
    delay(500);
    digitalWrite(BELL_2, 0);
    Blynk.notify("Alguem chamando Milza no portao!");
    delay(5000);
    lcd_msg_main();
 }
 else if (!digitalRead(BUTTON_3)){
    lcd.clear();
    lcd.print(MSG_BUTTON_PRESSED);
    lcd.setCursor(0,1);
    lcd.print(MSG_BUTTON_3_PRESSED);
    Serial.println("Button 3 is pressed.");
    digitalWrite(BELL_3, 1);
    delay(500);
    digitalWrite(BELL_3, 0);
    Blynk.notify("Alguem chamando Gata no portao!"); 
    delay(5000);
    lcd_msg_main();    
 }
 else if (!digitalRead(BUTTON_4)){
    lcd.clear();
    lcd.print(MSG_BUTTON_PRESSED);
    lcd.setCursor(0,1);
    lcd.print(MSG_BUTTON_4_PRESSED);
    Serial.println("Button 4 is pressed.");
    digitalWrite(BELL_4, 1);
    delay(500);
    digitalWrite(BELL_4, 0);
    Blynk.notify("Alguem chamando Pedro no portao!"); 
    delay(5000);
    lcd_msg_main();    
 }

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
  
  // check if has master card. if yes, check if the read card is the master one
  if (hasMaster()){
    readMaster(); // read the master card stored
    // check if the read card is the master
    if (checkTwo(readCard, masterCard)){ // is Master
      if(programMode == false){
        Serial.println(F("Hello Master - Entered Program Mode"));
        uint8_t count = EEPROM.read(0);   // Read the first Byte of EEPROM that
        Serial.print(F("I have "));     // stores the number of ID's in EEPROM
        Serial.print(count);
        Serial.print(F(" record(s) on EEPROM"));
        Serial.println("");
        Serial.println(F("Scan a PICC to ADD or REMOVE to EEPROM"));
        Serial.println(F("Scan Master Card again to Exit Program Mode"));
        Serial.println(F("-----------------------------"));
        Blynk.virtualWrite(V0, "Apresente outro cartao para registra-lo ou cancele apresentando o cartao Master novamente.\n");
        lcd.clear();
        lcd.print(MSG_ADD_REMOVE_TAG_1);
        lcd.setCursor(0,1);
        lcd.print(MSG_ADD_REMOVE_TAG_2);
        programMode = true;
      }else{
        Serial.println(F("Exiting Program Mode"));
        Serial.println(F("-----------------------------"));
        Blynk.virtualWrite(V0, "Modo Programação Cancelado\n");
        lcd.clear();
        lcd.print(MSG_MODE_CANCEL_1);
        lcd.setCursor(0,1);
        lcd.print(MSG_MODE_CANCEL_2);
        programMode = false;
        delay(2000);
        lcd_msg_main();
      }

    }else if(programMode == true){
      if(findID(readCard)){ // card is already stored
        Serial.println(F("I know this PICC, removing..."));
        Blynk.virtualWrite(V0, "Removendo cartão... ");
        deleteID(readCard);
        Blynk.virtualWrite(V0, "OK\n");
        lcd.clear();
        lcd.print(MSG_REMOVED_TAG_1);
        lcd.setCursor(0,1);
        lcd.print(MSG_REMOVED_TAG_2);
        programMode = false;
        delay(2000);
        lcd_msg_main();
      }else{
        Serial.println(F("I do not know this PICC, adding..."));
        Blynk.virtualWrite(V0, "Salvando cartão... ");
        writeID(readCard);
        Blynk.virtualWrite(V0, "OK\n");
        lcd.clear();
        lcd.print(MSG_ADDED_TAG_1);
        lcd.setCursor(0,1);
        lcd.print(MSG_ADDED_TAG_2);
        programMode = false;
        delay(2000);
        lcd_msg_main();
      }
      
    }else{
      Serial.println(F("-----------------------------"));
      if(findID(readCard)){ // card is stored
        lcd.clear();
        lcd.print(MSG_ACCESS_OK_1);
        lcd.setCursor(0,1);
        lcd.print(MSG_ACCESS_OK_2);
        Serial.println(F("Access granted!"));
        Blynk.virtualWrite(V0, "Acesso permitido\n");
        digitalWrite(DOOR, LOW);
        delay(1000);
        digitalWrite(DOOR, HIGH);
        delay(2000);
        lcd_msg_main();
      }else{ //card is not stored
        lcd.clear();
        lcd.print(MSG_ACCESS_DENIED);
        Serial.println(F("Access denied!"));
        Blynk.virtualWrite(V0, "Acesso negado\n");
        delay(2000);
        lcd_msg_main();
      }
    }
  }
  else{
    lcd.clear();
    lcd.print(MSG_DEFINED_MASTER_1);
    lcd.setCursor(0,1);
    lcd.print(MSG_DEFINED_MASTER_2);
    Serial.println(F("-----------------------------"));
    Serial.println(F("There is no Master card stored."));
    Serial.println(F("Writing this card as Master!"));
    Serial.println(F("-----------------------------"));
    Blynk.virtualWrite(V0, "Não tem cartão Master registrado\n");
    Blynk.virtualWrite(V0, "Salvando este cartão como Master\n");
    writeMaster(readCard);
    Serial.println(F("Master card stored."));
    Blynk.virtualWrite(V0, "Cartão Master registrado\n");
    delay(2000);
    lcd_msg_main();
  } 
}

void setup()
{
  Serial.begin(115200);     // Taxa de transmissão 115200
  // Initialize LCD
  lcd.begin(16, 2);
  lcd_msg_main();
  
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
  EEPROM.begin(EEPROM_SIZE);
  // EEPROM.write(0,0x00); // I use for reset the number of stored cards
  // EEPROM.commit();

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