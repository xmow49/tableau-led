#include <wiringPi.h>
#include <stdio.h>
#include <unistd.h>

unsigned int currentSensor[3] = {0, 0, 0};
const unsigned char sensorsPins[3] = {8, 9, 25}; // https://fr.pinout.xyz/pinout/wiringpi //25 8 9

#define DEBUG false

int getSensor(unsigned char nSensor, int minDistance = 5, int maxDistance = 300) // récupère la valeur d'un capeur en foncion de nsensor n° 0 1 2
{
  if (nSensor > 2)
    return -1;

  long ping = 0;
  long pong = 0;
  int distance = 0;
  long timeout = 50000;

  // preapare the sensor, Ensure trigger is low.
  pinMode(sensorsPins[nSensor], OUTPUT); // set pin as output

  digitalWrite(sensorsPins[nSensor], LOW); // set pin low
  delayMicroseconds(50);                   // 50us

  // Trigger the ping.
  digitalWrite(sensorsPins[nSensor], HIGH);
  delayMicroseconds(10);
  digitalWrite(sensorsPins[nSensor], LOW);

  // Wait for the ping to complete, pin in input mode.
  pinMode(sensorsPins[nSensor], INPUT);
  // Wait for ping response, or timeout.
  unsigned int end = micros() + timeout;

  while (digitalRead(sensorsPins[nSensor]) == LOW && micros() < end)
  {
  }
  // Cancel on timeout.
  if (micros() + 10 > end) //+10 to be sure
  {
    if (DEBUG)
    {
      printf("Out of range.\n");
      currentSensor[nSensor] = -1;
      return -1;
    }
  }

  ping = micros(); // get ping time

  // Wait for pong response, or timeout.
  while (digitalRead(sensorsPins[nSensor]) == HIGH && micros() < end)
  {
  }
  // Cancel on timeout.
  if (micros() + 10 > end) //+10 to be sure


  {
    if (DEBUG)
    {
      printf("Out of range.\n");
      currentSensor[nSensor] = -1;
      return -1;
    }
  }

  pong = micros(); // get pong time

  // Convert ping duration to distance.
  distance = (pong - ping) * 0.017150;

  if (distance == 0)
  {
    if (DEBUG)
    {
      printf("Out of range.\n");
      currentSensor[nSensor] = -1;
      return -1;
    }
  }
  // test the distance if is real
  if (distance > maxDistance)
  {
    distance = maxDistance;
  }
  else if (distance < minDistance)
  {
    distance = minDistance;
  }

  currentSensor[nSensor] = distance;

  return distance;
}

int main(){

wiringPiSetup();
while(1){
    getSensor(1, 50, 350);
    printf("%d\n", currentSensor[0]);
    usleep(50000);

}
}