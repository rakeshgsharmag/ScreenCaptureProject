#ifndef __VIDEOCAPTURE__
#define __VIDEOCAPTURE__

#include <gst/gst.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
int flag = 0;


struct stat st, st1;
typedef int bool;
#define true 1
#define false 0


//default values for co-ordinates and frame rate and co-ordinates
#define MAX_TOP_LEFT_X                1023
#define MAX_TOP_LEFT_Y                765
#define MAX_WIDTH                     1024
#define MAX_HEIGHT                    768
#define MAX_FPS                       60
#define MIN_TOP_LEFT_X                1
#define MIN_TOP_LEFT_Y                1
#define MIN_WIDTH                     1
#define MIN_HEIGHT                    1
#define MIN_FPS                       5
#define DEFAULT_TOP_LEFT_X            1
#define DEFAULT_TOP_LEFT_Y            1000
#define DEFAULT_WIDTH                 640
#define DEFAULT_HEIGHT                480
#define DEFAULT_FPS                   20



//thread creation
typedef pthread_t ThreadTimePrint;



typedef struct
{
    int   topLeftX;
    int   topLeftY;
    int   height;
    int   width;
    int   fps;
    char* encoderType;
    char* containerFormat;

}InitQtVariables;



typedef struct
{
    GstElement*       pipeline;
    GstElement*       source;
    GstElement*       filter;
    GstElement*       ffmpegcolorspace;
    GstElement*       accel;
    GstElement*       encoder;
    GstElement*       queue;
    GstElement*       muxer;
    GstElement*       sink;
    GMainLoop*        loop;
    GstCaps*          video_caps;
    GstBus*           bus;
    guint             bus_watch_id;
    GObject           *startButton;
    GObject           *stopButton;
    GObject           *TimeDisplay;
    GObject           *TimeRemained;
    char              *fileLocation;
    time_t            t_start, t_end;
    time_t            t_SetTime,t_Rawtime;
    InitQtVariables*  qtVars;
    char*             command;

}Gst;
#endif /* __VIDEOCAPTURE__*/
