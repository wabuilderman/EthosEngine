#pragma once
#include "Host.hpp"
#include "Client.hpp"
#include "Gamestate.hpp"

class NetworkManager {
private:
	static constexpr unsigned int DEFAULT_PORT = 8003;
	static NetworkManager& getInst() {
		static NetworkManager instance;
		return instance;
	}

public:
	Host localHost;
	Client localClient;
	unsigned int port = DEFAULT_PORT;

	static void hostGame(unsigned int port) {
		if (!getInst().localHost.isActive) {
			getInst().localHost.Launch(port);
			SteamNetworkingIPAddr hostAddr;
			hostAddr.Clear();
			hostAddr.SetIPv6LocalHost(port);

			char buf[80];
			hostAddr.ToString(&buf[0], 80, true);
			std::cout << "Hosting game on " << buf << std::endl;

			getInst().localClient.Connect(hostAddr);
		}
	}

	static void joinGame(SteamNetworkingIPAddr address) {
		//SteamNetworkingIPAddr hostAddr;
		//hostAddr.Clear();
		//hostAddr.SetIPv6LocalHost(hostPort);
		getInst().localClient.Connect(address);
	}

	static unsigned getPlayerID() {
		return getInst().localClient.playerID;
	}

	static Gamestate& getCurrentGamestate() {
		return getInst().localClient.gamestate;
	}

	static void updateActor(const ActorUpdateData& data) {
		if (getInst().localClient.isConnected) {
			getInst().localClient.SendActorUpdate(data);
		}
	}

	static void sendChatMessage(std::string str) {
		getInst().localClient.SendChatMessage(str);
	}

	NetworkManager() {}
	~NetworkManager() {}
};