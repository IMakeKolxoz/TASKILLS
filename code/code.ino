                                                        //библиотеки
#include <ESP8266WiFi.h>                                //специальные методы для работы ESP8266 с WiFi
#include <Wire.h>                                       //связь по i2c   
#include <LiquidCrystal_I2C.h>                          //работа с экраном
#include <NTPClient.h>                                  //синхронизацию с NTP-сервером
#include <WiFiUdp.h>                                    //обрабатывает протокол UDP
#include <IRremote.h>                                   //для IR приемника

#define rele 3
 
const char* ssid = "Kvantorium75";                      // Название Вашей WiFi сети
const char* password = "TechKvant752019";               // Пароль от Вашей WiFi сети
 
WiFiServer server(80);                                  // Указываем порт Web-сервера
String header;

WiFiUDP ntpUDP;                                         //создание вещей для ntpUDP
NTPClient timeClient(ntpUDP);                  

LiquidCrystal_I2C lcd(0x27, 20, 4);                     //пишем адрес и размер экрана

IRrecv irrecv(D4);                                      // указываем пин, к которому подключен IR приемник
decode_results results;


int timer = 0;
unsigned long HEXcoman;
 
void setup() {

  pinMode(rele, OUTPUT);

  Serial.begin(115200);                                 
  bool status;
                                                       
  if (0) {                                              // Проверка инициализации датчика
    Serial.println("Could not find a valid BME280 sensor, check wiring!"); 
    while (1);                                          
  }
 
  Serial.print("Connecting to ");                       // Отправка в Serial port 
  Serial.println(ssid);                                 
  WiFi.begin(ssid, password);                           // Подключение к WiFi Сети
  while (WiFi.status() != WL_CONNECTED) {               // Проверка подключения к WiFi сети
    delay(500);                                         
    Serial.print(".");                                  
  }
 
  Serial.println("");                                   // Отправка в Serial port 
  Serial.println("WiFi connected.");                    
  Serial.println("IP address: ");                       
  Serial.println(WiFi.localIP());                        
  server.begin(); 

  timeClient.begin(); // получаем вренмя 
  timeClient.setTimeOffset(32400); // указываем часовой пояс (GMT +1 = 3600)


  lcd.init();
  // включаем подсветку
  lcd.backlight();

  irrecv.enableIRIn(); // запускаем прием
}
 
void loop(){
  
  timeClient.update();                                  // обновляем время
  //Serial.println(timeClient.getFormattedTime());        //выводим время
  lcd.setCursor(0, 0);
  lcd.print(timeClient.getFormattedTime());
  lcd.setCursor(0, 1);
  lcd.print(String(HEXcoman, HEX));

  if ( irrecv.decode( &results )) { // если данные пришли
    Serial.println( results.value, HEX ); // печатаем данные
    Serial.println( results.value);
    irrecv.resume(); // принимаем следующую команду
    HEXcoman = (results.value);
    Serial.println(HEXcoman);
  }

  WiFiClient client = server.available();               // Получаем данные, посылаемые клиентом 
 
  if (client) {                                         
    Serial.println("New Client.");                      // Отправка "Новый клиент"
    String currentLine = "";                            // Создаем строку для хранения входящих данных от клиента
    while (client.connected()) {                        // Пока есть соединение с клиентом 
      if (client.available()) {                         // Если клиент активен 
        char c = client.read();                         // Считываем посылаемую информацию в переменную "с"
        Serial.write(c);                                // Отправка в Serial port 
        header += c;
        if (c == '\n') {                                // Вывод HTML страницы 
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");          
            client.println("Content-type:text/html ");
            client.println("Connection: close");        // Соединение будет закрыто после завершения ответа
            client.println("Refresh: 10");              // Автоматическое обновление каждые 10 сек 
            client.println();
            
            client.println("<!DOCTYPE html><html>");    // Веб-страница создается с использованием HTML
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<meta charset='UTF-8'>");   // Делаем русскую кодировку
            client.println("<link rel=\"icon\" href=\"data:,\">");
                     
            client.println("<style>body { text-align: center; font-family: \"Trebuchet MS\", Arial;}");
            client.println("table { border-collapse: collapse; width:50%; margin-left:auto; margin-right:auto; }");
            client.println("th { padding: 12px; background-color: #0043af; color: white; }");
            client.println("tr { border: 1px solid #ddd; padding: 12px; }");
            client.println("tr:hover { background-color: #bcbcbc; }");
            client.println("td { border: none; padding: 12px; }");
            client.println(".sensor { color:0043af; font-weight: bold;}");
            
            client.println("</style></head><body><h1>Умная рjзетка</h1>");
            client.println("<table><tr><th>Параметр</th><th>Показания</th></tr>");
            client.println("<tr><td>Время</td><td><span class=\"sensor\">");
            client.println(timeClient.getFormattedTime());  
            client.println("<tr><td>Таймер</td><td><span class=\"sensor\">");
            client.println(timer);
            client.println(" Мин</span></td></tr>");
            client.println("<tr><td>ERROR</td><td><span class=\"sensor\">");
            client.println("0");
            client.println("</body></html>");
            
            client.println();
            break;
          } else { 
            currentLine = "";
          }
        } else if (c != '\r') {  
          currentLine += c;      
        }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

  
}
