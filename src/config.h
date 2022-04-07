#define DEBUG false

#define MAX_DISTANCE 200 // cm


#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (2048 * (EVENT_SIZE + 16))

#define IPC_PATH "/tmp/matrix"
#define IPC_FILE "IPC"
#define IPC_ALL IPC_PATH "/" IPC_FILE

int SensorMesures[3] = {0, 0, 0};
unsigned int currentSensor[3] = {0, 0, 0};
unsigned int smoothSensorsValues[3] = {100, 100, 100};
const unsigned char sensorsPins[3] = {8, 9, 25}; // https://fr.pinout.xyz/pinout/wiringpi //25 8 9