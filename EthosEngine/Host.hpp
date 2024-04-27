#pragma once
#include "SteamManager.hpp"
#include <string>
#include <map>
#include "UpdateEvent.hpp"
#include "Message.hpp"
#include "Gamestate.hpp"

class Host {
private:
	HSteamListenSocket m_hListenSock = 0;
	HSteamNetPollGroup m_hPollGroup = 0;
	ISteamNetworkingSockets* m_pInterface = nullptr;

	struct Client_t {
		std::string m_sNick;
		unsigned char playerID = 0;
	};

	std::map< HSteamNetConnection, Client_t > m_mapClients;

	void SendStringToAllClients(const char* str,
		HSteamNetConnection except = k_HSteamNetConnection_Invalid) {
		static char temp[1024];
		sprintf_s(temp, "%c%s", ((char)NetMessageType::Text), str);
		for (auto& c : m_mapClients) {
			if (c.first != except)
				m_pInterface->SendMessageToConnection(c.first, temp, (uint32)strlen(temp),
					k_nSteamNetworkingSend_Reliable, nullptr);
		}
	}

	void PollIncomingMessages() {
		static char temp[1024];

		ISteamNetworkingMessage* pIncomingMsg = nullptr;
		int numMsgs;
		while (numMsgs = m_pInterface->ReceiveMessagesOnPollGroup(m_hPollGroup, &pIncomingMsg, 1) > 0) {
			if (numMsgs < 0)
				throw ("Error checking for messages");

			//assert(numMsgs == 1 && pIncomingMsg);
			auto itClient = m_mapClients.find(pIncomingMsg->m_conn);

			//assert(itClient != m_mapClients.end());

			if (((unsigned char*)pIncomingMsg->m_pData)[0] == (unsigned char)NetMessageType::ActorUpdate) {
				ActorUpdateData data = *((ActorUpdateData*)pIncomingMsg->m_pData);
				gamestate.addUpdate(data);

			}
			else {
				// '\0'-terminate it to make it easier to parse
				//std::string sCmd;
				//sCmd.assign((const char*)pIncomingMsg->m_pData, pIncomingMsg->m_cbSize);
				//const char* cmd = sCmd.c_str();
			}
			// We don't need this anymore.
			pIncomingMsg->Release();
		}
		

		// Check for known commands.  None of this example code is secure or robust.
		// Don't write a real server like this, please.
		/*
		if (strncmp(cmd, "/nick", 5) == 0) {
			const char* nick = cmd + 5;
			while (isspace(*nick))
				++nick;

			// Let everybody else know they changed their name
			sprintf_s(temp, "%s shall henceforth be known as %s", itClient->second.m_sNick.c_str(), nick);
			SendStringToAllClients(temp, itClient->first);

			// Respond to client
			//sprintf_s(temp, "%cYe shall henceforth be known as %s", ((char)NetMessageType::Text), nick);
			//SendStringToClient(itClient->first, temp);

			// Actually change their name
			SetClientNick(itClient->first, nick);

		}
		else {
			// Assume it's just a ordinary chat message, dispatch to everybody else
			sprintf_s(temp, "%s: %s", itClient->second.m_sNick.c_str(), cmd);
			SendStringToAllClients(temp, itClient->first);
		}*/
	}

	void SetClientNick(HSteamNetConnection hConn, const char* nick) {
		// Remember their nick
		m_mapClients[hConn].m_sNick = nick;

		// Set the connection name, too, which is useful for debugging
		m_pInterface->SetConnectionName(hConn, nick);
	}

	void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo) {
		char temp[1024];

		// What's the state of the connection?
		switch (pInfo->m_info.m_eState) {
		case k_ESteamNetworkingConnectionState_None:
			// NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
			break;

		case k_ESteamNetworkingConnectionState_ClosedByPeer:
		case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
		{
			// Ignore if they were not previously connected.  (If they disconnected
			// before we accepted the connection.)
			if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connected)
			{

				// Locate the client.  Note that it should have been found, because this
				// is the only codepath where we remove clients (except on shutdown),
				// and connection change callbacks are dispatched in queue order.
				auto itClient = m_mapClients.find(pInfo->m_hConn);
				//assert(itClient != m_mapClients.end());

				// Select appropriate log messages
				const char* pszDebugLogAction;
				if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
				{
					pszDebugLogAction = "problem detected locally";
					sprintf_s(temp, "Alas, %s hath fallen into shadow.  (%s)", itClient->second.m_sNick.c_str(), pInfo->m_info.m_szEndDebug);
				}
				else
				{
					// Note that here we could check the reason code to see if
					// it was a "usual" connection or an "unusual" one.
					pszDebugLogAction = "closed by peer";
					sprintf_s(temp, "%s hath departed", itClient->second.m_sNick.c_str());
				}

				// Spew something to our own log.  Note that because we put their nick
				// as the connection description, it will show up, along with their
				// transport-specific data (e.g. their IP address)
				printf("Connection %s %s, reason %d: %s\n",
					pInfo->m_info.m_szConnectionDescription,
					pszDebugLogAction,
					pInfo->m_info.m_eEndReason,
					pInfo->m_info.m_szEndDebug
				);

				m_mapClients.erase(itClient);

				// Send a message so everybody else knows what happened
				SendStringToAllClients(temp);
			}
			else
			{
				//assert(pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting);
			}

			// Clean up the connection.  This is important!
			// The connection is "closed" in the network sense, but
			// it has not been destroyed.  We must close it on our end, too
			// to finish up.  The reason information do not matter in this case,
			// and we cannot linger because it's already closed on the other end,
			// so we just pass 0's.
			m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
			break;
		}

		case k_ESteamNetworkingConnectionState_Connecting:
		{
			// This must be a new connection
			//assert(m_mapClients.find(pInfo->m_hConn) == m_mapClients.end());

			printf("Connection request from %s", pInfo->m_info.m_szConnectionDescription);

			// A client is attempting to connect
			// Try to accept the connection.
			if (m_pInterface->AcceptConnection(pInfo->m_hConn) != k_EResultOK)
			{
				// This could fail.  If the remote host tried to connect, but then
				// disconnected, the connection may already be half closed.  Just
				// destroy whatever we have on our side.
				m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
				printf("Can't accept connection.  (It was already closed?)");
				break;
			}

			// Assign the poll group
			if (!m_pInterface->SetConnectionPollGroup(pInfo->m_hConn, m_hPollGroup))
			{
				m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
				printf("Failed to set poll group?");
				break;
			}

			// Generate a random nick.  A random temporary nick
			// is really dumb and not how you would write a real chat server.
			// You would want them to have some sort of signon message,
			// and you would keep their client in a state of limbo (connected,
			// but not logged on) until them.  I'm trying to keep this example
			// code really simple.
			char nick[64];
			sprintf_s(nick, "%cBraveWarrior%d\0", ((char)NetMessageType::Text), 10000 + (rand() % 100000));

			// Send them a welcome message
			//sprintf_s(temp, "Welcome, stranger.  Thou art known to us for now as '%s'; upon thine command '/nick' we shall know thee otherwise.", nick);
			//SendStringToClient(pInfo->m_hConn, temp);
			m_pInterface->SendMessageToConnection(pInfo->m_hConn, nick, (uint32)64,
				k_nSteamNetworkingSend_Reliable, nullptr);
			

			unsigned char playerIDmsg[2];
			playerIDmsg[0] = (unsigned char) NetMessageType::PlayerID;
			if (m_mapClients.empty()) {
				playerIDmsg[1] = 0;
			}
			else {
				assert(m_mapClients.size() < 0xFFull);
				playerIDmsg[1] = (unsigned char) m_mapClients.size();
			}
			
			m_pInterface->SendMessageToConnection(pInfo->m_hConn, playerIDmsg, (uint32)2,
				k_nSteamNetworkingSend_Reliable, nullptr);
			

			// Also send them a list of everybody who is already connected
			if (m_mapClients.empty())
			{
				SendStringToAllClients("Thou art utterly alone.");
			}
			else
			{
				sprintf_s(temp, "%d companions greet you:", (int)m_mapClients.size());
				for (auto& c : m_mapClients)
					SendStringToAllClients(c.second.m_sNick.c_str());
				//pInfo->m_hConn is local client?
			}

			// Let everybody else know who they are for now
			sprintf_s(temp, "Hark!  A stranger hath joined this merry host.  For now we shall call them '%s'\0", nick + 1);
			SendStringToAllClients(temp, pInfo->m_hConn);

			// Add them to the client list, using std::map wacky syntax
			m_mapClients[pInfo->m_hConn];
			SetClientNick(pInfo->m_hConn, nick);
			m_mapClients[pInfo->m_hConn].playerID = playerIDmsg[1];
			break;
		}

		case k_ESteamNetworkingConnectionState_Connected:
			// We will get a callback immediately after accepting the connection.
			// Since we are the server, we can ignore this, it's not news to us.
			break;

		default:
			// Silences -Wswitch
			break;
		}
	}

	static Host* s_pCallbackInstance;
	static void SteamNetConnectionStatusChangedCallback(
		SteamNetConnectionStatusChangedCallback_t* pInfo) {
		s_pCallbackInstance->OnSteamNetConnectionStatusChanged(pInfo);
	}

	void PollConnectionStateChanges() {
		s_pCallbackInstance = this;
		m_pInterface->RunCallbacks();
	}

	void update(UpdateEvent& e) {
		PollIncomingMessages();
		PollConnectionStateChanges();

		static float acc = 0.f;
		acc += e.dt;
		if (acc > 0.01f) {
			BroadcastGamestate();
			acc = 0.f;
		}
		
	}

public:
	SteamNetworkingIPAddr serverLocalAddr = { 0 };
	bool isActive = false;
	Gamestate gamestate;

	void BroadcastGamestate() {
		for (auto& c : m_mapClients) {
			if (c.first != k_HSteamNetConnection_Invalid) {
				m_pInterface->SendMessageToConnection(c.first, &gamestate, sizeof(Gamestate),
					k_nSteamNetworkingSend_UnreliableNoDelay, nullptr);
			}
		}
	}

	bool Launch(unsigned int nPort) {
		// Select instance to use.  For now we'll always use the default.
		// But we could use SteamGameServerNetworkingSockets() on Steam.
		m_pInterface = SteamNetworkingSockets();

		// Start listening
		serverLocalAddr.Clear();
		serverLocalAddr.m_port = nPort;

		SteamNetworkingConfigValue_t opt;
		opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
			(void*)SteamNetConnectionStatusChangedCallback);

		m_hListenSock = m_pInterface->CreateListenSocketIP(serverLocalAddr, 1, &opt);
		if (m_hListenSock == k_HSteamListenSocket_Invalid)
			return false;

		m_hPollGroup = m_pInterface->CreatePollGroup();
		if (m_hPollGroup == k_HSteamNetPollGroup_Invalid)
			return false;

		printf("Server listening on port %d\n", nPort);

		Message<UpdateEvent>::Connect(&Host::update, this);
		return true;
	}

	void Shutdown() {
		Message<UpdateEvent>::Disconnect(&Host::update, this);

		// Close all the connections
		printf("Closing connections...\n");
		for (auto it : m_mapClients)
		{
			// Send them one more goodbye message.  Note that we also have the
			// connection close reason as a place to send final data.  However,
			// that's usually best left for more diagnostic/debug text not actual
			// protocol strings.
			//SendStringToClient(it.first, "Server is shutting down.  Goodbye.");

			// Close the connection.  We use "linger mode" to ask SteamNetworkingSockets
			// to flush this out and close gracefully.
			m_pInterface->CloseConnection(it.first, 0, "Server Shutdown", true);
		}
		m_mapClients.clear();

		m_pInterface->CloseListenSocket(m_hListenSock);
		m_hListenSock = k_HSteamListenSocket_Invalid;

		m_pInterface->DestroyPollGroup(m_hPollGroup);
		m_hPollGroup = k_HSteamNetPollGroup_Invalid;
	}
};
