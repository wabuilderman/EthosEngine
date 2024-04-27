#pragma once

class Actor;

class Component {
public:
	Actor& owner;

	Component(Actor& owner);
	virtual ~Component() = 0;
};