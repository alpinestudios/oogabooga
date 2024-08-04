import engine;
import entity;


def Matrix4 = engine::Matrix4;

macro @v3(x, y, z) {
    return engine::Vector3 {{ x, y, z }};
}

macro @v2(x, y) {
    return engine::Vector2 {{ x, y }};
}


fn void draw_health_bar(Matrix4 xform, int max_hp, int current_hp) {
    xform = engine::m4_translate(xform, {{0, entity::SPRITE_PIXEL_SIZE, 0}});
    engine::draw_rect_xform(xform, @v2(entity::SPRITE_PIXEL_SIZE + 3, 3), {{ 0, 0, 0, 1 }});

    float hp_bar_width = entity::SPRITE_PIXEL_SIZE;

    float percentage = (float)current_hp / (float)max_hp; 

    engine::draw_rect_xform(xform, @v2(entity::SPRITE_PIXEL_SIZE * percentage, 2), {{ 1, 0, 0, 1 }});

}