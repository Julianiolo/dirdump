#include "ArduboyBackend.h"

#include "imgui.h"
#include "rlImGui/rlImGui.h"

ABB::ArduboyBackend::ArduboyBackend() : displayBackend(&ab.display) {

}

void ABB::ArduboyBackend::draw() {
	if (ImGui::Begin("lul")) {
		RLImGuiImage(&displayBackend.getTex());
	}
	ImGui::End();
}