#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

// =================================================================
// SEU LINK DO GOOGLE APPS SCRIPT (APP DA WEB) CONFIGURADO!
// =================================================================
String URL_DO_GOOGLE = "https://script.google.com/macros/s/AKfycbxbeYkmxLtzrdt7WGx2PgVb7fc_UVgBTdQXTueUipQH_OoCT2Z_4wAQcTXXDWwtDb6IGw/exec";

// ================= CONFIGURAÇÃO WIFI =================
const char* ssid = "S24 Ultra de Thiago";
const char* password = "12345678";

// ========================= PINAGEM =============================
#define DHTPIN 40         // G40 - Sensor de Temperatura e Umidade
#define DHTTYPE DHT11

#define BUZZER 2          // G2  - Sirene/Bip
#define PIN_GAS 1         // G1  - Sensor de Gás (Analógico)
#define PIN_SOLO 5        // G5 Sensor humidade solo

// --- CONTROLES DE ATUAÇÃO ---
#define PIN_FITA 18       // Dados da Fita LED Endereçável
#define NUM_LEDS 300      // Quantidade de LEDs
#define PIN_RELE1 19      // Relé 1 (Cooler)
#define PIN_RELE2 21      // Relé 2 (Bomba d'Água / Umidificador)
#define PIN_RELE_PULSO 45 // NOVO: Relé extra que pulsa

// --- PINOS DO OLED ---
#define SDA_PIN 41        // G41 
#define SCL_PIN 42        // G42 

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
DHT dht(DHTPIN, DHTTYPE);
Adafruit_NeoPixel fita(NUM_LEDS, PIN_FITA, NEO_GRB + NEO_KHZ800);

// --- VARIÁVEIS DE ESTADO ---
float ultimaTemp = -999;
bool estadoFita = false;
bool estadoRele1 = false;
bool estadoRele2 = false;

// --- VARIÁVEIS PARA O AUTODESLIGAMENTO DA BOMBA ---
unsigned long tempoInicioBomba = 0;
const long intervaloBomba = 5000; // 5 segundos

// --- TEMPORIZADOR PARA NUVEM ---
unsigned long tempoAnterior = 0;
const long intervaloEnvio = 3000; // Envia os dados e checa botões a cada 5 segundos

// ================= NOTAS STAR WARS =================
#define NOTE_AS4 466
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_F6  1397

void tocarStarWars() {
  int melody[] = { NOTE_AS4, NOTE_AS4, NOTE_AS4, NOTE_F5, NOTE_C6, NOTE_AS5, NOTE_A5, NOTE_G5, NOTE_F6, NOTE_C6, NOTE_AS5, NOTE_A5, NOTE_G5, NOTE_F6, NOTE_C6, NOTE_AS5, NOTE_A5, NOTE_AS5, NOTE_G5, NOTE_C5, NOTE_C5, NOTE_C5, NOTE_F5, NOTE_C6 };
  int durations[] = { 8, 8, 8, 2, 2, 8, 8, 8, 2, 4, 8, 8, 8, 2, 4, 8, 8, 8, 2, 8, 8, 8, 2, 2 };
  int tamanho = sizeof(melody) / sizeof(melody[0]);
  for (int thisNote = 0; thisNote < tamanho; thisNote++) {
    int noteDuration = 1000 / durations[thisNote];
    tone(BUZZER, melody[thisNote], noteDuration);
    delay(noteDuration * 1.30);
    noTone(BUZZER);
  }
}

// ========================= ATUADORES ======================
void atualizarFita() {
  if(estadoFita) {
    for(int i=0; i<NUM_LEDS; i++) {
      if (i % 2 == 0) {
        // Se o número do LED for par (0, 2, 4...), pinta de Roxo
        fita.setPixelColor(i, fita.Color(160, 32, 240)); 
      } else {
        // Se o número do LED for ímpar (1, 3, 5...), pinta de Rosa
        fita.setPixelColor(i, fita.Color(255, 20, 147)); 
      }
    }
  } else {
    fita.clear();
  }
  fita.show();
}

// NOVO: Função para dar um pulso rápido no relé 45
void pulsarReleExtra() {
  pinMode(PIN_RELE_PULSO, OUTPUT);
  digitalWrite(PIN_RELE_PULSO, LOW); // Liga o relé
  delay(300);                        // Mantém ligado por 300ms (rapidamente)
  pinMode(PIN_RELE_PULSO, INPUT);    // Desliga usando o truque da alta impedância
}

// Essa função lê o que o Google Script respondeu
void processarComando(String cmd) {
  cmd.trim(); 
  if (cmd == "OK" || cmd == "") return; // Sem comandos novos

  Serial.println("Comando do Google: " + cmd);
  
  if (cmd == "TOGGLE_FITA") {
    estadoFita = !estadoFita;
    atualizarFita();
  } 
  else if (cmd == "TOGGLE_RELE1") {
    estadoRele1 = !estadoRele1;
    // Dispara o pulso do relé 45 assim que o estado do relé 2 (Umidificador) muda
    pulsarReleExtra();

    if (estadoRele1) {
      // Para LIGAR: Configura como saída e manda 0V
      pinMode(PIN_RELE1, OUTPUT);
      digitalWrite(PIN_RELE1, LOW);
    } else {
      // Para DESLIGAR: Truque da Alta Impedância
      pinMode(PIN_RELE1, INPUT);
    }
  } 
  else if (cmd == "TOGGLE_RELE2") {
    estadoRele2 = !estadoRele2;
    
    if (estadoRele2) {
      // Para LIGAR: Configura como saída e manda 0V
      pinMode(PIN_RELE2, OUTPUT);
      digitalWrite(PIN_RELE2, LOW);
      tempoInicioBomba = millis(); // <--- INICIA O CRONÔMETRO DE 5 SEGUNDOS
    } else {
      // Para DESLIGAR manualmente: Truque da Alta Impedância
      pinMode(PIN_RELE2, INPUT);
    }
  } 
  else if (cmd == "RESTART") {
    display.clearDisplay(); display.setCursor(0,20); display.print("REINICIANDO..."); display.display();
    delay(2000);
    ESP.restart();
  }
}

// ========================= LETREIRO OLED ======================
void mostrarLetreiro() {
  display.clearDisplay();
  display.setTextSize(2); 
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false); 

  String texto = "ESTUFA INTELIGENTE TIGAS    ";
  int larguraTexto = texto.length() * 12;

  tocarStarWars();

  for (int x = SCREEN_WIDTH; x > -larguraTexto; x -= 5) {
    display.clearDisplay();
    display.setCursor(x, 25);
    display.print(texto);
    display.display();
    delay(10);
  }
  display.setTextWrap(true); 
}

// ========================= SETUP ======================
void setup() {
  Serial.begin(115200);

  pinMode(BUZZER, OUTPUT);
  
  // TRUQUE PARA INICIAR DESLIGADO NO BOOT
  pinMode(PIN_RELE1, INPUT); 
  // TRUQUE PARA INICIAR DESLIGADO NO BOOT
  pinMode(PIN_RELE2, INPUT); 
  // TRUQUE PARA INICIAR DESLIGADO NO BOOT (NOVO RELÉ G45)
  pinMode(PIN_RELE_PULSO, INPUT);
  
  pinMode(PIN_SOLO, INPUT);
  pinMode(PIN_GAS, INPUT); 

  Wire.begin(SDA_PIN, SCL_PIN);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("AVISO: Falha no OLED."));
  } else {
    mostrarLetreiro();
  }

  fita.begin();
  fita.setBrightness(50);
  fita.show(); 

  dht.begin();

  // Conectar no WiFi
  WiFi.begin(ssid, password);
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("Conectando WiFi...");
  display.display();

  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 20) { 
    delay(500); 
    Serial.print("."); 
    tentativas++;
  }

  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConectado!");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFALHA NO WIFI!");
  }
}

// ========================= LOOP ======================
void loop() {
  float temp = dht.readTemperature();
  float umid = dht.readHumidity();
  
  if (isnan(temp)) temp = 0.0;
  if (isnan(umid)) umid = 0.0;

  // ALTERADO: Limite de 3100 para compensar a leitura do seu sensor na água
  // Serial.println(analogRead(PIN_SOLO)); // Linha de calibração de sensor
  int s = map(analogRead(PIN_SOLO), 4092, 2110, 0, 100);
  if (s > 100) s = 100;
  if (s < 0) s = 0;

  int gasRaw = analogRead(PIN_GAS);

  // Bip Inteligente (só bipa se mudar temperatura)
  if (abs(temp - ultimaTemp) >= 1.0 && temp != 0.0) { 
    if(ultimaTemp != -999) tone(BUZZER, 2000, 50);
    ultimaTemp = temp;
  }

  // =====================================================================
  // --- MÁGICA: AUTODESLIGAMENTO DA BOMBA (5 SEGUNDOS) ---
  // =====================================================================
  if (estadoRele2) {
    if (millis() - tempoInicioBomba >= intervaloBomba) {
      estadoRele2 = false;       // Muda a variável interna
      pinMode(PIN_RELE2, INPUT); // Desliga a bomba de verdade no hardware
      
      // Dispara o pulso do relé extra quando a bomba desliga sozinha
      pulsarReleExtra();
      
      Serial.println(">>> BOMBA DESLIGADA PELO TIMER DO ESP32 <<<");
    }
  }

  // --- OLED ---
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0); display.print("T:"); display.print(temp, 1); display.print("C  U:"); display.print(umid, 1); display.print("%");
  display.setTextSize(2);
  display.setCursor(0, 18); display.print("SOLO:"); display.print(s); display.print("%");
  display.setCursor(0, 36); display.print("GAS:"); display.print(gasRaw);
  display.setTextSize(1);
  display.setCursor(0, 55); display.print("Link: Google Nuvem");
  display.display();

  // =====================================================================
  // --- COMUNICAÇÃO COM O GOOGLE A CADA 5 SEGUNDOS ---
  // =====================================================================
  unsigned long tempoAtual = millis();
  if (tempoAtual - tempoAnterior >= intervaloEnvio) {
    tempoAnterior = tempoAtual;

    if (WiFi.status() == WL_CONNECTED) {
      
      WiFiClientSecure client;
      client.setInsecure(); // Necessário para sites HTTPS (Google)
      HTTPClient http;

      // Cria a URL enviando todos os dados como parâmetros
      String urlFinal = URL_DO_GOOGLE + 
                        "?temp=" + String(temp, 1) + 
                        "&umid=" + String(umid, 1) + 
                        "&solo=" + String(s) + 
                        "&gas=" + String(gasRaw) + 
                        "&fita=" + String(estadoFita) + 
                        "&r1=" + String(estadoRele1) + 
                        "&r2=" + String(estadoRele2) +  // Aqui o ESP32 avisa a planilha se a bomba está ligada ou não
                        "&esp_ip=" + WiFi.localIP().toString();

      Serial.println("-> Enviando dados para o Google...");
      
      http.begin(client, urlFinal);
      
      // =================================================================
      // A MÁGICA DOS BOTÕES: Força o ESP32 a seguir o Google Apps Script!
      // =================================================================
      http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
      
      int httpCode = http.GET(); 

      // Se a conexão com o Google deu certo
      if (httpCode > 0) {
        String respostaGoogle = http.getString();
        processarComando(respostaGoogle); // Checa se o botão foi apertado no HTML
      } else {
        Serial.println("Erro HTTPS: " + String(httpCode));
      }
      http.end();
    }
  }
}