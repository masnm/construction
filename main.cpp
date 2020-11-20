// think about xseticon 
#include <iostream>

#include "header.h"

class game : public engine {
	
	bool on_create () override
	{
		return true;
	}

pixel p = pixel(0,255,255);

	bool on_update () override
	{
		//clear();
		for ( int i = 0 ; i<90 ; i++ )
			for ( int j=0 ; j<90 ; j++ ) {
				ui2d ps = ui2d(i,j);
				draw ( ps, pixel( 0, 150, 0));
			}
		return true;
	}

	bool on_delete () override
	{
		return true;
	}
};

int main ()
{
	game ins;
	ins.create ( 120, 100, 8, 8, false );
	ins.start();

	return 0;
}
