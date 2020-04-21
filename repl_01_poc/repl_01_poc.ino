#define DELAY 125

void CE_Disable() {
  PORTC |= B00010000;
}

void CE_Enable() {
  PORTC &= B11101111;
}

void OE_Disable() {
  PORTC |= B00100000;
}

void OE_Enable() {
  PORTC &= B11011111;
}

void dataBusAsInput() {
  PORTB = 0x11;
  DDRB = 0x00;
}

void dataBusAsOutput() {
  PORTB = 0x00; 
  DDRB = 0xff;
}

void controlLinesEnable() { 
  DDRC |= B00110000;
}

void addressBusEnable() {
  DDRC |= B00001111;      // address bus[3:0]
}

void fixBaudRateDivisor() {
  UBRR0L = B01100111;
}
void setup() {
  
  Serial.begin(9600);
  fixBaudRateDivisor();
  dataBusAsInput();
  CE_Disable();
  OE_Disable();
  controlLinesEnable();
  addressBusEnable();
  
  Serial.write("   Init Atmega328p\n");
}

enum State { waitingCommand, readingAddress, readingData, readingReadSize };

State state = waitingCommand;

void setAddress(unsigned int address) {
  PORTC &= B11110000;
  PORTD &= B00000011;

  // address 0 PORTC0
  if ( address % 2 ) {
    PORTC |= B00000001;
  }

  // address 1 PORTC1
  address /= 2;
  if ( address % 2 ) {
    PORTC |= B00000010;
  }

  // address 2 PORTC2
  address /= 2;
  if ( address % 2 ) {
    PORTC |= B00000100;
  }

  // address 3 PORTC3
  address /= 2;
  if ( address % 2 ) {
    PORTC |= B00001000;
  }

}

void printMenu(int address, byte data, unsigned int readSize, bool autoincrement) {
  Serial.print("#####################################################\n");
  Serial.print("MENU\n");
  Serial.print("#####################################################\n");
  Serial.print("a  set address               : (");
  printAligned(address,4);
  Serial.print(")\n");
  Serial.print("d  set data                  : (");
  printAligned(data,2);
  Serial.print(")\n");
  Serial.print("s  set read size             : (");
  printAligned(readSize,4);
  Serial.print(")\n");
  Serial.print("r  read\n");
  Serial.print("w  write (not implemented)\n");
  Serial.print("@  print current address values\n");
  Serial.print("+  increment current address\n"); 
  Serial.print("-  decrement current address\n"); 
  Serial.print("i  autoincrement mode        : ("); 
  if (autoincrement) {
    Serial.print("ON");
  } else {
    Serial.print("OFF");
  }
  Serial.print(")\n");
  Serial.print("\n");
}

byte hexa2int(char cmd) {
  byte result = 0;;
  if (cmd >= 'a' && cmd <= 'f') {
    return  10 + cmd - 'a';
  } else if ( cmd >= '0' && cmd <= '9') {
    return cmd - '0';
  }
}

void printAligned(unsigned int value, byte cols) {
  if (cols == 2) {
    if (value < 16) {
      Serial.print("0");
    }

  } else if (cols == 4) {
    if (value < 4096) {
      Serial.print("0");
    }
    if (value < 256) {
      Serial.print("0");
    }
    
    if (value < 16) {
      Serial.print("0");
    }
   
  } else {
    // error
  }
  Serial.print(value,HEX);  
}

byte pos;
byte data;
unsigned int address;
bool autoincrement = false;
unsigned int readSize = 1;

void loop() {
  byte col = 0;
  if (Serial.available()) {
    char cmd = Serial.read();
    switch(state) {
      case waitingCommand :
        switch (cmd) {
          case 'r': // read
            if (readSize == 1) {
              Serial.print("\nReading address ");
              printAligned(address,4);
            } else {
              Serial.print("Reading address from: ");
              printAligned(address,4);
              Serial.print(" to : ");
              printAligned(address + readSize ,4);
              Serial.print("\n");
              printAligned(address,4);
              Serial.print("  ");
              autoincrement = true;
            }
            
            dataBusAsInput();
            
            while ( readSize > 0 ) {
              setAddress(address);
              delay(1);
              CE_Enable();
              delay(1);
              OE_Enable();
              delay(1);
            
              data = PINB;
              printAligned(data,2);
              Serial.print("  ");
              if (col == 7 ) {
                Serial.print("\n");
                printAligned(address,4);
                Serial.print("  ");
                col = 0;
              } else {
                ++col;
              }
              OE_Disable();
              CE_Disable();
              if (autoincrement) {
                ++address;
              }
              --readSize;
            }
            readSize = 1;
            Serial.print("\n");
            state = waitingCommand;
          break;
          case 's':
            state = readingReadSize;
            pos = 0;
            readSize = 0;
            Serial.print("Size? (xxxx): ");
           break;
          case 'a': // set address
            state = readingAddress;
            pos = 0;
            address = 0;
            Serial.print("\nAddress? (xxxx): ");
          break;
          
          case 'd': // set data
            state = readingData;
            pos = 0;
            data = 0;
            Serial.print("Data? (xx):");
          break;
          
          
          case 'w': // write
            Serial.print("\nWriting ");
            Serial.print(data, HEX);
            Serial.print(" in ");
            Serial.print(address, HEX);
            Serial.print("\n");
            // TODO: write to eprom            
            if (autoincrement) {
              ++address;
            }

            state = waitingCommand;
          break;
          case '@': // read address
            Serial.print("Current address : ");
            printAligned(address,4);
            Serial.print(" Current data : ");
            printAligned(data,2);
            ++address;
            Serial.print("\n");
          break;
          case '+':
            ++address; 
            Serial.print("Current address : ");
            printAligned(address,4);
            Serial.print("\n");
          break;
          case '-':
            --address; 
            Serial.print("Current address : ");
            printAligned(address,4);
            Serial.print("\n");
          break;
          case '?':
            printMenu(address, data, readSize, autoincrement);
          break;
          case 'i':
            autoincrement = !autoincrement;
            Serial.print("Autoincrement mode ");
            if ( autoincrement ) {
              Serial.print(" ON\n");  
            } else {
              Serial.print(" OFF\n");
            }
          break;
        }
      break;
      case readingAddress:
        Serial.write(cmd);
        address *= 16;
        address += hexa2int(cmd);
        ++pos;
        if (pos == 4) {
          Serial.print("\n");
          state = waitingCommand;
        }
      break;

      case readingReadSize:
        Serial.write(cmd);
        readSize *= 16;
        readSize += hexa2int(cmd);
        ++pos;
        if (pos == 4) {
          Serial.print("\n");
          state = waitingCommand;
        }
      break;

      case readingData:
        Serial.write(cmd);
        data *= 16;
        data += hexa2int(cmd);
        ++pos;
        if (pos == 2) {
          Serial.print("\n");
          state = waitingCommand;
        }
      
      break;
    }
  }
}
