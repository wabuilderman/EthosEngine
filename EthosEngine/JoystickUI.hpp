#pragma once
#include "Actor.hpp"
#include "RingCircleMesh.hpp"
#include "MyControls.hpp"

#include "UIComponent.hpp"
#include "RenderComponentManager.hpp"
#include <sstream>
#include <iomanip>
#include "TextManager.hpp"

class JoystickUI : public Actor {
public:

	Mesh* rangeMesh;
	Model* rangeModel;

	Mesh* stickMesh;
	Model* stickModel;

	UIComponent* rightStickRange;
	UIComponent* rightStickAxis;
	UIComponent* rightStickAccel;

	UIComponent* leftStickRange;
	UIComponent* leftStickAxis;

	TextComponent* debugInfo;

	void handleAccel(CameraControl& e) {
		rightStickAccel->transform.setPosition({ e.yawAxis, e.pitchAxis });
	}

	void handleAxis(GamepadUpdateEvent& e) {
		leftStickAxis->transform.setPosition({ e.state->left_stick.xAxis, e.state->left_stick.yAxis });
		rightStickAxis->transform.setPosition({ e.state->right_stick.xAxis, e.state->right_stick.yAxis });
	}

	void update(UpdateEvent& e) {
		std::stringstream stream;
		stream << "dt: " << std::fixed << std::setprecision(7) << e.dt;
		std::string s = stream.str();

		debugInfo->editText(s, 0);
		RenderComponentManager::updateUIComponentVBO(debugInfo);
	}

	JoystickUI(Actor * parent = nullptr) : Actor(parent) {
		constexpr float joystickSeperation = 1.5f;

		rangeMesh = new RingCircleMesh(1.f, 0.9f, 3, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f });
		stickMesh = new RingCircleMesh(1.f, 0.0f, 3, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f });

		rangeModel = new Model(rangeMesh, nullptr, nullptr);
		stickModel = new Model(stickMesh, nullptr, nullptr);

		rightStickRange = new UIComponent(*this, rangeModel);
		rightStickRange->transform.setPosition({ joystickSeperation * 0.1f, 0.0f });
		rightStickRange->transform.scale(0.1f);

		rightStickAccel = new UIComponent(*this, stickModel, rightStickRange);
		rightStickAccel->transform.scale(0.1f);
		rightStickAccel->color = { 1.0f, 1.0f, 0.0f, 1.0f };
		rightStickAxis  = new UIComponent(*this, stickModel, rightStickRange);
		rightStickAxis->transform.scale(0.1f);
		rightStickAxis->color = { 1.0f, 0.0f, 0.0f, 1.0f };

		leftStickRange = new UIComponent(*this, rangeModel);
		leftStickRange->transform.setPosition({ -joystickSeperation * 0.1f, 0.0f });
		leftStickRange->transform.scale(0.1f);

		leftStickAxis = new UIComponent(*this, stickModel, leftStickRange);
		leftStickAxis->transform.scale(0.1f);
		leftStickAxis->color = { 1.0f, 0.0f, 0.0f, 1.0f };

		std::string text = "dt:          ";
		debugInfo->transform.scale(0.1f);
		transformComponent = new TransformComponent(*this);

		Message<UpdateEvent>::Connect(&JoystickUI::update, this);
		Message<CameraControl>::Connect(&JoystickUI::handleAccel, this);
		Message<GamepadUpdateEvent>::Connect(&JoystickUI::handleAxis, this);
	}

	~JoystickUI() {
		delete rightStickRange;
		delete rightStickAxis;
		delete rightStickAccel;

		delete leftStickRange;
		delete leftStickAxis;

		delete stickModel;
		delete stickMesh;
		delete rangeModel;
		delete rangeMesh;

		delete transformComponent;
	}
};