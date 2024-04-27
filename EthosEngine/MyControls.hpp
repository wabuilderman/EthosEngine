#pragma once
#include "InputManager.hpp"
#include "Message.hpp"
#include "UpdateEvent.hpp"
#include "PhysicsManager.hpp"
#include "CameraManager.hpp"
#include "SelectableActor.hpp"
#include <set>
#include <limits>

static constexpr float getLargerAxis(float a, float b) {
	return (abs(a) > abs(b)) ? a : b;
}

struct CameraControl {
	// Time elapsed since last update
	float dt = 1.0f;

	// Degrees of Rotation

	float yawAxisMouse = 0.0f;
	float pitchAxisMouse = 0.0f;

	float yawAxisJoystick = 0.0f;
	float pitchAxisJoystick = 0.0f;

	float yawAxis = 0.0f;
	float pitchAxis = 0.0f;

	static constexpr float maxVel = 1.5f;
	static constexpr float accel = 10.0f;
	static constexpr float baseVel = 0.1f;

	float xVel = baseVel;
	float yVel = baseVel;

	//-------------------------------------------------------------------------
	void mouseMoveCallback(MouseMovementEvent& mouse_raw) {
		static constexpr float sensitivity = 0.003f;

		if (InputManager::isMouseLocked) {
			yawAxisMouse = (mouse_raw.dX * sensitivity) / dt;
			pitchAxisMouse = (-mouse_raw.dY * sensitivity) / dt;
		}

		
	}

	void keyPressCallback(KeyPress& e) {

		if (e.keycode == GLFW_KEY_P) {
			if (PhysicsManager::getIsPaused()) {
				PhysicsManager::setPause(false);
			}
			else {
				PhysicsManager::setPause(true);
			}
		}

		if (e.keycode == GLFW_KEY_L) {
			if (InputManager::isMouseLocked) {
				InputManager::unlockMouse();
			}
			else {
				InputManager::lockMouse();
			}
		}
	}



	void mouseHoldCallback(MouseHoldEvent& e) {
		if (e.buttonID == GLFW_MOUSE_BUTTON_MIDDLE) {
			InputManager::lockMouse();
		}
	}

	void mouseReleaseCallback(MouseReleaseEvent& e) {
		if (e.buttonID == GLFW_MOUSE_BUTTON_MIDDLE) {
			InputManager::unlockMouse();
		}
	}

	void controllerCallback(GamepadUpdateEvent& e) {
		// Set Axis Info
		yawAxisJoystick = e.state->right_stick.xAxis * xVel;
		pitchAxisJoystick = e.state->right_stick.yAxis * yVel;

		// Increase Rotational Velocity
		xVel += abs(e.state->right_stick.xAxis) * dt * accel;
		yVel += abs(e.state->right_stick.yAxis) * dt * accel;

		// Dead-stop on axis release
		if (e.state->right_stick.xAxis == 0) { xVel = baseVel; }
		if (e.state->right_stick.yAxis == 0) { yVel = baseVel; }

		// Max Acceleration Clamp
		xVel = (xVel < maxVel) ? xVel : maxVel;
		yVel = (yVel < maxVel) ? yVel : maxVel;
	}

	void onUpdate(UpdateEvent& update) {
		dt = update.dt;

		yawAxis = getLargerAxis(yawAxisMouse, yawAxisJoystick);
		pitchAxis = getLargerAxis(pitchAxisMouse, pitchAxisJoystick);

		yawAxisMouse = 0.0f;
		pitchAxisMouse = 0.0f;

		//if (yawAxis != 0 || pitchAxis != 0) {
			Message<CameraControl>::Send(*this);
		//}
	}

	CameraControl() {
		Message<GamepadUpdateEvent>::Connect(&CameraControl::controllerCallback, this);
		Message<MouseMovementEvent>::Connect(&CameraControl::mouseMoveCallback, this);
		Message<MouseHoldEvent>::Connect(&CameraControl::mouseHoldCallback, this);
		Message<MouseReleaseEvent>::Connect(&CameraControl::mouseReleaseCallback, this);
		Message<KeyPress>::Connect(&CameraControl::keyPressCallback, this);
		Message<UpdateEvent>::Connect(&CameraControl::onUpdate, this);
	}

	~CameraControl() {
		Message<GamepadUpdateEvent>::Disconnect(&CameraControl::controllerCallback, this);
		Message<MouseMovementEvent>::Disconnect(&CameraControl::mouseMoveCallback, this);
		Message<MouseHoldEvent>::Disconnect(&CameraControl::mouseHoldCallback, this);
		Message<MouseReleaseEvent>::Disconnect(&CameraControl::mouseReleaseCallback, this);
		Message<KeyPress>::Disconnect(&CameraControl::keyPressCallback, this);
		Message<UpdateEvent>::Disconnect(&CameraControl::onUpdate, this);
	}
};

struct OpenConsole {
	void onKeypress(KeyPress& e) {
		if (e.keycode == GLFW_KEY_GRAVE_ACCENT)
			Message<OpenConsole>::Send(*this);
	}

	OpenConsole() {
		Message<KeyPress>::Connect(&OpenConsole::onKeypress, this);
	}

	~OpenConsole() {
		Message<KeyPress>::Disconnect(&OpenConsole::onKeypress, this);
	}
};

struct ExitProgram {
	void onEscape(KeyPress& e) {
		if (e.keycode == GLFW_KEY_ESCAPE)
			Message<ExitProgram>::Send(*this);
	}

	ExitProgram() {
		Message<KeyPress>::Connect(&ExitProgram::onEscape, this);
	}

	~ExitProgram() {
		Message<KeyPress>::Disconnect(&ExitProgram::onEscape, this);
	}
};

struct PhysObjMovement {
	// Time elapsed since last update
	float dt = 1.0f;

	// Movement Axes
	float forwardAxis = 0.0f;
	float verticalAxis = 0.0f;
	float sidewaysAxis = 0.0f;

	//-------------------------------------------------------------------------
	void onUpdate(UpdateEvent& update) {
		dt = update.dt;

		if (forwardAxis != 0 || verticalAxis != 0 || sidewaysAxis != 0) {
			Message<PhysObjMovement>::Send(*this);
		}
	}

	void keyPressCallback(KeyPress& key) {
		switch (key.keycode) {
			case GLFW_KEY_KP_8: forwardAxis  += 1.0f; break;
			case GLFW_KEY_KP_2: forwardAxis  -= 1.0f; break;
			case GLFW_KEY_KP_6: sidewaysAxis += 1.0f; break;
			case GLFW_KEY_KP_4: sidewaysAxis -= 1.0f; break;
			case GLFW_KEY_KP_9: verticalAxis += 1.0f; break;
			case GLFW_KEY_KP_7: verticalAxis -= 1.0f; break;
		}
	}

	void keyReleaseCallback(KeyRelease& key) {
		switch (key.keycode) {
			case GLFW_KEY_KP_8: forwardAxis  -= 1.0f; break;
			case GLFW_KEY_KP_2: forwardAxis  += 1.0f; break;
			case GLFW_KEY_KP_6: sidewaysAxis -= 1.0f; break;
			case GLFW_KEY_KP_4: sidewaysAxis += 1.0f; break;
			case GLFW_KEY_KP_9: verticalAxis -= 1.0f; break;
			case GLFW_KEY_KP_7: verticalAxis += 1.0f; break;
		}
	}

	//-------------------------------------------------------------------------

	PhysObjMovement() {
		Message<KeyPress>::Connect(&PhysObjMovement::keyPressCallback, this);
		Message<KeyRelease>::Connect(&PhysObjMovement::keyReleaseCallback, this);
		Message<UpdateEvent>::Connect(&PhysObjMovement::onUpdate, this);
	}

	~PhysObjMovement() {
		Message<KeyPress>::Disconnect(&PhysObjMovement::keyPressCallback, this);
		Message<KeyRelease>::Disconnect(&PhysObjMovement::keyReleaseCallback, this);
		Message<UpdateEvent>::Disconnect(&PhysObjMovement::onUpdate, this);
	}
};

struct PlayerMovement {
	// Time elapsed since last update
	float dt = 1.0f;
	bool lock = false;

	float forwardAxisKeyboard = 0.0f;
	float verticalAxisKeyboard = 0.0f;
	float sidewaysAxisKeyboard = 0.0f;

	float forwardAxisJoystick = 0.0f;
	float verticalAxisJoystick = 0.0f;
	float sidewaysAxisJoystick = 0.0f;


	// Movement Axes
	float forwardAxis = 0.0f;
	float verticalAxis = 0.0f;
	float sidewaysAxis = 0.0f;

	//-------------------------------------------------------------------------
	void onUpdate(UpdateEvent& update) {
		dt = update.dt;
		if (lock) return;

		float forwardAxis_old = forwardAxis;
		float sidewaysAxis_old = sidewaysAxis;
		float verticalAxis_old = verticalAxis;

		forwardAxis = getLargerAxis(forwardAxisKeyboard, forwardAxisJoystick);
		sidewaysAxis = getLargerAxis(sidewaysAxisKeyboard, sidewaysAxisJoystick);
		verticalAxis = getLargerAxis(verticalAxisKeyboard, verticalAxisJoystick);

		if (
			forwardAxis != 0 || verticalAxis != 0 || sidewaysAxis != 0 || 
			(forwardAxis == forwardAxis_old && sidewaysAxis == sidewaysAxis_old && verticalAxis == verticalAxis_old)
		) {
			Message<PlayerMovement>::Send(*this);
		}
	}

	void keyPressCallback(KeyPress& key) {
		switch (key.keycode) {
			case GLFW_KEY_W: forwardAxisKeyboard += 1.0f; break;
			case GLFW_KEY_A: sidewaysAxisKeyboard -= 1.0f; break;
			case GLFW_KEY_S: forwardAxisKeyboard -= 1.0f; break;
			case GLFW_KEY_D: sidewaysAxisKeyboard += 1.0f; break;
			case GLFW_KEY_SPACE: verticalAxisKeyboard += 1.0f; break;
			case GLFW_KEY_LEFT_SHIFT: verticalAxisKeyboard -= 1.0f; break;
		}
	}

	void keyReleaseCallback(KeyRelease& key) {
		switch (key.keycode) {
			case GLFW_KEY_W: forwardAxisKeyboard -= 1.0f; break;
			case GLFW_KEY_A: sidewaysAxisKeyboard += 1.0f; break;
			case GLFW_KEY_S: forwardAxisKeyboard += 1.0f; break;
			case GLFW_KEY_D: sidewaysAxisKeyboard -= 1.0f; break;
			case GLFW_KEY_SPACE: verticalAxisKeyboard -= 1.0f; break;
			case GLFW_KEY_LEFT_SHIFT: verticalAxisKeyboard += 1.0f; break;
		}
	}

	void controllerCallback(GamepadUpdateEvent& e) {
		// Controller ID
		// e.controllerID

		sidewaysAxisJoystick = e.state->left_stick.xAxis;
		forwardAxisJoystick = e.state->left_stick.yAxis;
		verticalAxisJoystick = -(e.state->left_trigger) + e.state->right_trigger;
	}

	//-------------------------------------------------------------------------

	PlayerMovement() {
		Message<GamepadUpdateEvent>::Connect(&PlayerMovement::controllerCallback, this);
		Message<KeyPress>::Connect(&PlayerMovement::keyPressCallback, this);
		Message<KeyRelease>::Connect(&PlayerMovement::keyReleaseCallback, this);
		Message<UpdateEvent>::Connect(&PlayerMovement::onUpdate, this);
	}

	~PlayerMovement() {
		Message<GamepadUpdateEvent>::Disconnect(&PlayerMovement::controllerCallback, this);
		Message<KeyPress>::Disconnect(&PlayerMovement::keyPressCallback, this);
		Message<KeyRelease>::Disconnect(&PlayerMovement::keyReleaseCallback, this);
		Message<UpdateEvent>::Disconnect(&PlayerMovement::onUpdate, this);
	}
};

struct ObjectSelector {
	std::set<ActorID> selectableActors;
	bool isDragging = false;
	float time = 0.0f;

	// --------------------------------
	static glm::vec3 castMouseOntoPlane(glm::vec3 position, glm::vec3 normal) {
		// Convert from screen position to ray in world-space
		float mousePosX = (InputManager::localMouseState.position.x * 2.f / (float)Window::width) - 1.f;
		float mousePosY = 1.f - (InputManager::localMouseState.position.y * 2.f / (float)Window::height);
		glm::vec4 screenspaceRay = glm::vec4({ mousePosX, mousePosY, 1.f, 1 });
		// First, convert from screenspace to viewspace
		glm::vec4 eyespaceRay = glm::inverse(CameraManager::getProjectionMatrix()) * screenspaceRay;
		glm::vec4 viewspaceRay = glm::vec4(eyespaceRay.x, eyespaceRay.y, -1, 0);
		// Then, convet from viewspace to world space
		glm::vec4 worldspaceRay = glm::inverse(CameraManager::getViewMatrix()) * viewspaceRay;
		glm::vec3 camPos = CameraManager::cam->transformComponent->getGlobalTransform().getPosition();
		glm::vec3 ray = glm::normalize(glm::vec3(worldspaceRay.x, worldspaceRay.y, worldspaceRay.z));

		// calculate ray-plane intersection
		float denominator = glm::dot(ray, normal);
		if (denominator != 0) {
			float t = glm::dot(position - camPos, normal) / denominator;
			if (t >= 0) {
				return camPos + t * ray;
			}
		}
		return glm::vec3(NAN, NAN, NAN);
	}

	void updateCallback(UpdateEvent& e) {

		// Don't update mouse-over stuff every frame; only sample it every-so-often
		if (time > 0.03f) {
			time = 0.0f;
			
			// get the current mouse position in screenspace
			float mousePosX = (InputManager::localMouseState.position.x * 2.f / (float)Window::width) - 1.f;
			float mousePosY = 1.f - (InputManager::localMouseState.position.y * 2.f / (float)Window::height);
			glm::vec4 screenspaceRay = glm::vec4({ mousePosX, mousePosY, 1.f, 1 });

			// First, convert from screenspace to viewspace
			glm::vec4 eyespaceRay = glm::inverse(CameraManager::getProjectionMatrix()) * screenspaceRay;
			glm::vec4 viewspaceRay = glm::vec4(eyespaceRay.x, eyespaceRay.y, -1, 0);

			// Then, convet from viewspace to world space
			glm::vec4 worldspaceRay = glm::inverse(CameraManager::getViewMatrix()) * viewspaceRay;
			glm::vec3 camPos = CameraManager::cam->transformComponent->getGlobalTransform().getPosition();
			glm::vec3 ray = glm::normalize(glm::vec3(worldspaceRay.x, worldspaceRay.y, worldspaceRay.z));

			if (isDragging) {
				for (ActorID id : selectableActors) {
					SelectableActor* actor = (SelectableActor*)ActorManager::getActorByID(id);
					if (actor->IsBeingDragged()) {
						actor->OnDrag(camPos, ray);
					}
				}
			}
			else {
				float closestDist = std::numeric_limits<float>::quiet_NaN();
				ActorID closestActorID = -1;

				// do a raycast from the mouse against all selectable objects to see if they should be mouseovered
				for (ActorID id : selectableActors) {
					SelectableActor* actor = (SelectableActor*)ActorManager::getActorByID(id);
					float dist = actor->ComputeRaycastDist(camPos, ray);
					if (isnan(dist)) {
						continue;
					}
					else if (closestActorID == -1 || dist < closestDist) {
						closestDist = dist;
						closestActorID = id;
					}
				}

				for (ActorID id : selectableActors) {
					SelectableActor* actor = (SelectableActor*)ActorManager::getActorByID(id);
					actor->SetMouseoverState(id == closestActorID);
				}
			}
		}
		time += e.dt;

	}

	void mouseClickCallback(MouseClickEvent& e) {
		for (ActorID id : selectableActors) {
			SelectableActor* actor = (SelectableActor*)ActorManager::getActorByID(id);

			if (actor->IsMouseovered()) {
				if (!actor->IsSelected()) {
					actor->Select();
				}
			}
			else {
				if (actor->IsSelected()) {
					actor->Deselect();
				}
			}

		}
	}

	void mouseHoldCallback(MouseHoldEvent& e) {
		if (!isDragging) {
			for (ActorID id : selectableActors) {
				SelectableActor* actor = (SelectableActor*)ActorManager::getActorByID(id);
				actor->SetDragState(actor->IsSelected());
			}

			isDragging = true;
		}
	}

	void mouseReleaseCallback(MouseReleaseEvent& e) {
		if (isDragging) {
			for (ActorID id : selectableActors) {
				SelectableActor* actor = (SelectableActor*)ActorManager::getActorByID(id);
				actor->SetDragState(false);
			}
			isDragging = false;
		}
	}

	//-------------------------------------------------------------------------

	ObjectSelector() {
		Message<UpdateEvent>::Connect(&ObjectSelector::updateCallback, this);
		Message<MouseClickEvent>::Connect(&ObjectSelector::mouseClickCallback, this);
		Message<MouseHoldEvent>::Connect(&ObjectSelector::mouseHoldCallback, this);
		Message<MouseReleaseEvent>::Connect(&ObjectSelector::mouseReleaseCallback, this);
	}

	~ObjectSelector() {
		Message<UpdateEvent>::Disconnect(&ObjectSelector::updateCallback, this);
		Message<MouseClickEvent>::Disconnect(&ObjectSelector::mouseClickCallback, this);
		Message<MouseHoldEvent>::Disconnect(&ObjectSelector::mouseHoldCallback, this);
		Message<MouseReleaseEvent>::Disconnect(&ObjectSelector::mouseReleaseCallback, this);
	}
};

struct MyControls {

	ObjectSelector objectSelector;
	PlayerMovement playerMovement;
	PhysObjMovement physObjMovement;
	CameraControl cameraControl;
	ExitProgram exitProgram;
	OpenConsole openConsole;

	static MyControls& getControls() {
		static MyControls singleton;
		return singleton;
	}
};