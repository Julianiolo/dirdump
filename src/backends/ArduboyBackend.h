#ifndef _ARDUBOY_BACKEND
#define _ARDUBOY_BACKEND

#include "Arduboy.h"
#include "raylib.h"

#include "DisplayBackend.h"

namespace ABB {
	class ArduboyBackend {
	public:
		Arduboy ab;
	private:
		DisplayBackend displayBackend;

		void update();
	public:

		ArduboyBackend();

		void draw();
	};
}

#endif