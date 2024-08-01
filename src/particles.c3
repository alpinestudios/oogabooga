
module particles;
import engine;
import utils;
import std::collections;
import std::io;

def Vector2 = engine::Vector2;


struct Particle {
    bool active;
	int current_frame;
	int sprite_cell_start_x;
	int sprite_cell_start_y;
    float time_per_frame;
    float current_time;
    List(<int>) frames;
    Vector2 position;
    Vector2 velocity;
    float rotation;
}

fn void update_and_draw_particles(GfxImage *sprite, List(<Particle>)* particles, double delta_t) {


    foreach (&particle : particles)
    {   
        if (!particle.active) {
            continue;
        }

        particle.current_time+= (float)delta_t;

        if (particle.current_time >= particle.time_per_frame) {
            particle.current_time = 0.0f;
            if (particle.current_frame >= particle.frames.size - 1) {
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
                (int)particle.frames.size,
                1
        );
    }

    particles.remove_if(fn (p) => !p.active);

}