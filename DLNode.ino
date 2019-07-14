/*
 * Se desea establecer la comunicación entre el arduino nano y la raspberry pi 3 por medio del módulo MAX485.
 * El arduino deberá ser capaz de enviar y recibir información por comunicación serie half-duplex.
*/

#include <DHT.h>
#define DHTTYPE DHT11   // DHT 22  (AM2302), AM2321
#define DHTPIN_A 4     // what pin we're connected to
#define DHTPIN_B 5     // what pin we're connected to
#define DHTPIN_C 6     // what pin we're connected to

// Inicializamos el sensor DHT22
/*DHT dht[] = {
  {DHTPIN_A, DHTTYPE},
  {DHTPIN_B, DHTTYPE},
  {DHTPIN_C, DHTTYPE}
};*/

DHT dhtA(DHTPIN_A, DHTTYPE);
DHT dhtB(DHTPIN_B, DHTTYPE);
DHT dhtC(DHTPIN_C, DHTTYPE);

DHT dht[] = {dhtA, dhtB, dhtC};

float humidity[3];
float temperature[3];

const int ReDePin =  2;  // HIGH = Driver / LOW = Receptor
const int MCUstatus = 3;

//Estos pins se encargarán de darle el ID al nodo
const int pinID0 = 9;
const int pinID1 = 10;
const int pinID2 = 11;
const int pinID3 = 12;

String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete
bool transmit = false;
uint8_t nodeID = 0;

// Prototipos de funciones
void readDHT();
void readMQ3();
void processData(String);
void answer(String);
void receiveMode();
void transmitMode();
void checkMcuStatus();

void setup() 
{ 
  Serial.begin(9600);
  
  //Output pins
  pinMode(ReDePin, OUTPUT);
  digitalWrite(ReDePin, LOW);

  //Input pins
  pinMode(MCUstatus, INPUT);
  pinMode(pinID0, INPUT);
  pinMode(pinID1, INPUT);
  pinMode(pinID2, INPUT);
  pinMode(pinID3, INPUT);
  
  // Comenzamos el sensor DHT
  dht[0].begin();
  dht[1].begin();
  dht[2].begin();
  nodeID = 8*digitalRead(pinID3) + 4*digitalRead(pinID2) + 2*digitalRead(pinID1) + digitalRead(pinID0);
} 
void loop() 
{ 
} 

void serialEvent() 
{
  while (Serial.available()) 
  {
    char inChar = (char)Serial.read();    // get the new byte
    if (inChar == '\n') 
    {
      processData(inputString);
      inputString = "";
      break;
    }
    inputString += inChar;                // add it to the inputString
  }
}

void processData(String inputData)
{
  if (!inputData.startsWith(String(nodeID))) return;
  
  if (inputData.indexOf("all") != -1) { readDHT(); readMQ3();}
  else if (inputData.indexOf("dht") != -1) {readDHT();}
  else if (inputData.indexOf("mq3") != -1) {readMQ3();}
  else if (inputData.indexOf("MCUstatus") != -1){answer(String(digitalRead(MCUstatus)));}
  else if (inputData.equals(String(nodeID))){answer("ID" + String(nodeID));}
  else {answer(inputData);}
}

void answer(String resp)
{
  transmitMode();
  resp = "!!!!!" + resp + "?????\n";
  Serial.print(resp);
  Serial.flush();
  receiveMode();
}

void readDHT()
{
  delay(2000);
  for (int i = 0; i < 3; i++) {
    temperature[i] = dht[i].readTemperature();
    humidity[i] = dht[i].readHumidity();
    
    // Comprobamos si ha habido algún error en la lectura
    if (isnan(temperature[i]) || isnan(humidity[i])) {
      temperature[i] = -1000;
      humidity[i] = -1000;
    }
  }
  
  String data_json = "{\"tempA\": " + String(temperature[0]) + ", \"tempB\": " + String(temperature[1]) + ", \"tempC\": " + String(temperature[2]) +  
                    ", \"humidA\": " + String(humidity[0]) + ", \"humidB\": " + String(humidity[1]) +  ", \"humidC\": " + String(humidity[2]) + "}";
  
  answer(data_json);
}

void readMQ3()
{
  int adc_MQ = analogRead(A0); //Lemos la salida analógica del MQ
  float voltaje = adc_MQ * (5.0 / 1023.0); //Convertimos la lectura en un valor de voltaje

  String data = String(adc_MQ) + "ADC " + String(voltaje) + "VOLT";
  answer(data);
}

void receiveMode()
{
  delay(100);
  digitalWrite(ReDePin, LOW);
  transmit = false;
  delay(100);
}

void transmitMode()
{
  delay(100);
  digitalWrite(ReDePin, HIGH);
  transmit = true;
  delay(100);
}

void checkMcuStatus()
{
  String mcustatus = "transmitMode() MCUstatus: " + String(digitalRead(MCUstatus));
  Serial.println(mcustatus);
}
