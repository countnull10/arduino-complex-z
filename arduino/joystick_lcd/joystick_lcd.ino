/*
 The LiquidCrystal library works with all LCD displays that are
 compatible with the Hitachi HD44780 driver. There are many of
 them out there, and you can usually tell them by the 16-pin interface.

  The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 * 
 * AnalogStick
 * GND pin to ground
 * +5V pin to 5V
 * VRx pin to A0
 * VRy pin to A1
*/

#include <LiquidCrystal.h>
#include <math.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
}

void loop() {
  // set the cursor to column 0, line 1

  char buffer[17];
  char buffer2[8];
 
  // read the input on analog pin 0 and 1:
  int raw_x (analogRead(A0)-512);
  int raw_y (analogRead(A1)-512);
  
  int x = (raw_x / 50);
  int y = (raw_y / 50);
 
  // 1. Complex Number
  lcd.setCursor(0, 0);
  char sign1 = (x >= 0) ? ' ' : '-';
  char sign2 = (y >= 0) ? '+' : '-';
  sprintf(buffer, "z =%c%d %c %di    ", sign1, abs(x), sign2, abs(y));
  lcd.print(buffer);

  // Avoid division by zero if x == 0
  double fx = (x == 0) ? 0.0001 : (double)x;
  double fy = (double)y;

  // Correct calculations (using float/double and atan for the angle)
  double r = sqrt((double)(x * x + y * y)); // Multiplication is more efficient than pow()
 
  // Arc tangent returns the angle in radians
  double teta = atan2(fy, fx);

  // 2. Degrees
  double grad;

  if (x == 0 && y == 0) {
    grad = 0;
  } else {
    grad = ((teta * 180)/3.14)+180;
  }
    
  dtostrf(grad, 6, 0, buffer2);
  sprintf(buffer, "Deg =%s    ", buffer2);

  // print degrees
  lcd.setCursor(0, 1);
  lcd.print(buffer);

  //Send serial data
  sprintf(buffer, "%d,%d", raw_x, raw_y);
  Serial.println(buffer);
  
  delay(100);
}
