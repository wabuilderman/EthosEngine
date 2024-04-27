#include <Windows.h>
#include "pch.hpp"

#include <iostream>
#include <stdexcept>
#include <chrono>

#include "Engine.hpp"
#include "SteamManager.hpp"
#include "FontManager.hpp"
#include "InputManager.hpp"
#include "MyControls.hpp"
#include "AssetManager.hpp"
#include "MyMenuScene.hpp"
#include "MyTestScene.hpp"
#include "Collision.hpp"
#include "UpdateEvent.hpp"

constexpr unsigned numExtraClients = 0;

void printFramerate(float dt) {
	std::cout << "Frametime: " << dt << " | FPS: " << 1.0f / dt << std::endl;
}

void trackFramerate(UpdateEvent& e) {
	constexpr float printInterval = 1.0f;

	static float time = 0.0f;
	static unsigned count = 0;

	time += e.dt;
	count++;

	if (time > printInterval) {

		float averageDt = time / count;
		//time = fmod(time, printInterval);
		time = 0.0f;
		count = 0;
		//printFramerate(averageDt);
	}
}

class Application {
	bool shouldClose = false;
	MyTestScene* testScene = nullptr;

	void handleEsacpe(ExitProgram& e) {
		shouldClose = true;
	}

	void onOpenConsole(OpenConsole& e) {
		HWND consoleHwnd = GetConsoleWindow();
		if (consoleHwnd) {
			SetForegroundWindow(consoleHwnd);
			SetActiveWindow(consoleHwnd);
		}
	}

public:

	void rebuildAssetFile() {
		std::vector<AssetManager::AssetFileEntry> assets;
		AssetManager::AssetFileEntry modelAsset;
		modelAsset.type = AssetManager::AssetFileEntry::AssetType::Model;

		AssetManager::AssetFileEntry::ModelDescription& modelInfo =
			std::get<AssetManager::AssetFileEntry::ModelDescription>(modelAsset.info);

		modelInfo.name = "MyTestModel";
		modelInfo.mesh.type = AssetManager::AssetFileEntry::MeshDescription::MeshType::Default;
		modelInfo.mesh.value = "DefaultMesh";
		modelInfo.texture.type = AssetManager::AssetFileEntry::TextureDescription::TextureType::Default;
		modelInfo.texture.value = "textures/texture.jpg";

		assets.push_back(modelAsset);
		assets.push_back(modelAsset);
		AssetManager::saveAssetFile("assets/myAssetFile.gameasset", assets);
	}

	Application() {
		Engine::init();
		MyControls::getControls();

		//InputManager::lockMouse();
		Message<ExitProgram>::Connect(&Application::handleEsacpe, this);
		Message<OpenConsole>::Connect(&Application::onOpenConsole, this);
		Message<UpdateEvent>::Connect(trackFramerate);
		
		AssetManager::loadAssetFile("assets/myAssetFile.gameasset");

		testScene = new MyTestScene();
	}

	void launch() {
		while (!Engine::shouldClose() && !shouldClose) {
			Engine::update();
		}
	}

	~Application() {
		Engine::rest();
		delete testScene;
		Engine::shutdown();
	}
};



void testGJK() {
	GJK::runUnitTest();
	std::cout << "All GJK tests completed successfully." << std::endl;
}

int main(int argc, char** argv) {
	if (CoInitialize(0) != S_OK) {
		exit(EXIT_FAILURE);
	}
	
	testGJK();
	
	if (argc > 1) {
		Application* app = new Application();
		std::string command = std::string(argv[1]);

		if (command.compare("-host") == 0) {
			assert(argc > 2);
			unsigned port = atoi(argv[2]);

			Engine::window->setWindowName(
				(std::string("Ethos Engine - Host - ")
					+ std::to_string(port)).c_str());
			NetworkManager::hostGame(port);
		}
		else if (command.compare("-client") == 0) {
			assert(argc > 2);
			Engine::window->setWindowName(
				(std::string("Ethos Engine - Client - ")
					+ std::string(argv[2])).c_str());

			SteamNetworkingIPAddr address;
			address.Clear();
			address.ParseString(argv[2]);
			NetworkManager::joinGame(address);
		}
		
		app->launch();
		delete app;
		return EXIT_SUCCESS;
	}
	else {
		constexpr unsigned port = 8001;
		Application* app = new Application();
		Engine::window->setWindowName(
			(std::string("Ethos Engine - Main Client - ")
				+ std::to_string(port)).c_str()
		);
		NetworkManager::hostGame(port);

		std::vector<PROCESS_INFORMATION> pis;
		if constexpr (numExtraClients > 0) {
			std::wstring commandw;
			MultiByteToWideChar(CP_UTF8, 0, argv[0], (int)strlen(argv[0]), &commandw[0], (int)commandw.size());
			commandw += L" -client [0::1]:" + std::to_wstring(port);

			STARTUPINFO si;
			ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);
			#pragma warning(suppress : 6294)
			for (unsigned i = 0; i < numExtraClients; ++i) {
				pis.push_back(PROCESS_INFORMATION());
				ZeroMemory(&pis.back(), sizeof(pis.back()));
				if (!CreateProcess(0L, const_cast<wchar_t*>(commandw.c_str()),
					0, 0, false, NORMAL_PRIORITY_CLASS, 0, 0, &si, &pis[i]))
				{
					printf("CreateProcess failed (%d).\n", GetLastError());
					return EXIT_FAILURE;
				}
			}
		}

		app->launch();
		delete app;

		if constexpr (numExtraClients > 0) {
			#pragma warning(suppress : 6294)
			for (unsigned i = 0; i < numExtraClients; ++i) {
				// Wait until child process exits.
				WaitForSingleObject(pis[i].hProcess, INFINITE);

				// Close process and thread handles. 
				CloseHandle(pis[i].hProcess);
				CloseHandle(pis[i].hThread);
			}
		}
		return EXIT_SUCCESS;
	}
}