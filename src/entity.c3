module entity;

import engine;
import utils;
import std::math;
import game;
import std::io;
import particles;

enum Weapons {
	NIL,
	SWORD,
	SHIELD
}

macro @v3(x, y, z) {
    return engine::Vector3 {{ x, y, z }};
}

macro @v2(x, y) {
    return engine::Vector2 {{ x, y }};
}

const int SPRITE_PIXEL_SIZE = 16;

struct Entity
{
    bool active;
    bool flip_x;
    // Vector2 grid_pos;
    int health;
    int max_health;
    float speed;
    float weapon_angle;
    float current_weapon_angle_offset;
    bool flip_weapon;
    /* Player specific data, we will just set to -1 for enemy ents*/
    engine::Vector2 position;

	Weapons right_hand_weapon;
	Weapons left_hand_weapon;
}

def COLOR_WHITE = engine::COLOR_WHITE;
def Vector2 = engine::Vector2;
def Vector3 = engine::Vector3;
def Vector4 = engine::Vector4;
def Matrix4 = engine::Matrix4;
def m4_translate = engine::m4_translate;
def m4_rotate_z = engine::m4_rotate_z;
def InputKeyCode = engine::InputKeyCode;



fn engine::Matrix4 render_player(Entity *entity, engine::GfxImage* player_sprite) {
    Vector2 sprite_size = utils::get_image_size(player_sprite);
    Matrix4 player_xform = engine::m4_scalar(1.0f);
	player_xform = engine::m4_translate(player_xform, @v3(entity.position.x, entity.position.y, 0.0f));

    engine::DrawQuad *quad = engine::draw_image_xform(player_sprite, player_xform, @v2(SPRITE_PIXEL_SIZE, SPRITE_PIXEL_SIZE), engine::COLOR_WHITE);
	quad.uv = utils::get_uv_coords(sprite_size, @v2(0, 0), SPRITE_PIXEL_SIZE, 3, 1);
	return player_xform;
}



const float ANIMATE_DOWN_VALUE = 90;
const float ANIMATE_SPEED = 20;
fn void update_weapon_logic(Game *game, Vector2 player_velocity, double delta_t) {
    Entity *player = &game.player_entity;
    if (engine::is_key_just_pressed(InputKeyCode.MOUSE_BUTTON_LEFT.value)) {
        player.flip_weapon = !player.flip_weapon;

        float rotation_z = -utils::calc_rotation_to_target(game::world_frame.world_mouse_pos, player.position);
        float moveDistance = 15.0f;
        float delta_x = moveDistance * $$cos(rotation_z);
        float delta_y = moveDistance * $$sin(rotation_z);


        Particle particle;
        particle.active = true;
        particle.time_per_frame = 0.1;
        particle.rotation = rotation_z;
        particle.frames.new_init_with_array({0, 1, 2});
        particle.position = player.position.add({{ delta_x + player_velocity.x * 1.5f, -delta_y + player_velocity.y * 1.5f }});
        particle.velocity = engine::v2_mulf(game::world_frame.world_mouse_pos.normalize(), 50f);
        game.particles.push(
           particle
        );
    }

    if (player.flip_weapon) {
        utils::animate_f32_to_target(&player.current_weapon_angle_offset, utils::deg_to_rad(ANIMATE_DOWN_VALUE), (float)delta_t, ANIMATE_SPEED);
        // io::printn(player.current_weapon_angle_offset);
    } else {
        utils::animate_f32_to_target(&player.current_weapon_angle_offset, utils::deg_to_rad(0), (float)delta_t, ANIMATE_SPEED);
    }


    if (player.flip_x) {
        float angle_to_target = utils::calc_rotation_to_target(@v2(player.position.x + SPRITE_PIXEL_SIZE * 0.5f, player.position.y), game::world_frame.world_mouse_pos);
        player.weapon_angle = angle_to_target;
    } else {
        float angle_to_target = utils::calc_rotation_to_target(game::world_frame.world_mouse_pos, player.position);
        player.weapon_angle = -angle_to_target;
    }
}


bool attack_down = false;
float offset_angle = 0.0f;
float offset_angle_bump_amount = 5 * (math::PI / 180);

fn void render_player_weapons(Entity *player, Matrix4 player_xform, engine::GfxImage* weapons_sprite, double delta_t) {


    bool flip_y = utils::almost_equals(player.current_weapon_angle_offset, utils::deg_to_rad(ANIMATE_DOWN_VALUE), 1);

    Vector2 sprite_size = utils::get_image_size(weapons_sprite);
    if (player.right_hand_weapon != Weapons.NIL)
    {

        Matrix4 xform = player_xform;
        xform = m4_translate(xform, @v3(SPRITE_PIXEL_SIZE * 0.5f, SPRITE_PIXEL_SIZE * 0.5f, 0));

        float x_scale = 1.0;
        float y_scale = 1.0;


        if (player.flip_x)
        {
            x_scale = -1.0;
        }

        float angle = player.weapon_angle + player.current_weapon_angle_offset;

        if (flip_y) {
            y_scale = -1.0;
            angle = -angle;
        }

        xform = engine::m4_scale(xform, @v3(x_scale, y_scale, 1));

        xform = m4_rotate_z(xform, angle);

        if (player.right_hand_weapon == Weapons.SHIELD)
        {
            xform = m4_translate(xform, @v3(0, -SPRITE_PIXEL_SIZE * 0.5f, 0));
        }

        engine::DrawQuad *quad = engine::draw_image_xform(weapons_sprite, xform, @v2(SPRITE_PIXEL_SIZE, SPRITE_PIXEL_SIZE), engine::COLOR_WHITE);
        quad.uv = utils::get_uv_coords(sprite_size, @v2((int)player.right_hand_weapon, 0), SPRITE_PIXEL_SIZE, 3, 1);
    }

    if (player.left_hand_weapon != Weapons.NIL)
    {
        Matrix4 xform = player_xform;
        if (!player.flip_x)
        {
            xform = engine::m4_translate(xform, @v3(-4, 0, 0));
            xform = engine::m4_scale(xform, @v3(-1, 1, 1));
            xform = engine::m4_translate(xform, @v3(-SPRITE_PIXEL_SIZE, 0, 0));
        }
        else
        {
            xform = engine::m4_translate(xform, @v3(4, 0, 0));
        }

        engine::DrawQuad *quad = engine::draw_image_xform(weapons_sprite, xform, @v2(SPRITE_PIXEL_SIZE, SPRITE_PIXEL_SIZE), COLOR_WHITE);
        quad.uv = utils::get_uv_coords(sprite_size, @v2((int)player.left_hand_weapon, 0), SPRITE_PIXEL_SIZE, 3, 1);
    }
}
