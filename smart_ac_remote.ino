#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
extern "C" {
  #include "user_interface.h"
}

const char* ssid = "";
const char* password = "";

const char* mqtt_server = "192.168.1.172";
const char* mqtt_user = "";
const char* mqtt_pass = "";

// --- CODICI IR (LG) ---
const uint32_t IR_ON_CODE = 0x8800549;
const uint32_t IR_OFF_CODE = 0x88C0051;
// Tabella dei codici: [Temperatura (0-7 per 18-25)][Ventola (0-3)]
const uint32_t codici_lg[8][4] = {
  {0x880830B, 0x880832D, 0x880834F, 0x8808350}, // 18 gradi (esempi)
  {0x880840C, 0x880842E, 0x8808440, 0x8808451}, // 19 gradi
  {0x880850D, 0x880852F, 0x8808541, 0x8808552}, // 20 gradi
  {0x880860E, 0x8808620, 0x8808642, 0x8808653}, // 21 gradi
  {0x880870F, 0x8808721, 0x8808743, 0x8808754}, // 22 gradi
  {0x8808800, 0x8808822, 0x8808844, 0x8808855}, // 23 gradi
  {0x8808901, 0x8808923, 0x8808945, 0x8808956}, // 24 gradi
  {0x8808A02, 0x8808A24, 0x8808A46, 0x8808A57}  // 25 gradi
};

//Velocità ventola in modalità deumidificatore
const uint32_t fan1_deum = 0x8809801;
const uint32_t fan2_deum  = 0x8809823;
const uint32_t fan3_deum  = 0x8809845;
const uint32_t fan_R_deum  = 0x8809856;
//DEUM e FREDDO
const uint32_t freddo  = 0x8808541;
const uint32_t deum  = 0x8809801;
//Da 18° aumento fino a 22°
const uint32_t t18  = 0x880834F;
const uint32_t t19  = 0x8808440;
const uint32_t t20  = 0x8808541;
const uint32_t t21  = 0x8808642;
const uint32_t t22  = 0x8808743;
const uint32_t t23  = 0x8808844;
const uint32_t t24  = 0x8808945;
const uint32_t t25  = 0x8808A46;

bool is_deum=false; //variabile che se è a true il condizionatore è in modalità deumidificatore
int temperature=18;
int ventola=3;
const uint16_t kIrLed = 4; // Pin D2 sulla Wemos
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//const uint16_t kIrLed = D1; // Pin D1 sulla Wemos
//////////////////////////////////////////////////////////////////////////////////////////////////////////
IRsend irsend(kIrLed);
WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connessione a ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connesso!");
  Serial.print("IP Wemos: ");
  Serial.println(WiFi.localIP());
}

void set_ventola_deum(){
  if(ventola == 1){
    irsend.sendLG(fan1_deum);
  }else if(ventola == 2){
    irsend.sendLG(fan2_deum);
  }else if(ventola == 3){
    irsend.sendLG(fan3_deum);
  }else if(ventola == 4){
    irsend.sendLG(fan_R_deum);
  }
}

void set_temp_fun(){
  if(is_deum){
    set_ventola_deum();
  }else{
    uint32_t codice = codici_lg[temperature - 18][ventola - 1];
    irsend.sendLG(codice);
  }
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Comando ricevuto [");
  Serial.print(topic);
  Serial.print("]: ");
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);
  bool comandoReale = true; // Flag per inviare IR solo se serve
//ON/OFF
  if (message == "ON") {
    Serial.println("AZIONE: Invio segnale ACCENSIONE LG...");
    irsend.sendLG(IR_ON_CODE);
  }
  else if (message == "OFF") {
    Serial.println("AZIONE: Invio segnale SPEGNIMENTO LG...");
    irsend.sendLG(IR_OFF_CODE);
  
//Velocità ventola
  }else if (message == "FAN1") {
    Serial.println("AZIONE: Invio segnale FAN1 LG...");
    ventola = 1;
  }else if (message == "FAN2") {
    Serial.println("AZIONE: Invio segnale FAN2 LG...");
    ventola = 2;
  }else if (message == "FAN3") {
    Serial.println("AZIONE: Invio segnale FAN3 LG...");
    ventola = 3;
  }else if (message == "FAN_R") {
    Serial.println("AZIONE: Invio segnale FAN_R LG...");
    ventola = 4;
//DEUM e FREDDO
  }else if (message == "DEUM" || message == "Deumidificatore") {
    Serial.println("AZIONE: Invio segnale DEUM LG...");
    is_deum=true;
    irsend.sendLG(deum);
    delay(1000);
  } 
  else if (message == "FREDDO" || message == "Freddo") {
    Serial.println("AZIONE: Invio segnale FREDDO LG...");
    is_deum=false;
    irsend.sendLG(freddo);
    delay(1000);
//cambio temperatura
  }else if (message == "t18") {
    Serial.println("AZIONE: Invio segnale t18 LG...");
    is_deum=false;
    temperature=18;
  }else if (message == "t19") {
    Serial.println("AZIONE: Invio segnale t19 LG...");
    is_deum=false;
    temperature=19;
  }else if (message == "t20") {
    Serial.println("AZIONE: Invio segnale t20 LG...");
    is_deum=false;
    temperature=20;
  }else if (message == "t21") {
    Serial.println("AZIONE: Invio segnale t21 LG...");
    is_deum=false;
    temperature=21;  
  }else if (message == "t22") {
    Serial.println("AZIONE: Invio segnale t22 LG...");
    is_deum=false;
    temperature=22;
  }else if (message == "t23") {
    Serial.println("AZIONE: Invio segnale t23 LG...");
    is_deum=false;
    temperature=23;
  }else if (message == "t24") {
    Serial.println("AZIONE: Invio segnale t24 LG...");
    is_deum=false;
    temperature=24;
  }else if (message == "t25") {
    Serial.println("AZIONE: Invio segnale t25 LG...");
    is_deum=false;
    temperature=25;
  }else if (message == "PING") {
     ////////////////////////////////////////////////////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////////////////////////////////////
      //Da cambiare nomi
    // Il Wemos conferma di essere vivo scrivendo online sul topic dello stato
    client.publish("telecomando4/stato", "online", true);
    Serial.println("Ricevuto PING, inviato ONLINE");
    comandoReale = false;
  }
  if(comandoReale){
    set_temp_fun();
  }
}

void reconnect() {
  int tentativi_falliti=0;
  while (!client.connected()) {
    Serial.print("Tentativo connessione MQTT...");
    
    // Parametri LWT: Topic "condizionatore/stato", Payload "offline", QOS 1, Retain true
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Da cambiare nome WemosClient_1 e telecomando1/stato
    if (client.connect("WemosClient_3", mqtt_user, mqtt_pass, "telecomando3/stato", 1, true, "offline")) {
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
      Serial.println("CONNESSO A MQTT!");
      
      // Appena connesso, invia lo stato "online" con RETAIN a true
      // Il retain serve a far sì che Home Assistant legga lo stato anche se si riavvia
      ////////////////////////////////////////////////////////////////////////////////////////////////////////
      //Da cambiare nomi telecomando1
      client.publish("telecomando3/stato", "online", true);
      ////////////////////////////////////////////////////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////////////////////////////////////
      //Da cambiare nomi condizionatore_1
      client.subscribe("condizionatore_3/comando");
      ////////////////////////////////////////////////////////////////////////////////////////////////////////
    } else if(tentativi_falliti <= 8){
      Serial.print("FALLITO, rc=");
      Serial.print(client.state());
      Serial.println(" riprovo tra 5 secondi");
      tentativi_falliti++;
      delay(5000);
    }else{
      tentativi_falliti = 0;
      Serial.println("Troppi tentativi fatti, riprovo fra 10 minuti");
      delay(30000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  irsend.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  client.setKeepAlive(60);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  delay(200);
}