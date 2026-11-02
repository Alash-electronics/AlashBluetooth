// [ru] AlashBluetooth: текстовые команды от телефона. Приложение BLE Controller (режим Terminal) или любой Bluetooth-терминал
//      шлёт строки вроде "led=1", "speed=80", "stop". Библиотека собирает строку без блокировки и разбирает "имя=число".
//      Светодиод включается командой led=1 и выключается led=0. Команда speed=N печатается в монитор порта.
// [kk] AlashBluetooth: телефоннан мәтіндік командалар. BLE Controller қолданбасы (Terminal режимі) немесе кез келген Bluetooth-терминал
//      "led=1", "speed=80", "stop" сияқты жолдар жібереді. Кітапхана жолды блоктамай жинайды және "аты=сан" талдайды.
//      Жарықдиод led=1 командасымен қосылады, led=0 командасымен өшеді. speed=N командасы порт мониторына басылады.
// [en] AlashBluetooth: text commands from a phone. The BLE Controller app (Terminal mode) or any Bluetooth terminal
//      sends lines like "led=1", "speed=80", "stop". The library collects the line without blocking and parses "name=number".
//      The LED switches on with the command led=1 and off with led=0. The command speed=N is printed to the serial monitor.
#include <AlashBluetooth.h>
#if defined(ESP32)
#define BT Serial2                         // ESP32: аппаратный порт Serial2, RX = GPIO16, TX = GPIO17
void setBaud(long b) { Serial2.begin(b, SERIAL_8N1, 16, 17); }
#else
#include <SoftwareSerial.h>
SoftwareSerial BT(10, 11);                 // Uno / Nano: RX = D10 (сюда TX модуля), TX = D11 (к RX модуля через делитель 1 кОм / 2 кОм)
void setBaud(long b) { BT.begin(b); }
#endif
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

AlashBluetooth bluetooth(BT);

void setup() {
  Serial.begin(115200);
  setBaud(9600);                           // скорость обмена обычного режима (у HC-06 и HM-10 9600 по умолчанию)
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  char line[32], name[16];
  long value;
  if (bluetooth.readLine(line, sizeof line)) {                 // строка закончилась символом \n, \r или ;
    if (AlashBluetooth::parseCommand(line, name, sizeof name, &value)) {
      if (strcmp(name, "led") == 0) digitalWrite(LED_BUILTIN, value ? HIGH : LOW);
      Serial.print(name);
      Serial.print(" = ");
      Serial.println(value);
      BT.print("ok ");                                          // ответ телефону
      BT.println(name);
    }
  }
}
