#pragma once

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>

#include <GL/gl.h>
#include <GL/glx.h>

#include <atomic>
#include <chrono>
#include <thread>

#include <iostream>


typedef int(glSwapInterval_t)(Display* dpy, GLXDrawable drawable, int interval);
static glSwapInterval_t* glSwapIntervalEXT;

class engine {

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
		Atom delete_window_message;
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
		void sync_hardware ();
		void engine_thread ();

	protected:
		bool show_window ( );

	protected:
		bool delete_opengl ( );
		bool delete_window ( );

	protected: // temporary variable
		std::atomic<bool> running;
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

void engine::sync_hardware ()
{
	while ( XPending ( display ) ) {

		XNextEvent ( display, &event );

		switch ( event.type ) {
			case Expose:
				XWindowAttributes attribs;
				XGetWindowAttributes ( display, window, &attribs );
				glViewport ( 0, 0, attribs.width, attribs.height );
				std::cout << "Expose event fired" << std::endl;
				break;
			case ClientMessage:
				if ( event.xclient.data.l[0] == delete_window_message ) {
					running = false;
				}
				break;
		}
	}
}

void engine::engine_thread ()
{
	if ( !create_opengl() )
		return ;

	if ( !show_window() )
		return ;

	// timeing, FPS yeahh......
	auto left_time = std::chrono::system_clock::now ();
	auto right_time = std::chrono::system_clock::now ();
	float frame_time;

	glClearColor ( 0.0f, 0.0f, 0.0f, 1.0f );

	while ( running ) {

		auto right_time = std::chrono::system_clock::now ();
		std::chrono::duration<float> frame_time_counter = right_time - left_time;
		left_time = right_time;
		frame_time = frame_time_counter.count ();
		
		sync_hardware ();


		// clear the canvous
		glClear ( GL_COLOR_BUFFER_BIT );	
		
		// draw on canvous
		glBegin(GL_TRIANGLES);
			glColor3f(  1.0f,  0.0f, 0.0f);
			glVertex3f( 0.0f, -1.0f, 0.0f);
			glColor3f(  0.0f,  1.0f, 0.0f);
			glVertex3f(-1.0f,  1.0f, 0.0f);
			glColor3f(  0.0f,  0.0f, 1.0f);
			glVertex3f( 1.0f,  1.0f, 0.0f);
		glEnd();	
		
		// present the canvous
		glXSwapBuffers ( display, window );

		// rename the window
		sprintf ( app_name, "%s %s %.0f", "Failure", "FPS : ", 1/frame_time );
		XStoreName ( display, window, app_name );
	}


	if ( !delete_opengl() )
		return ;

}

bool engine::start ()
{
	if ( !create_window() )
		return false;

	bool running = true;

	// creating and executing the thread
	std::thread t = std::thread ( &engine::engine_thread, this );
	t.join();

	if ( !delete_window() )
		return false;

	return true;
}


bool engine::create_window ()
{
	XInitThreads ();

	display = XOpenDisplay ( NULL );
	if ( display == NULL ) {
		std::cout << "Failed to open display" << std::endl;
		return false;
	}
	screen = DefaultScreenOfDisplay ( display );
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
		return false;
	}

	// setting the window attributes
	window_attributes.border_pixel = BlackPixel(display, screen_id);
	window_attributes.background_pixel = WhitePixel(display, screen_id);
	window_attributes.override_redirect = True;
	window_attributes.colormap = XCreateColormap(display, RootWindow(display, screen_id), visual->visual, AllocNone);
	window_attributes.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | KeymapStateMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask;
	window = XCreateWindow(display, RootWindow(display, screen_id), 0, 0, screen_width*pixel_width, screen_height*pixel_height, 0, visual->depth, InputOutput, visual->visual, CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, &window_attributes);
	
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

	if (glSwapIntervalEXT != nullptr && !v_sync)
		glSwapIntervalEXT ( display, window, 0);


	delete_window_message = XInternAtom ( display, "WM_DELETE_WINDOW", False );
	XSetWMProtocols ( display, window, &delete_window_message, 1 );
	

	return true;
}

bool engine::show_window ()
{
	XClearWindow ( display, window );
	XMapRaised ( display, window );

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
