#include <WiFi.h>
#include <PubSubClient.h>

// Configuración de la red Wi-Fi
const char* ssid = "";         // Nombre de tu red Wi-Fi
const char* password = "";     // Contraseña de tu red Wi-Fi

// Configuración de MQTT
const char* mqtt_server = "";  // Dirección del broker MQTT
const char* mqtt_user = "";    // Usuario para el broker MQTT
const char* mqtt_pass = "";    // Contraseña para el broker MQTT

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);

  // Conectar a la red Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a Wi-Fi...");
  }
  Serial.println("Conectado a Wi-Fi");

  // Configurar el servidor MQTT
  client.setServer(mqtt_server, 1883);
  connectToMQTT();
}


void loop() {
  // Reconectar si la conexión MQTT se ha perdido
  if (!client.connected()) {
    connectToMQTT();
  }
  
  client.loop();  // Mantener la conexión MQTT activa

  // Comprobar si han pasado 5 minutos (300,000 milisegundos)
  if (millis() - lastSendTime >= sendInterval) {
    // Actualizar el tiempo del último envío
    lastSendTime = millis();

    // Leer la temperatura y la humedad
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    
    // Verificar si la lectura fue exitosa
    if (isnan(h) || isnan(t)) {
      Serial.println("Error al leer el sensor DHT11");
      return;
    }

    // Imprimir los datos en el puerto serie
    Serial.print("Humedad: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperatura: ");
    Serial.print(t);

    // Crear el mensaje JSON con la estructura deseada
    String payload = "{";
    payload += "\"device\": \"sensor_001\",";
    payload += "\"data\": {";
    payload += "\"temperature\": " + String(t) + ",";
    payload += "\"humidity\": " + String(h);
    payload += "},";
    payload += "\"timestamp\": \"" + String(millis()) + "\""; // Timestamp como el tiempo en milisegundos desde que comenzó el programa
    payload += "}";

    // Verificar el tamaño del payload
    if (payload.length() > MAX_PAYLOAD_SIZE) {
      Serial.println("El payload es demasiado grande. No se enviará.");
      return;  // Si el payload excede el tamaño máximo, no se publica
    }

    // Publicar los datos en el tópico MQTT
    client.publish(mqtt_topic, payload.c_str());
  }
}


void connectToMQTT() {
  while (!client.connected()) {
    Serial.println("Conectando a MQTT...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_pass)) {
      Serial.println("Conectado a MQTT");
    } else {
      Serial.print("Error de conexión: ");
      Serial.print(client.state());
      delay(5000);
    }
  }
}
