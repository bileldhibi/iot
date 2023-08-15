#include <FirebaseArduino.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <TimeLib.h> // inclure la bibliothèque TimeLib
#include <ACS712.h> // inclure la bibliothèque ACS712

SoftwareSerial espSerial(2, 3); // RX, TX
#define FIRE_SENSOR_PIN D2
#define VOLT_SENSOR_PIN A0
#define ACS712_30A 1
#define CURRENT_SENSOR_PIN D1 // Pin utilisé pour le capteur de courant
#define VOLT_FACTOR 0.0049
float sensitivity = 66; // Sensitivity of ACS712 30A
const int FIRE_THRESHOLD = 50;
bool flameSensorConnected = true; 

char* ssid = "Airbox-49CB";
char* password = "Bilel246@";
char* firebaseHost = "esp8266arduino-c7efc-default-rtdb.firebaseio.com";
char* firebaseAuth = "wlARkx0Ge9OAvhTMcsXkjtgdgsvum1yySKAiwcpB";

ACS712 currentSensor(ACS712_30A, CURRENT_SENSOR_PIN); // Initialisation du capteur de courant

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);
  pinMode(FIRE_SENSOR_PIN, INPUT);
  pinMode(VOLT_SENSOR_PIN, INPUT);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Firebase.begin(firebaseHost, firebaseAuth);
  Serial.println("Connected to Firebase");

  // Configuration de TimeLib pour utiliser le serveur NTP
  configTime(0, 0, "pool.ntp.org");
   if (!digitalRead(FIRE_SENSOR_PIN)) {
    flameSensorConnected = false;
    Serial.println("ERREUR : Capteur de flamme non branché !");
  }
}

void loop() {
   // Lecture de la valeur du capteur de flamme
  int fireValue = analogRead(FIRE_SENSOR_PIN);

  // Envoi de la valeur à Firebase
  if (flameSensorConnected) { // Vérification que le capteur est bien branché
    if (fireValue > FIRE_THRESHOLD) {
      Firebase.setBool("/flamme", false);
      Serial.println(false);
    } else {
      Firebase.setBool("/flamme", true);
      Serial.println(true);
      time_t now = time(nullptr);
      struct tm* localTime = localtime(&now);
      char dateBuffer[11];
      char timeBuffer[9];
      strftime(dateBuffer, 11, "%Y-%m-%d", localTime); // format YYYY-MM-DD
      strftime(timeBuffer, 9, "%H:%M:%S", localTime); // format HH:MM:SS
      Firebase.setString("/dateHeure", dateBuffer + String(" ") + timeBuffer);
      Serial.print("dateHeure: ");
      Serial.println(dateBuffer + String(" ") + timeBuffer);
    }
  }

  float voltage = 0;
float voltValue = analogRead(VOLT_SENSOR_PIN) * VOLT_FACTOR;
float referenceVoltage = 0.1; // Valeur de référence pour détecter si le capteur de tension est branché

if (voltValue < referenceVoltage) {
  Serial.println("ERREUR : Capteur de tension non branché !");
  time_t now = time(nullptr);
      struct tm* localTime = localtime(&now);
      char dateBuffer1[11];
      char timeBuffer1[9];
      strftime(dateBuffer1, 11, "%Y-%m-%d", localTime); // format YYYY-MM-DD
      strftime(timeBuffer1, 9, "%H:%M:%S", localTime); // format HH:MM:SS
      Firebase.pushString("/dateHeureVoltage", dateBuffer1 + String(" ") + timeBuffer1);
      Serial.print("dateHeureVoltage: ");
      Serial.println(dateBuffer1 + String(" ") + timeBuffer1);
} else {
  voltage = voltValue;
   time_t now = time(nullptr);
      struct tm* localTime = localtime(&now);
      char dateBuffer1[11];
      char timeBuffer1[9];
      strftime(dateBuffer1, 11, "%Y-%m-%d", localTime); // format YYYY-MM-DD
      strftime(timeBuffer1, 9, "%H:%M:%S", localTime); // format HH:MM:SS
      Firebase.pushString("/dateHeureVoltage", dateBuffer1 + String(" ") + timeBuffer1);
      Serial.print("dateHeureVoltage: ");
      Serial.println(dateBuffer1 + String(" ") + timeBuffer1);
}

Firebase.pushFloat("/voltage", voltage);
Serial.print("Voltage: ");
Serial.println(voltage);
float current;
float voltValu = analogRead(VOLT_SENSOR_PIN) * VOLT_FACTOR;
if (voltValu < 2.5) {
  Serial.println("ERREUR : Capteur de courant non branché !");
  current = 0;
  Firebase.pushFloat("/courant", current);
   time_t now = time(nullptr);
      struct tm* localTime = localtime(&now);
      char dateBuffer2[11];
      char timeBuffer2[9];
      strftime(dateBuffer2, 11, "%Y-%m-%d", localTime); // format YYYY-MM-DD
      strftime(timeBuffer2, 9, "%H:%M:%S", localTime); // format HH:MM:SS
      Firebase.pushString("/dateHeureCourant", dateBuffer2 + String(" ") + timeBuffer2);
      Serial.print("dateHeureCourant: ");
      Serial.println(dateBuffer2 + String(" ") + timeBuffer2);
} else {
  float sensitivity = 0.03 / 100;
  current = (voltValu - 2.5) * sensitivity * 1000;
  Firebase.pushFloat("/courant", current);
  Serial.print("Courant: ");
  Serial.print(current);
  Serial.println(" A");
   time_t now = time(nullptr);
      struct tm* localTime = localtime(&now);
      char dateBuffer2[11];
      char timeBuffer2[9];
      strftime(dateBuffer2, 11, "%Y-%m-%d", localTime); // format YYYY-MM-DD
      strftime(timeBuffer2, 9, "%H:%M:%S", localTime); // format HH:MM:SS
      Firebase.pushString("/dateHeureCourant", dateBuffer2 + String(" ") + timeBuffer2);
      Serial.print("dateHeureCourant: ");
      Serial.println(dateBuffer2 + String(" ") + timeBuffer2);
}

  IPAddress ip = WiFi.localIP();
  String ipString = String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
  Firebase.setString("/adresse_ip", ipString);

  // Ajouter l'adresse physique (adresse MAC) de l'ESP8266
  byte mac[6];
  WiFi.macAddress(mac);
  String macString = "";
  for (int i = 0; i < 6; i++) {
    macString += String(mac[i], HEX);
    if (i < 5) {
      macString += ":";
    }
  }
  Firebase.setString("/adresse_mac", macString);


  delay(5000);
}

