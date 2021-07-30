#include "DisplayBackend.h"

ABB::DisplayBackend::DisplayBackend(AB::Display* display) : display(display) {
	displayImg.width = AB::Display::WIDTH;
	displayImg.height = AB::Display::HEIGHT;
	displayImg.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
	displayImg.data = new uint8_t[displayImg.width * displayImg.height * 3];
	displayImg.mipmaps = 1;

	displayTex = LoadTextureFromImage(displayImg);
}
ABB::DisplayBackend::~DisplayBackend() {
	UnloadImage(displayImg);
	UnloadTexture(displayTex);
}

void ABB::DisplayBackend::update() {
	for (int y = 0; y < AB::Display::HEIGHT; y++) {
		for (int x = 0; x < AB::Display::WIDTH; x++) {
			((Color3*)displayImg.data)[y * AB::Display::WIDTH + x] = display->getPixel(x, y) ? Color3{255, 255, 255} : Color3{0,0,0};
		}
	}
	UpdateTexture(displayTex, displayImg.data);
}

Texture& ABB::DisplayBackend::getTex() {
	return displayTex;
}