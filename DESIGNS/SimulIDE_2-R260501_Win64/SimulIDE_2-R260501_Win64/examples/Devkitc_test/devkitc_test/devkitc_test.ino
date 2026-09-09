

#include <LiquidCrystal_AIP31068_I2C.h>
#include <Adafruit_PCD8544.h>

#define BUTTON 13
#define BUTPIN 12
#define BLINK  27
#define PWMPIN 17

int duty = 0;
bool rise = true;

LiquidCrystal_AIP31068_I2C lcd( 0x3E, 16, 2 );

Adafruit_PCD8544 display = Adafruit_PCD8544( /*18, 23,*/ 16, 5, 19);

void setup()
{
  Serial.begin(115200);

  pinMode( BUTTON, INPUT );
  pinMode( BLINK, OUTPUT );
  pinMode( BUTPIN, OUTPUT );
  pinMode( PWMPIN, OUTPUT );

  Serial.println(" ");
  Serial.print("MOSI: ");
  Serial.println(MOSI);
  Serial.print("MISO: ");
  Serial.println(MISO);
  Serial.print("SCK: ");
  Serial.println(SCK);
  Serial.print("SS: ");
  Serial.println(SS);

  lcd.init();
  lcd.setCursor(3,0);
  lcd.print("Hello, world!");
  delay( 1500 );
  
  display.begin();
  delay( 1500 );

  lcd.clear();
}

void loop() 
{
  display.clearDisplay();
  if( digitalRead( BUTTON ) == LOW)
  {
    Serial.println("BUTTON Low");
    digitalWrite( BUTPIN, LOW);
    delay( 50 );
    lcd.setCursor(3,0);
    lcd.print("BUTTON Low ");
    display.setCursor(10,10);
    display.println("BUTTON Low ");
  }
  else
  {
    Serial.println("BUTTON High");
    digitalWrite( BUTPIN, HIGH);
    delay( 50 );
    lcd.setCursor(3,0);
    lcd.print("BUTTON High");
    display.setCursor(10,10);
    display.println("BUTTON High");
  }
  delay( 50 );
  display.display();
  delay( 400 );
  if( rise ) {
    duty += 100000;
    if( duty > 1048500 ){
      duty = 1048500;
      rise = false;
    } 
  }else{
    duty -= 100000;
    if( duty < 100000 ) rise = true;
  }
  analogWrite( PWMPIN, duty );
  digitalWrite( BLINK, HIGH);
  delay( 500 );
  digitalWrite( BLINK, LOW);
}
