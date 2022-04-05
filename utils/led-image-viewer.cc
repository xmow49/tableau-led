#include "led-matrix.h"
#include "pixel-mapper.h"
#include "content-streamer.h"

#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <thread>
#include <sstream>

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include <Magick++.h>
#include <magick/image.h>

#include <errno.h>
#include <sys/inotify.h>
#include <iostream>
#include <fstream>

#include <nlohmann/json.hpp> //sudo apt-get install nlohmann-json-dev
using json = nlohmann::json;
using rgb_matrix::Canvas;
using rgb_matrix::FrameCanvas;
using rgb_matrix::RGBMatrix;
using rgb_matrix::StreamReader;

#include <wiringPi.h>

#define MAX_DISTANCE 200 // cm
bool DEBUG = false;
int SensorMesures[3] = {0, 0, 0};
unsigned int currentSensor[3] = {0, 0, 0};

unsigned char currentMode = 0; // 0: GIF - 1: DRAW

#define FILTRE 1

//--------- Custom LED Matrix Struct----------
struct MatrixLedData
{
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};
struct MatrixFrameBuffer
{
  MatrixLedData buffer[128][128];
};
struct MatrixAnimationBuffer
{
  MatrixFrameBuffer animation[400];
  uint16_t currentGifFrameCount = 0;
};

struct MatrixDrawBuffer
{
  MatrixFrameBuffer frames;
};

struct GifInfo
{
  uint16_t currentFrame = 0;
  uint8_t currentGIF = 0;
  uint8_t currentSpeed = 100;
};

MatrixAnimationBuffer matrixGifsList[10];
GifInfo gifInfo;
// MatrixDrawBuffer matrixDraw;
uint16_t test = 0;

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

#define IPC_PATH "/tmp/matrix"
#define IPC_FILE "IPC"
#define IPC_ALL IPC_PATH "/" IPC_FILE
//--------- LED Matrix Struct from Lib ----------
typedef int64_t tmillis_t;
static const tmillis_t distant_future = (1LL << 40); // that is a while.

struct ImageParams
{
  ImageParams() : anim_duration_ms(distant_future), wait_ms(1500),
                  anim_delay_ms(-1), loops(-1), vsync_multiple(1) {}
  tmillis_t anim_duration_ms; // If this is an animation, duration to show.
  tmillis_t wait_ms;          // Regular image: duration to show.
  tmillis_t anim_delay_ms;    // Animation delay override.
  int loops;
  int vsync_multiple;
};
struct FileInfo
{
  ImageParams params; // Each file might have specific timing settings
  bool is_multi_frame;
  rgb_matrix::StreamIO *content_stream;
};

volatile bool interrupt_received = false;

static void InterruptHandler(int signo)
{
  interrupt_received = true;
}

static tmillis_t GetTimeInMillis()
{
  struct timeval tp;
  gettimeofday(&tp, NULL);
  return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

static void SleepMillis(tmillis_t milli_seconds)
{
  if (milli_seconds <= 0)
    return;
  struct timespec ts;
  ts.tv_sec = milli_seconds / 1000;
  ts.tv_nsec = (milli_seconds % 1000) * 1000000;
  nanosleep(&ts, NULL);
}

std::string split(const std::string &chaine, char delimiteur, char index)
{
  std::stringstream ss(chaine);
  std::string sousChaine;
  for (int i = 0; i <= index && getline(ss, sousChaine, delimiteur); i++)
  {
    if (i == index)
    {
      // printf("%d: %s\n", i, sousChaine.c_str());
      return sousChaine;
    }
  }
  return "";
}

//----------- File between c and php ------------

void checkIPCFile()
{

  while (1)
  {
    //---- variables ----
    int length, i = 0;
    int fd;
    int wd;
    char buffer[BUF_LEN];
    //-------------------
    fd = inotify_init(); // init file lisen

    if (fd < 0) // check if init is ok
      printf("inotify_init\n");

    wd = inotify_add_watch(fd, IPC_PATH, IN_MODIFY | IN_CREATE | IN_DELETE); // setup watch event
    length = read(fd, buffer, BUF_LEN);

    if (length < 0)
      printf("read\n");

    if (i < length)
    {
      struct inotify_event *event = (struct inotify_event *)&buffer[i];
      if (event->len)
      {
        if (event->mask & IN_MODIFY) // if new modify event
        {
          bool interupt = false;
          // printf("The file %s was modified.\n", event->name);
          if (strcmp(event->name, IPC_FILE) == 0) // if its the matrix temp file
          {
            printf("New event from web interface\n");

            // Create a text string, which is used to output the text file
            std::string line;
            // Open the file
            std::ifstream FileIN;
            FileIN.open(IPC_ALL);
            while (getline(FileIN, line))
            {
              printf("read: %s\n", line.c_str());
              json j_complete = json::parse(line);
              // std::cout << std::setw(4) << j_complete << std::endl;
              // std::cout << std::setw(4) << j_complete["MODE"] << std::endl;

              std::string to = j_complete["TO"];
              if (to == "CPP")
              {
                printf("ITS for me\n");
              }
              else
              {
                interupt = true;
              }

              std::string mode = j_complete["MODE"];

              if (mode == "GIF")
              {
                printf("ITS a GIF\n");
                std::string gif = j_complete["GIF"];
                printf("GIF: %s\n", gif.c_str());
                gifInfo.currentGIF = atoi(gif.c_str()); // applique le changement

                std::string speed = j_complete["SPEED"];
                printf("SPEED: %s\n", speed.c_str());
                gifInfo.currentSpeed = atoi(speed.c_str()); // applique le changement
                printf("currentSpeed: %d\n", gifInfo.currentSpeed);

              }

              // if (key == "TO")
              // {
              //   if (value == "CPP")
              //   {
              //     printf("ITS for me\n");
              //   }
              //   else
              //   {
              //     interupt = true;
              //   }
              // }
              // else if (key == "MODE")
              // {
              //   if (value == "GIF")
              //   {
              //     printf("GIF MODE\n");
              //     currentMode = 0;
              //   }

              //   else if (value == "DRAW")
              //   {
              //     printf("DRAW MODE\n");
              //     currentMode = 1;
              //   }
              // }
              // else if (key == "GIF")
              // {
              //   gifInfo.currentGIF = atoi(value.c_str());
              //   printf("GIF %s\n", value.c_str());
              // }
              // else if (key == "LED")
              // {
              //   // std::string strLed = value.substr(0, value.find(':'));

              //   // unsigned char led[2] = {std::stoi(split(strLed, ',', 0)), std::stoi(split(strLed, ',', 1))}; // get x and y of the led, convert it into char

              //   // std::string strColor = value.substr(value.find(':') + 1);
              //   // unsigned char color[3] = {std::stoi(split(strColor, ',', 0)), std::stoi(split(strColor, ',', 1)), std::stoi(split(strColor, ',', 2))};

              //   // printf("SET LED %d:%d TO %d:%d:%d\n", led[0], led[1], color[0], color[1], color[2]);
              //   // matrixDraw.frames.buffer[led[1]][led[0]].red = color[0];
              //   // matrixDraw.frames.buffer[led[1]][led[0]].green = color[1];
              //   // matrixDraw.frames.buffer[led[1]][led[0]].blue = color[2];
              // }
            }

            FileIN.clear();

            FileIN.close();
            if (!interupt)
            {
              std::ifstream File;
              File.open("matrix", std::ifstream::out | std::ifstream::trunc);
              if (!File.is_open() || File.fail())
              {
                File.close();
                printf("\nError : failed to erase file content !");
              }
              File.close();
              // printf("Clearing file\n\n");
              // // delete all file
              // std::ofstream ofs;

              // ofs.open(IPC_FILE, std::ofstream::out | std::ofstream::trunc);
              // if (!ofs)
              // {
              //   std::cout << "Could not truncate \n";
              // }
              // ofs.close();
            }
          }
        }
      }
      i += EVENT_SIZE + event->len;
    }
  }
}

//--------- Ultrasonic Sensor ----------

// pins
int smoothSensorsValues[3] = {100, 100, 100};
const unsigned char sensorsPins[3] = {8, 9, 25}; // https://fr.pinout.xyz/pinout/wiringpi //25 8 9

int getSensor(unsigned char nSensor, int minDistance = 5, int maxDistance = 300) // sensor n° 0 1 2
{
  if (nSensor > 2)
  {
    return -1;
  }

  long ping = 0;
  long pong = 0;
  int distance = 0;
  long timeout = 20000;

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

void sensorLoop(int minDistance = 5, int maxDistance = 300)
{
  while (1)
  {
    getSensor(0, minDistance, maxDistance);
    SleepMillis(20);
    getSensor(1, minDistance, maxDistance);
    SleepMillis(20);
    getSensor(2, minDistance, maxDistance);
    SleepMillis(20);
    // printf("%d %d %d\n", currentSensor[0], currentSensor[1], currentSensor[2]);
  }
}

unsigned char fixBlack(unsigned char value)
{
  if (value < 5)
  {
    return 0;
  }
  else
  {
    return value;
  }
}

static void StoreInStream(const Magick::Image &img, int delay_time_us,
                          bool do_center,
                          rgb_matrix::FrameCanvas *scratch,
                          rgb_matrix::StreamWriter *output)
{
  scratch->Clear();
  const int x_offset = do_center ? (scratch->width() - img.columns()) / 2 : 0;
  const int y_offset = do_center ? (scratch->height() - img.rows()) / 2 : 0;

  for (size_t y = 0; y < img.rows(); ++y)
  {
    for (size_t x = 0; x < img.columns(); ++x)
    {
      const Magick::Color &c = img.pixelColor(x, y);
      if (c.alphaQuantum() < 256)
      {
        scratch->SetPixel(x + x_offset, y + y_offset,
                          ScaleQuantumToChar(c.redQuantum()),
                          ScaleQuantumToChar(c.greenQuantum()),
                          ScaleQuantumToChar(c.blueQuantum()));

        matrixGifsList[gifInfo.currentGIF].animation[gifInfo.currentFrame].buffer[y + y_offset][x + x_offset].red = fixBlack(c.redQuantum());
        matrixGifsList[gifInfo.currentGIF].animation[gifInfo.currentFrame].buffer[y + y_offset][x + x_offset].green = fixBlack(c.greenQuantum());
        matrixGifsList[gifInfo.currentGIF].animation[gifInfo.currentFrame].buffer[y + y_offset][x + x_offset].blue = fixBlack(c.blueQuantum());
      }
    }
  }

  output->Stream(*scratch, delay_time_us);
}

static void CopyStream(rgb_matrix::StreamReader *r,
                       rgb_matrix::StreamWriter *w,
                       rgb_matrix::FrameCanvas *scratch)
{
  uint32_t delay_us;
  while (r->GetNext(scratch, &delay_us))
  {
    w->Stream(*scratch, delay_us);
  }
}

// Load still image or animation.
// Scale, so that it fits in "width" and "height" and store in "result".
static bool LoadImageAndScale(const char *filename,
                              int target_width, int target_height,
                              bool fill_width, bool fill_height,
                              std::vector<Magick::Image> *result,
                              std::string *err_msg)
{
  std::vector<Magick::Image> frames;
  try
  {
    readImages(&frames, filename);
  }
  catch (std::exception &e)
  {
    if (e.what())
      *err_msg = e.what();
    return false;
  }
  if (frames.size() == 0)
  {
    fprintf(stderr, "No image found.");
    return false;
  }

  // Put together the animation from single frames. GIFs can have nasty
  // disposal modes, but they are handled nicely by coalesceImages()
  if (frames.size() > 1)
  {
    Magick::coalesceImages(result, frames.begin(), frames.end());
  }
  else
  {
    result->push_back(frames[0]); // just a single still image.
  }

  const int img_width = (*result)[0].columns();
  const int img_height = (*result)[0].rows();
  const float width_fraction = (float)target_width / img_width;
  const float height_fraction = (float)target_height / img_height;
  if (fill_width && fill_height)
  {
    // Scrolling diagonally. Fill as much as we can get in available space.
    // Largest scale fraction determines that.
    const float larger_fraction = (width_fraction > height_fraction)
                                      ? width_fraction
                                      : height_fraction;
    target_width = (int)roundf(larger_fraction * img_width);
    target_height = (int)roundf(larger_fraction * img_height);
  }
  else if (fill_height)
  {
    // Horizontal scrolling: Make things fit in vertical space.
    // While the height constraint stays the same, we can expand to full
    // width as we scroll along that axis.
    target_width = (int)roundf(height_fraction * img_width);
  }
  else if (fill_width)
  {
    // dito, vertical. Make things fit in horizontal space.
    target_height = (int)roundf(width_fraction * img_height);
  }

  for (size_t i = 0; i < result->size(); ++i)
  {
    (*result)[i].scale(Magick::Geometry(target_width, target_height));
  }

  return true;
}

int map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void DisplayAnimation(const FileInfo *file,
                      RGBMatrix *matrix, FrameCanvas *offscreen_canvas)
{
  const tmillis_t duration_ms = (file->is_multi_frame
                                     ? file->params.anim_duration_ms
                                     : file->params.wait_ms);
  rgb_matrix::StreamReader reader(file->content_stream);
  int loops = file->params.loops;
  const tmillis_t end_time_ms = GetTimeInMillis() + duration_ms;
  // const tmillis_t override_anim_delay = file->params.anim_delay_ms;

  uint32_t delay_us = 0;
  int sensor = -1;
  int moyenne = 0;

  for (uint16_t frame = 0; (frame < matrixGifsList[gifInfo.currentGIF].currentGifFrameCount - 1) && !interrupt_received; frame++)
  {

    
    //-------------------Gestion des transition fluides ------------------------
    for (int i = 0; i < 3; i++)
    {
      if (currentSensor[i] < smoothSensorsValues[i])// si la nouvelle valeur du capteur est inférieur a la dernière enregistrer,
      {
        smoothSensorsValues[i]--; //on décrémente de 1
      }
      else if (currentSensor[i] > smoothSensorsValues[i]) // si la nouvelle valeur du capteur est supérieur a la dernière enregistrer,
      {
        smoothSensorsValues[i]++;//on incrémente de 1
      }
    }

    if(DEBUG){
      printf("%d %d %d\n", smoothSensorsValues[0], smoothSensorsValues[1], smoothSensorsValues[2]);
    }
  
    //----------------------------------------------------------------------------
    
    for (uint16_t y = 0; y < 128; y++)
    {
      for (uint16_t x = 0; x < 128; x++)
      {
        if (FILTRE)
        {

          offscreen_canvas->SetPixel(x, y,
                                     map(smoothSensorsValues[0], 5, MAX_DISTANCE, 255, 20) * matrixGifsList[gifInfo.currentGIF].animation[frame].buffer[y][x].red / 255,
                                     map(smoothSensorsValues[1], 5, MAX_DISTANCE, 255, 20) * matrixGifsList[gifInfo.currentGIF].animation[frame].buffer[y][x].green / 255,
                                     map(smoothSensorsValues[2], 5, MAX_DISTANCE, 255, 20) * matrixGifsList[gifInfo.currentGIF].animation[frame].buffer[y][x].blue / 255);
          // offscreen_canvas->SetPixel(x, y, 255,255,255);
          // printf("before:%d after:%d\n", matrixGifsList[gifInfo.currentGIF].animation[frame].buffer[y][x].red, map(currentSensor[0], 5, MAX_DISTANCE, 0, 255) * matrixGifsList[gifInfo.currentGIF].animation[frame].buffer[y][x].red / 255);
        }
        else
        {
          offscreen_canvas->SetPixel(x, y,
                                     matrixGifsList[gifInfo.currentGIF].animation[frame].buffer[y][x].red,
                                     matrixGifsList[gifInfo.currentGIF].animation[frame].buffer[y][x].green,
                                     matrixGifsList[gifInfo.currentGIF].animation[frame].buffer[y][x].blue);
        }
      }
    }
    offscreen_canvas = matrix->SwapOnVSync(offscreen_canvas,
                                           file->params.vsync_multiple);

    SleepMillis( (15 * (100-gifInfo.currentSpeed)) / 100);
  }
}

static int usage(const char *progname) // command usage
{
  fprintf(stderr, "usage: %s [options] <image> [option] [<image> ...]\n",
          progname);

  fprintf(stderr, "C pas ouf ce que ta mis comme parametres");

  fprintf(stderr, "\nGeneral LED matrix options:\n");
  rgb_matrix::PrintMatrixFlags(stderr);

  fprintf(stderr,
          "\nSwitch time between files: "
          "-w for static images; -t/-l for animations\n"
          "Animated gifs: If both -l and -t are given, "
          "whatever finishes first determines duration.\n");

  fprintf(stderr, "\nThe -w, -t and -l options apply to the following images "
                  "until a new instance of one of these options is seen.\n"
                  "So you can choose different durations for different images.\n");
  return 1;
}

int main(int argc, char *argv[])
{

  wiringPiSetup(); // setup pins

  std::thread CheckSensor(sensorLoop, 5, 200); // thread to check sensor

  std::thread CheckIPC(checkIPCFile); // thread to check if there is a new content in file

  Magick::InitializeMagick(*argv); // Initialize ImageMagick

  RGBMatrix::Options matrix_options; // options for the matrix

  //---------- Matrix options ----------
  matrix_options.rows = 64;
  matrix_options.cols = 64;
  matrix_options.chain_length = 2;
  matrix_options.parallel = 2;
  // matrix_options.show_refresh_rate = true;
  matrix_options.brightness = 100;

  matrix_options.pwm_bits = 11;
  matrix_options.pwm_lsb_nanoseconds = 50;

  //-------------------------------------

  rgb_matrix::RuntimeOptions runtime_opt;
  if (!rgb_matrix::ParseOptionsFromFlags(&argc, &argv,
                                         &matrix_options, &runtime_opt))
  {
    return usage(argv[0]);
  }

  bool do_forever = false;
  bool do_center = false;
  bool do_shuffle = false;

  // We remember ImageParams for each image, which will change whenever
  // there is a flag modifying them. This map keeps track of filenames
  // and their image params (also for unrelated elements of argv[], but doesn't
  // matter).
  // We map the pointer instad of the string of the argv parameter so that
  // we can have two times the same image on the commandline list with different
  // parameters.
  std::map<const void *, struct ImageParams> filename_params;

  // Set defaults.
  ImageParams img_param;
  for (int i = 0; i < argc; ++i)
  {
    filename_params[argv[i]] = img_param;
  }

  const char *stream_output = NULL;

  int opt;
  while ((opt = getopt(argc, argv, "w:t:l:fr:c:P:LhCR:sO:V:D:")) != -1)
  {
    switch (opt)
    {
    case 'w':
      img_param.wait_ms = roundf(atof(optarg) * 1000.0f);
      break;
    case 't':
      img_param.anim_duration_ms = roundf(atof(optarg) * 1000.0f);
      break;
    case 'l':
      img_param.loops = atoi(optarg);
      break;
    case 'D':
      img_param.anim_delay_ms = atoi(optarg);
      break;
    case 'f':
      do_forever = true;
      break;
    case 'C':
      do_center = true;
      break;
    case 's':
      do_shuffle = true;
      break;
    case 'r':
      fprintf(stderr, "Instead of deprecated -r, use --led-rows=%s instead.\n",
              optarg);
      matrix_options.rows = atoi(optarg);
      break;
    case 'c':
      fprintf(stderr, "Instead of deprecated -c, use --led-chain=%s instead.\n",
              optarg);
      matrix_options.chain_length = atoi(optarg);
      break;
    case 'P':
      matrix_options.parallel = atoi(optarg);
      break;
    case 'L':
      fprintf(stderr, "-L is deprecated. Use\n\t--led-pixel-mapper=\"U-mapper\" --led-chain=4\ninstead.\n");
      return 1;
      break;
    case 'R':
      fprintf(stderr, "-R is deprecated. "
                      "Use --led-pixel-mapper=\"Rotate:%s\" instead.\n",
              optarg);
      return 1;
      break;
    case 'O':
      stream_output = strdup(optarg);
      break;
    case 'V':
      img_param.vsync_multiple = atoi(optarg);
      if (img_param.vsync_multiple < 1)
        img_param.vsync_multiple = 1;
      break;
    case 'h':
    default:
      return usage(argv[0]);
    }

    // Starting from the current file,
    // the latest change.
    for (int i = optind; i < argc; ++i)
    {
      filename_params[argv[i]] = img_param;
    }
  }

  const int filename_count = argc - optind;
  if (filename_count == 0)
  {
    fprintf(stderr, "Expected image filename.\n");
    return usage(argv[0]);
  }

  // --------------- Prepare matrix ---------------
  runtime_opt.do_gpio_init = (stream_output == NULL);
  RGBMatrix *matrix = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
  if (matrix == NULL)
    return 1;

  FrameCanvas *offscreen_canvas = matrix->CreateFrameCanvas();

  printf("Size: %dx%d. Hardware gpio mapping: %s\n",
         matrix->width(), matrix->height(), matrix_options.hardware_mapping);

  // These parameters are needed once we do scrolling.
  const bool fill_width = false;
  const bool fill_height = false;

  // In case the output to stream is requested, set up the stream object.
  rgb_matrix::StreamIO *stream_io = NULL;
  rgb_matrix::StreamWriter *global_stream_writer = NULL;
  if (stream_output)
  {
    int fd = open(stream_output, O_CREAT | O_WRONLY, 0644);
    if (fd < 0)
    {
      perror("Couldn't open output stream");
      return 1;
    }
    stream_io = new rgb_matrix::FileStreamIO(fd);
    global_stream_writer = new rgb_matrix::StreamWriter(stream_io);
  }

  const tmillis_t start_load = GetTimeInMillis();
  fprintf(stderr, "Loading %d files...\n", argc - optind);
  // Preparing all the images beforehand as the Pi might be too slow to
  // be quickly switching between these. So preprocess.
  std::vector<FileInfo *> file_imgs;
  printf("Analyzing Gifs...\n");
  for (int imgarg = optind; imgarg < argc; ++imgarg)
  {
    printf("for %d\n", imgarg);
    gifInfo.currentGIF = imgarg - 1;
    const char *filename = argv[imgarg];
    FileInfo *file_info = NULL;

    std::string err_msg;
    std::vector<Magick::Image> image_sequence;
    if (LoadImageAndScale(filename, matrix->width(), matrix->height(),
                          fill_width, fill_height, &image_sequence, &err_msg))
    {
      file_info = new FileInfo();
      file_info->params = filename_params[filename];
      file_info->content_stream = new rgb_matrix::MemStreamIO();
      file_info->is_multi_frame = image_sequence.size() > 1;
      rgb_matrix::StreamWriter out(file_info->content_stream);
      printf("Gif Frame count: %d\n", image_sequence.size());
      matrixGifsList[gifInfo.currentGIF].currentGifFrameCount = image_sequence.size();

      for (size_t i = 0; i < image_sequence.size(); ++i)
      {
        const Magick::Image &img = image_sequence[i];
        int64_t delay_time_us;
        if (file_info->is_multi_frame)
        {
          delay_time_us = img.animationDelay() * 10000; // unit in 1/100s
        }
        else
        {
          delay_time_us = file_info->params.wait_ms * 1000; // single image.
        }
        if (delay_time_us <= 0)
          delay_time_us = 100 * 1000; // 1/10sec
        StoreInStream(img, delay_time_us, do_center, offscreen_canvas,
                      global_stream_writer ? global_stream_writer : &out);

        gifInfo.currentFrame = i;
      }
    }
    else
    {
      // Ok, not an image. Let's see if it is one of our streams.
      int fd = open(filename, O_RDONLY);
      if (fd >= 0)
      {
        file_info = new FileInfo();
        file_info->params = filename_params[filename];
        file_info->content_stream = new rgb_matrix::FileStreamIO(fd);
        StreamReader reader(file_info->content_stream);
        if (reader.GetNext(offscreen_canvas, NULL))
        { // header+size ok
          file_info->is_multi_frame = reader.GetNext(offscreen_canvas, NULL);
          reader.Rewind();
          if (global_stream_writer)
          {
            CopyStream(&reader, global_stream_writer, offscreen_canvas);
          }
        }
        else
        {
          err_msg = "Can't read as image or compatible stream";
          delete file_info->content_stream;
          delete file_info;
          file_info = NULL;
        }
      }
      else
      {
        perror("Opening file");
      }
    }

    if (file_info)
    {
      file_imgs.push_back(file_info);
    }
    else
    {
      fprintf(stderr, "%s skipped: Unable to open (%s)\n",
              filename, err_msg.c_str());
    }
  }

  printf(" OK\n");
  if (stream_output)
  {
    delete global_stream_writer;
    delete stream_io;
    if (file_imgs.size())
    {
      fprintf(stderr, "Done: Output to stream %s; "
                      "this can now be opened with led-image-viewer with the exact same panel configuration settings such as rows, chain, parallel and hardware-mapping\n",
              stream_output);
    }
    if (do_shuffle)
      fprintf(stderr, "Note: -s (shuffle) does not have an effect when generating streams.\n");
    if (do_forever)
      fprintf(stderr, "Note: -f (forever) does not have an effect when generating streams.\n");
    // Done, no actual output to matrix.
    return 0;
  }

  printf("file size: %d\n", file_imgs.size());
  // Some parameter sanity adjustments.
  if (file_imgs.empty())
  {
    // e.g. if all files could not be interpreted as image.
    fprintf(stderr, "No image could be loaded.\n");
    return 1;
  }
  else if (file_imgs.size() == 1)
  {
    // Single image: show forever.
    file_imgs[0]->params.wait_ms = distant_future;
  }
  else
  {
    for (size_t i = 0; i < file_imgs.size(); ++i)
    {
      ImageParams &params = file_imgs[i]->params;
      // Forever animation ? Set to loop only once, otherwise that animation
      // would just run forever, stopping all the images after it.
      if (params.loops < 0 && params.anim_duration_ms == distant_future)
      {
        params.loops = 1;
      }
    }
  }


  fprintf(stderr, "Loading took %.3fs; now: Display.\n",
          (GetTimeInMillis() - start_load) / 1000.0);


  // signal pour fermer le programme
  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);
  //------------------------------------
  do
  {

    switch (currentMode)
    {
    case 0: // GIF MODE
      DisplayAnimation(file_imgs[0], matrix, offscreen_canvas);
      break;
    case 1: //draw mode

    default:
      break;
    }

  } while (!interrupt_received);

  if (interrupt_received)
  {
    fprintf(stderr, "Caught signal. Exiting.\n");
  }

  // Animation finished. Shut down the RGB matrix.
  matrix->Clear();
  delete matrix;
  // Leaking the FileInfos, but don't care at program end.

  CheckSensor.detach();
  CheckIPC.detach();

  return 0;
}