
#include "video_capture.h"


int state_handler( Gst* gst, GstStateChangeReturn state )
{
	GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;
	if( state == (GstStateChangeReturn) GST_STATE_NULL )
	{
        flag = 1;
		ret = gst_element_set_state( gst->source, GST_STATE_PAUSED );
		if( ret == GST_STATE_CHANGE_FAILURE )
		{
			g_print("failed to change state ret = %d\n",ret);
			return -1;
		}
		ret = gst_element_set_state ( gst->source, GST_STATE_READY);
		if( ret == GST_STATE_CHANGE_FAILURE )
		{
			g_print("failed to change state ret = %d\n",ret);
			return -1;
		}
		ret = gst_element_set_state( gst->source, GST_STATE_NULL );
		if( ret == GST_STATE_CHANGE_FAILURE )
		{
			g_print("failed to change state ret = %d\n",ret);
			return -1;
		}

		ret = gst_element_set_state( gst->pipeline, GST_STATE_PAUSED );
		if( ret == GST_STATE_CHANGE_FAILURE )
		{
			g_print("failed to change state ret = %d\n",ret);
			return -1;
		}
		ret = gst_element_set_state( gst->pipeline, GST_STATE_READY);
		if( ret == GST_STATE_CHANGE_FAILURE )
		{
			g_print("failed to change state ret = %d\n",ret);
			return -1;
		}
		ret = gst_element_set_state( gst->pipeline, GST_STATE_NULL );
		if( ret == GST_STATE_CHANGE_FAILURE )
		{
			g_print("failed to change state ret = %d\n",ret);
			return -1;
		}
	}
	else if (state == (GstStateChangeReturn) GST_STATE_PLAYING)
	{

		ret = gst_element_set_state ( gst->pipeline, GST_STATE_READY);
		if (ret == GST_STATE_CHANGE_FAILURE)
		{
			g_print("failed to change state ret = %d\n",ret);
			return -1;
		}
		ret = gst_element_set_state ( gst->pipeline, GST_STATE_PAUSED);
		if (ret == GST_STATE_CHANGE_FAILURE)
		{
			g_print("failed to change state ret = %d\n",ret);
			return -1;
		}
		ret = gst_element_set_state ( gst->pipeline, GST_STATE_PLAYING);
		if (ret == GST_STATE_CHANGE_FAILURE)
		{
			g_print("failed to change state ret = %d\n",ret);
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
       g_print ("\nGot %s message\n", GST_MESSAGE_TYPE_NAME (message));
       GError *err;
       gchar *debug;
       gst_message_parse_error( message, &err, &debug );
       g_print ("\nError: %s\n", err->message);
       g_error_free (err);
       g_free (debug);
       g_main_loop_quit ( gst->loop );
       break;
     }
     case GST_MESSAGE_EOS:
     {
       g_print ("\nGot %s message\n", GST_MESSAGE_TYPE_NAME (message));
       state_handler ( gst, GST_STATE_NULL);
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

int create_elements( Gst* gst, char* location )
{
	if ( location == NULL )
	{
		location = ( char*)malloc(100);
		if( location == NULL )
		{
			g_printerr( "Malloc Failed\n" );
			return -1;
		}
		strcpy( location,"D:\\test.avi" );
	}

	/* Create gstreamer elements */
	gst->pipeline         = gst_pipeline_new( "pipeline" );
	gst->source           = gst_element_factory_make( "gdiscreencapsrc",	"videotestsrc" );
	gst->filter           = gst_element_factory_make( "capsfilter", "filter" );
	gst->ffmpegcolorspace = gst_element_factory_make( "ffmpegcolorspace", "ffmpegcolorspace" );
     
	g_print("\nEncoder type in create_elements %s\n", gst->gtkVars->encoderType );
	
    if( strcmp( gst->gtkVars->encoderType, "Encoder_H264" ) == 0 )
	{
		gst->encoder          = gst_element_factory_make( "x264enc",	"x264enc" );
	}
	else 
	{
		gst->encoder          = gst_element_factory_make( "x264enc",	"x264enc" );
	}
	
    if( strcmp( gst->gtkVars->containerFormat, "Container_AVI" ) == 0 )
	{
			gst->muxer            = gst_element_factory_make( "avimux", "mux" );
	}
	else if( strcmp( gst->gtkVars->containerFormat, "Container_mp4" ) == 0 )
	{
		gst->muxer            = gst_element_factory_make( "mp4mux", "mux" );
	}
    else
	{
			gst->muxer            = gst_element_factory_make( "avimux", "mux" );
	}
	gst->sink             = gst_element_factory_make( "filesink", "filesink" );

    /*if (gst->gtkVars->containerFormat)
    {
        if( strcmp( gst->gtkVars->containerFormat, "Container_AVI" ) == 0 )
        {
                gst->muxer = gst_element_factory_make( "avimux", "mux" );
        }
        else
        {
                gst->muxer = gst_element_factory_make( "mp4mux", "mux" );

        }
    }
    else
    {
            gst->muxer = gst_element_factory_make( "avimux", "mux" );
    }
	g_print("muxer type in create_elements %s\n", gst_element_get_name(gst->muxer) );*/

	gst->sink = gst_element_factory_make( "filesink", "filesink" );
	g_print("sink type in create_elements %s\n", gst_element_get_name(gst->sink) );

    if (!gst->gtkVars->fps)
        gst->gtkVars->fps = DEFAULT_FPS;

	/* Video caps */
    gst->video_caps = gst_caps_new_simple( "video/x-raw-rgb", "framerate", GST_TYPE_FRACTION, gst->gtkVars->fps, 1, NULL );

	if( !gst->pipeline || !gst->source || !gst->filter || !gst->encoder || !gst->muxer || !gst->sink )  
	{
			g_printerr ( "One element could not be created. Exiting.\n" );
			return -1;
	}

	/* Set up elements */
	g_object_set( G_OBJECT ( gst->filter ), "caps", gst->video_caps, NULL );
	gst_caps_unref( gst->video_caps );

    if (gst->gtkVars->topLeftX)
        g_object_set( G_OBJECT( gst->source ), "x", gst->gtkVars->topLeftX, NULL );
    if (gst->gtkVars->topLeftY)
        g_object_set( G_OBJECT( gst->source ), "y", gst->gtkVars->topLeftY, NULL );
    if (gst->gtkVars->width)
        g_object_set( G_OBJECT( gst->source ), "width", gst->gtkVars->width, NULL );
    if (gst->gtkVars->height)
        g_object_set( G_OBJECT( gst->source ), "height", gst->gtkVars->height, NULL );

    g_print("(%dx%d) (%dx%d)",gst->gtkVars->topLeftX,gst->gtkVars->topLeftY,
                    gst->gtkVars->width,gst->gtkVars->height);

	/* we set the input filename to the source element */
	g_object_set( G_OBJECT( gst->sink ), "location", location, NULL );

	return 0;

}

int pipeline_make( Gst* gst )
{
		/* Add all elements into the pipeline */
		gst_bin_add_many( GST_BIN ( gst->pipeline), gst->source, gst->filter, gst->ffmpegcolorspace, gst->encoder, 
						 gst->muxer, gst->sink, NULL);

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
		gtk_widget_set_sensitive (widget,FALSE);
		gtk_widget_set_sensitive ( (GtkWidget*)gst->startButton,TRUE);
		g_print(" stop pipeline\n");
		
	   
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
				g_print(" stop pipeline\n");
				if( state_handler( gst, GST_STATE_NULL) !=0 )
						return -1;
				g_main_loop_quit ( gst->loop );
		}
		gtk_main_quit ();
		return 0;

}
int run_pipeline( GtkWidget *widget, gpointer   data )
{

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

		g_print ("Running...\n");
		g_main_loop_run ( gst->loop); 

		return 0;
}


static void DropboxEncoderFormat( GtkComboBox *widget, gpointer data )
{

		const gchar *pTEMP = gtk_combo_box_get_active_id (widget);


		strcpy( (( Gst* )data)->gtkVars->encoderType, pTEMP ); 


}

static void DropboxContainerFormat( GtkComboBox *widget, gpointer data )
{
		const gchar *pTEMP = gtk_combo_box_get_active_id (widget);


		strcpy( (( Gst* )data)->gtkVars->containerFormat, pTEMP ); 

}

static void DropboxFPS( GtkComboBox *widget, gpointer data )
{
		gtk_combo_box_set_id_column (widget,0);
		const gchar *pTEMP=gtk_combo_box_get_active_id (widget);
		(( Gst* )data)->gtkVars->fps = atoi( pTEMP );
}

static void EntryTopX (GtkEntry *widget, gpointer   data)
{

		(( Gst* )data)->gtkVars->topLeftX = ( atoi( gtk_entry_get_text( (GtkEntry*)widget ) ) );
		g_print( "\nvalue of topLeftX = %d\n", (( Gst* )data)->gtkVars->topLeftX );

}

static void EntryTopY (GtkEntry *widget, gpointer   data)
{
		(( Gst* )data)->gtkVars->topLeftY = ( atoi( gtk_entry_get_text( (GtkEntry*)widget ) ) );
		g_print( "\nvalue of topLeftY = %d\n", (( Gst* )data)->gtkVars->topLeftY );
}

static void EntryHeight (GtkEntry *widget, gpointer   data)
{
		(( Gst* )data)->gtkVars->height = ( atoi( gtk_entry_get_text( (GtkEntry*)widget ) ) );
		g_print( "\nvalue of height = %d\n", (( Gst* )data)->gtkVars->height );
}

static void EntryWidth (GtkEntry *widget, gpointer   data)
{
		(( Gst* )data)->gtkVars->width = ( atoi( gtk_entry_get_text( (GtkEntry*)widget ) ) );
		g_print( "\nvalue of width = %d\n", (( Gst* )data)->gtkVars->width );

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
				g_printerr ( "Malloc failed :: gst.\n" );
				return -1;   
		}

		gst->gtkVars = ( InitGtkVariables* )malloc( sizeof( InitGtkVariables)  );
		if( gst->gtkVars == NULL )
		{
				g_printerr ( "Malloc failed :: gtkVars.\n" );
				return -1;   
		}
		gst->gtkVars->encoderType = ( char* )malloc( 20  );
		if( gst->gtkVars->encoderType == NULL )
		{
				g_printerr ( "Malloc failed :: encoderType.\n" );
				return -1;   
		}

		gst->gtkVars->containerFormat = ( char* )malloc( 20  );
		if( gst->gtkVars->containerFormat == NULL )
		{
				g_printerr ( "Malloc failed :: encoderType.\n" );
				return -1;   
		}

                gst->gtkVars->topLeftX = 0;
                gst->gtkVars->topLeftY = 0;
                gst->gtkVars->width    = 0;
                gst->gtkVars->height   = 0;

                gst->loop = g_main_loop_new (NULL, FALSE);

		/* Construct a GtkBuilder instance and load our UI description */
		builder = gtk_builder_new ();
		gtk_builder_add_from_file (builder, "builder.ui", NULL);

		/* Connect signal handlers to the constructed widgets. */
		window = gtk_builder_get_object (builder, "Main_Application");
		g_signal_connect (window, "destroy", G_CALLBACK (MainWindowQuit), gst);

		gst->startButton  = gtk_builder_get_object (builder, "Button_Start");
		gst->stopButton = gtk_builder_get_object (builder, "Button_Stop");
		gtk_widget_set_sensitive ( (GtkWidget*)gst->stopButton,FALSE);


		g_signal_connect ( gst->startButton, "clicked", G_CALLBACK (run_pipeline), gst );


		g_signal_connect ( gst->stopButton, "clicked", G_CALLBACK (stop_pipeline), gst );

		button = gtk_builder_get_object (builder, "Button_Quit");
		g_signal_connect (button, "destroy", G_CALLBACK (MainWindowQuit), gst );


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

		gtk_main ();
		return 0;
}

