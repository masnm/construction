#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>

#include <iostream>
#include <chrono>


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

	// Open the window
	window = XCreateSimpleWindow(display, RootWindowOfScreen(screen), 0, 0, 320, 200, 1, BlackPixel(display, screenId), WhitePixel(display, screenId));

	XSelectInput(display, window, ExposureMask | KeyPressMask | KeyReleaseMask | KeymapStateMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask );
	
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
    	unsigned int change_values = CWWidth | CWHeight;
    	XWindowChanges values;
    	values.width = 800;
    	values.height = 600;
    	XConfigureWindow(display, window, change_values, &values);

	// redirect window delete
	Atom atomWmDeleteWindow = XInternAtom ( display, "WM_DELETE_WINDOW", False );
	XSetWMProtocols ( display, window, &atomWmDeleteWindow, 1 );
	
	// Variables used in message loop
	char str[25] = {0}; 
    	KeySym keysym = 0;
    	int len = 0;
    	bool running = true;
    	int x, y;

	// inetializing for timeing
	auto tp1 = std::chrono::system_clock::now();
	auto tp2 = std::chrono::system_clock::now();

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
		}
	}

	// Cleanup
	XDestroyWindow(display, window);
	XCloseDisplay(display);
	return 0;
}
