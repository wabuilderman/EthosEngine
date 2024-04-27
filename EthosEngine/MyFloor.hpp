#pragma once
#include "Actor.hpp"
#include "Message.hpp"
#include "AssetManager.hpp"
#include "UpdateEvent.hpp"
#include "CubeMesh.hpp"

class MyFloor : public Actor {
public:
	const char* getClassName() const override {
		return "MyFloor";
	};

	//Mesh* mesh;
	Model* floorModel = nullptr;

	Material myMat = Material();
	//PhysicsBody* pBody = nullptr;

	Texture* textureC = nullptr;
	Texture* textureN = nullptr;
	Texture* textureS = nullptr;
	Texture* textureA = nullptr;
	Texture* textureD = nullptr;

	PhysicsShape* physicsShape;
	//btTypedConstraint* pinConstraint;

	void update(UpdateEvent& e) {
	}

	MyFloor() {

		float xDim = 10.0f;
		float yDim = 10.0f;
		float zDim = 1.0f;

		//textureC = new Texture("textures/Brick_wall_002/Brick_wall_002_COLOR.jpg");
		//textureN = new Texture("textures/Brick_wall_002/Brick_wall_002_NORM.jpg");
		//textureS = new Texture("textures/Brick_wall_002/Brick_wall_002_SPEC.jpg");
		//textureA = new Texture("textures/Brick_wall_002/Brick_wall_002_AO.jpg");
		//textureD = new Texture("textures/Brick_wall_002/Brick_wall_002_DISP.png");

		textureC = new Texture("textures/testGrid.png");
		//textureC = new Texture("textures/gravel/albedo_1024.png");
		//textureN = new Texture("textures/gravel/normal_1024.png");
		//textureA = new Texture("textures/gravel/height_1024.png");
		//textureD = new Texture("textures/gravel/height_1024.png");

		myMat.setTexture(textureC, Material::TextureType::Color);
		//myMat.setTexture(textureN, Material::TextureType::Normal);
		//myMat.setTexture(textureA, Material::TextureType::Ambient);
		//myMat.setTexture(textureD, Material::TextureType::Displacement);

		//myMat.baseColor = glm::vec3(0.f, 1.f, 0.f);
		//myMat.emission = glm::vec4(1.f, 1.f, 1.f, 0.5f);

		floorModel = new Model(CubeMesh::get(), &myMat, nullptr);

		myMat.diffuseUVScale  = { 2.0f, 2.0f };
		myMat.normalUVScale   = { 2.0f, 2.0f };
		myMat.specularUVScale = { 2.0f, 2.0f };
		myMat.ambientUVScale  = { 2.0f, 2.0f };
		myMat.displaceUVScale = { 2.0f, 2.0f };

		addRenderComponent(floorModel);
		addTransformComponent();
		addPhysicsComponent();

		transformComponent->accessLocalTransform().scale(1.0f);
		
		physicsShape = new BoxPhysicsShape({ xDim, yDim, zDim });
		physicsComponent->physicsBody.shapes.push_back(physicsShape);
		physicsComponent->physicsBody.targetTransform.setScale({ xDim, yDim, zDim });
		physicsComponent->physicsBody.currentTransform.setScale({ xDim, yDim, zDim });
		physicsComponent->physicsBody.isStatic = true;
	}

	~MyFloor() {
		delete floorModel;

		//delete pBody;

		delete textureC;
		//delete textureN;
		//delete textureA;
		//delete textureD;

		delete physicsShape;
	}
};