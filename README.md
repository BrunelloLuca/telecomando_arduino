# Telecomando Arduino
Sistema per la domotizzazione e il controllo remoto dei climatizzatori, basato su microcontrollore **ESP8266 (Wemos D1 Mini)**, trasmissione a infrarossi (IR) e integrazione con server domotico locale tramite protocollo **MQTT**.

## Struttura del funzionamento
Il dispositivo si connette alla rete Wi-Fi locale e si sottoscrive a un broker MQTT ospitato su un server domotico virtualizzato (VirtualBox). Ricevuti i payload di comando, la scheda genera e trasmette i pacchetti a infrarossi codificati secondo il protocollo nativo del climatizzatore.
