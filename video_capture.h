
#ifndef __VIDEOCAPTURE__
#define __VIDEOCAPTURE__

#include <gst/gst.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>


typedef struct
{
	int   topLeftX;
	int   topLeftY;
	int   height;
	int   width;
    int   fps;
    char* encoderType;
    char* containerFormat;

}InitGtkVariables;

typedef struct
{
	GstElement* pipeline;
	GstElement* source;
	GstElement* filter; 
    GstElement* ffmpegcolorspace; 
    GstElement* accel; 
    GstElement* encoder; 
    GstElement* queue; 
    GstElement* muxer; 
    GstElement* sink;
	 GMainLoop* loop;
	   GstCaps* video_caps;
	    GstBus* bus;
	   guint    bus_watch_id;
   InitGtkVariables* gtkVars; 
	GObject *startButton;
	GObject *stopButton;
   

   
}Gst;
#endif /* __VIDEOCAPTURE__*/
