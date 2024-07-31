String ssid = "Simulator Wifi";        // SSID to connect to
String password = "";                  // No password for the virtual Wi-Fi
String host = "api.thingspeak.com";    // API Requests - Write a Channel Feed
const int httpPort = 80;               // Port number
String apiKey = "J79HQ7TLFYR98S17";    // Your ThingSpeak API key
String uri = "/update?api_key=" + apiKey + "&field1=";

// Função para criar o shield e conectá-lo
int setupESP8266(void) {
  Serial.begin(115200);   // Serial connection over USB to computer
  Serial.println("AT");   // Serial connection on Tx / Rx port to ESP8266
  delay(1000);            // Wait a little for the ESP to respond
  if (!Serial.find("OK")) return 1;
    
  // Connect to the Wi-Fi
  Serial.println("AT+CWJAP=\"" + ssid + "\",\"" + password + "\"");
  delay(5000);            // Wait a little for the ESP to respond
  if (!Serial.find("OK")) return 2;
  
  // Open TCP connection to the host:
  Serial.println("AT+CIPSTART=\"TCP\",\"" + host + "\"," + httpPort);
  delay(5000);            // Wait a little for the ESP to respond
  if (!Serial.find("OK")) return 3;
  
  return 0;
}

// Função para enviar os dados dos sensores
void enviaDadosESP8266(float temperature, int ldrValue, int soilMoisture) {
  // Constrói o HTTP request
  String httpPacket = "GET " + uri + String(temperature) +
                      "&field2=" + String(ldrValue) +
                      "&field3=" + String(soilMoisture) + 
                      " HTTP/1.1\r\nHost: " + host + "\r\nConnection: close\r\n\r\n";
  
  int length = httpPacket.length();
  
  // Envia o comprimento da mensagem
  Serial.print("AT+CIPSEND=");
  Serial.println(length);
  delay(100); // Aguarda a resposta do ESP

  // Envia o HTTP request
  Serial.print(httpPacket);
  delay(100); // Aguarda a resposta do ESP
  if (!Serial.find("SEND OK\r\n")) return;
}

void setup() {
  // Inicia o shield
  int status = setupESP8266();
  if (status != 0) {
    Serial.println("Erro ao configurar o ESP8266");
    while (true);  // Fica preso aqui se houver erro
  }
}

void loop() {
  // Leitura dos sensores
  int temperatureRaw = analogRead(A0); // Supondo que o LM35 está no pino A0
  float voltage = temperatureRaw * (5.0 / 1023.0); // Converte para tensão (0-5V)
  float temperature = voltage * (165.0 / 5.0) - 40; // Converte a tensão para temperatura em Celsius no intervalo de -40°C a 125°C
  
  int ldrValue = analogRead(A1);  // LDR conectado ao pino A1
  int soilMoisture = analogRead(A2); // Sensor de umidade do solo conectado ao pino A2
  
  // Envia os dados para o ThingSpeak
  enviaDadosESP8266(temperature, ldrValue, soilMoisture);
  
  // Exibe os valores lidos no monitor serial para depuração
  Serial.print("Temperature: ");
  Serial.print(temperature); // Valor calculado da temperatura
  Serial.print(" C, LDR Value: ");
  Serial.print(ldrValue); // Valor bruto do LDR
  Serial.print(" , Soil Moisture: ");
  Serial.println(soilMoisture); // Valor bruto do sensor de umidade do solo
  
  delay(10000); //10s para o processamento