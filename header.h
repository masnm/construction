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

template<typename t>
struct c2d {
	t x;
	t y;
	c2d () : x(0), y(0) {}
	c2d ( t _x, t _y ) : x(_x), y(_y) {}
};
typedef c2d<uint32_t> ui2d;

struct pixel {
	union {
		uint32_t n = 0xFF000000;
		struct {
			uint8_t r; uint8_t g; uint8_t b; uint8_t a;
		};
	};

	pixel () : r(0), g(0), b(0), a(255) {}
	pixel (uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255) : r(red), g(green), b(blue), a(alpha) {}	
};

class picture {
	protected:
		uint32_t picture_widht;
		uint32_t picture_height;

		pixel* picture_data;
	
	public:
		picture ( uint32_t width, uint32_t height );
		uint32_t get_width () { return picture_widht; }
		uint32_t get_height () { return picture_height; }
		pixel* get_data ();
};

picture::picture ( uint32_t width, uint32_t height )
{
	picture_widht = width;
	picture_height = height;

	picture_data = new pixel[picture_widht*picture_height];
}

pixel* picture::get_data ()
{
	return picture_data;
}




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

	protected:
		picture* canvus;

	private:
		GLXContext context;

	public:
		bool create ( uint32_t Screen_Width, uint32_t Screen_Height, uint32_t Pixel_Widht, uint32_t Pixel_Height, bool vertical_sync );
		bool start ( );

	protected:
		virtual bool on_create () = 0;
		virtual bool on_update () = 0;
		virtual bool on_delete () = 0;

	protected:
		void clear ( pixel clear_colour );
		void clear ();
		void draw ( uint32_t x, uint32_t y, pixel color );
		void draw ( ui2d position, pixel color );

	protected:
		bool create_window ( );
		bool create_opengl ( );

		bool create_canvus ( );

	protected:
		void sync_hardware ();
		void engine_thread ();

	protected:
		bool show_window ( );

	protected:
		bool delete_opengl ( );
		bool delete_window ( );

		bool delete_canvus ( );

	protected: // temporary variable
		std::atomic<bool> running;

	protected:
		//static std::string error_string;
		std::string error_string;
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

void engine::clear ( pixel clear_colour )
{
	uint32_t len = canvus->get_width() * canvus->get_height();
	pixel* p = canvus->get_data();
	for ( uint32_t i = 0; i<len ; i++ ) p[i] = clear_colour;
}

void engine::clear ()
{
	pixel black = pixel( 0, 0, 0 );
	uint32_t len = canvus->get_width() * canvus->get_height();
	pixel* p = canvus->get_data();
	for ( uint32_t i = 0; i<len ; i++ ) p[i] = black;	
}

void engine::draw ( uint32_t x, uint32_t y, pixel color )
{
	canvus->get_data()[ y * screen_width + x ] = color;
}

void engine::draw ( ui2d position, pixel color )
{
	draw ( position.x, position.y, color );
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
	if ( !create_opengl() ) {
		error_string += "create_opengl failed.\n";
		running = false;
	}

	if ( !show_window() ) {
		error_string += "show_window failed.\n";
		running = false;
	}

	if ( !create_canvus() ) {
		error_string += "create_canvus failed.\n";
		running = false;
	}

	// creating screen texture
	glEnable(GL_TEXTURE_2D);
	//glGenTextures(1, &glBuffer);
	//glBindTexture(GL_TEXTURE_2D, glBuffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screen_width, screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, canvus->get_data() );

	// timeing, FPS yeahh......
	auto left_time = std::chrono::system_clock::now ();
	auto right_time = std::chrono::system_clock::now ();
	float frame_time;

	glClearColor ( 0.0f, 0.0f, 0.0f, 1.0f );

	if ( !on_create() ) {
		running = false;
		error_string += "on_create failed.\n";
	}

	while ( running ) {

		auto right_time = std::chrono::system_clock::now ();
		std::chrono::duration<float> frame_time_counter = right_time - left_time;
		left_time = right_time;
		frame_time = frame_time_counter.count ();
		
		sync_hardware ();

		if ( !on_update() ) {
			running = false;
			error_string += "on_update failed.\n";
			break;
		}

		// clear the canvous
		glClear ( GL_COLOR_BUFFER_BIT );	
		
		// draw on canvous
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, screen_width, screen_height, GL_RGBA, GL_UNSIGNED_BYTE, canvus->get_data() );

		// Display texture on screen
		glBegin(GL_QUADS);
			glTexCoord2f(0.0, 1.0); glVertex3f(-1.0f, -1.0f, 0.0f);
			glTexCoord2f(0.0, 0.0); glVertex3f(-1.0f,  1.0f, 0.0f);
			glTexCoord2f(1.0, 0.0); glVertex3f( 1.0f,  1.0f, 0.0f);
			glTexCoord2f(1.0, 1.0); glVertex3f( 1.0f, -1.0f, 0.0f);
		glEnd();	
		
		// present the canvous
		glXSwapBuffers ( display, window );

		// rename the window
		sprintf ( app_name, "%s %s %.0f", "Failure", "FPS : ", 1/frame_time );
		XStoreName ( display, window, app_name );
	}

	if ( !on_delete() )
		error_string += "on_delete failed.\n";

	if ( !delete_canvus () )
		error_string += "delete_canvus failed.\n";

	if ( !delete_opengl() )
		error_string += "delete_opengl failed.\n";

	// printing the errors
	if ( !error_string.empty() ) {
		std::cout << error_string << std::endl;
	}

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
		error_string += "Failed to open display.\n";
		return false;
	}
	screen = DefaultScreenOfDisplay ( display );
	screen_id = DefaultScreen ( display );

	GLint major_glx, minor_glx;
	glXQueryVersion ( display, &major_glx, &minor_glx );
	if ( major_glx <= 1 && minor_glx < 2 ) {
		error_string += "GLX 1.2 or greater is requird.\n";
		XCloseDisplay ( display );
		return false;
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
		error_string += "Failed to create correct visual window.\n";
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

//	std::cout << "GL Vendor: " << glGetString(GL_VENDOR) << "\n";
//	std::cout << "GL Renderer: " << glGetString(GL_RENDERER) << "\n";
//	std::cout << "GL Version: " << glGetString(GL_VERSION) << "\n";
//	std::cout << "GL Shading Language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";

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

bool engine::create_canvus ( )
{
	canvus = new picture ( screen_width, screen_height );

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

bool engine::delete_canvus ( )
{
	pixel* p = canvus->get_data();
	delete[] p;
	delete canvus;

	return true;
}
