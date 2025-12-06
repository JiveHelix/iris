#include <wxpex/app.h>
#include <wxpex/wxshim_app.h>

#include "demo_brain.h"

// Creates the main function for us, and initializes the app's run loop.
wxshimAPP(wxpex::App<DemoBrain>)
