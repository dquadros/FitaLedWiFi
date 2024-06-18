/*
  Controle de fita de LEDs via WiFi
  Daniel Quadros, junho/24
*/

#include <SPIFFS.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiSettings.h>
#include <FastLED.h>

// Controle da fita de LEDs
#define PIN_LED  6
#define PIN_PWR  7
#define NUM_LEDS 40

static CRGB leds[NUM_LEDS];
static bool powerOn;

// Página HTML de controle da fita
static WebServer http(80);
static const char *pagina = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'>"
  "<title>Fita de LED</title></head><body><h2>Fita de LED</h2><form method=post>"
  "<h3>Modo:</h3><input type='radio' id='desligado' name='modo' value='desligado' checked>"
  "<label for='desligado'>Desligado</label><br><input type='radio' id='aceso' name='modo' value='aceso'>"
  "<label for='aceso'>Aceso</label><br><input type='radio' id='pulsante' name='modo' value='pulsante'>"
  "<label for='apagado'>Pulsante</label><br><input type='radio' id='sequencial' name='modo' value='sequencial'>"
  "<label for='apagado'>Sequencial</label><br><h3>Cor:</h3><input type='radio' id='branco' name='cor' value='branco' checked>"
  "<label for='desligado'>Branco</label><br><input type='radio' id='azul' name='cor' value='azul'>"
  "<label for='azul'>Azul</label><br><input type='radio' id='verde' name='cor' value='verde'>"
  "<label for='verde'>Verde</label><br><input type='radio' id='vermelho' name='cor' value='vermelho'>"
  "<label for='vermelho'>Vermelho</label><br><input type='radio' id='amarelo' name='cor' value='amarelo'>"
  "<label for='amarelo'>Amarelo</label><br><h3>Intensidade:</h3>"
  "<input type='range' id='intensidade' name='intensidade' min='0' max='5'><br>"
  "<input type='submit' value='Execute'></form></body></html>";

// Opções para a animacao
static enum {
  APAGADO = 0,
  ACESO,
  DESLOCA,
  PULSA
} animacao = ACESO;
static CRGB corBase(3,3,3);
static CRGB corPulsa;
static int passo = 0;

// Iniciação
void setup() {
    Serial.begin(115200);

    pinMode (PIN_PWR, OUTPUT);
    digitalWrite (PIN_PWR, HIGH);
    powerOn = true;
    FastLED.addLeds<NEOPIXEL, PIN_LED>(leds, NUM_LEDS);
    apaga_leds();
    FastLED.show();

    SPIFFS.begin(true);  // On first run, will format after failing to mount

    Serial.println("Conectando");
    WiFiSettings.onWaitLoop = []() { busy_led(); return 30; };  // delay 30 ms    
    WiFiSettings.connect();
    Serial.println("Conectado");

    mostraIP();
    acende_leds(corBase);

    http.on("/", HTTP_GET, configura);
    http.on("/", HTTP_POST, atualiza);
    http.begin();
}

// Envia a página de configuração
void configura() {
    http.setContentLength(CONTENT_LENGTH_UNKNOWN);
    http.send(200, "text/html");
    http.sendContent(pagina);
}

// Atualiza a fita conforme as seleções na página de configuração
void atualiza() {
  Serial.print("POST modo=");
  Serial.print(http.arg("modo"));
  Serial.print(" cor=");
  Serial.print(http.arg("cor"));
  Serial.print(" intensidade=");
  Serial.println(http.arg("intensidade"));

  configura();

  String cor = http.arg("cor");
  int brilho = atoi(http.arg("intensidade").c_str());
  if (cor == "branco") {
    corBase = CRGB(3 << brilho, 3 <<brilho, 3 << brilho);
  } else if (cor == "azul") {
    corBase = CRGB(3 << brilho, 3 <<brilho, 7 << brilho);
  } else if (cor == "verde") {
    corBase = CRGB(3 << brilho, 7 <<brilho, 3 << brilho);
  } else if (cor == "vermelho") {
    corBase = CRGB(7 << brilho, 3 <<brilho, 3 << brilho);
  } else if (cor == "amarelo") {
    corBase = CRGB(7 << brilho, 7 <<brilho, 3 << brilho);
  }

  String modo = http.arg("modo");
  if (modo == "desligado") {
    Serial.println("Apagado");
    animacao = APAGADO;
    apaga_leds();
    delay(30);
    ledPower(false);
  } else {
    if (!powerOn) {
      ledPower(true);
      delay(100); // para leds iniciarem
    }
    if (modo == "aceso") {
      Serial.println("Aceso");
      animacao = ACESO;
      acende_leds(corBase);
    } else if (modo == "pulsante") {
      Serial.println("Pulsa");
      animacao = PULSA;
      corPulsa.red = corBase.red;
      corPulsa.green = corBase.green;
      corPulsa.blue = corBase.blue;
    } else if (modo == "sequencial") {
      Serial.println("Sequencial");
      animacao = DESLOCA;
    }
  }

  passo = 0;
}

// Laço principal
void loop() {
  http.handleClient();
  if (animacao == PULSA) {
    if (passo == 0) {
      // reduz luminosidade
      corPulsa.red--;
      corPulsa.green--;
      corPulsa.blue--;
      if ((corPulsa.red == 0) || (corPulsa.green == 0) || (corPulsa.blue == 0)) {
        passo = 1;
      }
    } else {
      // aumenta luminosidade
      corPulsa.red++;
      corPulsa.green++;
      corPulsa.blue++;
      if ((corPulsa.red == corBase.red) || (corPulsa.green == corBase.green) || (corPulsa.blue == corBase.blue)) {
        passo = 0;
      }
    }
    acende_leds(corPulsa);
  } else if (animacao == DESLOCA) {
    leds[passo] = CRGB::Black;
    passo = (passo+1) % NUM_LEDS;
    leds[passo] = corBase;
    FastLED.show();
  }
  delay(30);
}

static void apaga_leds() {
    for (int i = 0; i < NUM_LEDS; i++)
      leds[i] = CRGB::Black;
    FastLED.show();
}

// Acende todos os LEDs com uma mesma cor
static void acende_leds (CRGB cor) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = cor;
  }
  FastLED.show();
}

// Coloca padrão que indica ocupado
static void busy_led() {
  static int iled = (NUM_LEDS-1);
  leds[iled] = CRGB::Black;
  iled = (iled+1) % NUM_LEDS;
  leds[iled] = CRGB(0,0,3);
  FastLED.show();
}

// Mostra o último byte do endereço IP na fita de LED
// 0 = vermelho, 1 = verde
static void mostraIP() {
  apaga_leds();
  IPAddress ip = WiFi.localIP();
  uint8_t ender = ip[3];
  for (int i = 0; i < 8; i++) {
    uint8_t mask = 0x01 << (7-i);
    leds[i] = (ender & mask) ? CRGB(0,32,0) : CRGB(32,0,0);
  }
  FastLED.show();
  delay(10000);
  apaga_leds();
}

// Liga/desliga a alimentação da fita de LED
static void ledPower(bool on) {
  digitalWrite (PIN_PWR, on? HIGH : LOW);
  powerOn = on;
}
