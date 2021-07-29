#ifndef _ARDUBOY_DISPLAY_BACKEND
#define _ARDUBOY_DISPLAY_BACKEND

#include "raylib.h"
#include "Arduboy.h"

namespace ABB {
	class DisplayBackend {
	private:
		AB::Display* display;

		Texture2D displayTex;
		Image displayImg;

	public:
		DisplayBackend(AB::Display* display);
		~DisplayBackend();

		void update();
		Texture& getTex();
	};
}

#endif