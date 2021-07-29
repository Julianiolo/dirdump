#include "ArdEmu.h"

std::vector<ABB::ArduboyBackend*> ArduEmu::instances;

void ArduEmu::init() {

}

void ArduEmu::destroy() {
	for (auto& i : instances) {
		delete i;
	}
}

void ArduEmu::draw() {
	for (auto& i : instances) {
		i->draw();
	}
}

ABB::ArduboyBackend& ArduEmu::addEmulator() {
	ABB::ArduboyBackend* ptr = new ABB::ArduboyBackend();
	instances.push_back(ptr);
	return *ptr;
}