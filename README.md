# FitaLedWiFi
Projeto de Fita LED com Controle via WiFi

Este projeto permite controlar uma fita de leds "endereçáveis" (WS2812B) a partir de um browser.

## Hardware
Eu utilizei uma placa WeActStudio Core Board ESP32-C3, mas é fácil adaptar o projeto para outras placas com este ou outros modelos de ESP32.

São usados apenas dois GPIOs, um para comunicação com a fita de LED e outro para controlar a alimentação da fita. Este controle é feito por um MOSFET que é acionado por um transistor NPN. O arquivo PowerSwitch.pdf contém esta parte do circuito.

Na montagem eu cortei um cabo USB para colocar um chave nos 5V. Tanto os 5V como o terra são conectados à ponta do cabo que vai para a placa do microntrolador como para o circuito de chaveamento da alimentação da fita de LEDs (para evitar que a corrente da fita de LED passe pela placa do microcontrolador). Os fios de dados da USB das duas pontas são mantidos conectados.

## Software
O software foi desenvolvido na IDE Arduino.

A biblioteca fastled foi usada para controlar os LEDs, a biblioteca ESP_WiFiSettings foi usada para a configuração do WiFi.

Para implementar a página web de controle da fita foi usada a biblioteca WebServer (que também é usada pelo WiFiSettings). A página é bem minimalista e é estática (o que acarreta em sempre apresentar as opções default).

./Config.png

São implementados quatro modos de operação para a fita de LEDs:

* APAGADO: os LEDs são apagados e a alimentação da fita é desligada.
* ACESO: todos os LEDs são acesos com a mesma cor. A página web permite definir a cor e a intensidade.
* PULSANTE: todos os LEDs são acesos com uma cor com intensidade variando. A página web permite definir a cor e a intensidade máxima.
* SEQUENCIAL: um LED é aceso por vez. A página web permite definir a cor e a intensidade.

As cores usadas são tonalidades de branco (azul = branco azulado, etc).

Obs: Encontrei um conflito entre o ESP-WiFiSettings (v3.8.0)  e o suporte atual do ESP32 (v3.01): https://github.com/Juerd/ESP-WiFiSettings/issues/35. Espero que isto seja resolvido em breve.


