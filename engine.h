#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glx.h>



/* |---------------------------------------------------------------------------|
 * |   start   implementation of 2dVector                                      |
 * |---------------------------------------------------------------------------| */
template <class T>
struct v2d_generic
{
	T x = 0;
	T y = 0;
	v2d_generic() : x(0), y(0) {}
	v2d_generic(T _x, T _y) : x(_x), y(_y) {}
	v2d_generic(const v2d_generic& v) : x(v.x), y(v.y) {}
	v2d_generic  operator +  (const v2d_generic& rhs) const { return v2d_generic(this->x + rhs.x, this->y + rhs.y); }
	v2d_generic  operator -  (const v2d_generic& rhs) const { return v2d_generic(this->x - rhs.x, this->y - rhs.y); }
	v2d_generic  operator *  (const T& rhs)           const { return v2d_generic(this->x * rhs, this->y * rhs); }
	v2d_generic  operator *  (const v2d_generic& rhs) const { return v2d_generic(this->x * rhs.x, this->y * rhs.y); }
	v2d_generic  operator /  (const T& rhs)           const { return v2d_generic(this->x / rhs, this->y / rhs); }
	v2d_generic  operator /  (const v2d_generic& rhs) const { return v2d_generic(this->x / rhs.x, this->y / rhs.y); }
	v2d_generic& operator += (const v2d_generic& rhs) { this->x += rhs.x; this->y += rhs.y; return *this; }
	v2d_generic& operator -= (const v2d_generic& rhs) { this->x -= rhs.x; this->y -= rhs.y; return *this; }
	v2d_generic& operator *= (const T& rhs) { this->x *= rhs; this->y *= rhs; return *this; }
	v2d_generic& operator /= (const T& rhs) { this->x /= rhs; this->y /= rhs; return *this; }
	bool operator == (const v2d_generic& rhs) const { return (this->x == rhs.x && this->y == rhs.y); }
	bool operator != (const v2d_generic& rhs) const { return (this->x != rhs.x || this->y != rhs.y); }
	operator v2d_generic<int32_t>() const { return { static_cast<int32_t>(this->x), static_cast<int32_t>(this->y) }; }
	operator v2d_generic<float>() const { return { static_cast<float>(this->x), static_cast<float>(this->y) }; }
};

typedef v2d_generic<int32_t> vi2d;
typedef v2d_generic<uint32_t> vu2d;
typedef v2d_generic<float> vf2d;
typedef v2d_generic<double> vd2d;
/* |---------------------------------------------------------------------------|
 * |   end     implementation of 2dVector                                      |
 * |---------------------------------------------------------------------------| */






/* |---------------------------------------------------------------------------|
 * |   start   implementation of Colour                                        |
 * |---------------------------------------------------------------------------| */
struct colour {
	float red;
	float green;
	float blue;
	colour () { red = 0; green = 0; blue = 0; }
	colour ( float r, float g, float b ) { red = r; green = g; blue = b; }
};

static const colour
	GREY ( 0.75f, 0.75f, 0.75f ),
	RED ( 1.0f, 0.0f, 0.0f ),
	YELLOW ( 1.0f, 1.0f, 0.0f ),
	GREEN ( 0.0f, 1.0f, 0.0f ),
	CYAN ( 0.0f, 1.0f, 1.0f ),
	BLUE ( 0.0f, 0.0f, 1.0f ),
	MAGENTA ( 1.0f, 0.0f, 1.0f ),
	WHITE ( 1.0f, 1.0f, 1.0f ),
	BLACK ( 0.0f, 0.0f, 0.0f );


/* |---------------------------------------------------------------------------|
 * |   end     implementation of Colour                                        |
 * |---------------------------------------------------------------------------| */






/* |---------------------------------------------------------------------------|
 * |   start   DUMMY ENGINE decleration                                        |
 * |---------------------------------------------------------------------------| */
class dummy_engine {
	private:
		vf2d window_size;

		Display* display;
		Window window;
		Screen* screen;
		int screenId;
		XEvent ev;
		XSetWindowAttributes windowAttribs;
		XVisualInfo* visual;

	public:
		dummy_engine ();

	public:
		void update_by_user ();
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
	window_size.x = 600;
	window_size.y = 400;
}

bool dummy_engine::prepare_engine () {
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

	// Open the window
	windowAttribs.border_pixel = BlackPixel(display, screenId);
	windowAttribs.background_pixel = WhitePixel(display, screenId);
	windowAttribs.override_redirect = True;
	windowAttribs.colormap = XCreateColormap(display, RootWindow(display, screenId), visual->visual, AllocNone);
	windowAttribs.event_mask = ExposureMask;


	// creating or setting up the window
	// needs to be in another method
	window = XCreateWindow(display, RootWindow(display, screenId), 0, 0, window_size.x, window_size.y, 0, visual->depth, InputOutput, visual->visual, CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, &windowAttribs);

	// Show the window
	XClearWindow(display, window);
	XMapRaised(display, window);

	return true;
}

void dummy_engine::update_by_user ()
{
	
	gluOrtho2D ( 0.0f, window_size.x, 0.0f, window_size.y );
	glBegin(GL_TRIANGLES);
		glColor3f ( 0.0f, 0.5f, 0.0f ); glVertex3f ( 100.0f,  200.0f, 0.0f );
		glColor3f ( 0.0f, 0.0f, 0.5f ); glVertex3f ( 50.0f, 100.0f, 0.0f );
		glColor3f ( 0.5f, 0.0f, 0.0f ); glVertex3f (  0.0f, 50.0f, 0.0f );
	glEnd();
//	glBegin(GL_TRIANGLES);
//		glColor3f ( 0.0f, 0.0f, 0.5f ); glVertex3f (  0.5f, -0.5f, 0.0f );
//		glColor3f ( 0.0f, 0.5f, 0.0f ); glVertex3f (  0.5f,  0.5f, 0.0f );
//		glColor3f ( 0.5f, 0.0f, 0.0f ); glVertex3f (  0.0f, -0.5f, 0.0f );
//	glEnd();

	glPointSize ( 10 );
	glBegin ( GL_POINTS );
		glColor3f ( 1.0f, 0.0f, 0.0f ); glVertex3i ( 200.0f, -400.0f, 0.0f );
	glEnd();
}

void dummy_engine::start ()
{
	// Redirect Close
	Atom atomWmDeleteWindow = XInternAtom ( display, "WM_DELETE_WINDOW", False );
	XSetWMProtocols ( display, window, &atomWmDeleteWindow, 1 );
	// Create GLX OpenGL context
	GLXContext context = glXCreateContext(display, visual, NULL, GL_TRUE);
	glXMakeCurrent(display, window, context);
	while (true) {
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
		}
		//std::this_thread::sleep_for ( std::chrono::milliseconds(20) );
		// OpenGL Rendering
		update_by_user ();
		// Clear screen and Present frame
		glXSwapBuffers(display, window);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}
/* |---------------------------------------------------------------------------|
 * |                                    end of    DUMMY ENGINE defination      |
 * |---------------------------------------------------------------------------| */

