#include <WiFi.h>
#include <Wire.h>

const char* ssid = "UVG_mobile";         // Nombre de la red Wi-Fi
const char* password = "UVGTHOMAS"; // Contraseña de la red Wi-Fi

#define I2C_SLAVE_ADDRESS 0x08

WiFiServer server(80);
uint8_t sensorStates[4] = {0, 0, 0, 0};  // Estados de los sensores de parqueo

void setup() {
  Serial.begin(115200);

  // Conectar a la red Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado a WiFi");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());

  server.begin();
  
  // Inicializar I2C y definir la función de recepción
  Wire.begin(I2C_SLAVE_ADDRESS);
  Wire.onReceive(receiveEvent);
}

void loop() {
  // Manejar clientes de Wi-Fi
  WiFiClient client = server.available();
  if (client) {
    String request = client.readStringUntil('\r');
    client.flush();

    if (request.indexOf("/status") >= 0) {
      // Enviar el estado en formato JSON
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type: application/json");
      client.println();
      client.print("{\"sensorStates\": [");
      for (int i = 0; i < 4; i++) {
        client.print(sensorStates[i]);
        if (i < 3) client.print(", ");
      }
      client.println("]}");
    } else {
      // Página HTML principal
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/html");
      client.println();
      client.println("<!DOCTYPE html><html>");
      client.println("<head><title>Parqueo-matic</title>");
      client.println("<style>");
      client.println("body { font-family: Arial, sans-serif; text-align: center; margin: 0; padding: 0; }");
      client.println("header { padding: 20px; background-color: #333; color: white; font-size: 24px; }");
      client.println(".parking-lot { display: flex; justify-content: center; margin-top: 20px; }");
      client.println(".parking-slot { margin: 10px; padding: 20px; width: 100px; height: 150px; background-color: #333; color: white; border-radius: 10px; }");
      client.println(".parking-slot.available { background-color: green; }");
      client.println(".parking-slot.occupied { background-color: red; }");
      client.println(".parking-slot p { margin: 5px 0; font-size: 20px; }");
      client.println(".available-count-container { margin-top: 30px; text-align: center; }");
      client.println(".available-count-title { font-size: 24px; font-weight: bold; }");
      client.println(".available-count-circle { font-size: 48px; width: 100px; height: 100px; border-radius: 50%; border: 2px solid black; display: inline-flex; align-items: center; justify-content: center; margin-top: 10px; }");
      client.println("footer { padding: 10px; background-color: #333; color: white; font-size: 14px; position: fixed; bottom: 0; width: 100%; }");
      client.println("</style>");
      client.println("<script>");
      client.println("function fetchData() {");
      client.println("  fetch('/status').then(response => response.json()).then(data => {");
      client.println("    let availableCount = data.sensorStates.filter(state => state == 0).length;");
      client.println("    document.getElementById('availableCount').innerHTML = availableCount;");
      for (int i = 0; i < 4; i++) {
        client.printf("    document.getElementById('slot%d').className = data.sensorStates[%d] == 0 ? 'parking-slot available' : 'parking-slot occupied';\n", i+1, i);
        client.printf("    document.getElementById('status%d').innerHTML = data.sensorStates[%d] == 0 ? 'Disponible' : 'Ocupado';\n", i+1, i);
      }
      client.println("  });");
      client.println("}");
      client.println("setInterval(fetchData, 1000);"); // Actualiza cada segundo
      client.println("</script>");
      client.println("</head>");
      client.println("<body onload='fetchData()'>");
      client.println("<header>Parqueo-matic</header>");
      
      // Imagen decorativa debajo del título
      client.println("<div><img src='https://media.istockphoto.com/id/1083622428/es/vector/icono-de-aparcamiento-de-coche.jpg?s=612x612&w=0&k=20&c=XNtE9FbFoCi4Pop_S63HY9VujXx4dvJWTKVvit9g0f4=' alt='Parking Logo' style='width:150px; height:auto; margin-top: 10px;'></div>");

      // Visualización de los "Parqueos disponibles" en el formato circular
      client.println("<div class='available-count-container'>");
      client.println("<div class='available-count-title'>Parqueos disponibles</div>");
      client.println("<div id='availableCount' class='available-count-circle'>Cargando...</div>");
      client.println("</div>");
      
      client.println("<div class='parking-lot'>");
      for (int i = 1; i <= 4; i++) {
        client.printf("<div id='slot%d' class='parking-slot'><p>P%d</p><p id='status%d'>Cargando...</p></div>\n", i, i, i);
      }
      client.println("</div>");

      // Footer con los derechos de autor
      client.println("<footer>&copy; 2024 Thomas Solis 21626. Universidad del Valle de Guatemala. Todos los derechos reservados.</footer>");

      client.println("</body></html>");
    }
    client.stop();
  }
}

// Función para recibir datos desde el STM32
void receiveEvent(int howMany) {
  int index = 0;
  while (Wire.available() && index < 4) {
    sensorStates[index] = Wire.read();  // Leer el estado de cada sensor y guardarlo en sensorStates
    index++;
  }
}
