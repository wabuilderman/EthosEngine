#pragma once
#include "UITextbox.hpp"
#include "SteamManager.hpp"

class IPentryTextbox : public UITextbox {
public:
	SteamNetworkingIPAddr address;

	IPentryTextbox(Actor& owner, UIComponent* parent, float width, float height);

	void onEngage() override;
	void onDisengage() override;
};