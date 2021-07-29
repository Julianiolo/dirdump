#ifndef _ARDUBOY_BACKEND
#define _ARDUBOY_BACKEND

#include "Arduboy.h"
#include "raylib.h"

#include "DisplayBackend.h"

namespace ABB {
	class ArduboyBackend {
	private:
		DisplayBackend displayBackend;

	public:
		Arduboy ab;

		ArduboyBackend();

		void draw();
	};
}

#endif