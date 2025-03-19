#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>
#include <SDL3/SDL.h>
#include <stdio.h>

#include "SupaNES.h"

typedef struct {
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* texture;
	SupaNES* supaNES;
} AppState;

const int width = 256;
const int height = 224;
int windowSizeMultiplier = 3;

char windowTitle[128];
uint64_t deltaTime = 0;
uint64_t lastFrame = 0;
uint64_t currentTime = 0;
int fps = 60;
float desiredDelta = 1000.0f / fps;

// Main loop bool
bool done = false;

// Layer Display Settings
bool BG1Selected = true;
bool BG2Selected = true;
bool BG3Selected = true;
bool BG4Selected = true;
bool OBJSelected = true;

bool displayVRAM = false;
bool displayPalette = false;

bool pause = false;

void RenderFrame(AppState* appstate);

void SDLCALL openFileCallback(void* userdata, const char* const* filelist, int filter);
void HandleEvents(AppState* appstate);
void MenuBar(AppState* appstate);

void ShowVRAMWindow(AppState* appstate);
void ShowPaletteWindow(AppState* appstate);

int main(int argc, char* argv[]) {
	// Init systems
	if(!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO)) {
		SDL_Log("Couldn't initialize SDL: %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	// Create window and renderer
	AppState* as = (AppState*)SDL_calloc(1, sizeof(AppState));
	if(!as) {
		return SDL_APP_FAILURE;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui::StyleColorsDark();

	if (!SDL_CreateWindowAndRenderer("SupaNES2", width * windowSizeMultiplier, (height + ImGui::GetFrameHeight()) * windowSizeMultiplier,
		0, &as->window, &as->renderer)) {
		SDL_Log("Couldn't craete window and renderer: %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}
	as->texture = SDL_CreateTexture(as->renderer, SDL_PIXELFORMAT_ABGR1555, SDL_TEXTUREACCESS_STREAMING, width, height);

	ImGui_ImplSDL3_InitForSDLRenderer(as->window, as->renderer);
	ImGui_ImplSDLRenderer3_Init(as->renderer);

	as->supaNES = new SupaNES();
	int cycles = 0;

	// Main Loop
	while(!done) {
		currentTime = SDL_GetTicks();
		
		// SupaNes clock
		if(!pause && as->supaNES->RomLoaded) {
			cycles += as->supaNES->Clock();

			if (as->supaNES->ppu->frameCompleted) {
			
				HandleEvents(as);
		
				RenderFrame(as);
			}
		}
		else {
			HandleEvents(as);
		
			RenderFrame(as);
		}
	}
	
	// Cleanup
	ImGui_ImplSDLRenderer3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();

	if(as) {
		SDL_DestroyRenderer(as->renderer);
		SDL_DestroyWindow(as->window);
		SDL_free(as);
	}

	SDL_Quit();

	return 0;
}

void RenderFrame(AppState* appstate) {
	// Start frame
	ImGui_ImplSDLRenderer3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	MenuBar(appstate);

	if (displayVRAM) {
		ShowVRAMWindow(appstate);
	}

	if (displayPalette) {
		ShowPaletteWindow(appstate);
	}

	ImGui::Render();

	// Render
	if (appstate->supaNES->RomLoaded) {
		SDL_SetRenderDrawColor(appstate->renderer, 0, 0, 0, 255);
		SDL_RenderClear(appstate->renderer);
		void* texturePixels;
		int pitch;
		SDL_LockTexture(appstate->texture, NULL, &texturePixels, &pitch);
		appstate->supaNES->ppu->SetFrame(texturePixels, pitch);
		SDL_UnlockTexture(appstate->texture);
		int windowWidth;
		int windowHeight;
		SDL_GetWindowSize(appstate->window, &windowWidth, &windowHeight);
		SDL_FRect destRect = { 0.0f, ImGui::GetFrameHeight(),
								(float)windowWidth, windowHeight - ImGui::GetFrameHeight() };
		SDL_RenderTexture(appstate->renderer, appstate->texture, NULL, &destRect);

		deltaTime = currentTime - lastFrame;
		lastFrame = currentTime;
		snprintf(windowTitle, 128, "SupaNES2 | %.0f", 1000.0f / deltaTime);
		SDL_SetWindowTitle(appstate->window, windowTitle);
	}
	else {
		SDL_SetRenderDrawColor(appstate->renderer, 255, 0, 0, 255);
		SDL_RenderClear(appstate->renderer);
	}
	ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), appstate->renderer);
	SDL_RenderPresent(appstate->renderer);
}

void HandleEvents(AppState* appstate) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		ImGui_ImplSDL3_ProcessEvent(&event);
		if (event.type == SDL_EVENT_KEY_DOWN) {
			switch(event.key.scancode) {
				case SDL_SCANCODE_ESCAPE:	// QUIT
					done = true;
					break;

				case SDL_SCANCODE_T:
				{
					const char* testFile = "/home/legolasbat/Documentos/C/SupaNES2/Tests/CPU/ADC-Passed/CPUADC.sfc";
					appstate->supaNES->InsertROM(testFile);
					break;
				}
				case SDL_SCANCODE_P:
					pause = !pause;
					break;

				case SDL_SCANCODE_F3:		// Show trace of CPU
					appstate->supaNES->cpu->debug = !appstate->supaNES->cpu->debug;
					break;

				case SDL_SCANCODE_H:		// B bottom
					appstate->supaNES->cpu->joypad1High |= 0x80;
					break;
				case SDL_SCANCODE_Y:		// Y bottom
					appstate->supaNES->cpu->joypad1High |= 0x40;
					break;
				case SDL_SCANCODE_RSHIFT:
				case SDL_SCANCODE_LSHIFT:	// Select bottom
					appstate->supaNES->cpu->joypad1High |= 0x20;
					break;
				case SDL_SCANCODE_RETURN:	// Start bottom
					appstate->supaNES->cpu->joypad1High |= 0x10;
					break;
				case SDL_SCANCODE_UP:
				case SDL_SCANCODE_W:		// Up bottom (reset down bottom, hardware limitations)
					appstate->supaNES->cpu->joypad1High |= 0x08;
					if (appstate->supaNES->cpu->joypad1High & 0x04)
						appstate->supaNES->cpu->joypad1High &= ~0x04;
					break;
				case SDL_SCANCODE_DOWN:
				case SDL_SCANCODE_S:		// Down bottom (reset up bottom, hardware limitations)
					appstate->supaNES->cpu->joypad1High |= 0x04;
					if (appstate->supaNES->cpu->joypad1High & 0x08)
						appstate->supaNES->cpu->joypad1High &= ~0x08;
					break;
				case SDL_SCANCODE_LEFT:
				case SDL_SCANCODE_A:		// Left bottom (reset right bottom, hardware limitations)
					appstate->supaNES->cpu->joypad1High |= 0x02;
					if (appstate->supaNES->cpu->joypad1High & 0x01)
						appstate->supaNES->cpu->joypad1High &= ~0x01;
					break;
				case SDL_SCANCODE_RIGHT:
				case SDL_SCANCODE_D:		// Right bottom (reset left bottom, hardware limitations)
					appstate->supaNES->cpu->joypad1High |= 0x01;
					if (appstate->supaNES->cpu->joypad1High & 0x02)
						appstate->supaNES->cpu->joypad1High &= ~0x02;
					break;
				case SDL_SCANCODE_J:		// A bottom
					appstate->supaNES->cpu->joypad1Low |= 0x80;
					break;
				case SDL_SCANCODE_U:		// X bottom
					appstate->supaNES->cpu->joypad1Low |= 0x40;
					break;
				case SDL_SCANCODE_Q:		// L bottom
					appstate->supaNES->cpu->joypad1Low |= 0x20;
					break;
				case SDL_SCANCODE_E:		// R bottom
					appstate->supaNES->cpu->joypad1Low |= 0x10;
					break;
			}
		}
		if (event.type == SDL_EVENT_KEY_UP) {
			switch(event.key.scancode) {
				case SDL_SCANCODE_H:		// B bottom
					appstate->supaNES->cpu->joypad1High &= ~0x80;
					break;
				case SDL_SCANCODE_Y:		// Y bottom
					appstate->supaNES->cpu->joypad1High &= ~0x40;
					break;
				case SDL_SCANCODE_RSHIFT:
				case SDL_SCANCODE_LSHIFT:	// Select bottom
					appstate->supaNES->cpu->joypad1High &= ~0x20;
					break;
				case SDL_SCANCODE_RETURN:	// Start bottom
					appstate->supaNES->cpu->joypad1High &= ~0x10;
					break;
				case SDL_SCANCODE_UP:
				case SDL_SCANCODE_W:		// Up bottom
					appstate->supaNES->cpu->joypad1High &= ~0x08;
					break;
				case SDL_SCANCODE_DOWN:
				case SDL_SCANCODE_S:		// Down bottom
					appstate->supaNES->cpu->joypad1High &= ~0x04;
					break;
				case SDL_SCANCODE_LEFT:
				case SDL_SCANCODE_A:		// Left bottom
					appstate->supaNES->cpu->joypad1High &= ~0x02;
					break;
				case SDL_SCANCODE_RIGHT:
				case SDL_SCANCODE_D:		// Right bottom
					appstate->supaNES->cpu->joypad1High &= ~0x01;
					break;
				case SDL_SCANCODE_J:		// A bottom
					appstate->supaNES->cpu->joypad1Low &= ~0x80;
					break;
				case SDL_SCANCODE_U:		// X bottom
					appstate->supaNES->cpu->joypad1Low &= ~0x40;
					break;
				case SDL_SCANCODE_Q:		// L bottom
					appstate->supaNES->cpu->joypad1Low &= ~0x20;
					break;
				case SDL_SCANCODE_E:		// R bottom
					appstate->supaNES->cpu->joypad1Low &= ~0x10;
					break;
			}
		}
		if (event.type == SDL_EVENT_QUIT) {
			done = true;
		}
	}
}

void MenuBar(AppState* appstate) {
	// Menu bar
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Load ROM")) {
				SDL_ShowOpenFileDialog(&openFileCallback, appstate, appstate->window, NULL, 0, NULL, false);
			}
			if (ImGui::MenuItem("Exit")) {
				done = true;
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Debug")) {
			if (ImGui::BeginMenu("Display BG")) {
				// TODO: Enable/Disable Backgrounds
				if (ImGui::MenuItem("BG1", 0, BG1Selected, true)) {
					BG1Selected = !BG1Selected;
					if (appstate->supaNES->RomLoaded) {
						appstate->supaNES->ppu->BG1Display = BG1Selected;
					}
				}
				if (ImGui::MenuItem("BG2", 0, BG2Selected, true)) {
					BG2Selected = !BG2Selected;
					if (appstate->supaNES->RomLoaded) {
						appstate->supaNES->ppu->BG2Display = BG2Selected;
					}
				}
				if (ImGui::MenuItem("BG3", 0, BG3Selected, true)) {
					BG3Selected = !BG3Selected;
					if (appstate->supaNES->RomLoaded) {
						appstate->supaNES->ppu->BG3Display = BG3Selected;
					}
				}
				if (ImGui::MenuItem("BG4", 0, BG4Selected, true)) {
					BG4Selected = !BG4Selected;
					if (appstate->supaNES->RomLoaded) {
						appstate->supaNES->ppu->BG4Display = BG4Selected;
					}
				}
				if (ImGui::MenuItem("OBJ", 0, OBJSelected, true)) {
					OBJSelected = !OBJSelected;
					if (appstate->supaNES->RomLoaded) {
						appstate->supaNES->ppu->ObjDisplay = OBJSelected;
					}
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Display Memory")) {
				if (ImGui::MenuItem("Display VRAM", 0, displayVRAM, true)) {
					displayVRAM = !displayVRAM;
				}
				if (ImGui::MenuItem("Display Palette", 0, displayPalette, true)) {
					displayPalette = !displayPalette;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

void ShowVRAMWindow(AppState* appstate) {
	ImGui::Begin("VRAM", &displayVRAM);

	SupaNES* supaNES = appstate->supaNES;

	for (int row = 0; row < 0x10000; row += 16) {
		ImGui::Text("%04x:", row);
		ImGui::SameLine();
		for (int i = row; i < row + 16; i++) {
			if (i % 2) {
				ImGui::Text("%02x ", supaNES->ppu->VRAM[i / 2] >> 8);
			}
			else {
				ImGui::Text("%02x ", supaNES->ppu->VRAM[i / 2] & 0xFF);
			}
			ImGui::SameLine();
		}
		ImGui::NewLine();
	}

	ImGui::End();
}

void ShowPaletteWindow(AppState* appstate) {
	ImGui::Begin("Palette", &displayPalette);
	SupaNES* supaNES = appstate->supaNES;

	for (int row = 0; row < 0x100; row += 16) {
		ImGui::Text("%04x:", row);
		ImGui::SameLine();
		for (int i = row; i < row + 16; i++) {
			uint16_t colorBit = supaNES->ppu->CGRAM[i];
			float color[] = {(float)(colorBit & 0x1F) / 31.0f, (float)((colorBit >> 5) & 0x1F) / 31.0f, (float)((colorBit >> 10) & 0x1F) / 31.0f };
			ImGui::PushID(i);
			ImGui::ColorEdit3("", color, ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker);
			ImGui::PopID();
			ImGui::SameLine();
		}
		ImGui::NewLine();
	}
	ImGui::End();
}

void SDLCALL openFileCallback(void* userdata, const char* const* filelist, int filter) {
	if (!filelist) {
		SDL_Log("An error occured: %s\n", SDL_GetError());
		return;
	}
	else if (!*filelist) {
		SDL_Log("No file selected.\n");
		return;
	}

	SDL_Log("Selected file: %s\n", *filelist);

	AppState* as = (AppState*)userdata;
	if (!as->supaNES->InsertROM(*filelist)) {
		SDL_Log("Fail to load cartridge.\n");
		return;
	}

	SDL_Log("Cartridge loaded.\n");
}
