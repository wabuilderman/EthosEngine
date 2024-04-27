#pragma once
#include "Actor.hpp"
#include "Message.hpp"
#include "AssetManager.hpp"
#include "UpdateEvent.hpp"
#include "CubeMesh.hpp"
#include "WorldSpaceMarker.hpp"
//#include "PhysicsBody.hpp"
//#include "PhysicsManager.hpp"
//#include "PhysicsShape.hpp"
#include "MyControls.hpp"

class PhysMarker : public Actor {
public:
	const char* getClassName() const override {
		return "PhysMarker";
	};
	Mesh* sphereMesh = DefaultMesh::get();
	Model* sphereModel = nullptr;
	Material myMat = Material();
	PhysMarker() {
		sphereModel = new Model(sphereMesh, nullptr, nullptr);
		addRenderComponent(sphereModel);
		transformComponent = new TransformComponent(*this);
	}
	~PhysMarker() {
		delete sphereModel;
	}
};

class TestPhysObj : public Actor {
public:

	const char* getClassName() const override {
		return "testPhysObj";
	};

	Model* sphereModel = nullptr;
	Model* boxModel = nullptr;
	Material myMat = Material();
	FBXloader loader;
	PhysicsShape* sphereShape = nullptr;
	PhysicsShape* boxShape = nullptr;
	glm::vec3 initialPosition;
	bool toggleGravity = false;
	bool togglePhysics = true;
	bool isSphere = false;

	void resetKeyCallback(KeyPress& key) {
		if (key.keycode == GLFW_KEY_KP_0) {
			reset();
		}
	}

	void keyPressCallback(KeyPress& key) {
		switch (key.keycode) {
		case GLFW_KEY_KP_8: physicsComponent->physicsBody.applyImpulse(Vector::forward); break;
		case GLFW_KEY_KP_2: physicsComponent->physicsBody.applyImpulse(Vector::backward); break;
		case GLFW_KEY_KP_6: physicsComponent->physicsBody.applyImpulse(Vector::left); break;
		case GLFW_KEY_KP_4: physicsComponent->physicsBody.applyImpulse(Vector::right); break;
		case GLFW_KEY_KP_9: physicsComponent->physicsBody.applyImpulse(Vector::up); break;
		case GLFW_KEY_KP_7: physicsComponent->physicsBody.applyImpulse(Vector::down); break;
		case GLFW_KEY_KP_1: physicsComponent->physicsBody.setAngularVelocity({ 1.f,0,0 }); break;
		case GLFW_KEY_KP_3: physicsComponent->physicsBody.setAngularVelocity({ 0.f,1.0f,0 }); break;
		case GLFW_KEY_KP_5:
			physicsComponent->physicsBody.setLinearVelocity({ 0, 0, 0 });
			physicsComponent->physicsBody.setAngularVelocity({ 0,0,0 });
			break;
		case GLFW_KEY_G:
			toggleGravity = !toggleGravity;
			physicsComponent->physicsBody.setEnableGravity(toggleGravity);
			break;
		case GLFW_KEY_T:
			togglePhysics = !togglePhysics;
			PhysicsManager::setPause(togglePhysics);
			break;
		case GLFW_KEY_Y:
			if (togglePhysics) {
				PhysicsManager::simulate(PhysicsManager::physicsTimestep);
			}
			break;
		case GLFW_KEY_H:
			if (isSphere = !isSphere) {
				physicsComponent->physicsBody.shapes[0] = sphereShape;
				renderComponents[0]->setModel(sphereModel);
			}
			else {
				physicsComponent->physicsBody.shapes[0] = boxShape;
				renderComponents[0]->setModel(boxModel);
			}
		}
		//if (key.keycode > GLFW_KEY_KP_0 && key.keycode < GLFW_KEY_KP_9) {
			//physicsComponent->rigidBodies[0]->activate();
		//}
	}

	void keyReleaseCallback(KeyRelease& key) {
		/*switch (key.keycode) {
			case GLFW_KEY_KP_8: rigidBody->applyImpulse(btVec(Vector::backward), btVector3()); break; // pBody->addImpulse(Vector::backward); break;
			case GLFW_KEY_KP_2: rigidBody->applyImpulse(btVec(Vector::forward), btVector3()); break; // pBody->addImpulse(Vector::forward); break;
			case GLFW_KEY_KP_6: rigidBody->applyImpulse(btVec(Vector::right), btVector3()); break; // pBody->addImpulse(Vector::right); break;
			case GLFW_KEY_KP_4: rigidBody->applyImpulse(btVec(Vector::left), btVector3()); break; // pBody->addImpulse(Vector::left); break;
			case GLFW_KEY_KP_9: rigidBody->applyImpulse(btVec(Vector::down), btVector3()); break; // pBody->addImpulse(Vector::down); break;
			case GLFW_KEY_KP_7: rigidBody->applyImpulse(btVec(Vector::up), btVector3()); break; // pBody->addImpulse(Vector::up); break;
		}*/
	}

	void reset() {
		physicsComponent->physicsBody.teleport(initialPosition);
		physicsComponent->physicsBody.setLinearVelocity({ 0, 0, 0 });
		physicsComponent->physicsBody.setAngularVelocity({ 0, 0, 0 });
		physicsComponent->physicsBody.currentTransform.setRotation({ 1, 0, 0, 0 });
		physicsComponent->physicsBody.targetTransform.setRotation({ 1, 0, 0, 0 });
	}

	void enableControl() {
		Message<KeyPress>::Connect(&TestPhysObj::keyPressCallback, this);
		//Message<KeyRelease>::Connect(&TestPhysObj::keyReleaseCallback, this);
	}

	TestPhysObj() {
		//loader.options.scale = 0.5f;
		//loader.loadFBX("models/wireBox.fbx");

		Mesh* sphereMesh = DefaultMesh::get();
		Mesh* boxMesh = CubeMesh::get();

		//if (loader.meshes.size() > 0) {
		//	mesh = &loader.meshes[0];
		//}

		sphereModel = new Model(sphereMesh, nullptr, nullptr);
		boxModel = new Model(boxMesh, nullptr, nullptr);
		if (isSphere) {
			addRenderComponent(sphereModel);
		}
		else {
			addRenderComponent(boxModel);
		}

		//PhysMarker* physMarker = new PhysMarker();
		
		//children.push_back(physMarker);
		
		transformComponent = new TransformComponent(*this);
		boxShape = new BoxPhysicsShape({ 1.f, 1.f, 1.f });
		sphereShape = new SpherePhysicsShape(0.5f);
		physicsComponent = new PhysicsComponent(*this);

		if (isSphere) {
			physicsComponent->physicsBody.shapes.push_back(sphereShape);
		}
		else {
			physicsComponent->physicsBody.shapes.push_back(boxShape);
		}
		
		

		initialPosition = {0, 0, 0};
		Message<KeyPress>::Connect(&TestPhysObj::resetKeyCallback, this);
		PhysicsManager::setPause(togglePhysics);
	}

	~TestPhysObj() {
		delete sphereModel;
		delete boxModel;
		delete sphereShape;
		delete boxShape;
	}
};