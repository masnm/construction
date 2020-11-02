#include <iostream>

#include "engine.h"

int main(int argc, char** argv)
{
	dummy_engine inistance;
	if ( inistance.prepare_engine () )
		inistance.start ();

	return 0;
}
