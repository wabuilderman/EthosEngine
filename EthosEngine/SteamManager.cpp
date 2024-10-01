#include "SteamManager.hpp"

#ifdef USE_STEAM_API
#include <iostream>
#include <stdlib.h>

#include "Message.hpp"

#if _WIN32
#include <Windows.h>
#define DebugMessage(msg) OutputDebugStringA(msg)
#else
#define DebugMessage(msg) std::cerr << msg << std::endl
#endif

extern "C" void __cdecl SteamAPIDebugTextHook(int nSeverity, const char *pchDebugText) {
	DebugMessage(pchDebugText);
	// Manual Breakpoint for API Errors
	if (nSeverity >= 1) {
		int x = 3;
		(void)x;
	}
}

InputHandle_t controllerHandles[STEAM_CONTROLLER_MAX_COUNT];

const char * SteamManager::getInputTypeName(ESteamInputType type) {
	switch (type) {
		case ESteamInputType::k_ESteamInputType_Unknown:
			return "Unknown Controller";
		case ESteamInputType::k_ESteamInputType_SteamController:
			return "Steam Controller";
		case ESteamInputType::k_ESteamInputType_XBox360Controller:
			return "Xbox 360 Controller";
		case ESteamInputType::k_ESteamInputType_XBoxOneController:
			return "Xbox One Controller";
		case ESteamInputType::k_ESteamInputType_GenericGamepad:
			return "Generic Gamepad";
		case ESteamInputType::k_ESteamInputType_PS4Controller:
			return "PS4 Controller";
		case ESteamInputType::k_ESteamInputType_AppleMFiController:
			return "Apple MFi Controller";
		case ESteamInputType::k_ESteamInputType_AndroidController:
			return "Android Controller";
		case ESteamInputType::k_ESteamInputType_SwitchJoyConPair:
			return "Switch Joycon Pair";
		case ESteamInputType::k_ESteamInputType_SwitchJoyConSingle:
			return "Switch JoyCon Single";
		case ESteamInputType::k_ESteamInputType_SwitchProController:
			return "Switch Pro Controler";
		case ESteamInputType::k_ESteamInputType_MobileTouch:
			return "Mobile Touch";
		case ESteamInputType::k_ESteamInputType_PS3Controller:
			return "PS3 Controller";
		default:
			return "Seriously Unknown Controller";
	}
}

void onControllerConnect(SteamManager::ControllerConnectEvent& e) {
	std::cout << "Controller Connected!" << std::endl;
	std::cout << "Controller Type: " << SteamManager::getInputTypeName(SteamInput()->GetInputTypeForHandle(controllerHandles[e.controllerHandleIndex])) << std::endl;
}

void SteamManager::init() {
	if (SteamAPI_RestartAppIfNecessary(0)) /* Replace with your App ID */ {
		exit(EXIT_SUCCESS);
	}

	if (!SteamAPI_Init()) {
		DebugMessage("Fatal Error - Steam must be running to play this game (SteamAPI_Init() failed).");
		exit(EXIT_FAILURE);
	}

	SteamClient()->SetWarningMessageHook(&SteamAPIDebugTextHook);

	if (!SteamUser()->BLoggedOn()) {
		DebugMessage("Fatal Error - Steam user must be logged in to play this game (SteamUser()->BLoggedOn() returned false).");
		exit(EXIT_FAILURE);
	}

	if (!SteamInput()->Init()) {
		DebugMessage("Fatal Error - SteamInput()->Init failed.");
		exit(EXIT_FAILURE);
	}
	DebugMessage("Initialized Steam API");

	Message<SteamManager::ControllerConnectEvent>::Connect(&onControllerConnect);
}

/*
InputHandle_t controllerHandles[STEAM_CONTROLLER_MAX_COUNT];

void onControllerConnected() {
	int count = SteamInput()->GetConnectedControllers(controllerHandles);

	ESteamInputType inputType = SteamInput()->GetInputTypeForHandle(controllerHandles[0]);
	switch (inputType) {
	case k_ESteamInputType_Unknown:
		printf("unknown\n!"); break;
	case k_ESteamInputType_SteamController:
		printf("Steam controller\n!"); break;
	case k_ESteamInputType_XBox360Controller:
		printf("XBox 360 controller\n!"); break;
	case k_ESteamInputType_XBoxOneController:
		printf("XBox One controller\n!"); break;
	case k_ESteamInputType_GenericGamepad:
		printf("Generic XInput\n!"); break;
	case k_ESteamInputType_PS4Controller:
		printf("PS4 controller\n!"); break;
	}
}*/



void queryControllerConnections() {
	static size_t numControllers = 0;
	size_t count = SteamInput()->GetConnectedControllers(controllerHandles);
	if (numControllers != count) {
		if (numControllers < count) {
			SteamManager::ControllerConnectEvent e = { count - 1 };
			Message<SteamManager::ControllerConnectEvent>::Send(e);
		}
		else {
			SteamManager::ControllerDisconnectEvent e = { numControllers - 1 };
			Message<SteamManager::ControllerDisconnectEvent>::Send(e);
		}
		numControllers = count;
	}
}

void SteamManager::update() {
	SteamAPI_RunCallbacks();
	if constexpr (SteamManager::useSteamInput) {
		queryControllerConnections();
	}
}

void SteamManager::shutdown() {
	SteamInput()->Shutdown();
	SteamAPI_Shutdown();
}

#endif