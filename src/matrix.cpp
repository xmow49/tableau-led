//----------------------- Déclarations des library -------------------------
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
#include <vector>

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

//-----------Librarys externes: info dans le fichier README----------------------

// #include <nlohmann/json.hpp>
#include <wiringPi.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

//-----------------------------------------------------------------------------

#include "config.h"

//----------------------- Déclarations des namespaces -------------------------
// using json = nlohmann::json;
using namespace rapidjson;
using rgb_matrix::Canvas;
using rgb_matrix::FrameCanvas;
using rgb_matrix::RGBMatrix;
using rgb_matrix::StreamReader;
using std::vector;

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//-----------------------------------------------------------------------------

//--------- Déclarations des  structures --------------------------------------
struct MatrixLedData
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};
struct MatrixFrameBuffer
{
    MatrixLedData buffer[128][128]; // une frame de 128*128 pixel
};
struct MatrixAnimationBuffer
{
    MatrixFrameBuffer animation[400]; // 400 frame max par gif
    uint16_t currentGifFrameCount = 0;
};

struct GifInfo
{
    uint16_t currentFrame = 0;
    uint8_t currentGIF = 0;
    uint8_t currentSpeed = 100;
    bool filterEnable = true;
    uint8_t loadingScreenFrameCount = 0;
    bool loadingScreenState = false;
    bool interruptDelayAnimation = false;
};

//-----------------------------------------------------------------------------

//----------------------- Déclarations des variables globales --------

MatrixAnimationBuffer matrixGifsList[12];
GifInfo gifInfo;
volatile bool interrupt_received = false;

//-----------------------------------------------------------------------------

//------------------ LED Matrix Struct from Lib --------------------------------
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

//-----------------------------------------------------------------------------

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

// bool jsonKeyExists(const json &j, const std::string &key)
// {
//   return j.find(key) != j.end();
// }

int getSensor(unsigned char nSensor, int minDistance = 5, int maxDistance = 300) // récupère la valeur d'un capeur en foncion de nsensor n° 0 1 2
{
    if (nSensor > 2)
        return -1;

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

void sensorLoop(int minDistance = 5, int maxDistance = 300) // loop qui check les sensor dans un thread parallèle
{
    printf("Sensor Loop thread: PID: %d, ID: %d\n", getpid(), gettid());

    while (1)
    {
        getSensor(0, minDistance, maxDistance);
        SleepMillis(100);
        getSensor(1, minDistance, maxDistance);
        SleepMillis(100);
        getSensor(2, minDistance, maxDistance);
        SleepMillis(100);
        //printf("%d %d %d\n", currentSensor[0], currentSensor[1], currentSensor[2]); //peueter faire val +1
    }
}

unsigned char fixBlack(unsigned char value) // met un offset pour la couleur noir du gif: évite les LED semis allumé avec le fond noir
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

int map(long x, long in_min, long in_max, long out_min, long out_max) // fonction map qui sert pour les valeurs des sensors de 5cm --> 200cm to 0 --> 255
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static void StoreInStream(const Magick::Image &img) // récupère les gif et les stock dans la structure + met dans un canvas (qui viens de la library, mais pas utiliser)
{
    for (size_t y = 0; y < img.rows(); ++y)
    {
        for (size_t x = 0; x < img.columns(); ++x)
        {
            const Magick::Color &c = img.pixelColor(x, y);
            if (c.alphaQuantum() < 256)
            {
                matrixGifsList[gifInfo.currentGIF].animation[gifInfo.currentFrame].buffer[y][x].red = fixBlack(c.redQuantum());     // on fix le black car quand on a 2-3 en valeur; les led s'allume un peu et c'est moche
                matrixGifsList[gifInfo.currentGIF].animation[gifInfo.currentFrame].buffer[y][x].green = fixBlack(c.greenQuantum()); // donc on remet a 0 0 0 pour avoir un noir parfait
                matrixGifsList[gifInfo.currentGIF].animation[gifInfo.currentFrame].buffer[y][x].blue = fixBlack(c.blueQuantum());
            }
        }
    }
}

// Load still image or animation.
// Scale, so that it fits in "width" and "height" and store in "result".
static bool LoadImageAndScale(const char *filename,
                              int target_width, int target_height,
                              bool fill_width, bool fill_height,
                              std::vector<Magick::Image> *result,
                              std::string *err_msg) // resize les gif
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
    return true;
}

void DisplayAnimation(RGBMatrix *matrix) // fonction appeler pour afficher les gif
{
    gifInfo.interruptDelayAnimation = false;
    FrameCanvas *offscreen_canvas = matrix->CreateFrameCanvas();
    for (uint16_t frame = 0; (frame <= matrixGifsList[gifInfo.currentGIF].currentGifFrameCount - 1) && !interrupt_received && !gifInfo.interruptDelayAnimation; frame++)
    {
        //printf("Frame: %d\n", frame);
        //-------------------Gestion des transition fluides ------------------------
        for (int i = 0; i < 3; i++)
        {
            if (currentSensor[i] < smoothSensorsValues[i]) // si la nouvelle valeur du capteur est inférieur a la dernière enregistrer,
            {
                smoothSensorsValues[i] -= 2; // on décrémente de 1
            }
            else if (currentSensor[i] > smoothSensorsValues[i]) // si la nouvelle valeur du capteur est supérieur a la dernière enregistrer,
            {
                smoothSensorsValues[i] += 2; // on incrémente de 1
            }
        }
        if (DEBUG)
        {
            printf("%d %d %d\n", smoothSensorsValues[0], smoothSensorsValues[1], smoothSensorsValues[2]);
        }
        //----------------------------------------------------------------------------

        for (uint16_t y = 0; y < 128; y++)
        {
            for (uint16_t x = 0; x < 128; x++)
            {
                if (gifInfo.filterEnable)
                {

                    offscreen_canvas->SetPixel(x, y,
                                               map(smoothSensorsValues[0], MIN_DISTANCE, MAX_DISTANCE, 255, 20) * matrixGifsList[gifInfo.currentGIF].animation[frame].buffer[y][x].red / 255,
                                               map(smoothSensorsValues[1], MIN_DISTANCE, MAX_DISTANCE, 255, 20) * matrixGifsList[gifInfo.currentGIF].animation[frame].buffer[y][x].green / 255,
                                               map(smoothSensorsValues[2], MIN_DISTANCE, MAX_DISTANCE, 255, 20) * matrixGifsList[gifInfo.currentGIF].animation[frame].buffer[y][x].blue / 255);
                    // offscreen_canvas->SetPixel(x, y, 255,255,255);
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
        offscreen_canvas = matrix->SwapOnVSync(offscreen_canvas, 1);
        if(gifInfo.currentSpeed == 0){
            SleepMillis(100);
        }
        else if( gifInfo.currentSpeed == 1){
            for(uint8_t i = 0; i < 50 && !gifInfo.interruptDelayAnimation; i++){
                SleepMillis(100);
            }
            
        }
        else {
            SleepMillis((15 * (100 - gifInfo.currentSpeed)) / 100);
        }
        

    }
}

static int usage(const char *progname) // fontion qui retourne la page d'aide, mais elle sert plus a grand chose, jai tous enlever pour simpifier le code
{
    fprintf(stderr, "usage: %s <loading gif> <gif 1> <gif 2> <gif 3> <gif 4>\n",
            progname);

    fprintf(stderr, "\nGeneral LED matrix options:\n");
    fprintf(stderr, "\nExemple:\n");
    fprintf(stderr, "\n%s ./gifs/loading.gif ./gifs/6.gif ./gifs/4.gif ./gifs/3.gif ./gifs/2.gif\n", progname);
    return 1;
}

void do_session(tcp::socket socket)
{
    try
    {
        // Construct the stream by moving in the socket
        websocket::stream<tcp::socket> ws{std::move(socket)};

        // Set a decorator to change the Server of the handshake
        ws.set_option(websocket::stream_base::decorator(
            [](websocket::response_type &res)
            {
                res.set(http::field::server,
                        std::string(BOOST_BEAST_VERSION_STRING) +
                            " websocket-server-sync");
            }));

        // Accept the websocket handshake
        ws.accept();

        for (;;)
        {
            // This buffer will hold the incoming message
            beast::flat_buffer buffer;

            // Read a message
            ws.read(buffer);

            // Echo the message back
            // ws.text(ws.got_text());
            // ws.write(buffer.data());

            std::string recevied = beast::buffers_to_string(buffer.data());
            // std::cout << "New Message from WebSocket: " << recevied << std::endl;

            beast::flat_buffer response;
            beast::ostream(response) << "OK";
            ws.write(response.data());

            Document json;
            json.Parse(recevied.c_str());

            if (json.HasMember("TO"))
            {
                if (json["TO"] == "CPP")
                {
                    if (json.HasMember("MODE"))
                    {
                        std::string mode = json["MODE"].GetString();

                        if (mode == "GIF")
                        {
                            if (json.HasMember("FILTER"))
                            {
                                gifInfo.filterEnable = json["FILTER"].GetBool();
                            }
                            else
                            {
                                gifInfo.filterEnable = true; // active le filtre
                            }

                            if (json.HasMember("GIF"))
                            {
                                uint8_t gif = atoi(json["GIF"].GetString());
                                gifInfo.currentGIF = gif;
                                gifInfo.currentFrame = 0;
                                gifInfo.interruptDelayAnimation = true;
                            }
                            if (json.HasMember("SPEED"))
                            {
                                uint8_t gif = atoi(json["SPEED"].GetString());
                                gifInfo.currentSpeed = gif;
                            }
                            printf("Changment de GIF: GIF%d , Vitesse: %d , Filtre: %d\n", gifInfo.currentGIF, gifInfo.currentSpeed, gifInfo.filterEnable);
                        }
                        else if (mode == "DRAW")
                        {
                            if (json.HasMember("DRAW"))
                            {
                                if (json["DRAW"] == "CLEAR")
                                {
                                    printf("Clearing the matrix...\n");
                                    for (uint8_t y = 0; y < 128; y++)
                                    {
                                        for (uint8_t x = 0; x < 128; x++)
                                        {
                                            matrixGifsList[12].animation[0].buffer[y][x].red = 0;
                                            matrixGifsList[12].animation[0].buffer[y][x].green = 0;
                                            matrixGifsList[12].animation[0].buffer[y][x].blue = 0;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                gifInfo.currentGIF = 12;                     // draw mode
                                gifInfo.filterEnable = false;               // enlève le filtre de couleur
                                matrixGifsList[12].currentGifFrameCount = 2; // gif de 2 frame

                                printf("DRAW\n");
                                if (json.HasMember("COLOR") && json.HasMember("LEDS"))
                                {
                                    printf("OK %d\n", json["LEDS"].Size());
                                    for (uint8_t i = 0; i < json["LEDS"].Size(); i++) // pour chaque led
                                    {
                                        int y = json["LEDS"][i].GetUint() / 128;
                                        int x = json["LEDS"][i].GetUint() % 128;

                                        matrixGifsList[12].animation[0].buffer[y][x].red = json["COLOR"][0].GetUint();
                                        matrixGifsList[12].animation[0].buffer[y][x].green = json["COLOR"][1].GetUint();
                                        matrixGifsList[12].animation[0].buffer[y][x].blue = json["COLOR"][2].GetUint();
                                       // printf("LED: %d %d: %d %d %d\n", x, y, matrixGifsList[12].animation[0].buffer[y][x].red, matrixGifsList[12].animation[0].buffer[y][x].green, matrixGifsList[12].animation[0].buffer[y][x].blue);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    catch (beast::system_error const &se)
    {
        // This indicates that the session was closed
        if (se.code() != websocket::error::closed)
            std::cerr << "Error: " << se.code().message() << std::endl;
    }
    catch (std::exception const &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void WebSocketServer()
{
    printf("Websocket thread: PID: %d, ID: %d\n", getpid(), gettid());
    try
    {
        auto const address = net::ip::make_address("0.0.0.0");
        auto const port = static_cast<unsigned short>(std::atoi("8083"));

        // The io_context is required for all I/O
        net::io_context ioc{1};

        // The acceptor receives incoming connections
        tcp::acceptor acceptor{ioc, {address, port}};
        for (;;)
        {
            // This will receive the new connection
            tcp::socket socket{ioc};

            // Block until we get a connection
            acceptor.accept(socket);

            // Launch the session, transferring ownership of the socket
            std::thread(
                &do_session,
                std::move(socket))
                .detach();
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void loadingScreenFunction(RGBMatrix *matrix)
{
    while (gifInfo.loadingScreenState)
    {
        FrameCanvas *offscreen_canvas = matrix->CreateFrameCanvas();
        for (uint16_t frame = 0; (frame < gifInfo.loadingScreenFrameCount - 1) && !interrupt_received; frame++)
        {
            for (uint16_t y = 0; y < 128; y++)
            {
                for (uint16_t x = 0; x < 128; x++)
                {
                    offscreen_canvas->SetPixel(x, y,
                                               matrixGifsList[0].animation[frame].buffer[y][x].red,
                                               matrixGifsList[0].animation[frame].buffer[y][x].green,
                                               matrixGifsList[0].animation[frame].buffer[y][x].blue);
                }
            }
            offscreen_canvas = matrix->SwapOnVSync(offscreen_canvas, 1);
            SleepMillis(100);
        }
    }
}

int main(int argc, char *argv[])
{

    wiringPiSetup(); // initialisation des GPIO

    Magick::InitializeMagick(*argv); // Initialize ImageMagick (pour la matrice)

    RGBMatrix::Options matrix_options; // options pour la matrice
    //---------- Matrix options ----------
    //-------Pixel Pour UN PANNEAU-------
    matrix_options.rows = MATRIX_ROWS;
    matrix_options.cols = MATRIX_COLS;

    //------regalge de la disposition de panneaux-------
    // on a 4 panneaux de 64x64 --> 128x128 pixels
    // ils sont disposer en carrés donc: 2x2 panneaus de 64x64
    matrix_options.chain_length = MATRIX_CHAIN; // 12 max
    matrix_options.parallel = MATRIX_PARALLEL;  // 3 MAX (il faut cabler différament pour en ajouter un 3eme)

    matrix_options.brightness = 100; // luminosité au max; on gère ca plus tard avec le filtre

    // options suplementaires
    matrix_options.pwm_bits = 11;
    matrix_options.pwm_lsb_nanoseconds = 50;
    //-------------------------------------

    rgb_matrix::RuntimeOptions runtime_opt;
    if (!rgb_matrix::ParseOptionsFromFlags(&argc, &argv,
                                           &matrix_options, &runtime_opt))
    {
        return usage(argv[0]);
    }

    std::map<const void *, struct ImageParams> filename_params;

    ImageParams img_param;
    for (int i = 0; i < argc; ++i)
    {
        filename_params[argv[i]] = img_param;
    }

    const char *stream_output = NULL;

    int opt;
    while ((opt = getopt(argc, argv, "w:t:l:fr:c:P:LhCR:sO:V:D:")) != -1)
    {
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

    // FrameCanvas *offscreen_canvas = matrix->CreateFrameCanvas();

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

    // --------------- Load images ---------------
    // on va charger tous les gifs dans le tableau matrixGifsList[]
    // ya encore le code de base de la librairie, mais ca marche
    std::vector<FileInfo *> file_imgs;
    std::string err_msg;
    printf("Loading...\n");
    std::thread loadingScreenThread;
    printf("Analyzing Gifs...\n");

    for (int imgarg = optind; imgarg < argc; ++imgarg)
    {
        gifInfo.currentGIF = imgarg - 1;
        const char *filename = argv[imgarg];
        FileInfo *file_info = NULL;

        std::vector<Magick::Image> image_sequence;
        if (LoadImageAndScale(filename, matrix->width(), matrix->height(),
                              fill_width, fill_height, &image_sequence, &err_msg))
        {
            file_info = new FileInfo();
            file_info->params = filename_params[filename];
            file_info->content_stream = new rgb_matrix::MemStreamIO();
            file_info->is_multi_frame = image_sequence.size() > 1;
            rgb_matrix::StreamWriter out(file_info->content_stream);
            printf("Gif n°%d Frame count: %d\n", gifInfo.currentGIF, image_sequence.size());
            matrixGifsList[gifInfo.currentGIF].currentGifFrameCount = image_sequence.size();

            for (size_t i = 0; i < image_sequence.size(); i++)
            {
                //printf("LoadFrame : %d\n", i);
                gifInfo.currentFrame = i;
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
                StoreInStream(img);
            }

            //------------------LOADING SCREEN -------------------
            if (imgarg == 1)
            {
                gifInfo.loadingScreenFrameCount = image_sequence.size();
                printf("Start Loading Gif...\n");
                gifInfo.loadingScreenState = true;
                loadingScreenThread = std::thread(loadingScreenFunction, matrix);
            }
        }
        else
        {
            perror("Opening file");
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

    printf("Start OK\n");

    gifInfo.loadingScreenState = false; // on arrete le loading screen
    loadingScreenThread.join();         // on attend la fin du loading screen
    gifInfo.currentGIF = 1;             // et on choisi le 1er gif

    std::thread CheckSensor(sensorLoop, MIN_DISTANCE, MAX_DISTANCE); // thread qui va lire les capteurs
    std::thread CheckWebSocket(WebSocketServer);                     // thread qui gère le websocket

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

    // signal pour fermer le programme (CTRL+C)
    signal(SIGTERM, InterruptHandler);
    signal(SIGINT, InterruptHandler);
    //------------------------------------

    //----------------- BOUCLE PRINCIPALE --------------
    while (!interrupt_received) // tant que l'utilisateur n'a pas appuyé sur CTRL+C
    {
        DisplayAnimation(matrix); // on affiche l'animation
    }

    if (interrupt_received) // si l'utilisateur a appuyé sur CTRL+C
    {
        fprintf(stderr, "Caught signal. Exiting.\n");
    }

    matrix->Clear(); // on efface l'écran
    delete matrix;   // on détruit la matrix

    CheckSensor.detach();    // on arrete le thread du capteur
    CheckWebSocket.detach(); // on arrete le thread du websocket
    return 0;
}
