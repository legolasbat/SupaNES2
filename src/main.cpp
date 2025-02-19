#include <cstdint>
#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>
#include <SDL3/SDL.h>
#include <stdint.h>
#include <stdio.h>

#include "SupaNES.h"

typedef struct {
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* texture;
	SupaNES* supaNES;
} AppState;

// Main loop bool
bool done = false;

// Background Display Settings
bool BG1Selected = true;
bool BG2Selected = true;
bool BG3Selected = true;
bool BG4Selected = true;

void SDLCALL openFileCallback(void* userdata, const char* const* filelist, int filter);
void HandleEvents(AppState* appstate);
void MenuBar(AppState* appstate);

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

	if(!SDL_CreateWindowAndRenderer("SupaNES2", 800, 600, 0, &as->window, &as->renderer)) {
		SDL_Log("Couldn't craete window and renderer: %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}
	as->texture = SDL_CreateTexture(as->renderer, SDL_PIXELFORMAT_ABGR1555, SDL_TEXTUREACCESS_STREAMING, 256, 224);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui::StyleColorsDark();

	ImGui_ImplSDL3_InitForSDLRenderer(as->window, as->renderer);
	ImGui_ImplSDLRenderer3_Init(as->renderer);

	as->supaNES = new SupaNES();
	int cycles = 0;

	char windowTitle[128];
	uint64_t deltaTime = 0;
	uint64_t lastFrame = 0;
	int fps = 60;
	float desiredDelta = 1000.0f / fps;

	// Main Loop
	while(!done) {
		uint64_t currentTime = SDL_GetTicks();
		
		// SupaNes clock
		if(as->supaNES->RomLoaded) {
			cycles += as->supaNES->Clock();

			if (as->supaNES->ppu->frameCompleted) {
			
				HandleEvents(as);
		
				// Start frame
				ImGui_ImplSDLRenderer3_NewFrame();
				ImGui_ImplSDL3_NewFrame();
				ImGui::NewFrame();
		
				MenuBar(as);

				ImGui::Render();
		
				// Render
				SDL_SetRenderDrawColor(as->renderer, 0, 0, 0, 255);
				SDL_RenderClear(as->renderer);
				void* texturePixels;
				int pitch;
				SDL_LockTexture(as->texture, NULL, &texturePixels, &pitch);
				as->supaNES->ppu->SetFrame(texturePixels, pitch);
				SDL_UnlockTexture(as->texture);
				//SDL_UpdateTexture(as->texture, NULL, as->supaNES->ppu->GetFrame(), 512);
				SDL_RenderTexture(as->renderer, as->texture, NULL, NULL);
				ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), as->renderer);
				SDL_RenderPresent(as->renderer);
				
				deltaTime = currentTime - lastFrame;
				lastFrame = currentTime;
				snprintf(windowTitle, 128, "SupaNES2 | %.0f", 1000.0f / deltaTime);
				SDL_SetWindowTitle(as->window, windowTitle);
			}
		}
		else {
			HandleEvents(as);
		
			// Start frame
			ImGui_ImplSDLRenderer3_NewFrame();
			ImGui_ImplSDL3_NewFrame();
			ImGui::NewFrame();
		
			MenuBar(as);

			ImGui::Render();
		
			// Render
			SDL_SetRenderDrawColor(as->renderer, 255, 0, 0, 255);
			SDL_RenderClear(as->renderer);
			ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), as->renderer);
			SDL_RenderPresent(as->renderer);
			deltaTime = currentTime - lastFrame;
			lastFrame = currentTime;
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

void HandleEvents(AppState* appstate) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		ImGui_ImplSDL3_ProcessEvent(&event);
		if (event.type == SDL_EVENT_KEY_DOWN) {
			switch(event.key.scancode) {
				case SDL_SCANCODE_Q:
					done = true;
					break;

				case SDL_SCANCODE_T:
					const char* testFile = "/home/legolasbat/Documentos/C/SupaNES2/Tests/CPU/ADC-Passed/CPUADC.sfc";
					appstate->supaNES->InsertROM(testFile);
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
				}
				if (ImGui::MenuItem("BG2", 0, BG2Selected, true)) {
					BG2Selected = !BG2Selected;
				}
				if (ImGui::MenuItem("BG3", 0, BG3Selected, true)) {
					BG3Selected = !BG3Selected;
				}
				if (ImGui::MenuItem("BG4", 0, BG4Selected, true)) {
					BG4Selected = !BG4Selected;
				}
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Log CPU")) {
				// TODO: Display Log
			}
			if (ImGui::MenuItem("Log APU")) {
				// TODO: Display Log
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
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
