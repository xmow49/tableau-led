#include <wiringPi.h>
#include <stdio.h>
#include <unistd.h>

const unsigned char sensorsPins[3] = {8, 9, 25}; // https://fr.pinout.xyz/pinout/wiringpi //25 8 9

#define DEBUG true

int getSensor(unsigned char nSensor) // récupère la valeur d'un capeur en foncion de nsensor n° 0 1 2
{
  long ping = 0;
  long pong = 0;
  int distance = 0;
  long timeout = 50000;

  // preapare the sensor, Ensure trigger is low.
  pinMode(sensorsPins[nSensor], OUTPUT); // set pin as output
  digitalWrite(sensorsPins[nSensor], LOW); // set pin low
  delayMicroseconds(50);                   // 50us
  
  printf("Send\n");
  // Trigger the ping.
  digitalWrite(sensorsPins[nSensor], HIGH);
  delayMicroseconds(10);
  digitalWrite(sensorsPins[nSensor], LOW);
  
  delayMicroseconds(10);
  pinMode(sensorsPins[nSensor], INPUT);

  // Wait for ping response, or timeout.
  unsigned int end = micros() + timeout;

  printf("Wait\n");
  //while (digitalRead(sensorsPins[nSensor]) == LOW && micros() < end) //on attend la réponse 
  while (digitalRead(sensorsPins[nSensor]) == LOW) //on attend la réponse 
  {}
  printf("Ping\n");

  ping = micros(); // get ping time

  printf("Wait\n");
  //while (digitalRead(sensorsPins[nSensor]) == HIGH && micros() < end) // on attend la fin de réponse
  while (digitalRead(sensorsPins[nSensor]) == HIGH) // on attend la fin de réponse
  {}
  printf("Pong\n");

  pong = micros(); // get pong time
  printf("%d\n", (pong - ping));

  distance = (pong - ping) * 0.017150;// Convert ping duration to distance.
  return distance;
}

int main(){

  wiringPiSetup();
int a = 29;
pinMode(a,OUTPUT);
while(1){
      digitalWrite(a, HIGH);
      printf("on\n");
	//printf("%d\n", getSensor(1));
      delay(1000);
      digitalWrite(a, LOW);
	printf("off\n");
      delay(1000);

  }

}
