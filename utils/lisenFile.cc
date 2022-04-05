#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <fstream>
using namespace std;

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

#define IPC_FILE "matrix"

string delimiter = "=";

int main(int argc, char **argv)
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
            perror("inotify_init");

        wd = inotify_add_watch(fd, ".", IN_MODIFY | IN_CREATE | IN_DELETE); // setup watch event
        length = read(fd, buffer, BUF_LEN);

        if (length < 0)
            perror("read");

        while (i < length)
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
                        string line;
                        // Open the file
                        ifstream FileIN;
                        FileIN.open(IPC_FILE);

                        while (getline(FileIN, line) && !interupt)
                        {
                            string key = line.substr(0, line.find(delimiter));    // get the action type
                            string value = line.substr(line.find(delimiter) + 1); // get the value

                            if (key == "TO")
                            {
                                if (value == "CPP")
                                {
                                    printf("ITS for me\n");
                                }
                                else
                                {
                                    interupt = true;
                                }
                            }
                            else if (key == "MODE")
                            {
                                if (value == "GIF")
                                {
                                    printf("GIF MODE\n");
                                }
                                else if (value == "DRAW")
                                {
                                    printf("DRAW MODE\n");
                                }
                            }
                            else if (key == "LED")
                            {
                                string led = value.substr(0, value.find(':'));
                                string color = value.substr(value.find(':') + 1);
                                printf("SET LED %s TO %s\n", led.c_str(), color.c_str());
                            }
                        }
                        FileIN.close();
                    }

                    if (!interupt)
                    {
                        printf("Clearing file\n\n");
                        // delete all file
                        ofstream ofs;
                        ofs.open(IPC_FILE, std::ofstream::out | std::ofstream::trunc);
                        ofs.close();
                    }
                }
            }
            i += EVENT_SIZE + event->len;
        }
    }
    // (void)inotify_rm_watch(fd, wd);
    // (void)close(fd);

    return 0;
}