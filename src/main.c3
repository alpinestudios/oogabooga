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

const Vector2 ROOM_SIZE = {{ 272, 156 }};

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


macro @create_bat() {
	Entity ent;
	ent.active = true;
	ent.attack_timer = 3f;
	ent.current_animation_frame = 0;
	ent.idle_animation_frame_start = 0;
    ent.idle_animation_count = 4;
    ent.idle_animation_play_time = 0.1f;
	ent.speed = 1.3f;
    ent.entity_type = EntType.BAT;
	ent.sprite_cell_count_x = 4;
	ent.sprite_cell_count_y = 4;
	ent.max_health = 20;
	ent.health = 20;
	ent.position = {{ 
		engine::get_random_float32_in_range(-ROOM_SIZE.x * 0.5f, ROOM_SIZE.x * 0.5f),
		engine::get_random_float32_in_range(-ROOM_SIZE.y * 0.5f, ROOM_SIZE.y * 0.5f)
	}};

	return ent;
}


def is_key_down = engine::is_key_down;
def is_key_just_released = engine::is_key_just_released;
def InputKeyCode = engine::InputKeyCode;

def Entity = entity::Entity;

struct Game {
    engine::Vector2 camera_pos;
	List(<entity::Entity>) room_enemies;
    Entity player_entity;
    engine::GfxFont *font;
	List(<particles::Particle>) particles;
	List(<particles::Projectile>) projectiles;
}

struct WorldFrame {
	Vector2 world_mouse_pos;
}

def GfxImage = engine::GfxImage;
struct Sprites {
	GfxImage *player;
	GfxImage *weapons;
	GfxImage *walls;
	GfxImage *particles;
	GfxImage *enemies;
	GfxImage *projectiles;
}

struct Cbuffer {
	Vector2 mouse_pos_screen; // We use this to make a light around the mouse cursor
	Vector2 window_size;
}

WorldFrame world_frame = {};

Sprites sprites = {};

bool draw_hitboxes = false;

fn int main(String[] args)
{

	engine::oogabooga_init(5 * 1024 * 1024);

	CString source;
	bool ok = engine::os_read_entire_file_s(@str("./shaders/shader.hlsl"), &source, engine::get_heap_allocator());
	assert(ok, "Could not read ./shaders/shader.hlsl");

	engine::shader_recompile_with_extension(source, Cbuffer.sizeof);
	engine::dealloc_string(engine::get_heap_allocator(), source);

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
	engine::seed_for_random = engine::os_get_current_cycle_count();
	Game game = {};
	
	sprites.weapons = engine::load_image_from_disk(@str("./assets/weapons.png"), engine::get_heap_allocator());
    sprites.player = engine::load_image_from_disk(@str("./assets/player.png"), engine::get_heap_allocator());
    sprites.walls = engine::load_image_from_disk(@str("./assets/walls.png"), engine::get_heap_allocator());
    sprites.particles = engine::load_image_from_disk(@str("./assets/particles.png"), engine::get_heap_allocator());
    sprites.enemies = engine::load_image_from_disk(@str("./assets/enemies.png"), engine::get_heap_allocator());
    sprites.projectiles = engine::load_image_from_disk(@str("./assets/projectiles.png"), engine::get_heap_allocator());



	game.player_entity.speed = 100;
	game.player_entity.right_hand_weapon = entity::Weapons.SWORD;
	game.player_entity.left_hand_weapon = entity::Weapons.SHIELD;
	game.player_entity.walk_animation_count = 6;
	game.player_entity.walk_animation_frame_start = 1;
	game.player_entity.walk_animation_play_time = 0.08f;
	game.player_entity.hitbox_size = {{ 8, 8 }};
	game.player_entity.hitbox_offset = {{ 4, 4 }};
	game.player_entity.strength = 2;



	game.room_enemies.push(@create_bat());
	game.room_enemies.push(@create_bat());

	double last_time = engine::os_get_current_time_in_seconds();

	int target_render_width = 320;
	float zoom = (float)engine::window.width / (float)target_render_width;
	float scaled_render_height = (float)engine::window.height / zoom;

	float half_width = target_render_width * 0.5f;
	float half_height = scaled_render_height * 0.5f;

	float fps_limit = 144;
    float min_frametime = 1.0 / fps_limit;


	Cbuffer cbuffer;

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
		engine::draw_frame.cbuffer = &cbuffer;

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


		
		float room_left_x = -(ROOM_SIZE.x / 2.0);
		float room_left_y = -(ROOM_SIZE.y / 2.0);

		float room_right_x = (ROOM_SIZE.x / 2.0) - 16;
		float room_right_y = (ROOM_SIZE.y / 2.0) - 6;

		player_input = engine::v2_normalize(player_input);
		

		game.player_entity.flip_x = world_frame.world_mouse_pos.x < game.player_entity.position.x + entity::SPRITE_PIXEL_SIZE * 0.5f;
		Vector2 player_velocity = engine::v2_mulf(player_input, game.player_entity.speed * (float)delta_t);
		Vector2 potential_pos = game.player_entity.position.add(player_velocity);
		potential_pos.x = math::clamp(potential_pos.x, room_left_x, room_right_x);
		potential_pos.y = math::clamp(potential_pos.y, room_left_y, room_right_y);
		game.player_entity.position = potential_pos;

		if (player_input.x != 0 || player_input.y != 0) {
			game.player_entity.animation = EntAnimation.WALK;
		} else {
			game.player_entity.animation = EntAnimation.IDLE;
		}

		entity::update_player_animation(&game.player_entity, delta_t);

		entity::update_weapon_logic(&game, player_velocity, delta_t);


		foreach (&enemy : game.room_enemies)
    	{	   
			entity::update_entity(enemy, &game, delta_t);
		}

		game.room_enemies.remove_if(fn (p) => !p.active);
		


		Matrix4 walls_xform = engine::m4_scalar(1.0f);
		Vector2 walls_size = utils::get_image_size(sprites.walls);
		walls_xform = engine::m4_translate(walls_xform, @v3(-walls_size.x * 0.5f, -walls_size.y * 0.5f, 0.0f));
		engine::draw_image_xform(sprites.walls, walls_xform, walls_size, engine::COLOR_WHITE);

		Matrix4 player_xform = entity::render_player(&game.player_entity, sprites.player);
		entity::render_player_weapons(&game.player_entity, player_xform, sprites.weapons, delta_t);

		foreach (enemies : game.room_enemies)
    	{	   
			entity::render_entity(enemies);
		}
		

		particles::update_and_draw_projectiles(sprites.projectiles, &game, delta_t);
		particles::update_and_draw_particles(sprites.particles, &game.particles, delta_t);





		if (draw_hitboxes) {

			
			foreach (enemies : game.room_enemies)
			{	   
				entity::render_entity(enemies);
			}
		}


		if (is_key_just_released((int)'E'))
		{
			draw_hitboxes = !draw_hitboxes;
			// io::printn("FPS: %.2f", 1.0 / delta_t);
			// io::printn("ms: %.2f", delta_t * 1000.0);
		}


		engine::os_update();
        engine::gfx_update();
	}
	return 0;
}
