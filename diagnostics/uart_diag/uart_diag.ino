void setup() {
  //Serial.begin(9600); 
  Serial.begin(19200); 
  DDRC = DDRC | B00111111;
  DDRB = 0xff;
  UBRR0H = B00000000;
  UBRR0L = B01100111;

  
}

void loop() {
    
  //page 181

  
  PORTB = UCSR0A;
  delay(1000);
  PORTB = UCSR0B;
  delay(1000);
  PORTB = UCSR0C;
  delay(1000);
  PORTB = UBRR0H;
  delay(1000);
  PORTB = UBRR0L;
  delay(1000);

  for (unsigned char i=0; i< 8; ++i) {
    PORTB = 0;
    delay(250);
    PORTB = 0xff;
    delay(250);
    Serial.write('a');
  }
  
  
  
}
