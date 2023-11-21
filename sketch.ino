#include <WiFi.h>
#include <PubSubClient.h>
#include <Keypad.h>

// Definições do teclado numérico
const byte LINHAS = 4;
const byte COLUNAS = 4;
char teclas[LINHAS][COLUNAS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte pinosLinhas[LINHAS] = {2, 16, 17, 18};
byte pinosColunas[COLUNAS] = {25, 26, 27, 14};

Keypad teclado = Keypad(makeKeymap(teclas), pinosLinhas, pinosColunas, LINHAS, COLUNAS);

// Lista de números e informações correspondentes
int numeros[] = {1, 2, 3, 4, 5};
const char *informacoes[] = {"Info 1\nPaciente com bronquite", "Info 2\nPaciente com ansiedade e TDAH", "Info 3\nPaciente com Osteoporose", "Info 4/nPaciente com Diabetes", "Info 5\nPaciente com arteriosclerose"};
int tamanhoLista = sizeof(numeros) / sizeof(numeros[0]);

// WiFi e MQTT
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "46.17.108.113";
const int mqtt_port = 1883;
const char* mqtt_topic = "/TEF/hosp202/attrs/id";
#define ID_MQTT  "hosp_201"

WiFiClient espClient;
PubSubClient MQTT(espClient);

void setup_wifi() {
  delay(10);
  Serial.begin(9600);
  Serial.println();
  Serial.print("Conectando-se a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password, 6);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Conectado à rede WiFi");
  Serial.println("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida no tópico: ");
  Serial.println(topic);
  char message[length + 1];
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  
  int numeroRecebido = atoi(message);
  bool encontrado = false;

  for (int i = 0; i < tamanhoLista; ++i) {
    if (numeros[i] == numeroRecebido) {
      Serial.print("Id encontrado no MQTT: ");
      Serial.println(numeroRecebido);
      Serial.print("Id correspondente: ");
      Serial.println(informacoes[i]);
      encontrado = true;
      break;
    }
  }

  if (!encontrado) {
    Serial.println("Id nao encontrado na lista.");
  }
}

void reconnect() {
  while (!MQTT.connected()) {
    Serial.print("Tentando se conectar ao MQTT...");
    if (MQTT.connect("ESP8266Client")) {
      Serial.println("Conectado ao servidor MQTT");
      MQTT.subscribe(mqtt_topic);
    } else {
      Serial.print("Falha na conexao, rc=");
      Serial.print(MQTT.state());
      Serial.println("Tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  setup_wifi();
  MQTT.setServer(mqtt_server, mqtt_port);
  MQTT.setCallback(callback);
}

void loop() {
  if (!MQTT.connected()) {
    reconnect();
  }
  MQTT.loop();

  char tecla = teclado.getKey();
  
  if (tecla) {
    int numeroDigitado = tecla - '0';
    bool encontrado = false;
    
    for (int i = 0; i < tamanhoLista; ++i) {
      if (numeros[i] == numeroDigitado) {
        Serial.print("Numero encontrado: ");
        Serial.println(numeroDigitado);
        Serial.print("Informacao correspondente: ");
        Serial.println(informacoes[i]);
        encontrado = true;
        break;
      }
    }
    
    if (!encontrado) {
      Serial.println("Numero nao encontrado na lista.");
    }
  }
}
