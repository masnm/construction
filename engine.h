#pragma once

// X11 and openGL including
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>

//typedef BOOL(WINAPI wglSwapInterval_t) (int interval);
//static wglSwapInterval_t *wglSwapInterval;


// default includes
#include <thread>
#include <chrono>
#include <string>


/* |---------------------------------------------------------------------------|
 * |   start   implementation of 2D class                                      |
 * |---------------------------------------------------------------------------| */
template <typename type>
	struct xd
	{
		type x, y;
		xd() : x(0), y(0) {}
		xd(type _x, type _y) : x(_x), y(_y) {}
		xd(const xd& r) : x(r.x), y(r.y) {}
		xd& operator = (const xd& r) = default;
		xd operator + (const xd& r) const { return xd(this->x + r.x, this->y + r.y); }
		xd operator - (const xd& r) const { return xd(this->x - r.x, this->y - r.y); }
		xd& operator += (const xd& r) { this->x += r.x; this->y += r.y; return *this; }
		xd& operator -= (const xd& r) { this->x -= r.x; this->y -= r.y; return *this; }
		bool operator != (const xd& r) const { return (this->x != r.x || this->y != r.y); }
		operator xd<int>() const { return { static_cast<int>(this->x), static_cast<int>(this->y) }; }
		operator xd<float>() const { return { static_cast<float>(this->x), static_cast<float>(this->y) }; }
	};
	typedef xd<int> i2d;
	typedef xd<float> f2d;
/* |---------------------------------------------------------------------------|
 * |   end     implementation of 2D class                                      |
 * |---------------------------------------------------------------------------| */






/* |---------------------------------------------------------------------------| *
 * |   start of                   PICTURE AND PIXEL                            | *
 * |---------------------------------------------------------------------------| */
struct pixel
{
	union {
		uint32_t n = 0xFF000000;
		struct {
			uint8_t r; uint8_t g; uint8_t b; uint8_t a;
		};
	};
	pixel ();
	pixel ( uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255 );
};
pixel::pixel ()
{
	r = 0; g = 255; b = 0; a = 255;
}
pixel::pixel ( uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha )
{
	r = red; g = green; b = blue; a = alpha;
}

class picture
{
	public:
		void create_image ( uint32_t w, uint32_t h );
		pixel* get_array ();
	public:
		uint32_t width, height;
		pixel* pixel_array;
};
void picture::create_image ( uint32_t w, uint32_t h )
{
	width = w; height = h;
	pixel_array = new pixel[width*height];
	for ( uint32_t i=0 ; i<width*height ; i++ ) pixel_array[i] = pixel();

	return;
}
pixel* picture::get_array ()
{
	return pixel_array;
}
/* |---------------------------------------------------------------------------|
 * |   end  of                   PICTURE AND PIXEL                             |
 * |---------------------------------------------------------------------------| */







/*********/

class dummy_engine {
	private:
		int window_width, window_height;

		Display* display;
		Window window;
		Screen* screen;
		int screenId;
		XEvent ev;
		XSetWindowAttributes windowAttribs;
		XVisualInfo* visual;
		
		GLuint glBuffer;
		
		picture* default_picture;

	public:
		dummy_engine ();

	public:
		void update_by_user ( float dur );
		void start ();
		bool prepare_engine ();
	public:
};
/* |---------------------------------------------------------------------------|
 * |                                    end of    DUMMY ENGINE decleration     |
 * |---------------------------------------------------------------------------| */













/* |---------------------------------------------------------------------------|
 * |   start   DUMMY ENGINE defination                                         |
 * |---------------------------------------------------------------------------| */

dummy_engine::dummy_engine ()
{
	window_width = 600;
	window_height = 400;

	picture* default_picture;
}

bool dummy_engine::prepare_engine () {
	// for pictuew
	default_picture = new picture;
	default_picture->create_image(window_width, window_height);

	// Open the display
	display = XOpenDisplay(NULL);
	screen = DefaultScreenOfDisplay(display);
	screenId = DefaultScreen(display);

	// GLX, create XVisualInfo, this is the minimum visuals we want
	GLint glxAttribs[] = {
		GLX_RGBA,
		GLX_DOUBLEBUFFER,
		GLX_DEPTH_SIZE,     24,
		GLX_STENCIL_SIZE,   8,
		GLX_RED_SIZE,       8,
		GLX_GREEN_SIZE,     8,
		GLX_BLUE_SIZE,      8,
		GLX_SAMPLE_BUFFERS, 0,
		GLX_SAMPLES,        0,
		None
	};
	visual = glXChooseVisual(display, screenId, glxAttribs);

	// vsync
	//glSwapIntervalEXT = nullptr;
	//glSwapIntervalEXT = (glSwapInterval_t*)glXGetProcAddress((unsigned char*)"glXSwapIntervalEXT");
	//if (glSwapIntervalEXT != nullptr && 0)
	//	glSwapIntervalEXT( display, window, 1);
	//GLXDrawable drawable = glXGetCurrentDrawable();
	//glXSwapIntervalEXT( display, drawable, 1);

	// Open the window
	windowAttribs.border_pixel = BlackPixel(display, screenId);
	windowAttribs.background_pixel = WhitePixel(display, screenId);
	windowAttribs.override_redirect = True;
	windowAttribs.colormap = XCreateColormap(display, RootWindow(display, screenId), visual->visual, AllocNone);
	windowAttribs.event_mask = ExposureMask;


	// creating or setting up the window
	// needs to be in another method
	window = XCreateWindow(display, RootWindow(display, screenId), 0, 0, window_width, window_height, 0, visual->depth, InputOutput, visual->visual, CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, &windowAttribs);
	
	// selecting inputs
	XSelectInput(display, window, ButtonPressMask );

	// Show the window
	XClearWindow(display, window);
	XMapRaised(display, window);

	return true;
}

void dummy_engine::update_by_user (float dur)
{
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window_width, window_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, default_picture->get_array());
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 1.0); glVertex3f(-1.0f, -1.0f, 0.0f);
	glTexCoord2f(0.0, 0.0); glVertex3f(-1.0f, 1.0f, 0.0f);
	glTexCoord2f(1.0, 0.0); glVertex3f(1.0f, 1.0f, 0.0f);
	glTexCoord2f(1.0, 1.0); glVertex3f(1.0f, -1.0f, 0.0f);
	glEnd();	
	
	// naming window
	char output[256];
	sprintf(output, "%s %3.2f", "Failure FPS : ", 1/dur);
	XStoreName( display, window, output);
}

void dummy_engine::start ()
{
	// Redirect Close
	Atom atomWmDeleteWindow = XInternAtom ( display, "WM_DELETE_WINDOW", False );
	XSetWMProtocols ( display, window, &atomWmDeleteWindow, 1 );
	// Create GLX OpenGL context
	GLXContext context = glXCreateContext(display, visual, NULL, GL_TRUE);
	glXMakeCurrent(display, window, context);
	// lets try to add some opengl things
	glEnable(GL_TEXTURE_2D);
	//glGenTextures(1, &glIuffer);
	//glBindTexture(GL_TEXTURE_2D, glBuffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);


	/*// timeing
	auto tp1 = std::chrono::system_clock::now();
	auto tp2 = std::chrono::system_clock::now();

	while (window_active) {
		tp2 = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();
	*/
	
	auto tp1 = std::chrono::system_clock::now();
	auto tp2 = std::chrono::system_clock::now();

	while (true) {
		// timeing 
		tp2 = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		
		float fElapsedTime = elapsedTime.count();

		while(XPending(display)) {
			XNextEvent(display, &ev);
			if (ev.type == Expose) {
				XWindowAttributes attribs;
				XGetWindowAttributes(display, window, &attribs);
				glViewport(0, 0, attribs.width, attribs.height);
			}
			if (ev.type == ClientMessage) {
				if (ev.xclient.data.l[0] == atomWmDeleteWindow) {
					glXDestroyContext(display, context);
					XFree(visual);
					XFreeColormap(display, windowAttribs.colormap);
					XDestroyWindow(display, window);
					XCloseDisplay(display);
					return ;
				}
			}
			else if (ev.type == DestroyNotify) {
				break;
			}
			// testing feature
			if ( ButtonPress ) {
				if (ev.xbutton.button == 1) {
					std::cout << "Left mouse down\n";
				}
				else if (ev.xbutton.button == 2) {
					std::cout << "Middle mouse down\n";
				}
				else if (ev.xbutton.button == 3) {
					std::cout << "Right mouse down\n";
				}
				else if (ev.xbutton.button == 4) {
					std::cout << "Mouse scroll up\n";
				} 
				else if (ev.xbutton.button == 5) {
					std::cout << "Mouse scroll down\n";
				}
			}
		}
		//std::this_thread::sleep_for ( std::chrono::milliseconds(20) );
		// OpenGL Rendering
		update_by_user ( fElapsedTime );
		// Clear screen and Present frame
		glXSwapBuffers(display, window);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}
/* |---------------------------------------------------------------------------|
 * |                                    end of    DUMMY ENGINE defination      |
 * |---------------------------------------------------------------------------| */

