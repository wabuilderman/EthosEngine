#include "SelectableActor.hpp"
#include "MyControls.hpp"
SelectableActor::SelectableActor() {
	MyControls::getControls().objectSelector.selectableActors.insert(actorID);
}

SelectableActor::~SelectableActor() {
	MyControls::getControls().objectSelector.selectableActors.erase(actorID);
}