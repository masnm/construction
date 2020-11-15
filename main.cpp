//#include <iostream>
//
//#include "engine.h"
//
//int main(int argc, char** argv)
//{
//	dummy_engine inistance;
//	if ( inistance.prepare_engine () )
//		inistance.start ();
//
//	return 0;
//}
//
//
#include <iostream>

#include "header.h"

int main ()
{
	engine ins;
	if ( ins.create (120, 100, 4, 4, false ) )
		ins.start();

	return 0;
}
