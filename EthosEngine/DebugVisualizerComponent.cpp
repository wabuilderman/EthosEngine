#include "DebugVisualizerComponent.hpp"
#include "RenderComponentManager.hpp"

DebugVisualizerComponent::DebugVisualizerComponent(Actor& owner) : Component(owner){
	RenderComponentManager::addDebugVisualizerComponent(this);
}

DebugVisualizerComponent::~DebugVisualizerComponent() {
	RenderComponentManager::removeDebugVisualizerComponent(this);
}