
typedef enum EntityArchetype {
	arch_nil = 0,
	arch_rock = 1,
	arch_tree = 2,
	arch_player = 3,
} EntityArchetype;

typedef struct Entity {
	bool is_valid;
	EntityArchetype arch;
	Vector2 pos;
} Entity;
#define MAX_ENTITY_COUNT 1024

typedef struct World {
	Entity entities[MAX_ENTITY_COUNT];
} World;
World* world = 0;

Entity* entity_create() {
	Entity* entity_found = 0;
	for (int i = 0; i < MAX_ENTITY_COUNT; i++) {
		Entity* existing_entity = &world->entities[i];
		if (!existing_entity->is_valid) {
			entity_found = existing_entity;
			break;
		}
	}
	assert(entity_found, "No more free entities!");
	entity_found->is_valid = true;
	return entity_found;
}

void entity_destroy(Entity* entity) {
	memset(entity, 0, sizeof(Entity));
}

void setup_rock(Entity* en) {
	en->arch = arch_rock;
	// ...
}

int entry(int argc, char **argv) {
	
	window.title = STR("Randy's Game");
	window.width = 1280;
	window.height = 720;
	window.x = 200;
	window.y = 200;
	window.clear_color = hex_to_rgba(0x2a2d3aff);

	world = alloc(get_heap_allocator(), sizeof(World));
	memset(world, 0, sizeof(World));
	
	Gfx_Image* player = load_image_from_disk(STR("player.png"), get_heap_allocator());
	assert(player, "fuckie wucky happen");
	Gfx_Image* tree0 = load_image_from_disk(STR("tree0.png"), get_heap_allocator());
	assert(tree0, "fuckie wucky happen");
	Gfx_Image* tree1 = load_image_from_disk(STR("tree1.png"), get_heap_allocator());
	assert(tree1, "fuckie wucky happen");
	Gfx_Image* rock0 = load_image_from_disk(STR("rock0.png"), get_heap_allocator());
	assert(rock0, "fuckie wucky happen");

	Entity* player_en = entity_create();

	for (int i = 0; i < 10; i++) {
		Entity* en = entity_create();
		setup_rock(en);
		en->pos = v2(i * 10.0, 0.0);
	}

	float64 seconds_counter = 0.0;
	s32 frame_count = 0;

	float64 last_time = os_get_current_time_in_seconds();
	while (!window.should_close) {
		reset_temporary_storage();

		draw_frame.projection = m4_make_orthographic_projection(window.width * -0.5, window.width * 0.5, window.height * -0.5, window.height * 0.5, -1, 10);
		float zoom = 5.3;
		draw_frame.view = m4_make_scale(v3(1.0/zoom, 1.0/zoom, 1.0));

		float64 now = os_get_current_time_in_seconds();
		float64 delta_t = now - last_time;
		last_time = now;
		
		os_update();

		for (int i = 0; i < MAX_ENTITY_COUNT; i++) {
			Entity* en = &world->entities[i];
			if (en->is_valid) {

				switch (en->arch) {

					case arch_rock:
					{
						Vector2 size = v2(7.0, 4.0);
						Matrix4 xform = m4_scalar(1.0);
						xform         = m4_translate(xform, v3(en->pos.x, en->pos.y, 0));
						xform         = m4_translate(xform, v3(size.x * -0.5, 0.0, 0));
						draw_image_xform(rock0, xform, size, COLOR_RED);
					}

					case arch_player:
					{

					}

					default: {}
				}

			}
		}

		if (is_key_just_pressed(KEY_ESCAPE)) {
			window.should_close = true;
		}

		Vector2 input_axis = v2(0, 0);
		if (is_key_down('A')) {
			input_axis.x -= 1.0;
		}
		if (is_key_down('D')) {
			input_axis.x += 1.0;
		}
		if (is_key_down('S')) {
			input_axis.y -= 1.0;
		}
		if (is_key_down('W')) {
			input_axis.y += 1.0;
		}
		input_axis = v2_normalize(input_axis);

		player_en->pos = v2_add(player_en->pos, v2_mulf(input_axis, 100.0 * delta_t));
		
		{
			Vector2 size = v2(6.0, 8.0);
			Matrix4 xform = m4_scalar(1.0);
			xform         = m4_translate(xform, v3(player_en->pos.x, player_en->pos.y, 0));
			xform         = m4_translate(xform, v3(size.x * -0.5, 0.0, 0));
			draw_image_xform(player, xform, size, COLOR_RED);
		}

		{
			Vector2 size = v2(7.0, 12.0);
			Matrix4 xform = m4_scalar(1.0);
			xform         = m4_translate(xform, v3(size.x * -0.5, 0.0, 0));
			draw_image_xform(tree0, xform, size, COLOR_RED);
		}
		
		gfx_update();
		seconds_counter += delta_t;
		frame_count += 1;
		if (seconds_counter > 1.0) {
			log("fps: %i", frame_count);
			seconds_counter = 0.0;
			frame_count = 0;
		}
	}

	return 0;
}