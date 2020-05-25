#define DELAY 25

enum State { waitingCommand, readingAddress, readingData, readingReadSize };


/*
 * portB 0-7 <-> data bus
 * 
 * portC 0  -> A8
 * portC 1  -> A9
 * portC 2  -> A10
 * portC 3  -
 * portC 4  -> CE 
 * portC 5  -> OE
 * portC 6 <- RESET
 * portC 7 --- N/A
 * 
 * portD 0 <-  Rx
 * portD 1  -> Tx
 * portD 2  - 
 * portD 3  - 
 * portD 4  - 
 * portD 5  -> BIT
 * portD 6  -> CLK
 * portD 7  -> 5/25 v selector for Vpp
 * 
 */


// use when reading
void CE_Disable() {
  PORTC |= B00010000;
}

// use when reading
void CE_Enable() {
  PORTC &= B11101111;
}

// use when writing
void CE_High() {
  CE_Disable();
}

// use when writing
void CE_Low() {
  CE_Enable();
}

// use when reading
void OE_Disable() {
  PORTC |= B00100000;
}

// use when reading
void OE_Enable() {
  PORTC &= B11011111;
}

// use when writing
void OE_High() {
  OE_Disable();  
}

// use when writing
void OE_Low() {
  OE_Enable();  
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
  DDRC |= B00110000;      // OE, CE
  DDRD |= B10000000;      // r/w

}

void setModeRead() {
  PORTD &= B01111111;
}

void setModeWrite() {
  PORTD |= B10000000;
}



void addressBusEnable() {
  DDRC |= B00000111;      // address bus[10:8]
  DDRD |= B01100000;      // serial control for address bus[7:0]
}

void fixBaudRateDivisor() {
  UBRR0L = B01100111;
}

void serialClockUp() {
  PORTD |= B01000000;
  delay(1);
}

void serialClockDown() {
  PORTD &= B10111111;
  delay(1);
}



void serialToParallel (byte address) {
  serialClockUp();
  for (unsigned int index = 128; index > 0; index /=2 ) {
     if ( address & index ) {
       PORTD |= B00100000;
     } else {
       PORTD &= B11011111;
     }
    //delay(5);
    serialClockDown();
    //delay(5);
    serialClockUp();
  }
  
}

void setAddress(word address) {
  PORTC &= B11111000;

  byte lowAddress = lowByte(address);
  byte highAddress =  highByte(address);
  
  serialToParallel(lowAddress);

  // address 8 PORTC0
  if ( highAddress % 2 ) {
    PORTC |= B00000001;
  }

  // address 9 PORTC1
  highAddress /= 2;
  if ( highAddress % 2 ) {
    PORTC |= B00000010;
  }

  // address 10 PORTC2
  highAddress /= 2;
  if ( highAddress % 2 ) {
    PORTC |= B00000100;
  }
}

void printMenu(int address, byte data, unsigned int readSize, bool autoincrement) {
  Serial.print("#####################################################\n");
  Serial.print("MENU Version 05 full memory\n");
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

void setup() {
  Serial.begin(9600);
  fixBaudRateDivisor();
  dataBusAsInput();
  CE_Disable();
  OE_Disable();
  controlLinesEnable();
  addressBusEnable();
  setModeRead();

  Serial.write("\n\nInit Atmega328p EPROM writer v0.5\n");
}



State state = waitingCommand;

byte pos;
byte data;
word address;
bool autoincrement = false;
unsigned int readSize = 1;
//bool writing = false;

void loop() {
  byte col = 0;
  if (Serial.available()) {
    char cmd = Serial.read();
    switch(state) {
      case waitingCommand :
        switch (cmd) {
          case 'w': // write
            Serial.print("\nWriting ");
            Serial.print(data, HEX);
            Serial.print(" in ");
            Serial.print(address, HEX);
            Serial.print("\n");

            Serial.print("ok 0\n");
            setModeWrite();
            
            delay(100);
            Serial.print("ok 1\n");
            OE_Low();

            delay(1);
            //Serial.print("ok 2  OE_low\n");            
            CE_Low();
            delay(1);
            //Serial.print("ok 3 CE_low\n");
            setAddress(address);
            delay(1);
            //Serial.print("ok 4  setAddress\n");
            dataBusAsOutput();
            delay(1);
            //Serial.print("ok 5 dataBusAsOutput\n");
            PORTB = data;
            delay(1);
            //Serial.print("ok 6 portb = data\n");
            OE_High();
            delay(1);   // Toes 2us
            //Serial.print("ok 7  OE_high\n");
            CE_High();
            delay(23);  // Tpw 50us
            Serial.print("ok 8 CE_high Tpw\n");
            CE_Low();
            
            //delay(1);
            Serial.print("ok 9  CE_Low\n");
            OE_Low();
            delay(1);
            Serial.print("ok 10 OE_low 2\n");
            
            
            Serial.print("ok\n");
            if (autoincrement) {
              ++address;
            }
            setModeRead();

            state = waitingCommand;
          break;
                    
          case 'r': // read
            if (readSize == 1) {
              Serial.print("\nReading address ");
              printAligned(address,4);
              Serial.print(" : ");
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

            setModeRead();
            delay(100);  
            
            
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
                printAligned(address + 1,4);
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
