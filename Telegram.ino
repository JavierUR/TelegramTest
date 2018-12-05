#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

//Pines a utilizar
int PinInput = A0;
int PinParlante = D7;
int PinRele = D4;
//Constantes
int R2 = 3900;
//Variables
int ledStatus = 0;
float R1;
int long lt = 0;
String ipAddress = "";
//Configuracion WIFI 
const char* SSID_NAME = "REPLACE"; // Nombre SSID
const char* SSID_PASS = "REPLACE"; // Contraseña



//SSL CLient
WiFiClientSecure client;


// ------- Telegram config --------
#define BOT_TOKEN "REPLACE"  // your Bot Token (Get from Botfather)
#define CHAT_ID "REPLACE" // Chat ID of where you want the message to go (You can use MyIdBot to get the chat ID)

//Conectar al bot
UniversalTelegramBot bot(BOT_TOKEN, client);


int Bot_mtbs = 1000; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done
bool Start = false;

//mapFloat:Funcion map para numeros float
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

WiFiClient clientUbi;

void setup() {
  //Configurar pines digitales
  pinMode(PinParlante, OUTPUT);
  pinMode(PinRele, OUTPUT);
  //Conexion Serial y wifi
  Serial.begin(9600);
  WiFi.begin(SSID_NAME, SSID_PASS);
  while (WiFi.status() != WL_CONNECTED) {//Esperar conexion
    delay(500);
    Serial.print(".");
  }
  pinMode(PinRele, OUTPUT);
  WiFi.setAutoReconnect(true);
  Serial.println(F("WiFi connected"));
  IPAddress ip = WiFi.localIP();
  Serial.println(F("IP address: "));
  Serial.println(ip);
  ipAddress = ip.toString();
}

void loop() {

  //Lectura de la luz
  if (millis() - lt > 100) {
    int reading = analogRead(PinInput);
    R1 = R2 * ((1024.0 / reading) - 1);
    //Serial.println(R1);
    lt = millis();
  }
  //Verifica mensajes nuevos y los maneja
  if (millis() > Bot_lasttime + Bot_mtbs)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    Bot_lasttime = millis();
  }

  if (R1 > 14000) { //Envía un mensaje si esta oscuro
    sendTelegramMessage();
  }

}

//Hace sonar el parlante en dos tonos como alarma
void alarma() {
  tone(PinParlante, 400);
  delay(500);
  noTone(PinParlante);
  delay(30);
  tone(PinParlante, 800);
  delay(500);
  noTone(PinParlante);
  delay(30);
}


//Mensaje Telegram con la ip y notfica que no hay luz
void sendTelegramMessage() {
  String message = "SSID:  ";
  message.concat(SSID_NAME);
  message.concat("\n");
  message.concat("IP: ");
  message.concat(ipAddress);
  message.concat("\n");
  message.concat("No hay luz");
  message.concat("\n");
  if (bot.sendMessage(CHAT_ID, message, "Markdown")) { //Revisa si se envió el mensaje
    Serial.println("TELEGRAM Successfully sent");
  }
}

//Funcion que maneja nuevos mensajes
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";

    if (text == "/ledon") {
      digitalWrite(PinRele, HIGH);   // Enciende la ampolleta
      ledStatus = 1;
      bot.sendMessage(chat_id, "Ampolleta Encendida", "");
    }

    if (text == "/ledoff") {
      ledStatus = 0;
      digitalWrite(PinRele, LOW);    // Apaga la ampolleta
      bot.sendMessage(chat_id, "Ampolleta Apagada", "");
    }

    if (text == "/status") { // Notifica el estado de la ampolleta
      if (ledStatus) {
        bot.sendMessage(chat_id, "Ampolleta Encendida", "");
      } else {
        bot.sendMessage(chat_id, "Ampolleta Apagada", "");
      }
    }
    if (text == "/alarma") { //Hace sonar la alarma
      alarma();
      bot.sendMessage(chat_id, "Alarma sonando", "");
    }
    if (text == "/start") { //Mensaje con los comaandos soportados
      String welcome = "Bot para lab 5, " + from_name + ".\n";
      welcome += "Ejmplo de Bot.\n\n";
      welcome += "/ledon : Eniende la ampolleta\n";
      welcome += "/ledoff : Apaga la ampolleta\n";
      welcome += "/status : Entrega el estado de la ampolleta\n";
      welcome += "/alarma : Suena la alarma";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
  }
}
