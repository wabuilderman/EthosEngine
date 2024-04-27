#pragma once
#define USE_STEAM_API

#ifdef USE_STEAM_API
#pragma warning(disable : 26812)
#include "steam/isteamnetworkingsockets.h"
#include "steam/isteamnetworkingutils.h"
#include "steam/steam_api.h"
#include "steam/isteamuser.h"
#include "steam/isteaminput.h"
#include "steam/isteamcontroller.h"
#pragma warning(default : 26812)
#endif

class SteamManager {
public:
#ifdef USE_STEAM_API
	static const char * getInputTypeName(ESteamInputType type);
	static constexpr bool useSteamInput = true;

	struct ControllerConnectEvent {
		size_t controllerHandleIndex;
	};

	struct ControllerDisconnectEvent {
		size_t controllerHandleIndex;
	};


	static void init();
	static void update();
	static void shutdown();
#else
	static inline void init() {}
	static inline void update() {}
	static inline void shutdown() {}
#endif

};