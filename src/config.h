#define DEBUG false


//-------Pixel Pour UN PANNEAU-------
#define MATRIX_ROWS 64
#define MATRIX_COLS 64
//------regalge de la disposition de panneaux-------
// on a 4 panneaux de 64x64 --> 128x128 pixels
// ils sont disposer en carrés donc: 2x2 panneaus de 64x64
#define MATRIX_CHAIN 2 //12 max
#define MATRIX_PARALLEL 2 //3 max


#define MAX_DISTANCE 200 // cm
#define MIN_DISTANCE 10 // cm

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (2048 * (EVENT_SIZE + 16))

#define IPC_PATH "/tmp/matrix"
#define IPC_FILE "IPC"
#define IPC_ALL IPC_PATH "/" IPC_FILE

int SensorMesures[3] = {0, 0, 0};
unsigned int currentSensor[3] = {0, 0, 0};
unsigned int smoothSensorsValues[3] = {100, 100, 100};
const unsigned char sensorsPins[3] = {8, 9, 25}; // https://fr.pinout.xyz/pinout/wiringpi //25 8 9



