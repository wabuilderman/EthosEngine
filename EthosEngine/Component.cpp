#include "Component.hpp"
#include "Actor.hpp"

Component::Component(Actor& owner) : owner(owner) {}
Component::~Component(){}
