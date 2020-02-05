#include <LiquidCrystal.h>
#define DBG(a)  Serial.println(a);
#define MAX_LCD_LENGTH  15
#define MINIMO(A,B) ( A < B ) ? A : B
#define BUTTON_PIN 13

LiquidCrystal lcd(7, 8, 9, 10, 11, 12); //    ( RS, EN, d4, d5, d6, d7)

void setup()
{
  lcd.begin(16, 2); //Inicializa la pantalla con el numero de caracteres y de filas
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT);
  lcd.print(" Interfaz serie"); //Pantalla de inicio
  lcd.setCursor(0,1);
  lcd.print("    Iniciada");
}

void loop()
{
  static bool msgLongRcv = false;
  static bool timeOutShiftLcd = false;
  static bool timeOutAntiRebote = false;
  static bool buttonPressed = false;
  static int cursorUp = 15;
  static int cursorVirtual = 0;
  static int dataRcvLength;
  static unsigned long t_initShiftLcd = millis();
  static unsigned long t_initAntiRebote = millis();
  static String dataRcv;
  static String data2Send = "02t010020030040050060070080090100";

  static int i_btnPrss = 0;

  if((millis()-t_initShiftLcd)>750){
    timeOutShiftLcd = true;
    t_initShiftLcd = millis();
  }
  if((millis()-t_initAntiRebote)>100){
    timeOutAntiRebote = true;
    t_initAntiRebote = millis();
  }
  if ((digitalRead(BUTTON_PIN) == HIGH) && (!buttonPressed) && timeOutAntiRebote){
    buttonPressed = true;
    timeOutAntiRebote = false;
    i_btnPrss++;
    DBG("i btn prss");
    DBG(i_btnPrss);
  }

  if (Serial.available()) {
    dataRcv = "";//Se vacían los datos recibidos
    cursorVirtual = 0;
    dataRcv = Serial.readStringUntil('\n');
    delay(5);
    if (dataRcv.charAt(0) == '#'){
      data2Send = dataRcv.substring(1);
      softClear(1);
      lcd.print(data2Send);
    }
    dataRcvLength = dataRcv.length();
    if (dataRcvLength > MAX_LCD_LENGTH) {
      msgLongRcv = true;
    }
    else {
      msgLongRcv = false;
      softClear(0);
      lcd.print(dataRcv);
    }
  }
  if (msgLongRcv && timeOutShiftLcd){
    timeOutShiftLcd = false;
    softClear(0);
    lcd.print(dataRcv.substring(cursorVirtual, MINIMO((cursorVirtual+MAX_LCD_LENGTH), dataRcvLength)));
    cursorVirtual = (++cursorVirtual) % dataRcvLength;
    //DBG(cursorVirtual);
  }
  if (buttonPressed && timeOutAntiRebote){ 
    DBG("ok   send");
    buttonPressed = false;
    timeOutAntiRebote = false;
    Serial.println(data2Send);
  }
  

}
void softClear(int fila){
  lcd.setCursor(0, fila);
  lcd.print("                ");
  lcd.setCursor(0, fila);
}