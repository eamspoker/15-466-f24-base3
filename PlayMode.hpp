#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();
	void nextScene(size_t level);

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, space;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;




	float wobble = 0.0f;

	Scene::Transform *map = nullptr;
	glm::quat map_base_rot;
	Scene::Transform *player = nullptr;
		struct LocalizedSound {
		Scene::Transform *transform;
		glm::quat base_rot;
		Sound::Sample const *sound;
	} local_sound1, local_sound2, local_sound3;

	size_t current_sample;


	//music coming from the tip of the leg (as a demonstration):
	std::shared_ptr< Sound::PlayingSample > curr_loop;
	std::shared_ptr< Sound::PlayingSample > curr_bgm;
	bool isLookingAtMap = false;
	//camera:
	Scene::Camera *camera = nullptr;

	glm::vec3 dist_to_camera;
	glm::vec3 dist_to_map;

	// next sound to destroy
	size_t next_sound = 0;
	std::vector<LocalizedSound> lvl1_sounds;

	const glm::uvec2 *curr_window_size;

	std::vector<std::string> buildings = {"Purnell", "WeanDoherty", "BakerPorter", "Hunt", "CFA"};
	size_t selected_index = 0;
	std::string selected_building = buildings[selected_index];

	size_t mistakes = 0;

	size_t level = 0;
	std::vector<std::string> correct_building = {"WeanDoherty", "Hunt"};

	std::string winlose_msg = "";
	
	size_t num_discovered = 0;
	std::vector<std::string> places = {"Sorrells Library, Wean Hall", "1st Floor Staircase, Hunt Library"};

	bool done = false;
};
