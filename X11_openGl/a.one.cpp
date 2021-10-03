#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>

#include <GL/gl.h>
#include <GL/glx.h>

#include <iostream>
#include <chrono>

// for desabling v_sync
typedef int(glSwapInterval_t)(Display* dpy, GLXDrawable drawable, int interval);
static glSwapInterval_t* glSwapIntervalEXT;

#define window_width 800
#define window_height 600
#define pixel_width 1
#define pixel_height 1


int main(int argc, char** argv) {
	Display* display;
	Window window;
	Screen* screen;
	int screenId;
	XEvent ev;

	// Open the display
	display = XOpenDisplay(NULL);
	if (display == NULL) {
		printf("%s\n", "Could not open display");
		return 1;
	}
	screen = DefaultScreenOfDisplay(display);
	screenId = DefaultScreen(display);
	
	// start openGL -> 1
	// Check GLX version
	GLint majorGLX, minorGLX = 0;
	glXQueryVersion(display, &majorGLX, &minorGLX);
	if (majorGLX <= 1 && minorGLX < 2) {
		std::cout << "GLX 1.2 or greater is required.\n";
		XCloseDisplay(display);
		return 1;
	}
	else {
		std::cout << "GLX version: " << majorGLX << "." << minorGLX << '\n';
	}

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
	XVisualInfo* visual = glXChooseVisual(display, screenId, glxAttribs);
	
	if (visual == 0) {
		std::cout << "Could not create correct visual window.\n";
		XCloseDisplay(display);
		return 1;
	}
	
	// Open the window
	XSetWindowAttributes windowAttribs;
	windowAttribs.border_pixel = BlackPixel(display, screenId);
	windowAttribs.background_pixel = WhitePixel(display, screenId);
	windowAttribs.override_redirect = True;
	windowAttribs.colormap = XCreateColormap(display, RootWindow(display, screenId), visual->visual, AllocNone);
	windowAttribs.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | KeymapStateMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask;
	window = XCreateWindow(display, RootWindow(display, screenId), 0, 0, window_width*pixel_width, window_height*pixel_height, 0, visual->depth, InputOutput, visual->visual, CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, &windowAttribs);
	
	// Create GLX OpenGL context
	GLXContext context = glXCreateContext(display, visual, NULL, GL_TRUE);
	glXMakeCurrent(display, window, context);

	std::cout << "GL Vendor: " << glGetString(GL_VENDOR) << "\n";
	std::cout << "GL Renderer: " << glGetString(GL_RENDERER) << "\n";
	std::cout << "GL Version: " << glGetString(GL_VERSION) << "\n";
	std::cout << "GL Shading Language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
	
	// Name the window
	XStoreName(display, window, "Failure");
	
    	// Show the window
	XClearWindow(display, window);
	XMapRaised(display, window);
	
	// How large is the window
	XWindowAttributes attribs;
    	XGetWindowAttributes(display, window, &attribs);
    	std::cout << "Window Width: " << attribs.width << ", Height: " << attribs.height << "\n";

    	// Resize window
    	//unsigned int change_values = CWWidth | CWHeight;
    	//XWindowChanges values;
    	//values.width = 800;
    	//values.height = 600;
    	//XConfigureWindow(display, window, change_values, &values);
	
	// desabling v_sync
	glSwapIntervalEXT = nullptr;
	glSwapIntervalEXT = (glSwapInterval_t*)glXGetProcAddress((unsigned char*)"glXSwapIntervalEXT");

	if (glSwapIntervalEXT == nullptr && !false)
	{
		std::cout << "Failed to desable v_sync" << std::endl;
	}

	if (glSwapIntervalEXT != nullptr && !false)
		glSwapIntervalEXT(display, window, 0);



	// redirect window delete
	Atom atomWmDeleteWindow = XInternAtom ( display, "WM_DELETE_WINDOW", False );
	XSetWMProtocols ( display, window, &atomWmDeleteWindow, 1 );
	
	// Variables used in message loop
	char str[25] = {0}; 
    	KeySym keysym = 0;
    	int len = 0;
    	bool running = true;
    	int x, y;

	char name[125]; sprintf( name , "%s", "Failed_again");

	// inetializing for timeing
	auto tp1 = std::chrono::system_clock::now();
	auto tp2 = std::chrono::system_clock::now();
	
	// Set GL Sample stuff
	glClearColor(0.5f, 0.6f, 0.7f, 1.0f);

	// Enter message loop
	while (running) {
		// start timeing and updating FPS
		auto tp2 = std::chrono::system_clock::now();
		std::chrono::duration<float> frame_time_counter = tp2 - tp1;
		tp1 = tp2;
		float frame_time = frame_time_counter.count();

		// entering input loop
		while ( XPending ( display ) ) {
			XNextEvent(display, &ev);
			switch(ev.type) {
				case Expose:
					XWindowAttributes attribs;
					XGetWindowAttributes(display, window, &attribs);
					glViewport(0, 0, attribs.width, attribs.height);
					std::cout << "Expose event fired\n";
					XGetWindowAttributes(display, window, &attribs);
	    			std::cout << "\tWindow Width: " << attribs.width << ", Height: " << attribs.height << "\n";
					break;
				// redirect close
				case ClientMessage:
					if (ev.xclient.data.l[0] == atomWmDeleteWindow) {
						running = false;
					}
					break;
				// for keyboard
	            		case KeymapNotify:
	                		XRefreshKeyboardMapping(&ev.xmapping);
	            			break;
	            		case KeyPress:
	                		len = XLookupString(&ev.xkey, str, 25, &keysym, NULL);
	                		if (len > 0) {
	                    			std::cout << "Key pressed: " << str << " - " << len << " - " << keysym <<'\n';
	                		}
	                		if (keysym == XK_Escape) {
	                    		running = false;
	                		}
	            			break;
	            		case KeyRelease:
	                		len = XLookupString(&ev.xkey, str, 25, &keysym, NULL);
	                		if (len > 0) {
	                    			std::cout << "Key released: " << str << " - " << len << " - " << keysym <<'\n';
	                		}
	            			break;
	            		
	            		// for mouse
	            		case ButtonPress:
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
					break;
				case ButtonRelease:
					if (ev.xbutton.button == 1) {
						std::cout << "Left mouse up\n";
					}
					else if (ev.xbutton.button == 2) {
						std::cout << "Middle mouse up\n";
					}
					else if (ev.xbutton.button == 3) {
						std::cout << "Right mouse up\n";
					}
					break;
				case MotionNotify:
					x = ev.xmotion.x;
					y = ev.xmotion.y;
					std::cout << "Mouse X:" << x << ", Y: " << y << "\n";
					break;
				case EnterNotify:
					std::cout << "Mouse enter\n";
					break;
				case LeaveNotify:
					std::cout << "Mouse leave\n";
					break;
	            	}
		} // end of XPending
		// OpenGL Rendering
		glClear(GL_COLOR_BUFFER_BIT);

		glBegin(GL_TRIANGLES);
			glColor3f(  1.0f,  0.0f, 0.0f);
			glVertex3f( 0.0f, -1.0f, 0.0f);
			glColor3f(  0.0f,  1.0f, 0.0f);
			glVertex3f(-1.0f,  1.0f, 0.0f);
			glColor3f(  0.0f,  0.0f, 1.0f);
			glVertex3f( 1.0f,  1.0f, 0.0f);
		glEnd();

		// naming the window
		sprintf( name, "%s %3.2f", "Failed_again : fps ", 1/frame_time );
		XStoreName(display, window, name);

		// Present frame
		glXSwapBuffers(display, window);
	}
	
	// Cleanup GLX
	glXDestroyContext(display, context);

	// Cleanup X11
	XFree(visual);
	XFreeColormap(display, windowAttribs.colormap);
	XDestroyWindow(display, window);
	XCloseDisplay(display);
	
	return 0;
}
