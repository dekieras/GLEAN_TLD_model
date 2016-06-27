#include "GLEANKernel/Output_tee_globals.h"
#include "TLD_device.h"

// for use in non-dynamically loaded models
Device_base * create_TLD_device()
{
	return new TLD_device("TLD Device", Normal_out);
}

// the class factory functions to be accessed with dlsym
extern "C" Device_base * create_device() 
{
    return create_TLD_device();
}

extern "C" void destroy_device(Device_base * p) 
{
    delete p;
}
