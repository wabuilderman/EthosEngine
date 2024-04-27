#pragma once
#include "PhsyicsObjectStateUpdate.hpp"
#include "InputRecord.hpp"
class SimulationPacket {
private:
	static constexpr unsigned maxPhysObjStateUpdates = 16;
	static constexpr unsigned maxInputRecords = 16;

public:
	unsigned sequence;
	InputRecord inputRecords[maxInputRecords];

	unsigned numPhysObjStateUpdates;
	PhysicObjectStateUpdate physObjStateUpdates[maxPhysObjStateUpdates];
};