#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"
#include "SpecialColorProgram.hpp"
#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>


GLuint meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("map.pnct"));
	meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

GLuint meshes_for_special_color_program = 0;
Load< MeshBuffer > meshes_special(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("map.pnct"));
	meshes_for_special_color_program = ret->make_vao_for_program(special_color_program->program);
	return ret;
});

GLuint meshes_for_lit_color_texture_program2 = 0;
Load< MeshBuffer > meshes2(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("map2.pnct"));
	meshes_for_lit_color_texture_program2 = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

GLuint meshes_for_special_color_program2 = 0;
Load< MeshBuffer > meshes_special2(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("map2.pnct"));
	meshes_for_special_color_program2 = ret->make_vao_for_program(special_color_program->program);
	return ret;
});
Load< Scene > main_scene1(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("map.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
			Mesh const &mesh =  meshes->lookup(mesh_name);

			scene.drawables.emplace_back(transform);
			Scene::Drawable &drawable = scene.drawables.back();

			drawable.pipeline = lit_color_texture_program_pipeline;
			drawable.pipeline.vao = meshes_for_lit_color_texture_program;
			drawable.pipeline.type = mesh.type;
			drawable.pipeline.start = mesh.start;
			drawable.pipeline.count = mesh.count;

			drawable.pipeline2 = special_color_program_pipeline;
			drawable.pipeline2.vao = meshes_for_special_color_program;
			drawable.pipeline2.type = mesh.type;
			drawable.pipeline2.start = mesh.start;
			drawable.pipeline2.count = mesh.count;
	});
});

Load< Scene > main_scene2(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("map2.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
			Mesh const &mesh =  meshes2->lookup(mesh_name);

			scene.drawables.emplace_back(transform);
			Scene::Drawable &drawable = scene.drawables.back();

			drawable.pipeline = lit_color_texture_program_pipeline;
			drawable.pipeline.vao = meshes_for_lit_color_texture_program2;
			drawable.pipeline.type = mesh.type;
			drawable.pipeline.start = mesh.start;
			drawable.pipeline.count = mesh.count;

			drawable.pipeline2 = special_color_program_pipeline;
			drawable.pipeline2.vao = meshes_for_special_color_program2;
			drawable.pipeline2.type = mesh.type;
			drawable.pipeline2.start = mesh.start;
			drawable.pipeline2.count = mesh.count;
	});
});


Load< Sound::Sample > sample1(LoadTagDefault, []() -> Sound::Sample const * {
	Sound::Sample *s = new Sound::Sample(data_path("lvl1_1.opus"));
	return s;
});

Load< Sound::Sample > sample2(LoadTagDefault, []() -> Sound::Sample * {
	Sound::Sample *s = new Sound::Sample(data_path("lvl1_2.opus"));
	return s;
});

Load< Sound::Sample > sample3(LoadTagDefault, []() -> Sound::Sample const * {
	Sound::Sample *s = new Sound::Sample(data_path("lvl1.opus"));
	return s;
});

Load< Sound::Sample > sample2_1(LoadTagDefault, []() -> Sound::Sample const * {
	Sound::Sample *s = new Sound::Sample(data_path("lvl2_1.opus"));
	return s;
});

Load< Sound::Sample > sample2_2(LoadTagDefault, []() -> Sound::Sample * {
	Sound::Sample *s = new Sound::Sample(data_path("lvl2_2.opus"));
	return s;
});

Load< Sound::Sample > sample2_3(LoadTagDefault, []() -> Sound::Sample const * {
	Sound::Sample *s = new Sound::Sample(data_path("lvl2_3.opus"));
	return s;
});

Load< Sound::Sample > bgm(LoadTagDefault, []() -> Sound::Sample const * {
	Sound::Sample *s = new Sound::Sample(data_path("spooky.opus"));
	return s;
});

size_t getClosestSound(std::vector<PlayMode::LocalizedSound> lvl1_sounds, Scene::Camera *camera)
{
	size_t min_index;
	float min_distance = FLT_MAX;
	for (size_t i = 0; i < lvl1_sounds.size(); i++) {
		PlayMode::LocalizedSound s = lvl1_sounds[i];
		float dist = glm::length(s.transform->position - camera->transform->position);
		if (dist < min_distance)
		{
			min_distance = dist;
			min_index = i;
		}
	}

	return min_index;

	
	
}

PlayMode::PlayMode() : scene(*main_scene1) {
	//get pointers to leg for convenience:
	for (auto &transform : scene.transforms) {
		if (transform.name == "Map") map = &transform;
		if (transform.name == "Sphere") player = &transform;
		if (transform.name == "Icosphere") 
		{ local_sound1.transform = &transform;
			local_sound1.base_rot = transform.rotation;
		}
		if (transform.name == "Icosphere.001") 
		{ local_sound2.transform = &transform;
		local_sound2.base_rot = transform.rotation;
		}
		if (transform.name == "Icosphere.002") 
		{ local_sound3.transform = &transform;
		local_sound3.base_rot = transform.rotation;
		}
	}

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	dist_to_camera = camera->transform->position - player->position;
	map->scale *= 0.0f;
	map_base_rot = map->rotation;

	local_sound1.sound = &(*sample1);
	local_sound2.sound = &(*sample2);
	local_sound3.sound = &(*sample3);


	lvl1_sounds = {local_sound1, local_sound2, local_sound3};
	


	//start music loop playing:
	// (note: position will be over-ridden in update())
	current_sample = getClosestSound(lvl1_sounds, camera);
	LocalizedSound closest_sound = lvl1_sounds[current_sample];
	curr_loop = Sound::loop_3D(*closest_sound.sound, 1.0f, closest_sound.transform->position, 10.0f);
	curr_bgm = Sound::loop(*bgm, 0.45f);
}

void PlayMode::nextScene(size_t level_)
{
	level = level_;
	mistakes = 0;
	isLookingAtMap = false;
	done = false;
	winlose_msg = "";
	next_sound = 0;
	
	
	Sound::stop_all_samples();

	if (level == 1)
	{
		scene = *main_scene2;
		//get pointers to leg for convenience:
		for (auto &transform : scene.transforms) {
			if (transform.name == "Map") map = &transform;
			if (transform.name == "Sphere") player = &transform;
			if (transform.name == "Icosphere") 
			{ local_sound1.transform = &transform;
				local_sound1.base_rot = transform.rotation;
			}
			if (transform.name == "Icosphere.001") 
			{ local_sound2.transform = &transform;
			local_sound2.base_rot = transform.rotation;
			}
			if (transform.name == "Icosphere.002") 
			{ local_sound3.transform = &transform;
			local_sound3.base_rot = transform.rotation;
			}
		}

		//get pointer to camera for convenience:
		if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
		camera = &scene.cameras.front();

		dist_to_camera = camera->transform->position - player->position;
		map->scale *= 0.0f;
		map_base_rot = map->rotation;

		local_sound1.sound = &(*sample2_1);
		local_sound2.sound = &(*sample2_2);
		local_sound3.sound = &(*sample2_3);

		next_sound = 0;
		lvl1_sounds = {local_sound1, local_sound2, local_sound3};
		


		//start music loop playing:
		// (note: position will be over-ridden in update())
		current_sample = getClosestSound(lvl1_sounds, camera);
		LocalizedSound closest_sound = lvl1_sounds[current_sample];
		curr_loop = Sound::loop_3D(*closest_sound.sound, 1.0f, closest_sound.transform->position, 10.0f);
		curr_bgm = Sound::loop(*bgm, 0.45f);
	} else {
		num_discovered = 0;

		scene = *main_scene1;
		//get pointers to leg for convenience:
		for (auto &transform : scene.transforms) {
			if (transform.name == "Map") map = &transform;
			if (transform.name == "Sphere") player = &transform;
			if (transform.name == "Icosphere") 
			{ local_sound1.transform = &transform;
				local_sound1.base_rot = transform.rotation;
			}
			if (transform.name == "Icosphere.001") 
			{ local_sound2.transform = &transform;
			local_sound2.base_rot = transform.rotation;
			}
			if (transform.name == "Icosphere.002") 
			{ local_sound3.transform = &transform;
			local_sound3.base_rot = transform.rotation;
			}
		}

		//get pointer to camera for convenience:
		if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
		camera = &scene.cameras.front();

		dist_to_camera = camera->transform->position - player->position;
		map->scale *= 0.0f;
		map_base_rot = map->rotation;

		local_sound1.sound = &(*sample1);
		local_sound2.sound = &(*sample2);
		local_sound3.sound = &(*sample3);


		lvl1_sounds = {local_sound1, local_sound2, local_sound3};
		


		//start music loop playing:
		// (note: position will be over-ridden in update())
		current_sample = getClosestSound(lvl1_sounds, camera);
		LocalizedSound closest_sound = lvl1_sounds[current_sample];
		curr_loop = Sound::loop_3D(*closest_sound.sound, 1.0f, closest_sound.transform->position, 10.0f);
		curr_bgm = Sound::loop(*bgm, 0.45f);

	}
}



PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	curr_window_size = &window_size;
	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_r) {
			if (done)
			{
				nextScene(0);

			}
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_TAB) {
			if (isLookingAtMap)
			{
				selected_index = selected_index + 1 >= buildings.size() ? 0 : selected_index+1;
				selected_building = buildings[selected_index];
			}
			return true;
		} else if (evt.key.keysym.sym == SDLK_KP_ENTER || 
			evt.key.keysym.sym == SDLK_RETURN2 || 
			evt.key.keysym.sym == SDLK_RETURN)
		{
			// std::cout << "done: " << done << ", map: " << isLookingAtMap << std::endl;

			if (!done && isLookingAtMap)
			{

				if (selected_building == correct_building[level])
				{
					num_discovered++;
					if (level == 0)
					{
						nextScene(1);
					} else {
						winlose_msg = "YOU HAVE ESCAPED CMU! (PRESS R TO RESTART)";
						done = true;
					}
				} else {
					if (mistakes < 3)
					{
						if (next_sound == current_sample)
						{
							curr_loop->stop();
						}
						lvl1_sounds[next_sound].sound = nullptr;
						next_sound++;
						mistakes++;
						if (mistakes > 2) {
							winlose_msg = "YOU USED UP ALL SOUNDS. YOU ARE TRAPPED IN CMU FOREVER (PRESS R TO RESTART)";
							done = true;
						}
					} 
					
				}
			}
			return true;
		}

	} else if (evt.type == SDL_KEYUP) {

		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.pressed = false;
			isLookingAtMap = !isLookingAtMap;
			return true;
		}
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	} else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {

		if (isLookingAtMap)
		{
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y)
			);
			map->rotation = glm::normalize(
				map->rotation
				* glm::angleAxis(-motion.x * camera->fovy, glm::vec3(0.0f, 1.0f, 1.0f))
				* glm::angleAxis(motion.y * camera->fovy, glm::vec3(1.0f, 0.0f, 0.0f))
			);
		} else {

			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel  / float(window_size.y)
			);

				camera->transform->rotation = glm::normalize(
				camera->transform->rotation
				* glm::quat(glm::vec3{0.0f, -motion.x * camera->fovy, 0.0f})
			);

		
			
		}
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {

	//slowly rotates through [0,1):
	wobble += elapsed / 10.0f;
	wobble -= std::floor(wobble);


	//move camera:
	{
		//combine inputs into a move:
		constexpr float PlayerSpeed = 10.0f;

		// x & y : rotation, z: movement forward (can only go forwards)
		glm::vec3 move = glm::vec3(0.0f);
		if (left.pressed && !right.pressed) move.x =-1.0f;
		if (!left.pressed && right.pressed) move.x = 1.0f;
		if (down.pressed && !up.pressed) move.y =-1.0f;
		if (!down.pressed && up.pressed) move.y = 1.0f;
		if (move != glm::vec3(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;



		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 frame_right = glm::vec3{frame[0].x, frame[0].y, 0.0f};
		glm::vec3 frame_up = glm::vec3{-frame[2].x, -frame[2].y, 0.0f};

		glm::vec3 new_position = 	player->position + move.x*frame_right + move.y*frame_up;

		if (new_position.x < 32.0f && new_position.x > -39.0f
			&& new_position.y < 20.0f && new_position.y > -33.0f)
		{
					player->position = new_position;

		}
		



		
		camera->transform->position = dist_to_camera + player->position;

		if (!isLookingAtMap && map->scale.x > 0.0f)
		{
			map->scale *= 0.0f;
		} else if (isLookingAtMap && map->scale.x <= 0.0f)
		{
			map->scale = glm::vec3(1.0f);
		}


		
	
	}

	{
		// update which loop is playing based on closeness
		size_t closest = getClosestSound(lvl1_sounds, camera);

		if (current_sample != closest)
		{
			current_sample = closest;
			curr_loop->stop();
		}

		if (curr_loop->stopped)
		{
			LocalizedSound curr_sound = lvl1_sounds[current_sample];

			if (curr_sound.sound != nullptr)
			{
				curr_loop = Sound::loop_3D(*curr_sound.sound, 1.0f, curr_sound.transform->position, 10.0f);
			}

		}


		lvl1_sounds[current_sample].transform->scale = glm::vec3(0.5f)*(1.5f + sin(wobble*5.0f));
		lvl1_sounds[current_sample].transform->rotation =  lvl1_sounds[current_sample].base_rot * glm::angleAxis(
				glm::radians(720.0f*std::sin(wobble)),
				glm::vec3(1.0f, 0.0f, 0.0f)
				);

	}

	{ //update listener to camera position:
		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 frame_right = frame[0];
		glm::vec3 frame_at = frame[3];
		Sound::listener.set_position_right(frame_at, frame_right, 1.0f / 60.0f);
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));

	glUseProgram(special_color_program->program);
	glUniform1i(special_color_program->LIGHT_TYPE_int, 1);
	glUniform3fv(special_color_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(special_color_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	if (level == 0)
	{
		scene.draw(*camera, selected_building);
	} else {
		scene.draw(*camera, selected_building);

	}

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;

		if (!isLookingAtMap)
		{
			lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse; space for mapspace for map",
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.5f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse; space for map",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + 0.5f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));

		} else {
			lines.draw_text("Tab to choose building, enter to guess, space to close",
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.5f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Tab to choose building, enter to guess, space to close",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + 0.5f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));

		}



		lines.draw_text("Sounds lost: " + std::to_string(mistakes),
			glm::vec3(-aspect + ((33.0f)* H), -1.0 + 20.0f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Sounds lost: " + std::to_string(mistakes),
			glm::vec3(-aspect +((33.0f)* H) + ofs, -1.0 + 20.0f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));


	constexpr float H3 = 0.07f;
	lines.draw_text("Places discovered: ",
			glm::vec3(-aspect + ((37.0f)* H3), -1.0 + (23.0f * H3), 0.0),
			glm::vec3(H3, 0.0f, 0.0f), glm::vec3(0.0f, H3, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		lines.draw_text("Places discovered: ",
			glm::vec3(-aspect +((37.0f)* H3) + ofs, -1.0 + (23.0f* H3) + ofs, 0.0),
			glm::vec3(H3, 0.0f, 0.0f), glm::vec3(0.0f, H3, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));

	for (size_t i = 0; i < num_discovered; i++)
	{
		lines.draw_text(places[i],
			glm::vec3(-aspect + ((37.0f)* H3), -1.0 + ((23.0f-(i+1.0)) * H3), 0.0),
			glm::vec3(H3, 0.0f, 0.0f), glm::vec3(0.0f, H3, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		lines.draw_text(places[i],
			glm::vec3(-aspect +((37.0f)* H3) + ofs, -1.0 + ((23.0f-(i+1.0)) * H3) + ofs, 0.0),
			glm::vec3(H3, 0.0f, 0.0f), glm::vec3(0.0f, H3, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));


	}

		constexpr float H2 = 0.12f;

		lines.draw_text(winlose_msg,
			glm::vec3(-aspect + (0.1f* H2), -1.0 + 7.0f * H2, 0.0),
			glm::vec3(H2, 0.0f, 0.0f), glm::vec3(0.0f, H2, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		lines.draw_text(winlose_msg,
			glm::vec3(-aspect +(0.1f* H2) + ofs, -1.0 + 7.0f * H2 + ofs, 0.0),
			glm::vec3(H2, 0.0f, 0.0f), glm::vec3(0.0f, H2, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
		
	}
	GL_ERRORS();
}

// glm::vec3 PlayMode::get_leg_tip_position() {
// 	//the vertex position here was read from the model in blender:
// 	return lower_leg->make_local_to_world() * glm::vec4(-1.26137f, -11.861f, 0.0f, 1.0f);
// }
