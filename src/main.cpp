#include "DHT_U.h"                                            // Подключаем библиотеку датчика влажности
#define DHTPIN 2                                              // Задаем имя константы для пина 2 (вход датчик давления)
#define RELAY_PIN 3                                           // Задаем имя константы для пина 3 (выход реле Карлсона)
#define BUTTON_PIN 4                                          // Задаем имя константы для пина 4 (вход кнопка)
#define DHTTYPE DHT11                                         // Задаем имя константы для типа датчика DHT11
//#define DHTTYPE DHT22                                       // Задаем имя константы для типа датчика DHT22
DHT dht(DHTPIN, DHTTYPE);                                     // Создаем объект класса "датчик"
boolean flag_button = false;                                  // Объявляем переменную для хранения состояния кнопки
boolean flag_karlson = false;                                 // Объявляем переменную выбора источника включения карлсона
unsigned long button_on_timer;                                // Объявляем переменную хранения времени нажатия кнопки
unsigned long sensor_timer;                                   // Объявляем переменную хранения времени опроса датчика
unsigned long show_timer;                                     // Объявляем переменную хранения времени вывода в монитор
float h;                                                      // Объявляем переменную для хранения показаний влажности
float hMin = 99.9;                                            // Объявляем переменную для хранения минимального значения влажности
int counter_errors = 0;                                       // Объявляем переменную счетчик ошибок чтения данных с датчика
inline void Reset() {asm("JMP 0");}

void setup() 
{
  pinMode (DHTPIN, INPUT);                                    // Инициализируем пин 2 как входящий
  pinMode (RELAY_PIN, OUTPUT);                                // Инициализируем пин 3 как исходящий
  pinMode (BUTTON_PIN, INPUT);                                // Инициализируем пин 4 как входящий
  digitalWrite(RELAY_PIN, HIGH);                              // Выключаем Карлсона (для обратного реле), для прямого реле закомментировать
  Serial.begin(9600);                                         // Инициализируем интерфейс связи с монитором порта
  dht.begin();                                                // Инициализируем интерфейс связи с датчиком влажности
  Serial.println("DHT-sensor start!");                        // Выводим информацию о успешной загрузке контроллера
  delay(100);                                                 // Останавливаем программу до выхода датчика в рабочий режим
}
void loop()
{
  if(counter_errors >= 5)                                     // Если количество ошибок чтения больше указанного
  {
    Serial.println("System Reloading...");
    Serial.println("...................");
    delay(500); 
    Reset();                                                  // Перезагружаем контроллер
  }
  if(millis() - sensor_timer > 5000)                          // Задаем переодичность опроса датчика в мс
  {
    h = dht.readHumidity();                                   // Считываем в переменную показания влажности
    delay(50);                                                // Останавливаем программу до получения данных от датчика
    if (isnan(h))
    {
      Serial.println("Failed to read from DHT");              // Если считывание не удалось выводим сообщение в монитор порта
      counter_errors++;                                       // Увеличиваем счетчик ошибок на единицу
    }
     else                                                     // Если удалось
     {
        if(h < hMin && h != 0) hMin = h;                      // Обучаем Карлсона нормальной влажности
        counter_errors = 0;                                   // Сбрасываем счетчик ошибок
     }
    sensor_timer = millis();                                  // Инициализируем таймер текущим значением времени в мс
  }
  if(digitalRead(BUTTON_PIN) == LOW)                          // Если кнопка нажата
  {
    delay(400);                                               // Пауза для отсечения дребезга и корректировки времени нажатия на кнопку
    if(flag_button)                                           // Если это повторное нажатие
    {
      flag_button = false;                                    // Опускаем флаг нажатия кнопки
      return;                                                 // Выходим в начало главного цикла
    }
    flag_button = true;                                       // Поднимаем флаг нажатия кнопки
    button_on_timer = millis();                               // Инициализируем переменную текущим временем в мс
  }
  if(millis() - button_on_timer > 300000)                     // Если время работы таймера превысило .. мс, таймер выключения Карлсона
  {
    flag_button = false;                                      // Выключаем кнопку
  }
  if(h >= hMin + 35 || flag_button)                           // Если превышено значение влажности или нажата кнопка
  {
    digitalWrite(RELAY_PIN, LOW);                             // Включаем Карлсона, для прямого реле установить HIGH   
    if(flag_button) flag_karlson = true;                      // Если включено с кнопки поднимаем флаг
  }
  if((h <= hMin + 5 && !flag_button) || (!flag_button && flag_karlson))    // Если значение влажности меньше указанного и кнопка не нажата                       
  {                                                           // или кнопка не нажата и поднят флаг включения Карлсона кнопкой
    digitalWrite(RELAY_PIN, HIGH);                            // Выключаем Карлсона, для прямого реле установить LOW  
    flag_karlson = false;                                     // Опускаем флаг включения Карлсона кнопкой
  }  

    if(millis() - show_timer > 3000)                          // Задаем периодичность вывода данных в монитор порта
    {
      Serial.print("Humidity - ");                            // Выводим в монитор порта текущую влажность
      Serial.print(h);
      if(isnan(h))
        Serial.print("%    ");
      else
        Serial.print("%  "); 
      
      if(digitalRead(RELAY_PIN))
        Serial.print("Karlson - OFF  ");                      // Выводим в монитор порта состояние Карлсона
      else
        Serial.print("Karlson - ON   ");                      // Выводим в монитор порта состояние Карлсона
      
      if(flag_button) 
        Serial.print("Knopka - ON   ");                       // Выводим в монитор порта состояние кнопки
      else 
        Serial.print("Knopka - OFF  ");

      Serial.print("hMin - ");                               
      Serial.print(hMin);                                     // Выводим в монитор порта нормальную влажность
      Serial.print("%   "); 
      Serial.print("Errors - ");
      Serial.println(counter_errors);
      show_timer = millis();
    }
}
