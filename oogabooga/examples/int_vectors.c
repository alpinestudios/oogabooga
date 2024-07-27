#define CAMERA_SCALE 100.0
#define MAP_WIDTH 300
#define MAP_HEIGHT 200
#define MAX_LIST_SIZE 1024
#define MIN_ROOM_COUNT (5 + 10)

typedef enum block_archetype_t {
    BLOCK_empty,
    BLOCK_floor,
    BLOCK_wall,
    BLOCK_MAX,
} block_archetype_t;

typedef struct world_t {
    block_archetype_t world_map[MAP_WIDTH * MAP_HEIGHT];
} world_t;

world_t* world = 0;

typedef struct aabb_room_t {
    Vector2i center;    
    Vector2i size;    
} aabb_room_t;

struct {
    int32_t map_width;
    int32_t map_height;

    int32_t min_tunnel_count; 
    int32_t max_tunnel_count; 

    int32_t min_tunnel_size; 
    int32_t max_tunnel_size; 

    int32_t min_room_size; 
    int32_t max_room_size; 

    int32_t min_rooms_per_parent;
    int32_t max_rooms_per_parent;

    int32_t distance_between_tunnels;
} gen_conf;

struct {
    size_t attempts_per_room;
} gen_loc_conf;

struct {
    aabb_room_t global_bounds;
    size_t current_index;
    aabb_room_t rooms[MAX_LIST_SIZE];
} room_list;

void generator_init(void) {
    gen_conf.min_tunnel_count = 2;
    gen_conf.max_tunnel_count = 5;

    gen_conf.min_tunnel_size = 6;
    gen_conf.max_tunnel_size = 8;

    gen_conf.min_room_size = 10;
    gen_conf.max_room_size = 30;
    gen_conf.distance_between_tunnels = 15;

    gen_conf.min_rooms_per_parent = 0;
    gen_conf.max_rooms_per_parent = 10;

    gen_conf.map_width = MAP_WIDTH;
    gen_conf.map_height = MAP_HEIGHT;
}

bool check_collision(aabb_room_t a, aabb_room_t b) {
    bool collide_x = absi(a.center.x - b.center.x) < (a.size.x + b.size.x);
    bool collide_y = absi(a.center.y - b.center.y) < (a.size.y + b.size.y);

    return (collide_x && collide_y);
}

bool room_contains_coord(aabb_room_t container, int32_t x, int32_t y) {
    bool contains_x = absi(container.center.x - x) < container.size.x;
    bool contains_y = absi(container.center.y - y) < container.size.y;

    return (contains_x && contains_y);
}

void generate_tunnels(void) {
    int tunnels_count = get_random_int_in_range(gen_conf.min_tunnel_count, gen_conf.max_tunnel_count);

    for (size_t i = 0; i < tunnels_count; i++) {
        aabb_room_t* room = &(room_list.rooms[room_list.current_index++]);
        bool is_vertical = i % 2;

        Vector2i position = { 0 };
        position.x = get_random_int_in_range(gen_conf.max_tunnel_size, gen_conf.map_width - gen_conf.max_tunnel_size);
        position.y = get_random_int_in_range(gen_conf.max_tunnel_size, gen_conf.map_height - gen_conf.max_tunnel_size);

        Vector2i size = { 0 };
        size.x = get_random_int_in_range(gen_conf.min_tunnel_size, gen_conf.max_tunnel_size) / 2;
        size.y = get_random_int_in_range(gen_conf.min_tunnel_size, gen_conf.max_tunnel_size) / 2;

        if (is_vertical) {
            position.y = gen_conf.map_height / 2;
            size.y = gen_conf.map_height / 2 - 1;
        } else {
            position.x = gen_conf.map_width / 2;
            size.x = gen_conf.map_width / 2 - 1;
        }

        *room = (aabb_room_t) {
            .center = position,
            .size = size, 
        };

        for (size_t j = 0; j < i; j++) {
            if ((j % 2) != (i % 2))
                continue;

            aabb_room_t collision = {
                .center = position,
                .size = v2i(size.x + gen_conf.distance_between_tunnels, 
                            size.y + gen_conf.distance_between_tunnels)
            };

            if (check_collision(collision, room_list.rooms[j])) {
                room_list.current_index--;
                i--;
                break;
            }
        }

        if (room_list.current_index >= MAX_LIST_SIZE) {
            break;
        }
    }
}

void bound_room_hor(aabb_room_t* result, aabb_room_t parent) {
    int32_t side = get_random_int_in_range(0, 100);

    int32_t y;
    if (side > 50) { 
        y = parent.center.y + parent.size.y + result->size.y + 1;
    } else {
        y = parent.center.y - parent.size.y - result->size.y - 1;
    }

    result->center.x = get_random_int_in_range(parent.center.x - parent.size.x + result->size.x, 
                                               parent.center.x + parent.size.x - result->size.x);
    result->center.y = y;
}
void bound_room_vert(aabb_room_t* result, aabb_room_t parent) {
    int32_t side = get_random_int_in_range(0, 100);
    
    int32_t x;

    if (side > 50) { 
        x = parent.center.x + parent.size.x + result->size.x + 1;
    } else {
        x = parent.center.x - parent.size.x - result->size.x - 1;
    }

    result->center.y = get_random_int_in_range(parent.center.y - parent.size.y + result->size.y,
                                               parent.center.y + parent.size.y - result->size.y);
    result->center.x = x;
}

void create_room_on_bounds(aabb_room_t parent) {
    int32_t attempts = gen_loc_conf.attempts_per_room;

    aabb_room_t* room = &(room_list.rooms[room_list.current_index++]);

    while (attempts-- > 0) {
        bool is_vertical = get_random_int_in_range(0, 1);

        *room = (aabb_room_t) { 
            .center = (Vector2i){ 0 },
            .size = v2i(get_random_int_in_range(gen_conf.min_room_size, gen_conf.max_room_size) / 2, 
                        get_random_int_in_range(gen_conf.min_room_size, gen_conf.max_room_size) / 2)
        };

        if (is_vertical) bound_room_vert(room, parent);
        else             bound_room_hor(room,  parent);

        if (!room_contains_coord(room_list.global_bounds, room->center.x, room->center.y)) {
            continue;
        }

        bool collided = false;

        for (size_t i = 0; i < (room_list.current_index - 1); i++) {
            if (check_collision(*room, room_list.rooms[i])) {
                collided = true;
                break;
            }
        }

        if (collided) continue;

        return;
    }

    room_list.current_index--;
}

void generate_child_rooms(void) {
    size_t child_start = room_list.current_index;

    if (room_list.current_index >= MAX_LIST_SIZE)
        return;

    for (size_t parent_id = 0; parent_id < child_start; parent_id++) {
        if (room_list.current_index >= MAX_LIST_SIZE)
            break;

        int child_count = get_random_int_in_range(gen_conf.min_rooms_per_parent, gen_conf.max_rooms_per_parent);

        for (size_t child_id = 0; child_id < child_count; child_id++) {
            create_room_on_bounds(room_list.rooms[parent_id]);
            if (room_list.current_index >= MAX_LIST_SIZE)
                break;
        }
    }
}

void generator_clear(void) {
    for (int32_t i = 0; i < (gen_conf.map_width * gen_conf.map_height); i++) {
        world->world_map[i] = BLOCK_empty;
    }
    room_list.current_index = 0;
}

void generator_draw_room(aabb_room_t room) {
    Vector2i start = v2i_sub(room.center, room.size);
    Vector2i end = v2i_add(room.center,   room.size);

    for (int32_t y = start.y; y < end.y; y++) {
        for (int32_t x = start.x; x < end.x; x++) {
            if ((x < 0 || x > gen_conf.map_width) || (y < 0 || y > gen_conf.map_height))
                continue;

            block_archetype_t block = BLOCK_floor;

            if (x == start.x || x == (end.x - 1)) {
                if (world->world_map[x + y * gen_conf.map_width] == BLOCK_empty) {
                    block = BLOCK_wall;
                }
            }

            if (y == start.y || y == (end.y - 1)) {
                if (world->world_map[x + y * gen_conf.map_width] == BLOCK_empty) {
                    block = BLOCK_wall;
                }
            }

            world->world_map[x + y * gen_conf.map_width] = block;
        }
    }
}

void generate_map(void) {
    room_list.global_bounds = (aabb_room_t) { 
        .center = v2i(gen_conf.map_width  / 2, gen_conf.map_height / 2),
        .size   = v2i(gen_conf.map_width  / 2 - gen_conf.max_room_size / 2,
                      gen_conf.map_height / 2 - gen_conf.max_room_size / 2)
    };

    generator_clear();

    while (room_list.current_index < MIN_ROOM_COUNT) {
        generator_clear();
        generate_tunnels();
        gen_loc_conf.attempts_per_room = 1000;
        generate_child_rooms();
        gen_loc_conf.attempts_per_room = 100;
        generate_child_rooms();
        gen_loc_conf.attempts_per_room = 10;
        generate_child_rooms();
    }

    for (size_t i = room_list.current_index; i > 0; i--) {
        size_t index = i - 1;
        aabb_room_t room = room_list.rooms[index];

        generator_draw_room(room);
    }
}

int entry(int argc, char **argv) {
	window.title = STR("Int Vectors Example");
	window.scaled_width = 1280; 
	window.scaled_height = 720; 
	window.x = 200;
	window.y = 90;
	window.clear_color = hex_to_rgba(0x6495EDff);

    world = alloc(get_heap_allocator(), sizeof(world_t));

    generator_init();
    generate_map();

    float scale = CAMERA_SCALE;

	while (!window.should_close) {
		reset_temporary_storage();
        reset_draw_frame(&draw_frame);
        draw_frame.view = m4_make_scale(v3(scale, scale, scale));

        if (is_key_just_pressed(KEY_F7)) {
            generate_map();
        }

        for (size_t i = 0; i < (MAP_WIDTH * MAP_HEIGHT); i++) {
            int32_t x = i % MAP_WIDTH;
            int32_t y = i / MAP_WIDTH;

            switch (world->world_map[i]) {
                case BLOCK_floor: 
                    draw_rect(v2(x - MAP_WIDTH / 2, y - MAP_HEIGHT / 2), v2(1, 1), hex_to_rgba(0x3f3f3fff));
                    break;
                case BLOCK_wall: 
                    draw_rect(v2(x - MAP_WIDTH / 2, y - MAP_HEIGHT / 2), v2(1, 1), hex_to_rgba(0x7f7f7fff));
                    break;
                default:
                    break;
            }
        }
        os_update(); 
        gfx_update();
    }

	return 0;
}
