#pragma once

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>

#include <GL/gl.h>
#include <GL/glx.h>

#include <iostream>
#include <chrono>

class engine {
	private:
		typedef int(glSwapInterval_t)(Display* dpy, GLXDrawable drawable, int interval);
		static glSwapInterval_t* glSwapIntervalEXT;

	private:
		uint32_t screen_width;
		uint32_t screen_height;
		uint32_t pixel_width;
		uint32_t pixel_height;

		char app_name[256];

		bool v_sync;

	private:
		Display* display;
		Window window;
		Screen* screen;
		int screen_id;
		XEvent event;

	protected:
		Atom delete_window;
		XSetWindowAttributes window_attributes;
		XVisualInfo* visual;

	private:
		GLXContext context;

	public:
		bool create ( uint32_t Screen_Width, uint32_t Screen_Height, uint32_t Pixel_Widht, uint32_t Pixel_Height, bool vertical_sync );
		bool start ( );

	protected:
		bool create_window ( );
		bool create_opengl ( );

	protected:
		bool delete_opengl ( );
		bool delete_window ( );
};


bool engine::create ( uint32_t Screen_Width, uint32_t Screen_Height, uint32_t Pixel_Widht, uint32_t Pixel_Height, bool vertical_sync )
{
	if ( Screen_Width < 1 || Screen_Height < 1 || Pixel_Widht < 0 || Pixel_Height < 0 ) {
		return false;
	}

	screen_width = Screen_Width;
	screen_height = Screen_Height;

	pixel_width = Pixel_Widht;
	pixel_height = Pixel_Height;

	v_sync = vertical_sync;

	return true;
}

bool engine::start ()
{
	if ( !create_window() )
		return false;

	if ( !create_opengl() )
		return false;

	bool running = true;
	while ( running ) {
		while ( XPending ( display ) ) {
			XNextEvent ( display, &event );
			switch ( event.type ) {
				case Expose:
					XWindowAttributes attribs;
					XGetWindowAttributes ( display, window, &attribs );
					glViewPort ( 0, 0, attribs.width, attribs.height );
					std::cout << "Expose event fired" << std::endl;
					break;
				case ClientMessage:
					if ( ev.xclient.data.l[0] == delete_window ) {
						running = false;
					}
					break;
			}
		}

		glClear ( GL_COLOR_BUFFER_BIT );	

		glXSwapBuffers ( display, window );
	}

	if ( !delete_opengl() )
		return false;

	if ( !delete_window() )
		return false;

	return true;
}


bool engine::create_window ()
{
	display = XOpenDisplay ( NULL );
	if ( display == NULL ) {
		std::cout << "Failed to open display" << std::endl;
		return false;
	}
	screen = DefaultScreen ( display );
	screen_id = DefaultScreen ( display );

	GLint major_glx, minor_glx;
	glXQueryVersion ( display, &major_glx, &minor_glx );
	if ( major_glx <= 1 && minor_glx < 2 ) {
		std::cout << "GLX 1.2 or greater is requird" << std::endl;
		XCloseDisplay ( display );
		return false;
	} else {
		std::cout << "GLX version: " << major_glx << "." << minor_glx << std::endl;
	}

	// this is the minimim visuals for now
	// not the modern one
	// ok for my work though
	GLint glx_attributes[] = {
		GLX_RGBA,
		GLX_DOUBLEBUFFER,
		GLX_DEPTH_SIZE, 24,
		GLX_STENCIL_SIZE, 8,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_SAMPLE_BUFFERS, 0,
		GLX_SAMPLES, 0,
		None
	};
	visual = glXChooseVisual ( display, screen_id, glx_attributes );

	if ( visual == NULL ) {
		std::cout << "Failed to create correct visual window." << std::endl;
		XCloseDisplay ( display );
		return fasle;
	}

	// setting the window attributes
	window_attributes.border_pixel = BlackPixel(display, screenId);
	window_attributes.background_pixel = WhitePixel(display, screenId);
	window_attributes.override_redirect = True;
	window_attributes.colormap = XCreateColormap(display, RootWindow(display, screenId), visual->visual, AllocNone);
	window_attributes.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | KeymapStateMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask;
	window = XCreateWindow(display, RootWindow(display, screenId), 0, 0, screen_width*pixel_width, screen_height*pixel_height, 0, visual->depth, InputOutput, visual->visual, CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, &window_attributes);
	
	return true;
}	


bool engine::create_opengl ( )
{
	context = glXCreateContext(display, visual, NULL, GL_TRUE);
	glXMakeCurrent(display, window, context);

	std::cout << "GL Vendor: " << glGetString(GL_VENDOR) << "\n";
	std::cout << "GL Renderer: " << glGetString(GL_RENDERER) << "\n";
	std::cout << "GL Version: " << glGetString(GL_VERSION) << "\n";
	std::cout << "GL Shading Language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";

	XStoreName ( display, window, app_name );

	// desabling v_sync // needs to do some shanges
	glSwapIntervalEXT = nullptr;
	glSwapIntervalEXT = (glSwapInterval_t*)glXGetProcAddress((unsigned char*)"glXSwapIntervalEXT");
	if ( glSwapIntervalEXT == nullptr && !v_sync ) {
		std::cout << "Failed to desable Vertical Sync" << std::endl;
	}
	if ( glSwapInterval_t != nullptr && !v_sync )
		glSwapIntervalEXT ( diaplay, window, 0 );



	delete_window = XInternAtom ( display, "WM_DELETE_WINDOW", False );
	XSetWMProtocols ( display, window, &delete_window, 1 );
	

	return true;
}

bool engine::delete_opengl ()
{
	glXDestroyContext ( display, context );
	return true;
}

bool engine::delete_window ()
{
	XFree ( visual );
	XFreeColormap ( display, window_attributes.colormap );
	XDestroyWindow ( display, window );
	XCloseDisplay ( display );

	return true;
}
