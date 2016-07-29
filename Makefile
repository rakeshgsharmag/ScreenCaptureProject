INCLUDES = -I "D:\gtk+-bundle_3.6.4\include\gtk-3.0" \
		   -I "D:\gtk+-bundle_3.6.4\include\cairo" \
		   -I "D:\gtk+-bundle_3.6.4\include\pango-1.0" \
		   -I "D:\gtk+-bundle_3.6.4\lib\gtk-3.0\include" \
		   -I "D:\gtk+-bundle_3.6.4\include\gdk-pixbuf-2.0" \
		   -I "D:\gtk+-bundle_3.6.4\include\glib-2.0" \
		   -I "D:\gtk+-bundle_3.6.4\lib\glib-2.0\include" \
		   -I "D:\gtk+-bundle_3.6.4\include\atk-1.0" \
		   -I "D:\gstreamer-sdk\0.10\x86_64\include\gstreamer-0.10" \
		   -I "D:\gstreamer-sdk\0.10\x86_64\include\glib-2.0" \
		   -I "D:\gstreamer-sdk\0.10\x86_64\lib\x86_64-linux-gnu\glib-2.0\include" \
		   -I "D:\gstreamer-sdk\0.10\x86_64\lib\glib-2.0\include" \
		   -I "D:\gstreamer-sdk\0.10\x86_64\include\libxml2"


LIBS = -lcairo \
	   -lpango-1.0 \
	   -lfontconfig \
	   -lgobject-2.0 \
	   -lglib-2.0 \
	   -lfreetype \
	   -lgtk-3 \
	   -lgstreamer-0.10 \
	   -lgobject-2.0 \
	   -lglib-2.0 \
	   -pthread  \
	   -latk-1.0 \
	   -lgio-2.0 \
	   -lpangoft2-1.0 \
	   -lpangocairo-1.0 \
	   -lgdk_pixbuf-2.0 

LDPATH = -L "D:\gtk+-bundle_3.6.4\lib" \
		 -L "D:\gstreamer-sdk\0.10\x86_64\lib"

CC = gcc

CFLAGS = -g -Wall

hellomake: gtk_app.c
	${CC} ${CFLAGS} $? ${INCLUDES} -o $@ ${OBJS} ${LIBS} ${LDPATH}

