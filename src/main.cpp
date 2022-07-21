#include <iostream>



#include "raylib.h"
#define IMGUI_DEFINE_MATH_OPERATORS 1
#include "imgui_internal.h"
#include "imgui.h"
#include "rlImGui/rlImGui.h"
#include "oneHeaderLibs/VectorOperators.h"
#include "ImGuiFD.h"
#include "dumper.h"
#include "Extensions/imguiExt.h"


void setup();
void draw();
void destroy();

int frameCnt = 0;

Vector2 lastMousePos;
Vector2 mouseDelta;


std::string path;


#if 0
void drawFolderStack(const dirdump::FolderScanData& scanData, float* scale, ImVec2 size, ImGuiID id) {
	if (size.x == 0)
		size.x = ImGui::GetContentRegionAvail().x;
	if (size.y == 0)
		size.y = 50;

	const float listByteLen = (float)scanData.folders[scanData.rootInd].size;

	ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 5);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0,0 });


	ImGui::BeginChild(id, size, true, ImGuiWindowFlags_HorizontalScrollbar);

	ImGui::BeginGroup();

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0,0 });

	symb_size_t lastSymbEnd = 0;
	for (size_t i = 0; i < list->size(); i++) {
		const Symbol* symbol = list->operator[](i);
		if (symbol->size == 0)
			continue;

		if (symbol->value > lastSymbEnd) {
			//ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (((float)(symbol->value - lastSymbEnd) / listByteLen)) * size.x * (*scale));
			if (i != 0)
				ImGui::SameLine();

			symb_size_t fillAmt = symbol->value - lastSymbEnd;
			ImGuiExt::Rect(ImGuiID(symbol->value * i), {0,0,0,0}, { (((float)fillAmt / listByteLen)) * size.x * (*scale), size.y });
			if (ImGui::IsItemHovered()) {
				ImGui::PopStyleVar();
				ImGui::PopStyleVar();

				ImGui::SetTooltip("Space Without Symbol: %" PRIu32 " bytes (%f%%)", fillAmt, ((float)fillAmt / listByteLen)*100);

				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0,0 });
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0,0 });
			}
		}

		if (i != 0 || symbol->value > lastSymbEnd)
			ImGui::SameLine();
		float width = ((float)symbol->size / listByteLen) * size.x * (*scale);
		ImGuiExt::Rect((ImGuiID)(symbol->value * i), symbol->col, {width, size.y});

		if (ImGui::IsItemHovered()) {
			ImGui::PopStyleVar();
			ImGui::PopStyleVar();

			ImGui::BeginTooltip();
			symbol->draw(-1, data);
			ImGui::EndTooltip();

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0,0 });
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0,0 });
		}

		lastSymbEnd = symbol->addrEnd();
	}

	if (lastSymbEnd < totalSize) {
		symb_size_t fillAmt = totalSize - lastSymbEnd;
		ImGui::SameLine();
		ImGuiExt::Rect(ImGuiID(totalSize * lastSymbEnd), {0,0,0,0}, { (((float)fillAmt / listByteLen)) * size.x * (*scale), size.y });
		if (ImGui::IsItemHovered()) {
			ImGui::PopStyleVar();
			ImGui::PopStyleVar();

			ImGui::SetTooltip("Space Without Symbol: %" PRIu64 " bytes (%f%%)", fillAmt, ((float)fillAmt / listByteLen)*100);

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0,0 });
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0,0 });
		}
	}


	ImGui::PopStyleVar();

	ImGui::EndGroup();

	if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
		ImGui::OpenPopup("settings");

	ImGui::PopStyleVar();
	if (ImGui::BeginPopup("settings")) {
		ImGui::SliderFloat("Scale", scale, 0.1f, 10);
		if (ImGui::Button("Reset"))
			*scale = 1;
		ImGui::EndPopup();
	}
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0,0 });

	ImGui::EndChild();

	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}
#endif

bool hasData = false;
dirdump::FolderScanData data;
dirdump::FolderScanDataEx ex;
std::vector<ImVec4> cols;


std::string getNumWithPnts(uint64_t num) {
	std::string out;

	uint8_t cnt = 0;
	while (num > 0) {
		char buf[2];
		buf[0] = '0' + (num % 10);
		buf[1] = 0;
		out = buf + out;
		num /= 10;

		cnt++;

		if (cnt >= 3) {
			cnt = 0;
			if (num > 0)
				out = "." + out;
		}
	}

	return out;
}

constexpr float height = 30;

void drawDiagramm(const char* str_id, const dirdump::FolderScanData& data, const dirdump::FolderScanDataEx& ex, ImVec2 size) {
	if (size.x == 0)
		size.x = ImGui::GetContentRegionAvail().x;

	constexpr float height = 30;
	float totalSize = (float)data.folders[data.rootInd].size;

	if (ImGui::BeginChild(str_id, size)) {
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		for (size_t d = 0; d < data.maxDepth+1; d++) {
			ImVec2 startPos = ImGui::GetCursorScreenPos();

			for (size_t i = 0; i < ex.depths[d].size(); i++) {
				const size_t ind = ex.depths[d][i];
				const dirdump::Folder& folder = data.folders[ind];
				const dirdump::FolderScanDataEx::ExData exData = ex.ex[ind];

				float width = folder.size / totalSize * size.x;
				if (width >= 1) {
					ImGui::SetCursorPos(ImVec2(exData.totalOff / totalSize * size.x, d * height));
					ImGuiExt::Rect((ImGuiID)folder.id, cols[ind], ImVec2(width, height));

					bool hovered = ImGui::IsItemHovered();
					
					size_t f_hovered = -1;
					for (size_t f = 0; f < folder.files.size(); f++) {
						float off = exData.filesTotalOff[f] / totalSize * size.x;
						float width = folder.files[f].size / totalSize * size.x;
						if (width >= 1) {
							ImRect rect(startPos + ImVec2(off, 1), startPos + ImVec2(off+width,height) - ImVec2(0, 1));
							ImGui::GetWindowDrawList()->AddRect(rect.Min, rect.Max, IM_COL32(51,51,51,255));
							if (f_hovered == (size_t)-1 && hovered) {
								if (rect.Contains(ImGui::GetMousePos()))
									f_hovered = f;
							}
						}
					}

					if (hovered) {
						ImGui::PopStyleVar();
						ImGui::BeginTooltip();

						ImGui::Text("%s", data.stable.getStr(folder.path));
						ImGui::Text(" Size: %s bytes", getNumWithPnts(folder.size).c_str());
						ImGui::Text("SubFs: %llu", folder.subfolders.size());
						ImGui::Text("Files: %llu", folder.files.size());

						if (f_hovered != (size_t)-1) {
							ImGui::Separator();

							ImGui::Text("File: %s", data.stable.getStr(folder.files[f_hovered].name));
							ImGui::Text("Size: %s", getNumWithPnts(folder.files[f_hovered].size).c_str());
						}

						ImGui::EndTooltip();
						ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
					}
				}
			}

			ImGui::GetWindowDrawList()->AddRect(startPos, startPos + ImVec2(size.x, height), ImColor(ImGui::GetStyleColorVec4(ImGuiCol_Border)));
		}
		ImGui::PopStyleVar();
	}
	ImGui::EndChild();
}


RenderTexture diagTex;
float scale = 1;
float scroll = 0;
bool dragging;

void drawDiag2RecTT(const dirdump::Folder& folder, size_t f_hovered, uint64_t totalSize) {
	ImGui::BeginTooltip();

	ImGui::Text("%s", data.stable.getStr(folder.path));
	ImGui::Text(" Size: %s (%.2f%%)", getNumWithPnts(folder.size).c_str(), (float)folder.size/totalSize * 100);
	ImGui::Text("SubFs: %llu", folder.subfolders.size());
	ImGui::Text("Files: %llu", folder.files.size());

	if (f_hovered != (size_t)-1) {
		ImGui::Separator();

		ImGui::Text("File: %s", data.stable.getStr(folder.files[f_hovered].name));
		ImGui::Text("Size: %s (%.2f%%)", getNumWithPnts(folder.files[f_hovered].size).c_str(), (float)folder.files[f_hovered].size/totalSize * 100);
	}

	ImGui::EndTooltip();
}
void drawDiag2Rec(size_t ind, const ImVec2& startPos, float sizeFac, size_t hoveredLine, float off, float max, size_t totalSize) {
	const dirdump::Folder& folder = data.folders[ind];
	
	float width = folder.size *sizeFac;
	if (width >= 1) {
		const dirdump::FolderScanDataEx::ExData exData = ex.ex[ind];

		float x = (exData.totalOff + off) * sizeFac;
		float y = folder.depth * height;

		if (x + width < 0 || x > max) // not visible
			return;

		const ImVec4& col = cols[ind];

		Color c = {(uint8_t)(col.x*255), (uint8_t)(col.y*255), (uint8_t)(col.z*255), 255};
		Rectangle rec = { x, y, width, height };
		DrawRectangleRec(rec, c);

		bool hovered = folder.depth == hoveredLine && CheckCollisionPointRec({ ImGui::GetMousePos().x - startPos.x, ImGui::GetMousePos().y-startPos.y }, rec);


		// draw Files
		size_t f_hovered = -1;
		for (size_t f = 0; f < folder.files.size(); f++) {
			float offs = (exData.filesTotalOff[f] + off) * sizeFac;
			float fwidth = folder.files[f].size * sizeFac;
			if (fwidth >= 1) {
				Rectangle frec = {offs, rec.y + 1, fwidth, height-2*1};
				DrawRectangleLinesEx(frec, 1, {51,51,51,255});


				if (hovered && f_hovered == (size_t)-1) {
					if (CheckCollisionPointRec({ ImGui::GetMousePos().x - startPos.x, ImGui::GetMousePos().y-startPos.y }, frec))
						f_hovered = f;
				}
			}
		}

		// draw Tooltip
		if (hovered) {
			drawDiag2RecTT(folder, f_hovered, totalSize);
		}

		for (size_t i = 0; i < folder.subfolders.size(); i++) {
			drawDiag2Rec(folder.subfolders[i], startPos, sizeFac, hoveredLine, off, max, totalSize);
		}
	}
}

void drawDiagrammV2(const char* str_id, const dirdump::FolderScanData& data, const dirdump::FolderScanDataEx& ex, ImVec2 size, float* scale, float* scroll, bool* dragging) {
	if (!ImGui::BeginChild(str_id, size)) {
		ImGui::EndChild();
		return;
	}
	
	if (size.x == 0)
		size.x = ImGui::GetContentRegionAvail().x;
	if (size.y == 0)
		size.y = ImGui::GetContentRegionAvail().y;

	if (GetScreenWidth() != diagTex.texture.width || GetScreenHeight() != diagTex.texture.height) {
		UnloadRenderTexture(diagTex);
		diagTex = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
	}

	

	
	float totalSize = (float)data.folders[data.rootInd].size;
	

	float maxHeight = height * data.maxDepth;

	//ImGui::Text("scl: %f, scrl: %f, off: %f", *scale, *scroll, *scroll / (size.x / totalSize));
	ImVec2 startPos = ImGui::GetCursorScreenPos();

	size_t hoveredLine = -1;
	if (ImGui::IsWindowHovered()) {
		if (ImGui::GetMousePos().y >= startPos.y && ImGui::GetMousePos().x >= startPos.x) {
			hoveredLine = (size_t)((ImGui::GetMousePos().y - startPos.y) / height);

			if (ImGui::GetIO().MouseWheel != 0) {
				float diff = 1 - ImGui::GetIO().MouseWheel * -0.25f;

				float mouseXOff = ImGui::GetMousePos().x - startPos.x;
				float mouseOver = mouseXOff / ((size.x / totalSize) * *scale);
				float mouseOverNow = mouseXOff / ((size.x / totalSize) * (*scale*diff));

				*scroll += (mouseOverNow-mouseOver) * (size.x / totalSize);

				*scale *= diff;
			}
			


			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
				*dragging = true;
			}
			else if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
				*dragging = false;
			}

			if (*dragging) {
				*scroll += mouseDelta.x / (*scale);
			}
		}
		
		//ImGui::SetItemUsingMouseWheel();

	}
	float sizeFac = (size.x / totalSize) * *scale;

	float off = *scroll / (size.x / totalSize);
	float max = size.x;

	RLImGuiImageRect(&diagTex.texture, (int)size.x, (int)size.y, {0, 0, size.x, -size.y});

	BeginTextureMode(diagTex);
	ClearBackground(BLANK);

	drawDiag2Rec(data.rootInd, startPos, sizeFac, hoveredLine, off, max, data.folders[data.rootInd].size);

	ImVec4 borderCol = ImGui::GetStyleColorVec4(ImGuiCol_Border);
	Color bCol = { (uint8_t)(borderCol.x * 255), (uint8_t)(borderCol.y * 255), (uint8_t)(borderCol.z * 255), 255 };
	for (size_t i = 0; i < ex.depths.size(); i++) {
		DrawRectangleLinesEx({ 0, height * i, size.x, height }, 1, bCol);
	}

	EndTextureMode();

	ImGui::EndChild();
}

void setup() {
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(1200, 800, "ABemu");

	diagTex = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

    SetWindowResizeDrawCallback(draw);
    SetTargetFPS(60);

    lastMousePos = GetMousePosition();
    mouseDelta = { 0,0 };

    SetupRLImGui(true);
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigWindowsResizeFromEdges = true;
    io.WantSaveIniSettings = false;
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    
    //printf("%s\n", GetWorkingDirectory());

    path = GetWorkingDirectory();
}
void draw() {
    BeginDrawing();

    mouseDelta = GetMousePosition() - lastMousePos;

    ClearBackground(DARKGRAY);

    BeginRLImGui();

    //ImGui::ShowDemoWindow(NULL);

    if (ImGui::Begin("DirDump")) {
        ImGui::TextUnformatted(path.c_str());
        if (ImGui::Button("Choose")) {
            ImGuiFD::OpenDirDialog("Choose Dir", path.c_str());
        }

		static bool scanAvail = false;

        if (!dirdump::isBusy()) {
			if (scanAvail) {
				scanAvail = false;

				data = dirdump::getData();
				ex = dirdump::generateExtraData(data);
				hasData = true;
				cols.resize(data.folders.size());
				for (size_t i = 0; i < data.folders.size(); i++) {
					ImGui::ColorConvertHSVtoRGB((float)(rand() % 256) / 256, 0.7f, 0.9f, cols[i].x, cols[i].y, cols[i].z);
					cols[i].w = 1;
				}
			}

			ImGui::SetNextItemWidth(100);
			static int32_t numThreads = 4;
			ImGui::InputInt("Number of Threads", &numThreads);
			if (numThreads > 8)
				numThreads = 8;

            if (ImGui::Button("Scan")) {
                dirdump::startDump(path.c_str(), numThreads);
				scanAvail = true;
            }

			
        }
        else {
			ImGui::BeginDisabled();
			ImGui::Button("Scanning...");
			ImGui::EndDisabled();
            dirdump::update();
        }

        dirdump::Stats stats = dirdump::getStats();
        ImGui::Text("%llu folders, %llu files scanned, %llu folder scans failed, %fs", stats.folders, stats.files, stats.folderErrs, (double)stats.nanos/1000000000);


		if (hasData)
			drawDiagrammV2("Diagram", data, ex, {0,0}, &scale, &scroll, &dragging);
    }
    ImGui::End();

    if (ImGuiFD::BeginDialog("Choose Dir")) {

        if (ImGuiFD::ActionDone()) {
            if (ImGuiFD::SelectionMade()) {
                path = ImGuiFD::GetSelectionPathString(0);
            }
            ImGuiFD::CloseCurrentDialog();
        }

        ImGuiFD::EndDialog();
    }

    EndRLImGui();

    lastMousePos = GetMousePosition();

    DrawFPS(0,0);

    EndDrawing();

    frameCnt++;
}
void destroy() {
	UnloadRenderTexture(diagTex);
    ShutdownRLImGui();
    CloseWindow();
}


int main(void) {
    setup();

    while (!WindowShouldClose()) {
        draw();
    }

    destroy();

    return 0;
}

//ababb..load("C:\\Users\\Julian\\source\\repos\\ArduboyTests\\ArduboyTest1\\ArduboyTest1\\ArduboyTest1\\ArduboyTest1.ino.hex");
//abb.ab.load("C:/Users/Julian/Desktop/Dateien/Arduino/Arduboy_supersimple/arduino_build_472331/Arduboy_supersimple.ino.hex");
//abb.ab.load("C:/Users/Julian/Desktop/Dateien/Arduino/Arduboy_supersimple2/arduino_build_737976/HelloWorld.ino.hex");
//abb.ab.load("C:/Users/Julian/Desktop/Dateien/scriipts/Processing 3 sketche/arduboyHexToImg/data/CastleBoy.hex"); // Chrashes
//abb.ab.load("C:/Users/Julian/Desktop/Dateien/scriipts/Processing 3 sketche/arduboyHexToImg/data/UnicornDash.hex");











/*


void printFlt(uint64_t f,uint8_t ebits,uint8_t fbits) {
uint64_t exp = f >> fbits & (((uint64_t)1 << ebits)-1);
const uint64_t exponentBias = ((uint64_t)1 << ebits) / 2 - 1;
printf("s: %d, f: %llx, e: %llx (%lld)\n",(int)(f>>(fbits+ebits)), f&(((uint64_t)1<<fbits)-1), exp, (int)exp-exponentBias);
}



double a = 11;
uint64_t a_ = *(uint64_t*)&a;
uint64_t b = StringUtils::stof("11",0, 11, 52);
double b_ = *(double*)&b;
printf("%.10f vs. %.10f\n", a, b_);
printf("%.50f\n", a);
printf("%.50f\n", b_);
printf("%llu vs. %llu\n", a_, b);
//printf("%08x vs. %08x\n", a_, b);
printf("same: %s\n", a_ == b ? "true" : "false");
printFlt(a_,11,52);
printFlt(b,11,52);
printf("diff: %.30f => %.10f%%\n", b_-a, (b_/a-1)*100);
return 0;


*/