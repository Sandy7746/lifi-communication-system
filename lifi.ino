#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

// LCD Configuration
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Keypad Configuration
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// LED Configuration
#define LED_PIN 13

// LDR Configuration
#define LDR_PIN A0

// Morse Code Configuration
#define UNIT 300
int ldrThreshold = 500; // Adjust based on your LDR

// Receiver Variables
String receivedMorse = "";
String decodedMessage = "";
unsigned long lastLightChange = 0;
bool lastLightState = false;
unsigned long lastActivityTime = 0;

void setup() {
  Serial.begin(9600);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  
  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Initialize LDR
  pinMode(LDR_PIN, INPUT);
  
  // Display startup message
  displayStartup();
}

void loop() {
  // First ask: Transmit or Receive
  char selectedMode = selectMode();
  
  if (selectedMode == '1') {
    // Transmission mode
    transmissionMode();
  } else if (selectedMode == '2') {
    // Reception mode
    receptionMode();
  }
}

char selectMode() {
  while(true) {
    Serial.println(F("=== SELECT MODE ==="));
    Serial.println(F("1. TRANSMIT MODE (Send Morse)"));
    Serial.println(F("2. RECEIVE MODE (Receive Morse)"));
    Serial.println(F("Press 1 or 2 to continue..."));
    
    lcd.clear();
    lcd.print(F("SELECT MODE:"));
    lcd.setCursor(0, 1);
    lcd.print(F("1.SEND 2.RECEIVE"));
    
    unsigned long startTime = millis();
    
    // Wait for input for 10 seconds
    while(millis() - startTime < 10000) {
      // Check keypad
      char key = keypad.getKey();
      if (key == '1' || key == '2') {
        Serial.print(F("Selected: "));
        Serial.println(key == '1' ? "TRANSMIT" : "RECEIVE");
        lcd.clear();
        lcd.print(F("Selected:"));
        lcd.setCursor(0, 1);
        lcd.print(key == '1' ? "TRANSMIT" : "RECEIVE");
        delay(2000);
        return key;
      }
      
      // Check serial input
      if (Serial.available()) {
        char serialInput = Serial.read();
        // Clear any remaining serial buffer
        while(Serial.available()) {
          Serial.read();
        }
        
        if (serialInput == '1' || serialInput == '2') {
          Serial.print(F("Selected: "));
          Serial.println(serialInput == '1' ? "TRANSMIT" : "RECEIVE");
          lcd.clear();
          lcd.print(F("Selected:"));
          lcd.setCursor(0, 1);
          lcd.print(serialInput == '1' ? "TRANSMIT" : "RECEIVE");
          delay(2000);
          return serialInput;
        }
      }
      
      delay(100);
    }
    
    // If no input after 10 seconds, show timeout message
    Serial.println(F("No input detected. Please try again..."));
    lcd.clear();
    lcd.print(F("No input!"));
    lcd.setCursor(0, 1);
    lcd.print(F("Try again..."));
    delay(2000);
  }
}

void transmissionMode() {
  // Ask for input method: Serial or Keypad
  char inputMethod = selectInputMethod();
  
  String inputMessage = "";
  
  if (inputMethod == '1') {
    // Keypad input
    keypadInput(inputMessage);
  } else if (inputMethod == '2') {
    // Serial input
    serialInput(inputMessage);
  }
  
  // If we have a message, transmit it via Morse code
  if (inputMessage.length() > 0) {
    transmitMessage(inputMessage);
  }
}

char selectInputMethod() {
  while(true) {
    Serial.println(F("=== SELECT INPUT METHOD ==="));
    Serial.println(F("1. KEYPAD INPUT"));
    Serial.println(F("2. SERIAL MONITOR INPUT"));
    Serial.println(F("Press 1 or 2 to continue..."));
    
    lcd.clear();
    lcd.print(F("SELECT INPUT:"));
    lcd.setCursor(0, 1);
    lcd.print(F("1.KEYPAD 2.SERIAL"));
    
    unsigned long startTime = millis();
    
    // Wait for input for 10 seconds
    while(millis() - startTime < 10000) {
      // Check keypad
      char key = keypad.getKey();
      if (key == '1' || key == '2') {
        Serial.print(F("Selected: "));
        Serial.println(key == '1' ? "KEYPAD" : "SERIAL");
        lcd.clear();
        lcd.print(F("Selected:"));
        lcd.setCursor(0, 1);
        lcd.print(key == '1' ? "KEYPAD" : "SERIAL");
        delay(2000);
        return key;
      }
      
      // Check serial input
      if (Serial.available()) {
        char serialInput = Serial.read();
        // Clear any remaining serial buffer
        while(Serial.available()) {
          Serial.read();
        }
        
        if (serialInput == '1' || serialInput == '2') {
          Serial.print(F("Selected: "));
          Serial.println(serialInput == '1' ? "KEYPAD" : "SERIAL");
          lcd.clear();
          lcd.print(F("Selected:"));
          lcd.setCursor(0, 1);
          lcd.print(serialInput == '1' ? "KEYPAD" : "SERIAL");
          delay(2000);
          return serialInput;
        }
      }
      
      delay(100);
    }
    
    // If no input after 10 seconds, show timeout message
    Serial.println(F("No input detected. Please try again..."));
    lcd.clear();
    lcd.print(F("No input!"));
    lcd.setCursor(0, 1);
    lcd.print(F("Try again..."));
    delay(2000);
  }
}

void keypadInput(String &message) {
  Serial.println(F("=== KEYPAD INPUT MODE ==="));
  Serial.println(F("Type using keypad, press # to end"));
  
  lcd.clear();
  lcd.print(F("KEYPAD MODE"));
  lcd.setCursor(0, 1);
  lcd.print(F("Press # to end"));
  delay(2000);
  
  message = "";
  lcd.clear();
  lcd.print(F("Message:"));
  lcd.setCursor(0, 1);
  
  while(true) {
    char key = keypad.getKey();
    
    if (key) {
      if (key == '#') {
        // End of input
        Serial.println(F("# pressed - Ending input"));
        lcd.clear();
        lcd.print(F("Input complete!"));
        delay(1000);
        break;
      } else if (key == '*') {
        // Backspace
        if (message.length() > 0) {
          message.remove(message.length() - 1);
          Serial.println(F("* pressed - Backspace"));
        }
      } else if ((key >= '0' && key <= '9') || (key >= 'A' && key <= 'D')) {
        // Valid input characters
        message += key;
        Serial.print(F("Key pressed: "));
        Serial.println(key);
      }
      
      // Update display
      lcd.clear();
      lcd.print(F("Message:"));
      lcd.setCursor(0, 1);
      if (message.length() <= 16) {
        lcd.print(message);
      } else {
        lcd.print(message.substring(message.length() - 16));
      }
      
      Serial.print(F("Current message: "));
      Serial.println(message);
    }
    
    delay(50);
  }
}

void serialInput(String &message) {
  Serial.println(F("=== SERIAL INPUT MODE ==="));
  Serial.println(F("Type your message in Serial Monitor and press ENTER"));
  
  lcd.clear();
  lcd.print(F("SERIAL MODE"));
  lcd.setCursor(0, 1);
  lcd.print(F("Type & press ENTER"));
  delay(2000);
  
  message = "";
  lcd.clear();
  lcd.print(F("Waiting for"));
  lcd.setCursor(0, 1);
  lcd.print(F("serial input..."));
  
  // Clear serial buffer first
  while(Serial.available()) {
    Serial.read();
  }
  
  // Wait for serial input
  while(!Serial.available()) {
    delay(100);
  }
  
  // Read the message from Serial
  if(Serial.available()) {
    message = Serial.readString();
    message.trim();
    
    Serial.print(F("Message received: "));
    Serial.println(message);
    
    // Display on LCD
    lcd.clear();
    lcd.print(F("RECEIVED:"));
    lcd.setCursor(0, 1);
    if(message.length() <= 16) {
      lcd.print(message);
    } else {
      lcd.print(message.substring(0, 16));
    }
    delay(2000);
  }
}

void transmitMessage(String &message) {
  // Send message via Morse code
  Serial.println(F("TRANSMITTING MORSE CODE..."));
  lcd.clear();
  lcd.print(F("TRANSMITTING..."));
  delay(1000);
  
  sendMorseCode(message);
  
  Serial.println(F("Morse transmission completed!"));
  Serial.print(F("MESSAGE SENT: "));
  Serial.println(message);
  
  lcd.clear();
  lcd.print(F("TRANSMISSION"));
  lcd.setCursor(0, 1);
  lcd.print(F("COMPLETE!"));
  delay(2000);
  
  lcd.clear();
  lcd.print(F("SENT:"));
  lcd.setCursor(0, 1);
  if(message.length() <= 16) {
    lcd.print(message);
  } else {
    lcd.print(message.substring(0, 16));
  }
  delay(3000);
}

void receptionMode() {
  Serial.println(F("=== RECEPTION MODE ==="));
  Serial.println(F("Ready to receive Morse code via LDR"));
  Serial.println(F("Point light source at LDR to transmit"));
  Serial.println(F("Press # to exit reception mode"));
  
  lcd.clear();
  lcd.print(F("RECEPTION MODE"));
  lcd.setCursor(0, 1);
  lcd.print(F("Waiting for signal"));
  
  // Reset receiver variables
  receivedMorse = "";
  decodedMessage = "";
  lastLightState = false;
  lastActivityTime = millis();
  
  bool receiving = true;
  
  while(receiving) {
    // Read LDR and decode Morse
    receiveMorseCode();
    
    // Check for manual exit
    char key = keypad.getKey();
    if (key == '#') {
      Serial.println(F("# pressed - Ending reception"));
      receiving = false;
    }
    
    if (Serial.available()) {
      char serialInput = Serial.read();
      if (serialInput == '#') {
        Serial.println(F("Serial # received - Ending reception"));
        receiving = false;
      }
    }
    
    // Auto-exit if no activity for 15 seconds and we have some message
    if (millis() - lastActivityTime > 15000 && receivedMorse.length() > 0) {
      Serial.println(F("No activity - Ending reception"));
      receiving = false;
    }
    
    delay(50);
  }
  
  // Display final received message
  displayReceivedMessage();
}

void receiveMorseCode() {
  int ldrValue = analogRead(LDR_PIN);
  bool currentLightState = (ldrValue > ldrThreshold);
  unsigned long currentTime = millis();
  
  // Light state changed
  if (currentLightState != lastLightState) {
    unsigned long duration = currentTime - lastLightChange;
    
    if (lastLightState) { // Light just turned OFF
      // Determine if it was a DOT or DASH
      if (duration < (UNIT * 1.8)) {
        receivedMorse += ".";
        Serial.println(F("DOT received"));
      } else {
        receivedMorse += "-";
        Serial.println(F("DASH received"));
      }
      
      // Update display
      lcd.clear();
      lcd.print(F("Receiving Morse:"));
      lcd.setCursor(0, 1);
      if (receivedMorse.length() <= 16) {
        lcd.print(receivedMorse);
      } else {
        lcd.print(receivedMorse.substring(receivedMorse.length() - 16));
      }
      
      // Try to decode the message
      decodeReceivedMessage();
    }
    
    lastLightState = currentLightState;
    lastLightChange = currentTime;
    lastActivityTime = currentTime;
  }
}

void decodeReceivedMessage() {
  // Simple decoding - you can enhance this
  decodedMessage = "";
  String currentSymbol = "";
  
  for (int i = 0; i < receivedMorse.length(); i++) {
    if (receivedMorse[i] == '.' || receivedMorse[i] == '-') {
      currentSymbol += receivedMorse[i];
    } else {
      if (currentSymbol.length() > 0) {
        decodedMessage += morseToChar(currentSymbol);
        currentSymbol = "";
      }
    }
  }
  
  // Add the last symbol
  if (currentSymbol.length() > 0) {
    decodedMessage += morseToChar(currentSymbol);
  }
}

void displayReceivedMessage() {
  Serial.println(F("=== RECEPTION COMPLETE ==="));
  Serial.print(F("Received Morse: "));
  Serial.println(receivedMorse);
  Serial.print(F("Decoded Message: "));
  Serial.println(decodedMessage);
  
  lcd.clear();
  lcd.print(F("RECEIVED MSG:"));
  lcd.setCursor(0, 1);
  if (decodedMessage.length() <= 16) {
    lcd.print(decodedMessage);
  } else {
    lcd.print(decodedMessage.substring(0, 16));
  }
  
  delay(5000);
}

char morseToChar(String morse) {
  if (morse == ".-") return 'A';
  if (morse == "-...") return 'B';
  if (morse == "-.-.") return 'C';
  if (morse == "-..") return 'D';
  if (morse == ".") return 'E';
  if (morse == "..-.") return 'F';
  if (morse == "--.") return 'G';
  if (morse == "....") return 'H';
  if (morse == "..") return 'I';
  if (morse == ".---") return 'J';
  if (morse == "-.-") return 'K';
  if (morse == ".-..") return 'L';
  if (morse == "--") return 'M';
  if (morse == "-.") return 'N';
  if (morse == "---") return 'O';
  if (morse == ".--.") return 'P';
  if (morse == "--.-") return 'Q';
  if (morse == ".-.") return 'R';
  if (morse == "...") return 'S';
  if (morse == "-") return 'T';
  if (morse == "..-") return 'U';
  if (morse == "...-") return 'V';
  if (morse == ".--") return 'W';
  if (morse == "-..-") return 'X';
  if (morse == "-.--") return 'Y';
  if (morse == "--..") return 'Z';
  if (morse == ".----") return '1';
  if (morse == "..---") return '2';
  if (morse == "...--") return '3';
  if (morse == "....-") return '4';
  if (morse == ".....") return '5';
  if (morse == "-....") return '6';
  if (morse == "--...") return '7';
  if (morse == "---..") return '8';
  if (morse == "----.") return '9';
  if (morse == "-----") return '0';
  return '?';
}

void displayStartup() {
  Serial.println(F("LIFI TRANSMITTER & RECEIVER"));
  lcd.clear();
  lcd.print(F("LIFI SYSTEM"));
  lcd.setCursor(0, 1);
  lcd.print(F("READY"));
  delay(2000);
}

void sendMorseCode(String &message) {
  // Convert to uppercase for Morse code
  message.toUpperCase();
  
  for(int i = 0; i < message.length(); i++) {
    char c = message[i];
    String morse = charToMorse(c);
    
    if(morse.length() > 0) {
      Serial.print(F("Transmitting: '"));
      Serial.print(c);
      Serial.print(F("' -> "));
      Serial.println(morse);
      
      // Update LCD with current character
      lcd.clear();
      lcd.print(F("Sending: "));
      lcd.print(c);
      lcd.setCursor(0, 1);
      lcd.print(morse);
      
      // Blink LED according to Morse code
      for(int j = 0; j < morse.length(); j++) {
        if(morse[j] == '.') {
          digitalWrite(LED_PIN, HIGH);
          delay(UNIT);
          digitalWrite(LED_PIN, LOW);
          delay(UNIT); // Gap between dots/dashes
        } else if(morse[j] == '-') {
          digitalWrite(LED_PIN, HIGH);
          delay(UNIT * 3);
          digitalWrite(LED_PIN, LOW);
          delay(UNIT); // Gap between dots/dashes
        }
      }
      delay(UNIT * 2); // Gap between letters
    } else if(c == ' ') {
      delay(UNIT * 4); // Gap between words
    }
  }
  
  digitalWrite(LED_PIN, LOW);
}

String charToMorse(char c) {
  switch(c) {
    case 'A': return ".-";
    case 'B': return "-...";
    case 'C': return "-.-.";
    case 'D': return "-..";
    case 'E': return ".";
    case 'F': return "..-.";
    case 'G': return "--.";
    case 'H': return "....";
    case 'I': return "..";
    case 'J': return ".---";
    case 'K': return "-.-";
    case 'L': return ".-..";
    case 'M': return "--";
    case 'N': return "-.";
    case 'O': return "---";
    case 'P': return ".--.";
    case 'Q': return "--.-";
    case 'R': return ".-.";
    case 'S': return "...";
    case 'T': return "-";
    case 'U': return "..-";
    case 'V': return "...-";
    case 'W': return ".--";
    case 'X': return "-..-";
    case 'Y': return "-.--";
    case 'Z': return "--..";
    case '0': return "-----";
    case '1': return ".----";
    case '2': return "..---";
    case '3': return "...--";
    case '4': return "....-";
    case '5': return ".....";
    case '6': return "-....";
    case '7': return "--...";
    case '8': return "---..";
    case '9': return "----.";
    case ' ': return "";
    default: return "";
  }
}