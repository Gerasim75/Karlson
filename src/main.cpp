#include "DHT_U.h"                                            // Подключаем библиотеку датчика влажности
#define DHTPIN 2                                              // Задаем имя константы для пина 2 (вход датчик давления)
#define RELEPIN 3                                             // Задаем имя константы для пина 3 (выход реле Карлсона)
#define KNOPKAPIN 4                                           // Задаем имя константы для пина 4 (вход кнопка)
#define DHTTYPE DHT11                                         // Задаем имя константы для типа датчика DHT11
//#define DHTTYPE DHT22                                         // Задаем имя константы для типа датчика DHT22
DHT dht(DHTPIN, DHTTYPE);                                     // Создаем объект класса "датчик"
boolean flagKnopka=false;                                     // Объявляем переменную для хранения состояния кнопки
boolean flagKarlson=false;                                    // Объявляем переменную выбора источника включения карлсона
unsigned long knopkaOnTime;                                   // Объявляем переменную хранения времени нажатия кнопки
unsigned long sensorTimer;                                    // Объявляем переменную хранения времени опроса датчика
unsigned long showTimer;                                      // Объявляем переменную хранения времени вывода в монитор
float h;                                                      // Объявляем переменную для хранения показаний влажности
float hMin=99.9;                                              // Объявляем переменную для хранения минимального значения влажности
int counterErrors=0;                                          // Объявляем переменную счетчик ошибок чтения данных с датчика
inline void Reset() {asm("JMP 0");}

void setup() 
{
  pinMode (DHTPIN, INPUT);                                    // Инициализируем пин 2 как входящий
  pinMode (RELEPIN, OUTPUT);                                  // Инициализируем пин 3 как исходящий
  pinMode (KNOPKAPIN, INPUT);                                 // Инициализируем пин 4 как входящий
  digitalWrite(RELEPIN, HIGH);                                // Выключаем Карлсона (для обратного реле), для прямого реле закомментировать
  Serial.begin(9600);                                         // Инициализируем интерфейс связи с монитором порта
  dht.begin();                                                // Инициализируем интерфейс связи с датчиком влажности
  Serial.println("DHT-sensor start!");                        // Выводим информацию о успешной загрузке контроллера
  delay(100);                                                 // Останавливаем программу до выхода датчика в рабочий режим
}
void loop()
{
  if(counterErrors>=5)                                        // Если количество ошибок чтения больше указанного
  {
    Serial.println("System Reloading...");
    Serial.println("...................");
    delay(500); 
    Reset();                                                  // Перезагружаем контроллер
  }
  if(millis()-sensorTimer>5000)                               // Задаем переодичность опроса датчика в мс
  {
    h=dht.readHumidity();                                     // Считываем в переменную показания влажности
    delay(50);                                                // Останавливаем программу до получения данных от датчика
    if (isnan(h))
    {
      Serial.println("Failed to read from DHT");              // Если считывание не удалось выводим сообщение в монитор порта
      counterErrors++;                                        // Увеличиваем счетчик ошибок на единицу
    }
     else                                                     // Если удалось
     {
        if(h<hMin&&h!=0) hMin=h;                              // Обучаем Карлсона нормальной влажности
        counterErrors=0;                                      // Сбрасываем счетчик ошибок
     }
    sensorTimer=millis();                                     // Инициализируем таймер текущим значением времени в мс
  }
  if(digitalRead(KNOPKAPIN)==LOW)                             // Если кнопка нажата
  {
    delay(400);                                               // Пауза для отсечения дребезга и корректировки времени нажатия на кнопку
    if(flagKnopka)                                            // Если это повторное нажатие
    {
      flagKnopka=false;                                       // Опускаем флаг нажатия кнопки
      return;                                                 // Выходим в начало главного цикла
    }
    flagKnopka=true;                                          // Поднимаем флаг нажатия кнопки
    knopkaOnTime=millis();                                    // Инициализируем переменную текущим временем в мс
  }
  if(millis()-knopkaOnTime>300000)                            // Если время работы таймера превысило .. мс, таймер выключения Карлсона
  {
    flagKnopka=false;                                         // Выключаем кнопку
  }
  if(h>=hMin+35||flagKnopka)                                  // Если превышено значение влажности или нажата кнопка
  {
    digitalWrite(RELEPIN, LOW);                               // Включаем Карлсона, для прямого реле установить HIGH   
    if(flagKnopka) flagKarlson=true;                          // Если включено с кнопки поднимаем флаг
  }
  if((h<=hMin+5&&!flagKnopka)||(!flagKnopka&&flagKarlson))    // Если значение влажности меньше указанного и кнопка не нажата                       
  {                                                           // или кнопка не нажата и поднят флаг включения Карлсона кнопкой
    digitalWrite(RELEPIN, HIGH);                              // Выключаем Карлсона, для прямого реле установить LOW  
    flagKarlson=false;                                        // Опускаем флаг включения Карлсона кнопкой
  }  

    if(millis()-showTimer>3000)                               // Задаем периодичность вывода данных в монитор порта
    {
      Serial.print("Humidity - ");                            // Выводим в монитор порта текущую влажность
      Serial.print(h);
      if(isnan(h))
        Serial.print("%    ");
      else
        Serial.print("%  "); 
      
      if(digitalRead(RELEPIN))
        Serial.print("Karlson - OFF  ");                      // Выводим в монитор порта состояние Карлсона
      else
        Serial.print("Karlson - ON   ");                      // Выводим в монитор порта состояние Карлсона
      
      if(flagKnopka) 
        Serial.print("Knopka - ON   ");                       // Выводим в монитор порта состояние кнопки
      else 
        Serial.print("Knopka - OFF  ");

      Serial.print("hMin - ");                               
      Serial.print(hMin);                                     // Выводим в монитор порта нормальную влажность
      Serial.print("%   "); 
      Serial.print("Errors - ");
      Serial.println(counterErrors);
      showTimer=millis();
    }
}