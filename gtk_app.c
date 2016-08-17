#include "gtk_app.h"

int stop_pipeline( GtkWidget *widget,gpointer   data );

bool fileExists(const char* file)
{
    struct stat buf;
    return (stat(file, &buf) == 0);
}

int state_handler( Gst* gst, GstStateChangeReturn state )
{
    GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;
    if( state == (GstStateChangeReturn) GST_STATE_NULL )
    {
        flag = 1;
        ret = gst_element_set_state( gst->source, GST_STATE_PAUSED );
        if( ret == GST_STATE_CHANGE_FAILURE )
        {
            GST_DEBUG ("failed to change state ret = %d\n",ret);
            return -1;
        }
        ret = gst_element_set_state ( gst->source, GST_STATE_READY);
        if( ret == GST_STATE_CHANGE_FAILURE )
        {
            GST_DEBUG ("failed to change state ret = %d\n",ret);
            return -1;
        }
        ret = gst_element_set_state( gst->source, GST_STATE_NULL );
        if( ret == GST_STATE_CHANGE_FAILURE )
        {
            GST_DEBUG ("failed to change state ret = %d\n",ret);
            return -1;
        }

        ret = gst_element_set_state( gst->pipeline, GST_STATE_PAUSED );
        if( ret == GST_STATE_CHANGE_FAILURE )
        {
            GST_DEBUG ("failed to change state ret = %d\n",ret);
            return -1;
        }
        ret = gst_element_set_state( gst->pipeline, GST_STATE_READY);
        if( ret == GST_STATE_CHANGE_FAILURE )
        {
            GST_DEBUG ("failed to change state ret = %d\n",ret);
            return -1;
        }
        ret = gst_element_set_state( gst->pipeline, GST_STATE_NULL );
        if( ret == GST_STATE_CHANGE_FAILURE )
        {
            GST_DEBUG ("failed to change state ret = %d\n",ret);
            return -1;
        }
    }
    else if (state == (GstStateChangeReturn) GST_STATE_PLAYING)
    {

        ret = gst_element_set_state ( gst->pipeline, GST_STATE_READY);
        if (ret == GST_STATE_CHANGE_FAILURE)
        {
            GST_DEBUG ("failed to change state ret = %d\n",ret);
            return -1;
        }
        ret = gst_element_set_state ( gst->pipeline, GST_STATE_PAUSED);
        if (ret == GST_STATE_CHANGE_FAILURE)
        {
            GST_DEBUG ("failed to change state ret = %d\n",ret);
            return -1;
        }
        ret = gst_element_set_state ( gst->pipeline, GST_STATE_PLAYING);
        if (ret == GST_STATE_CHANGE_FAILURE)
        {
            GST_DEBUG ("failed to change state ret = %d\n",ret);
            return -1;
        }
    }
    return 0;
}

static gboolean my_bus_callback( GstBus* bus, GstMessage *message, gpointer data )
{
   Gst* gst = ( Gst* ) data;
   switch( GST_MESSAGE_TYPE (message) )
   {
     case GST_MESSAGE_ERROR:
     {
       GST_DEBUG ("\nGot %s message\n", GST_MESSAGE_TYPE_NAME (message));
       GError *err;
       gchar *debug;
       gst_message_parse_error( message, &err, &debug );
       GST_DEBUG ("\nError: %s\n", err->message);
       g_error_free (err);
       g_free (debug);
       g_main_loop_quit ( gst->loop );
       break;
     }
     case GST_MESSAGE_EOS:
     {
       GST_DEBUG ("\nGot %s message\n", GST_MESSAGE_TYPE_NAME (message));
       /* end-of-stream */
       g_main_loop_quit ( gst->loop );
       break;
     }
     default:
       /* unhandled message */
       break;
  }
  /* we want to be notified again the next time there is a message
    * on the bus, so returning TRUE (FALSE means we want to stop watching
    * for messages on the bus and our callback should not be called again)
    */
  return TRUE;
}

static gboolean TimeUpdate( void* data)
{
        time_t TimeRemained;
        Gst* gst = ( Gst* ) data;
        time ( &gst->t_Rawtime );
        gst->t_Rawtime = difftime ( gst->t_Rawtime,gst->t_start)-19800;

        gchar *pSTR=ctime(&gst->t_Rawtime);
        *(pSTR+20)='\0';
        gtk_label_set_text ((GtkLabel*)gst->TimeDisplay,pSTR+10);
        TimeRemained=difftime (gst->t_SetTime,gst->t_Rawtime)-19800*2;

        gchar *pSTR2=ctime(&TimeRemained);
        *(pSTR2+20)='\0';
        gtk_label_set_text ((GtkLabel*)gst->TimeRemained,pSTR+10);
        gst->t_Rawtime=gst->t_Rawtime+19800;
        if((gst->t_SetTime-gst->t_Rawtime < 1)&&(gst->t_SetTime>0))
        {
            stop_pipeline( NULL,gst);
            gtk_widget_set_sensitive ((GtkWidget*)gst->stopButton,FALSE);
            return FALSE;
        }
         if (gtk_widget_get_sensitive ((GtkWidget*)gst->stopButton)==TRUE)
            return 1;
        else
            return FALSE;
}






int create_elements( Gst* gst, char* location )
{
    GstElement *encoder;
    GstElement *muxer;

    if ( location == NULL )
    {
        GST_DEBUG ("\nAllocating memory to location\n");
        location = ( char*)malloc(100);
        if( location == NULL )
        {
            GST_ERROR( "Malloc Failed\n" );
            return -1;
        }
    }

    /* Create gstreamer elements */
    if (!gst->pipeline)
    {
        GST_DEBUG ("\nCreating pipeline\n");
        gst->pipeline = gst_pipeline_new( "pipeline" );
    }

    if (!gst->source)
    {
        GST_DEBUG ("\nCreating source\n");
        gst->source = gst_element_factory_make( "gdiscreencapsrc",    "videotestsrc" );
    }

    if (!gst->filter)
    {
        GST_DEBUG ("\nCreating filter\n");
        gst->filter = gst_element_factory_make( "capsfilter", "filter" );
        /* Set up elements */
        g_object_set( G_OBJECT ( gst->filter ), "caps", gst->video_caps, NULL );
    }

    if (!gst->ffmpegcolorspace)
    {
        GST_DEBUG ("\nCreating ffmpegcolorspace\n");
        gst->ffmpegcolorspace = gst_element_factory_make( "ffmpegcolorspace", "ffmpegcolorspace" );
    }

    /* Create Encoder */
    encoder = gst_bin_get_by_name (GST_BIN (gst->pipeline), "encoder");
    if (encoder)
    {
        GST_DEBUG ("\nRemoving encoder\n");
        gst_bin_remove(GST_BIN (gst->pipeline), encoder);
        gst_object_unref (GST_OBJECT (gst->encoder)); 
    }

    GST_DEBUG ("\nCreating encoder\n");
    GST_DEBUG ("\nEncoder type in create_elements %s\n", gst->qtVars->encoderType );
    if( strcmp( gst->qtVars->encoderType, "Encoder_H264" ) == 0 )
    {
        gst->encoder = gst_element_factory_make( "x264enc",    "encoder" );
    }
    else
    {
        gst->encoder = gst_element_factory_make( "x264enc",    "encoder" );
    }

    if (encoder)
    {
        GST_DEBUG ("\nAdding encoder\n");
        gst_bin_add(GST_BIN (gst->pipeline), gst->encoder);
    }

    /* Create Muxer */
    muxer = gst_bin_get_by_name (GST_BIN (gst->pipeline), "mux");
    if (muxer)
    {
        GST_DEBUG ("\nRemoving muxer\n");
        gst_bin_remove(GST_BIN (gst->pipeline), muxer);
        gst_object_unref (GST_OBJECT (gst->muxer)); 
    }

    GST_DEBUG ("\nCreating muxer\n");
    if( strcmp( gst->qtVars->containerFormat, "Container_AVI" ) == 0 )
    {
        gst->muxer = gst_element_factory_make( "avimux", "mux" );
    }
    else if( strcmp( gst->qtVars->containerFormat, "Container_mp4" ) == 0 )
    {
        GST_DEBUG ("\n creating mpeg4mux\n");
        gst->muxer = gst_element_factory_make( "mp4mux", "mux" );
    }
    else
    {
        gst->muxer = gst_element_factory_make( "avimux", "mux" );
    }

    GST_DEBUG ("muxer type in create_elements %s\n", gst_element_get_name(gst->muxer) );

    if (encoder)
    {
        GST_DEBUG ("\nAdding Muxer\n");
        gst_bin_add(GST_BIN (gst->pipeline), gst->muxer);
    }

    if (!gst->sink)
    {
        GST_DEBUG ("\nCreating sink\n");
        gst->sink = gst_element_factory_make( "filesink", "filesink" );
        GST_DEBUG ("sink type in create_elements %s\n", gst_element_get_name(gst->sink) );

        /* we set the input filename to the source element */
        g_object_set( G_OBJECT( gst->sink ), "location", gst->fileLocation, NULL );
        GST_DEBUG ("\nLocation = %s\n",gst->fileLocation);
    }

    if (!gst->qtVars->fps)
        gst->qtVars->fps = DEFAULT_FPS;

    if (!gst->video_caps)
    {
        GST_DEBUG ("\nCreating video_caps\n");
        /* Video caps */
        gst->video_caps = gst_caps_new_simple( "video/x-raw-rgb", "framerate", GST_TYPE_FRACTION, gst->qtVars->fps, 1, NULL );
        gst_caps_unref( gst->video_caps );
    }

    if( !gst->pipeline || !gst->source || !gst->filter || !gst->encoder || !gst->muxer || !gst->sink )
    {
        GST_ERROR ( "One element could not be created. Exiting.\n" );
        return -1;
    }

    if (gst->qtVars->topLeftX)
        g_object_set( G_OBJECT( gst->source ), "x", gst->qtVars->topLeftX, NULL );
    if (gst->qtVars->topLeftY)
        g_object_set( G_OBJECT( gst->source ), "y", gst->qtVars->topLeftY, NULL );
    if (gst->qtVars->width)
        g_object_set( G_OBJECT( gst->source ), "width", gst->qtVars->width, NULL );
    if (gst->qtVars->height)
        g_object_set( G_OBJECT( gst->source ), "height", gst->qtVars->height, NULL );

    GST_DEBUG ("(%dx%d) (%dx%d)",gst->qtVars->topLeftX,gst->qtVars->topLeftY,
                    gst->qtVars->width,gst->qtVars->height);

    return 0;
}



int pipeline_make( Gst* gst )
{
    if (!gst_bin_get_by_name (GST_BIN (gst->pipeline), "filesink"))
    {
        /* Add all elements into the pipeline */
        gst_bin_add_many( GST_BIN ( gst->pipeline), gst->source, gst->filter, gst->ffmpegcolorspace, gst->encoder,
                         gst->muxer, gst->sink, NULL);
    }

    /* Link the elements together */
    gst_element_link_many( gst->source, gst->filter, gst->ffmpegcolorspace,  gst->encoder, gst->muxer, gst->sink, NULL );

    return 0;
}


int bus_watcher( Gst* gst )
{
    /* Add a message handler */
    gst->bus = gst_pipeline_get_bus( GST_PIPELINE( gst->pipeline) );
    gst->bus_watch_id = gst_bus_add_watch( gst->bus, my_bus_callback, gst );
    gst_object_unref( gst->bus );
    return 0;
}

/* This is a callback function. */
int stop_pipeline( GtkWidget *widget,
                gpointer   data )
{
        Gst* gst = ( Gst* ) data;
        double difft = 0;

        gtk_widget_set_sensitive (widget,FALSE);
        gtk_widget_set_sensitive ( (GtkWidget*)gst->startButton,TRUE);
        g_print ("Stop Recording ...\n");

        /* take the current time end time */
        time (&gst->t_end);

        /* calculate time spent */
        difft = difftime (gst->t_end,gst->t_start);
        gst->t_end=0;
        g_print ("Total time = %f seconds\n", difft);

        if( state_handler( gst, GST_STATE_NULL) !=0 )
                return -1;
        g_main_loop_quit ( gst->loop );
        return 0;
}
int MainWindowQuit( GtkComboBox *widget, gpointer data )
{
        if( !flag )
        {
                Gst* gst = ( Gst* ) data;
                GST_DEBUG (" stop pipeline\n");
                if( state_handler( gst, GST_STATE_NULL) !=0 )
                        return -1;
                g_main_loop_quit ( gst->loop );
        }
        gtk_main_quit ();
        return 0;

}
int start_pipeline( GtkWidget *widget, gpointer   data )
{

    //thred creation

        Gst* gst = ( Gst* ) data;
        gtk_widget_set_sensitive (widget,FALSE);
        gtk_widget_set_sensitive ( (GtkWidget*) gst->stopButton,TRUE);

        /* Initialize elements */
        if( create_elements( gst, "D:\\test.avi") != 0 )
                return -1;

        /* Add function to watch bus */
        if( bus_watcher( gst ) != 0 )
                return -1;

        /* Add elements to pipeline, and link them */
        if( pipeline_make( gst ) != 0 )
                return -1;

        /* Set the pipeline to "playing" state*/
        if( state_handler( gst, GST_STATE_PLAYING) !=0 )
                return -1;

        /* take the current time start time */
        time (&gst->t_start);

        if(gst->t_SetTime<1)
            gtk_widget_hide ((GtkWidget*)gst->TimeRemained);
        else
            gtk_widget_show((GtkWidget*)gst->TimeRemained);

        g_timeout_add (1000,(GSourceFunc)TimeUpdate,(gpointer)gst);

        /* To obtain .dot files, set the GST_DEBUG_DUMP_DOT_DIR environment
        variable to point to the folder where you want the files to be placed. */

        GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS (GST_BIN(gst->pipeline),
            GST_DEBUG_GRAPH_SHOW_ALL, "gstcapture-1.0-playing");

        g_print ("Start Recording ...\n");
        g_main_loop_run ( gst->loop);

        return 0;
}


static void DropboxEncoderFormat( GtkComboBox *widget, gpointer data )
{
        const gchar *pTEMP = gtk_combo_box_get_active_id (widget);
        strcpy( (( Gst* )data)->qtVars->encoderType, pTEMP );
}

static void DropboxContainerFormat( GtkComboBox *widget, gpointer data )
{
        const gchar *pTEMP = gtk_combo_box_get_active_id (widget);
        strcpy( (( Gst* )data)->qtVars->containerFormat, pTEMP );
}

static void DropboxFPS( GtkComboBox *widget, gpointer data )
{
        gtk_combo_box_set_id_column (widget,0);
        const gchar *pTEMP=gtk_combo_box_get_active_id (widget);
        (( Gst* )data)->qtVars->fps = atoi( pTEMP );
}

static void EntryTopX (GtkEntry *widget, gpointer   data)
{

        (( Gst* )data)->qtVars->topLeftX = ( atoi( gtk_entry_get_text( (GtkEntry*)widget ) ) );
        GST_DEBUG ( "\nvalue of topLeftX = %d\n", (( Gst* )data)->qtVars->topLeftX );

}

static void EntryTopY (GtkEntry *widget, gpointer   data)
{
        (( Gst* )data)->qtVars->topLeftY = ( atoi( gtk_entry_get_text( (GtkEntry*)widget ) ) );
        GST_DEBUG ( "\nvalue of topLeftY = %d\n", (( Gst* )data)->qtVars->topLeftY );
}

static void EntryHeight (GtkEntry *widget, gpointer   data)
{
        (( Gst* )data)->qtVars->height = ( atoi( gtk_entry_get_text( (GtkEntry*)widget ) ) );
        GST_DEBUG ( "\nvalue of height = %d\n", (( Gst* )data)->qtVars->height );
}

static void EntryWidth (GtkEntry *widget, gpointer   data)
{
        (( Gst* )data)->qtVars->width = ( atoi( gtk_entry_get_text( (GtkEntry*)widget ) ) );
        GST_DEBUG ( "\nvalue of width = %d\n", (( Gst* )data)->qtVars->width );

}

static void EntryTime (GtkEntry *widget, gpointer   data)
{

   char* copy = strdup( gtk_entry_get_text( (GtkEntry*)widget ));
   char *token;
   token = strtok(copy, ":");
   int i=2;
   time_t TotalSeconds=0;
   /* walk through other tokens */
   while( (token != NULL) ||(i==0))
   {
      if(atoi(token)>59)
          TotalSeconds=60*(pow (60,i))+TotalSeconds;
    else
      TotalSeconds=atoi(token)*(pow (60,i))+TotalSeconds;
      token = strtok(NULL, ":");
      i--;
   }
   (( Gst* )data)->t_SetTime=TotalSeconds;

}


static void EntryFile (GtkEntry *widget, gpointer   data)
{

        const char* temp = NULL;
        int fd;
        bool ret;

        temp = gtk_entry_get_text( (GtkEntry*)widget );
        stat( temp, &st );

        if( S_ISDIR( st.st_mode )  )
        {
                gtk_entry_set_text( (GtkEntry*)widget, "***It is directory enter path with file name***" );
        }
        else
        {
                ret = fileExists( temp );
                if( ret )
                {
                        strcpy( (( Gst* )data)->fileLocation, temp );
                }
                else
                {
                        fd = open(temp, O_WRONLY | O_CREAT, 0644);
                        if( fd == -1)
                        {
                                GST_ERROR ( "ERROR :: incorrect file.\n" );
                        }
                        strcpy( (( Gst* )data)->fileLocation, temp );
                }

        }
}




int main ( int   argc, char *argv[] )
{
    GtkBuilder *builder;
    GObject *window;
    GObject *button;
    GObject *dropdown;
    GObject *Entry;
    GObject* LinkButton;

    /* Gtk Initialisation */
    gtk_init (&argc, &argv);

    /* Gstreamer Initialisation */
    gst_init (&argc, &argv);



    Gst* gst = (Gst *) malloc (sizeof(Gst) );

    if( gst == NULL )
    {
            GST_ERROR ( "Malloc failed :: gst.\n" );
            return -1;
    }

    gst->qtVars = ( InitQtVariables* )malloc( sizeof( InitQtVariables)  );
    if( gst->qtVars == NULL )
    {
            GST_ERROR ( "Malloc failed :: qtVars.\n" );
            return -1;
    }

    /*gst->StopTimeVariables = ( TimeVariables* )malloc( sizeof( TimeVariables)  );
    if( gst->qtVars == NULL )
    {
            GST_ERROR ( "Malloc failed :: TimeVariables.\n" );
            return -1;
    }*/
    gst->qtVars->encoderType = ( char* )malloc( 20  );
    if( gst->qtVars->encoderType == NULL )
    {
            GST_ERROR ( "Malloc failed :: encoderType.\n" );
            return -1;
    }

    gst->fileLocation = ( char* )malloc( 100 );
    if( gst->fileLocation == NULL )
    {
            GST_ERROR ( "Malloc failed :: fileLocation.\n" );
            return -1;
    }

    gst->qtVars->containerFormat = ( char* )malloc( 20  );
    if( gst->qtVars->containerFormat == NULL )
    {
            GST_ERROR ( "Malloc failed :: encoderType.\n" );
            return -1;
    }

    gst->command = ( char* )malloc( 300  );
    if( gst->qtVars->containerFormat == NULL )
    {
            GST_ERROR ( "Malloc failed :: command.\n" );
            return -1;
    }

    strcpy(gst->fileLocation, "D:\\test.avi");

    gst->qtVars->topLeftX = 0;
    gst->qtVars->topLeftY = 0;
    gst->qtVars->width    = 0;
    gst->qtVars->height   = 0;
    gst->t_SetTime         = 0;

    gst->loop = g_main_loop_new (NULL, FALSE);

    /* Construct a GtkBuilder instance and load our UI description */
    builder = gtk_builder_new ();
    gtk_builder_add_from_file (builder, "ScreenRec.ui", NULL);

    /* Connect signal handlers to the constructed widgets. */
    window = gtk_builder_get_object (builder, "Main_Application");
    g_signal_connect (window, "destroy", G_CALLBACK (MainWindowQuit), gst);

    gst->startButton  = gtk_builder_get_object (builder, "Button_Start");
    gst->stopButton = gtk_builder_get_object (builder, "Button_Stop");
    gtk_widget_set_sensitive ( (GtkWidget*)gst->stopButton,FALSE);


    g_signal_connect ( gst->startButton, "clicked", G_CALLBACK (start_pipeline), gst );


    g_signal_connect ( gst->stopButton, "clicked", G_CALLBACK (stop_pipeline), gst );

    button = gtk_builder_get_object (builder, "Button_Quit");
    g_signal_connect (button, "clicked", G_CALLBACK (MainWindowQuit), gst );


    dropdown = gtk_builder_get_object (builder, "Combobox_Encoder");
    g_signal_connect( dropdown, "changed", G_CALLBACK (DropboxEncoderFormat), gst );


    dropdown = gtk_builder_get_object (builder, "Combobox_Container");
    g_signal_connect( dropdown, "changed", G_CALLBACK (DropboxContainerFormat), gst );


    dropdown = gtk_builder_get_object (builder, "Combobox_Framerate");
    g_signal_connect( dropdown, "changed", G_CALLBACK (DropboxFPS), gst );


    //co-ordinate entries
    Entry = gtk_builder_get_object (builder, "Entry_TopX");
    g_signal_connect (Entry, "activate", G_CALLBACK (EntryTopX), gst );

    Entry = gtk_builder_get_object (builder, "Entry_TopY");
    g_signal_connect (Entry, "activate", G_CALLBACK (EntryTopY), gst );

    Entry = gtk_builder_get_object (builder, "Entry_Height");
    g_signal_connect (Entry, "activate", G_CALLBACK (EntryHeight), gst);

    Entry = gtk_builder_get_object (builder, "Entry_Width");
    g_signal_connect (Entry, "activate", G_CALLBACK (EntryWidth), gst );

    // url action
    LinkButton = gtk_builder_get_object (builder, "linkbutton");
    g_signal_connect (LinkButton, "activate-link", NULL, NULL );

    gst->TimeDisplay = gtk_builder_get_object (builder, "Label_ActualTime");
    gst->TimeRemained=gtk_builder_get_object (builder,"Label_RemainedTime");

    Entry = gtk_builder_get_object (builder, "Entry_Time");
    g_signal_connect (Entry, "activate", G_CALLBACK (EntryTime), gst );

    Entry = gtk_builder_get_object (builder, "Entry_File");
    gtk_entry_set_text( (GtkEntry*)Entry, "***Default path will be ""D:\\test.avi""***" );
    g_signal_connect (Entry, "activate", G_CALLBACK (EntryFile), gst );

    gtk_main ();
    return 0;
}

