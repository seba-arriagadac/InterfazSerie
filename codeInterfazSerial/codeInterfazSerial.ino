/**
 * @file codeInterfazSerial.ino
 * @version 1.0
 * @date 05/02/2020
 * @author Sebastián Arriagada C.
 * @brief Interfáz realizada con arduino que permite enviarle 
 * Strings al puerto serial y los muestra por una pantalla lcd, 
 * además tiene integrada la funcionalidad de eco (si se envía 
 * como primer caracter un '#') controlada por un pulsador
 * @note Configuration:
 *                 +5V
 * ____________     |    ______________
 *            |     |    |
 *          5V|-----+----|VDD
 *            |     +----|A     LCD
 *            |          |     MODULE
 * arduino  07|----------|RS
 *          08|----------|EN
 *          09|----------|D4
 *          10|----------|D5
 *          11|----------|D6
 *          12|----------|D7
 *            |          |
 *         GND|-------+--|VSS
 *            |       +--|RW
 * ____13_____|       +--|K
 *     |              |  |_________Vo____
 *    pull            |            |
 *    down           GND          pot
 *   button                       10K
 * 
 */
#include <LiquidCrystal.h>
#define DBG(a)  Serial.println(a);
#define MAX_LCD_LENGTH  15
#define MINIMO(A,B) ( A < B ) ? A : B
#define BUTTON_PIN 13
#define TRIGGER_SHIFT 750
#define TRIGGER_ANTIREBOTE 100

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
  static bool timeOutAntiRebote = false;//Para antirrebote
  static bool buttonPressed = false; // Para antirrebote
  static int cursorVirtual = 0; //Para mostrar string largo por LCD
  static int dataRcvLength;
  static unsigned long t_initShiftLcd = millis(); //Para leer por LCD string largo (shift)
  static unsigned long t_initAntiRebote = millis(); // Para antirrebote
  static String dataRcv;
  static String data2Send = "02t010020030040050060070080090100";

  if((millis()-t_initShiftLcd)>TRIGGER_SHIFT){
    timeOutShiftLcd = true;
    t_initShiftLcd = millis();
  }
  if((millis()-t_initAntiRebote)>TRIGGER_ANTIREBOTE){
    timeOutAntiRebote = true;
    t_initAntiRebote = millis();
  }
  if ((digitalRead(BUTTON_PIN) == HIGH) && (!buttonPressed) && timeOutAntiRebote){
    buttonPressed = true;
    timeOutAntiRebote = false;
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
    //si el string cabe en la LCD lo muestra directamente, sino lo va corriendo para leerlo
    if (dataRcvLength > MAX_LCD_LENGTH) {
      msgLongRcv = true;
    }
    else {
      msgLongRcv = false;
      softClear(0);
      lcd.print(dataRcv);
    }
  }
  //El corrimiento del string si es largo
  if (msgLongRcv && timeOutShiftLcd){
    timeOutShiftLcd = false;
    softClear(0);
    lcd.print(dataRcv.substring(cursorVirtual, MINIMO((cursorVirtual+MAX_LCD_LENGTH), dataRcvLength)));
    cursorVirtual = (++cursorVirtual) % dataRcvLength;
    //DBG(cursorVirtual);
  }
  //se envía el string preparado al presionar el botón
  if (buttonPressed && timeOutAntiRebote){ 
    buttonPressed = false;
    timeOutAntiRebote = false;
    Serial.println(data2Send);
  }
}
/**
 * @fn softClear
 * @brief "Borra" la linea/fila indicada
 * @param fila Entero con la fila a borrar
 */
void softClear(int fila){
  lcd.setCursor(0, fila);
  lcd.print("                ");
  lcd.setCursor(0, fila);
}