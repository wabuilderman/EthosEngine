#include "VehiclePathTargetHandle.hpp"
#include "MyControls.hpp"
#include "VehiclePath.hpp"

void VehiclePathTargetHandle::OnDrag(const glm::vec3& rayOrigin, const glm::vec3& rayDirection) {
	glm::vec3 mouseWorldPos = ObjectSelector::castMouseOntoPlane(Vector::up * 0.3f, Vector::up);
	if (!isnan(mouseWorldPos.x)) {
		teleport(mouseWorldPos + Vector::up * nodeHeight);

		VehiclePath* vehiclePath = ((VehiclePath*)ActorManager::getActorByID(vehiclePathID));

		if (isStartHandle) {
			vehiclePath->startPos = mouseWorldPos + Vector::up * nodeHeight;
		}
		else {
			vehiclePath->endPos = mouseWorldPos + Vector::up * nodeHeight;
		}

		// Update vehicle path
		vehiclePath->LazyUpdatePath();
	}
}

