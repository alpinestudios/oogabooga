module game;

import std::io;
import engine;
import utils;
import entity;
import std::math;
import std::collections;
import particles;


def Vector2 = engine::Vector2;
def Matrix4 = engine::Matrix4;

macro @str(s) {
	engine::CString str;
	str.count = engine::length_of_null_terminated_string(s);
	str.data = s;
    return str;
}

macro @v2(x, y) {
    return Vector2 {{ x, y }};
}

macro @v3(x, y, z) {
    return engine::Vector3 {{ x, y, z }};
}

macro @v4(x, y, z, w) {
    return engine::Vector4 {{ x, y, z, w }};
}

def is_key_down = engine::is_key_down;
def is_key_just_released = engine::is_key_just_released;
def InputKeyCode = engine::InputKeyCode;

def Entity = entity::Entity;  




struct Game {
    engine::Vector2 camera_pos;
    Entity[20] room_enemies;
    Entity player_entity;
    engine::GfxImage *player_sprites;
    engine::GfxImage *weapon_sprites;
    engine::GfxImage *walls;
    engine::GfxImage *particle_sprites;
    engine::GfxFont *font;
	List(<particles::Particle>) particles;
}

struct WorldFrame {
	Vector2 world_mouse_pos;
}

WorldFrame world_frame = {};

fn int main(String[] args)
{
	
	engine::oogabooga_init(5 * 1024 * 1024);
	// engine::window.clear_color = hex_to_rgba(0x0f101dff);
	ulong b = 0x0f101dff;
	// ulong b = 0xFFAABBCCDDEEFF;
	engine::window.clear_color = engine::hex_to_rgba(b);
	// Vector4 vec4 = {0.1f, 0.1f, 0.1f, 1};
	engine::window.width = 1280;
    engine::window.height = 800;
    engine::window.x = 100;
    engine::window.y = 100;
	engine::window.title = @str("Hello from c3");
	Game game = {};
	
	game.weapon_sprites = engine::load_image_from_disk(@str("./assets/weapons.png"), engine::get_heap_allocator());
    game.player_sprites = engine::load_image_from_disk(@str("./assets/player.png"), engine::get_heap_allocator());
    game.walls = engine::load_image_from_disk(@str("./assets/walls.png"), engine::get_heap_allocator());
    game.particle_sprites = engine::load_image_from_disk(@str("./assets/particles.png"), engine::get_heap_allocator());



	game.player_entity.speed = 100;
	game.player_entity.right_hand_weapon = entity::Weapons.SWORD;
	game.player_entity.left_hand_weapon = entity::Weapons.SHIELD;
	double last_time = engine::os_get_current_time_in_seconds();

	int target_render_width = 320;
	float zoom = (float)engine::window.width / (float)target_render_width;
	float scaled_render_height = (float)engine::window.height / zoom;

	float half_width = target_render_width * 0.5f;
	float half_height = scaled_render_height * 0.5f;

	float fps_limit = 144;
    float min_frametime = 1.0 / fps_limit;
 	while (!engine::window.should_close)
    {
		double now = engine::os_get_current_time_in_seconds();
        double delta_t = now - last_time;
		

		if (delta_t < min_frametime)
        {
            engine::os_high_precision_sleep((min_frametime - delta_t) * 1000.0);
            now = engine::os_get_current_time_in_seconds();
            delta_t = now - last_time;
        }

		last_time = now;

		engine::reset_temporary_storage();


		utils::animate_v2_to_target(&game.camera_pos, game.player_entity.position, delta_t, 10.0f);
		engine::draw_frame.projection = engine::m4_make_orthographic_projection(-half_width, half_width, -half_height, half_height, -1, 10);
		engine::draw_frame.view = engine::m4_make_scale(@v3(1, 1, 1));
		engine::draw_frame.view = engine::m4_mul(engine::draw_frame.view, engine::m4_make_translation( @v3(game.camera_pos.x, game.camera_pos.y, 0)));


		world_frame.world_mouse_pos = utils::screen_to_world();
			
		Vector2 player_input = @v2(0, 0);
		if (is_key_down(InputKeyCode.KEY_ARROW_UP.value) || is_key_down((int)'W'))
		{
			player_input.y += 1;

		}

		if (is_key_down(InputKeyCode.KEY_ARROW_DOWN.value) || is_key_down((int)'S'))
		{
			player_input.y -= 1;

		}

		if (is_key_down(InputKeyCode.KEY_ARROW_RIGHT.value) || is_key_down((int)'D'))
		{
			player_input.x += 1;

		}

		if (is_key_down(InputKeyCode.KEY_ARROW_LEFT.value) || is_key_down((int)'A'))
		{
			player_input.x -= 1;
		}


		Vector2 room_size = @v2(272, 156);
		float room_left_x = -(room_size.x / 2.0);
		float room_left_y = -(room_size.y / 2.0);

		float room_right_x = (room_size.x / 2.0) - 16;
		float room_right_y = (room_size.y / 2.0) - 6;

		player_input = engine::v2_normalize(player_input);
		

		game.player_entity.flip_x = world_frame.world_mouse_pos.x < game.player_entity.position.x + entity::SPRITE_PIXEL_SIZE * 0.5f;
		Vector2 player_velocity = engine::v2_mulf(player_input, game.player_entity.speed * (float)delta_t);
		// Vector2 potential_pos = engine::v2_add(game.player_entity.position, engine::v2_mulf(player_input, game.player_entity.speed * (float)delta_t));
		Vector2 potential_pos = game.player_entity.position.add(player_velocity);
		potential_pos.x = math::clamp(potential_pos.x, room_left_x, room_right_x);
		potential_pos.y = math::clamp(potential_pos.y, room_left_y, room_right_y);
		game.player_entity.position = potential_pos;

		entity::update_weapon_logic(&game, player_velocity, delta_t);


		Matrix4 walls_xform = engine::m4_scalar(1.0f);
		Vector2 walls_size = utils::get_image_size(game.walls);
		walls_xform = engine::m4_translate(walls_xform, @v3(-walls_size.x * 0.5f, -walls_size.y * 0.5f, 0.0f));
		engine::draw_image_xform(game.walls, walls_xform, walls_size, engine::COLOR_WHITE);

		Matrix4 player_xform = entity::render_player(&game.player_entity, game.player_sprites);
		entity::render_player_weapons(&game.player_entity, player_xform, game.weapon_sprites, delta_t);


		particles::update_and_draw_particles(game.particle_sprites, &game.particles, delta_t);


		if (is_key_just_released((int)'E'))
		{
			// io::printn("FPS: %.2f", 1.0 / delta_t);
			// io::printn("ms: %.2f", delta_t * 1000.0);
		}


		engine::os_update();
        engine::gfx_update();
	}
	return 0;
}
