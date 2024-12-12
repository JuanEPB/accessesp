//////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////         LECTOR QR     ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Configuración de Wi-Fi
const char* ssid = "Totalplay-2.4G-6e18";
const char* password = "Gjr8TQgrktS9bSpN";
const char *serverUrl = "http://192.168.100.7:3000/api/v3.4/acces/sensor";

#define QR Serial1

TaskHandle_t Task1;

void loop2_escucha_escaner_qr(void *pvParameters)
{
  Serial.println("Iniciando la escucha del escáner QR...");
  for (;;)
  {
    if (QR.available()) {
      Serial.println("Datos disponibles en el lector QR."); // Mensaje de depuración
      String qrData = "";
      while (QR.available()) {
        char input = QR.read();
        qrData += input;
        delay(10); // Aumentar el delay
      }

      Serial.println("QR Escaneado: " + qrData); // Imprime los datos crudos

      // Solo deserializar si los datos parecen correctos
      enviarDatosAPI(qrData); // Enviar los datos solo si parecen un JSON válido

      
    }
    delay(10); // Pequeño retraso para evitar bloqueos
  }

  vTaskDelay(50);
}
void enviarDatosAPI(String qrData)
{
  // Imprimir los datos recibidos antes de deserializar
  Serial.println("Datos del QR recibido: " + qrData); 

  StaticJsonDocument<200> jsonDoc;
  DeserializationError error = deserializeJson(jsonDoc, qrData);

  if (error) {
    Serial.println("Error al procesar el JSON del QR: " + String(error.c_str()));
    return;
  }

  String matricula = jsonDoc["matricula"];
  String nombre = jsonDoc["nombre"];
  String email = jsonDoc["email"];

  // Mostrar los datos leídos
  Serial.println("Datos extraídos:");
  Serial.println("Matrícula: " + matricula);
  Serial.println("Nombre: " + nombre);
  Serial.println("Email: " + email);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    StaticJsonDocument<200> apiJson;
    apiJson["matricula"] = matricula;
    apiJson["nombre"] = nombre;
    apiJson["email"] = email;

    String payload;
    serializeJson(apiJson, payload);
    Serial.println("Enviando datos al servidor: " + payload);

    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Respuesta del servidor: " + response);
    } else {
      Serial.println("Error al enviar datos. Código: " + String(httpResponseCode));
    }

    http.end();
  } else {
    Serial.println("Wi-Fi desconectado.");
  }
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  QR.begin(9600, SERIAL_8N1, 26, 27);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Conectando a Wi-Fi...");
  }
  Serial.println("¡Conectado a Wi-Fi!");

  xTaskCreatePinnedToCore(loop2_escucha_escaner_qr, "Task1", 10000, NULL, 3, &Task1, 0);
}

void loop()
{

  Serial.println("Durmiendo");
  delay(10000); // Duerme 10 segundos
  Serial.println("Despertando");
  
}
