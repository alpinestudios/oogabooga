
module particles;
import engine;
import utils;
import std::collections;
import std::io;
import game;

def Vector2 = engine::Vector2;


struct Projectile {
    bool active;
    Vector2 position;
    Vector2 velocity;
    int sprite_cell_start_x;
	int sprite_cell_start_y;
    bool player_projectile;
}

struct Particle {
    bool active;
	int current_frame;
    int animation_frame_count;
	int sprite_cell_start_x;
	int sprite_cell_start_y;
    float time_per_frame;
    float current_time;
    Vector2 position;
    Vector2 velocity;
    float rotation;
}


fn void update_and_draw_projectiles(engine::GfxImage *sprite, game::Game *game, double delta_t) {
    foreach (&proj : game.projectiles)
    {   
        if (!proj.active) {
            continue;
        }


        proj.position.x+= proj.velocity.x * (float)delta_t;
        proj.position.y+= proj.velocity.y * (float)delta_t;
        engine::Matrix4 xform = engine::m4_scalar(1.0);
        xform  = engine::m4_translate(xform, {{ proj.position.x, proj.position.y, 0 }});
		xform  = engine::m4_translate(xform, {{-16 / 2, -16 / 2, 0}});

        Vector2 bottom_left = game.player_entity.position.add(game.player_entity.hitbox_offset);

		// engine::draw_circle_xform(xform, {{ 16, 16 }}, {{ 1, 0, 1, 1 }});
        if (utils::rect_circle_collision(bottom_left, game.player_entity.hitbox_size, proj.position, 4)) {
            proj.active = false;
        };


        DrawQuad* quad = engine::draw_image_xform(
            sprite,
            xform,
            {{ 16, 16 }},
            engine::COLOR_WHITE
        );
        quad.uv = utils::get_uv_coords(
                utils::get_image_size(sprite),
                {{ proj.sprite_cell_start_x, proj.sprite_cell_start_y }},
                16,
                {{ 3, 1 }}
        );
    }

    game.projectiles.remove_if(fn (p) => !p.active);

}

fn void update_and_draw_particles(engine::GfxImage *sprite, List(<Particle>)* particles, double delta_t) {
    foreach (&particle : particles)
    {   
        if (!particle.active) {
            continue;
        }

        particle.current_time+= (float)delta_t;

        if (particle.current_time >= particle.time_per_frame) {
            particle.current_time = 0.0f;
            if (particle.current_frame >= particle.animation_frame_count - 1) {
                particle.active = false;
            } else {
                particle.current_frame+= 1;
            }   
        }



        particle.position.x+= particle.velocity.x * (float)delta_t;
        particle.position.y+= particle.velocity.y * (float)delta_t;
        engine::Matrix4 xform = engine::m4_scalar(1.0);
        xform  = engine::m4_translate(xform, {{ particle.position.x, particle.position.y, 0 }});
        
        xform = engine::m4_translate(xform, {{16 / 2, 16 / 2, 0}});
        xform  = engine::m4_rotate_z(xform, particle.rotation);
        xform = engine::m4_translate(xform, {{-16 / 2, -16 / 2, 0}});



        DrawQuad* quad = engine::draw_image_xform(
            sprite,
            xform,
            {{ 16, 16 }},
            engine::COLOR_WHITE
        );
        quad.uv = utils::get_uv_coords(
                utils::get_image_size(sprite),
                {{ particle.sprite_cell_start_x + (float)particle.current_frame, particle.sprite_cell_start_y }},
                16,
                {{ 3, 1 }}
        );
    }

    particles.remove_if(fn (p) => !p.active);

}