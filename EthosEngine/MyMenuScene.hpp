
#include "Scene.hpp"
#include "MyCamera.hpp"
#include "MyMainMenuUI.hpp"
#include "TestPlayer.hpp"

class MyMenuScene : public Scene {
public:

	size_t playerActorIDs[4];

	MyMenuScene() {
		MyCamera* camera = new MyCamera();
		actors.push_back(camera);
		actors.push_back(new MyMainMenuUI());

		TestPlayer* player1 = new TestPlayer(0);
		playerActorIDs[0] = ((Actor*)player1)->actorID;

		TestPlayer* player2 = new TestPlayer(1);
		playerActorIDs[1] = ((Actor*)player2)->actorID;

		camera->lockCamera = true;
		Message<UpdateEvent>::Connect(&MyMenuScene::update, this);
	}

	void update(UpdateEvent& e) {
		Gamestate& gs = NetworkManager::getCurrentGamestate();
		for (unsigned i = 0; i < gs.updateCount; ++i) {
			if (gs.actorUpdates[i].actorID != playerActorIDs[NetworkManager::getPlayerID()]) {
				ActorManager::getActorByID(gs.actorUpdates[i].actorID)->applyUpdateData(gs.actorUpdates[i]);
			}
		}
	}

	~MyMenuScene() {
	}
};