#pragma once

#ifdef __linux__||__FreeBSD__				// include files for LIXUX

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>

#elif _WIN32					// include files for WINDOWS

#include <Windows.h>
#include <gdiplus.h>
#include <GL/gl.h>

#endif 

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






/* |---------------------------------------------------------------------------|
 * |   start  Picture Engine decleration                                       |
 * |---------------------------------------------------------------------------| */

class picture_engine {
	protected:
	picture_engine();
	~picture_engine();

protected:
	virtual bool onCreate() = 0;
	virtual bool onUpdate() = 0;
	virtual bool onDelete() = 0;

private:
	#ifdef __linux__||__FreeBSD__

	#elif _WIN32
	WNDCLASS wc;
	HWND hwnd;
	HDC glDeviceContext = nullptr;
	HGLRC	glRenderContext = nullptr;
	static LRESULT CALLBACK windowEventProgram(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	GLuint glBuffer;
	#endif


private:
	bool window_active;
	int screen_width, screen_height, pixel_width, pixel_height;
	std::wstring app_name;

	picture* default_picture;

protected:
	bool create_window();
	void picture_engine_thread();
	bool start_opengl();

protected:
	bool draw(int x, int y);
	bool draw(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

public:
	bool construct(int sw = 320, int sh = 240, int pw = 2, int ph = 2);
	bool start();
};


///////////////////////////////////////
picture_engine::picture_engine ()
{
	#ifdef __linux__||__FreeBSD__

	#elif _WIN32

	window_active = false;
	wc = { 0 };
	hwnd = NULL;
	app_name = L"Failure";

	#endif
}
picture_engine::~picture_engine()
{
	#ifdef __linux__||__FreeBSD__

	

	#elif _WIN32

	delete default_picture;
	
	#endif
}
#ifdef __linux__||__FreeBSD__
#elif _WIN32
LRESULT CALLBACK picture_engine::windowEventProgram(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static picture_engine* sge;
	switch (uMsg)
	{
	case WM_CREATE:		sge = (picture_engine*)((LPCREATESTRUCT)lParam)->lpCreateParams;	return 0;
	case WM_CLOSE:		sge->window_active = false;									return 0;
	case WM_DESTROY:	PostQuitMessage(0);			return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
#endif
bool picture_engine::create_window ()
{
	#ifdef __linux__||__FreeBSD__


	
	#elif _WIN32

	// setting up the window class
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_CROSS);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.lpfnWndProc = windowEventProgram;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.lpszMenuName = L"Menu Name";
	wc.hbrBackground = nullptr;

	wc.lpszClassName = L"PictureEngine";
	RegisterClass(&wc);

	// Define window furniture
	DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	DWORD dwStyle = WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
	RECT rWndRect = { 0, 0, (LONG)screen_width * (LONG)pixel_width, (LONG)screen_height * (LONG)pixel_height };

	// Keep client size as requested
	AdjustWindowRectEx(&rWndRect, dwStyle, FALSE, dwExStyle);

	int width = rWndRect.right - rWndRect.left;
	int height = rWndRect.bottom - rWndRect.top;

	hwnd = CreateWindowEx(dwExStyle, L"PictureEngine", L"", dwStyle,
		CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, GetModuleHandle(nullptr), this);

	#endif
	return true;
}
void picture_engine::picture_engine_thread ()
{
	if ( !start_opengl() )
		return;
	
	glEnable(GL_TEXTURE_2D);
	#ifdef __linux__||__FreeBSD__
	#elif _WIN32
	glGenTextures(1, &glBuffer);
	glBindTexture(GL_TEXTURE_2D, glBuffer);
	#endif
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	if (!onCreate())
		window_active = false;

	auto tp1 = std::chrono::system_clock::now();
	auto tp2 = std::chrono::system_clock::now();

	while (window_active) {
		tp2 = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();
		if (!onUpdate())
			window_active = false;

		// start opengl test
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screen_width, screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, default_picture->get_array());
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 1.0); glVertex3f(-1.0f, -1.0f, 0.0f);
		glTexCoord2f(0.0, 0.0); glVertex3f(-1.0f, 1.0f, 0.0f);
		glTexCoord2f(1.0, 0.0); glVertex3f(1.0f, 1.0f, 0.0f);
		glTexCoord2f(1.0, 1.0); glVertex3f(1.0f, -1.0f, 0.0f);
		glEnd();
		#ifdef __linux__||__FreeBSD__
		#elif _WIN32
		SwapBuffers(glDeviceContext);

		wchar_t sTitle[256];
		swprintf(sTitle, 256, L"%s - FPS: %3.2f", app_name.c_str(), 1.0f / fElapsedTime);
		SetWindowText(hwnd, sTitle);
		#endif
	}
	if (!onDelete())
		return;
	
	#ifdef __linux__||__FreeBSD__
	#elif _WIN32
	PostMessage(hwnd, WM_DESTROY, 0, 0);
	#endif

}
bool picture_engine::start_opengl ()
{
	#ifdef __linux__||__FreeBSD__

	#elif _WIN32
	glDeviceContext = GetDC(hwnd);
	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR), 1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		PFD_MAIN_PLANE, 0, 0, 0, 0
	};

	int pf = 0;
	if (!(pf = ChoosePixelFormat(glDeviceContext, &pfd))) return false;
	SetPixelFormat(glDeviceContext, pf, &pfd);

	if (!(glRenderContext = wglCreateContext(glDeviceContext))) return false;
	wglMakeCurrent(glDeviceContext, glRenderContext);

	wglSwapInterval = (wglSwapInterval_t*)wglGetProcAddress("wglSwapIntervalEXT");
	wglSwapInterval(0);

	#endif

	return true;
}
bool picture_engine::construct (int sw, int sh, int pw, int ph)
{
	screen_width = sw; screen_height = sh;
	pixel_width = pw; pixel_height = ph;

	default_picture = new picture;
	default_picture->create_image(screen_width, screen_height);

	return true;
}
bool picture_engine::start ()
{
	if (!create_window())
		return false;
	
	window_active = true;
	std::thread t = std::thread(&picture_engine::picture_engine_thread, this);

	#ifdef __linux__||__FreeBSD__

	#elif _WIN32

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	#endif

	t.join();

	return true;
}

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

	// Open the window
	windowAttribs.border_pixel = BlackPixel(display, screenId);
	windowAttribs.background_pixel = WhitePixel(display, screenId);
	windowAttribs.override_redirect = True;
	windowAttribs.colormap = XCreateColormap(display, RootWindow(display, screenId), visual->visual, AllocNone);
	windowAttribs.event_mask = ExposureMask;


	// creating or setting up the window
	// needs to be in another method
	window = XCreateWindow(display, RootWindow(display, screenId), 0, 0, window_width, window_height, 0, visual->depth, InputOutput, visual->visual, CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, &windowAttribs);

	// Show the window
	XClearWindow(display, window);
	XMapRaised(display, window);

	return true;
}

void dummy_engine::update_by_user ()
{
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window_width, window_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, default_picture->get_array());
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 1.0); glVertex3f(-1.0f, -1.0f, 0.0f);
	glTexCoord2f(0.0, 0.0); glVertex3f(-1.0f, 1.0f, 0.0f);
	glTexCoord2f(1.0, 0.0); glVertex3f(1.0f, 1.0f, 0.0f);
	glTexCoord2f(1.0, 1.0); glVertex3f(1.0f, -1.0f, 0.0f);
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
	// lets try to add some opengl things
	glEnable(GL_TEXTURE_2D);
	//glGenTextures(1, &glIuffer);
	//glBindTexture(GL_TEXTURE_2D, glBuffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

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

