#ifndef _ARDUBOY_BACKEND
#define _ARDUBOY_BACKEND

#include "Arduboy.h"
#include "raylib.h"
#include <string>

#include "DisplayBackend.h"

namespace ABB {
	class ArduboyBackend {
	public:
		Arduboy ab;
	private:
		DisplayBackend displayBackend;
		std::string name;
		std::string debWinName;

		void update();
	public:

		ArduboyBackend(const char* n);

		void draw();
	};
}

#endif