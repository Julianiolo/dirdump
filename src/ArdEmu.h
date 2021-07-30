#ifndef _ARDU_EMU
#define _ARDU_EMU

#include <vector>
#include "backends/ArduboyBackend.h"

class ArduEmu {
private:
	static std::vector<ABB::ArduboyBackend*> instances;
public:
	static void init();
	static void destroy();

	static void draw();

	static ABB::ArduboyBackend& addEmulator();
};

#endif