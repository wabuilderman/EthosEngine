#pragma once
#include "SteamManager.hpp"
#include <string>
#include "UpdateEvent.hpp"
#include "Message.hpp"
#include "Gamestate.hpp"
#include "NetMessageTypes.hpp"

class Client {

	HSteamNetConnection m_hConnection = 0;
	ISteamNetworkingSockets* m_pInterface = nullptr;

	void PollIncomingMessages() {
		
		ISteamNetworkingMessage* pIncomingMsg = nullptr;
		int numMsgs = 0;
		while (numMsgs = m_pInterface->ReceiveMessagesOnConnection(m_hConnection, &pIncomingMsg, 1) > 0) {
			if (numMsgs < 0)
				throw ("Error checking for messages");
			NetMessageType type = (NetMessageType)((unsigned char*)pIncomingMsg->m_pData)[0];

			switch (type) {
			case NetMessageType::Text: {
				// Just echo anything we get from the server
				fwrite(((char*)pIncomingMsg->m_pData) + 1, 1, pIncomingMsg->m_cbSize, stdout);
				fputc('\n', stdout);
				break;
			}
			case NetMessageType::Gamestate: {
				// Add new gamestate info to be processed
				if ((Gamestate*)pIncomingMsg->m_pData != nullptr) {
					assert(pIncomingMsg->m_cbSize == sizeof(Gamestate));
					Gamestate* gsPointer = ((Gamestate*)pIncomingMsg->m_pData);
					gamestate += gsPointer;
				}
				SteamNetworkingQuickConnectionStatus dumpInfo;
				m_pInterface->GetQuickConnectionStatus(m_hConnection, &dumpInfo);
				break;
			}
			case NetMessageType::PlayerID: {
				playerID = ((unsigned char*)pIncomingMsg->m_pData)[1];
				std::cout << "Player ID established: " << (int)playerID << std::endl;
				break;
			}
			default:
				std::cout << "Unknown Message Recieved." << std::endl;
			}

			// We don't need this anymore.
			pIncomingMsg->Release();
		}
	}

	void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo) {
		//assert(pInfo->m_hConn == m_hConnection || m_hConnection == k_HSteamNetConnection_Invalid);

		// What's the state of the connection?
		switch (pInfo->m_info.m_eState)
		{
		case k_ESteamNetworkingConnectionState_None:
			// NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
			break;

		case k_ESteamNetworkingConnectionState_ClosedByPeer:
		case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
		{
			// Print an appropriate message
			if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting)
			{
				// Note: we could distinguish between a timeout, a rejected connection,
				// or some other transport problem.
				printf("We sought the remote host, yet our efforts were met with defeat.  (%s)", pInfo->m_info.m_szEndDebug);
			}
			else if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
			{
				printf("Alas, troubles beset us; we have lost contact with the host.  (%s)", pInfo->m_info.m_szEndDebug);
			}
			else
			{
				// NOTE: We could check the reason code for a normal disconnection
				printf("The host hath bidden us farewell.  (%s)", pInfo->m_info.m_szEndDebug);
			}

			// Clean up the connection.  This is important!
			// The connection is "closed" in the network sense, but
			// it has not been destroyed.  We must close it on our end, too
			// to finish up.  The reason information do not matter in this case,
			// and we cannot linger because it's already closed on the other end,
			// so we just pass 0's.
			m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
			m_hConnection = k_HSteamNetConnection_Invalid;

			Disconnect();

			break;
		}

		case k_ESteamNetworkingConnectionState_Connecting:
			// We will get this callback when we start connecting.
			// We can ignore this.
			break;

		case k_ESteamNetworkingConnectionState_Connected:
			printf("Connected to server OK");
			break;

		default:
			// Silences -Wswitch
			break;
		}
	}

	static Client* s_pCallbackInstance;
	static void SteamNetConnectionStatusChangedCallback(
		SteamNetConnectionStatusChangedCallback_t* pInfo) {
		s_pCallbackInstance->OnSteamNetConnectionStatusChanged(pInfo);
	}

	void PollConnectionStateChanges() {
		s_pCallbackInstance = this;
		m_pInterface->RunCallbacks();
	}

public:
	Gamestate gamestate;
	unsigned char playerID = 0;
	bool isConnected = false;

	void update(UpdateEvent& e) {
		PollIncomingMessages();
		PollConnectionStateChanges();
	}

	void SendActorUpdate(const ActorUpdateData& data) {
		m_pInterface->SendMessageToConnection(m_hConnection, &data, sizeof(ActorUpdateData), k_nSteamNetworkingSend_Reliable, nullptr);
	}

	void SendChatMessage(std::string msg) {
		std::string taggedMessage = (char)(NetMessageType::Text) + msg;
		m_pInterface->SendMessageToConnection(m_hConnection, taggedMessage.c_str(), 
			(uint32)taggedMessage.length(), k_nSteamNetworkingSend_Reliable, nullptr);
	}

	void Connect(const SteamNetworkingIPAddr& serverAddr) {
		// Select instance to use.  For now we'll always use the default.
		m_pInterface = SteamNetworkingSockets();

		// Start connecting
		char szAddr[SteamNetworkingIPAddr::k_cchMaxString];
		serverAddr.ToString(szAddr, sizeof(szAddr), true);

		printf("Connecting to chat server at %s", szAddr);
		SteamNetworkingConfigValue_t opt;
		opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);
		m_hConnection = m_pInterface->ConnectByIPAddress(serverAddr, 1, &opt);
		if (m_hConnection == k_HSteamNetConnection_Invalid)
			throw ("Failed to create connection");
		isConnected = true;
		Message<UpdateEvent>::Connect(&Client::update, this);
	}

	void Disconnect() {
		isConnected = false;
		Message<UpdateEvent>::Disconnect(&Client::update, this);
	}
};