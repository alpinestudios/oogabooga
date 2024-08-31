
/*

	In this example we make use of a separate Draw_Frame to render it to a render target image.
	
	The global draw_frame is rendered to window and cleared each gfx_update().
	
	So if we want more control to be able to do offscreen rendering, we need to make a separate Draw_Frame
	and manually reset it & gfx_clear it before we draw to it and finally render it to a render target.
	
	To showcase this, I generate 3 worlds (immediate-mode generated with a consistent base seed for random),
	and draw each of them to their own world image which lets us easily hop between worlds and draw the
	final world image of the current world to the window, as well as draw the whole world image in a minimap
	in the top left.
	The minimaps also has a red blip showing the player position.
	
	WASD - Move the player
	
	Left & Right arrow keys - hop between worlds

*/

typedef enum World {
	WORLD1,
	WORLD2,
	WORLD3,
	
	WORLD_MAX,
} World;

int entry(int argc, char **argv) {
	
	window.title = STR("Offscreen Rendering Example");
	
	// Configuration of world generation
	u64 number_of_things_per_world = 3000;
	Vector2 world_min = v2(-3000, -3000);
	Vector2 world_max = v2( 3000,  3000);
	Vector2 world_size = v2(ceil(world_max.x-world_min.x), ceil(world_max.y-world_min.y));
	Vector4 terrain_colors[WORLD_MAX] = {
		v4(.5, .5, .5, 1),
		COLOR_BLACK,
		COLOR_WHITE,
	};
	Vector4 thing_colors[WORLD_MAX] = {
		v4(0, 0, 1, 1),
		COLOR_WHITE,
		COLOR_BLACK,
	};
	
	// Init world images & world draw frame
	Gfx_Image *world_images[WORLD_MAX];
	Draw_Frame world_draw_frame;
	
	for (World world = WORLD1; world < WORLD_MAX; world += 1) {
		world_images[world] = make_image_render_target(world_size.x, world_size.y, 4, 0, get_heap_allocator());
	}
	
	draw_frame_init_reserve(&world_draw_frame, number_of_things_per_world);
	
	// Load player sprite
	Gfx_Image *player_sprite = load_image_from_disk(STR("oogabooga/examples/player.png"), get_heap_allocator());
	assert(player_sprite, "Failed loading player sprite");
	
	
	// Randomize the base seed with current system cycle count
	u64 base_seed = rdtsc();
	
	Vector2 player_pos = v2(0, 0);
	Vector2 camera_pos = v2(0, 0);
	World current_world = WORLD1;
	
	float64 last_time = os_get_elapsed_seconds();
	while (!window.should_close) {
		reset_temporary_storage();
		float64 now = os_get_elapsed_seconds();
		float64 delta_time = now-last_time;
		last_time = now;
		
		///
		// Update each world
		//
		for (World world = WORLD1; world < WORLD_MAX; world += 1) {
		
			u64 seed = base_seed + world;
			
			Vector4 clear_color = terrain_colors[world];
			Vector4 thing_color = thing_colors[world];
			
			// Reset draw frame and clear the render target
			draw_frame_reset(&world_draw_frame);
			gfx_clear_render_target(world_images[world], clear_color);
			
			// We are drawing the whole world into one big texture, so the orthographic projection should be
			// the same size as the world.
			world_draw_frame.projection 
				= m4_make_orthographic_projection(-world_size.x/2, world_size.x/2, -world_size.y/2, world_size.y/2, -1, 10);
			
			// Start at the first seed for this world (this seed advances for each get_random_xxx call you make.
			seed_for_random = seed;
			
			//
			// Draw all the things in the world to the world image
			for (u64 i = 0; i < number_of_things_per_world; i += 1) {
				
				Vector2 pos = v2(
					get_random_float32_in_range(world_min.x, world_max.x),
					get_random_float32_in_range(world_min.y, world_max.y)
				);
				
				Vector2 size = v2(
					get_random_float32_in_range(16, 64),
					get_random_float32_in_range(16, 64)
				);
				
				switch (world) {
					// In world 1 and 3 the things are rectangles
					case WORLD1: 
					case WORLD3:
						draw_rect_in_frame(pos, size, thing_color, &world_draw_frame); break;
					
					// In world 2 the shapes are circles
					case WORLD2: 
						draw_circle_in_frame(pos, v2(size.x, size.x), thing_color, &world_draw_frame); break;
					
					case WORLD_MAX: panic("");
				}
			}
			
			// Since we have draw to a custom Draw_Frame (not the global one), we need to manually specify
			// when to render it and where to render it. Here we render it to the world image.
			// You can also render Draw_Frame's to the window directly with gfx_render_draw_frame_to_window()
			gfx_render_draw_frame(&world_draw_frame, world_images[world]);
		}
		
		// 
		// Determine what the next and previous worlds are depending on the current
		World next_world;
		World previous_world;
		
		switch (current_world) {
			case WORLD1: {
				previous_world = WORLD3;
				next_world = WORLD2;
				break;
			}
			case WORLD2: {
				previous_world = WORLD1;
				next_world = WORLD3;
				break;
			}
			case WORLD3: {
				previous_world = WORLD2;
				next_world = WORLD1;
				break;
			}
			
			case WORLD_MAX: panic("");
		}
		if (is_key_just_pressed(KEY_ARROW_LEFT))  current_world = previous_world;
		if (is_key_just_pressed(KEY_ARROW_RIGHT)) current_world = next_world;

		
		///
		// Basic player and camera movement
		//
		
		camera_pos = v2_smerp(camera_pos, player_pos, 0.07);
		
		camera_pos.x = min(camera_pos.x, world_max.x-window.width /2.0);
		camera_pos.x = max(camera_pos.x, world_min.x+window.width /2.0);
		camera_pos.y = min(camera_pos.y, world_max.y-window.height/2.0);
		camera_pos.y = max(camera_pos.y, world_min.y+window.height/2.0);
		
		draw_frame.camera_xform = m4_make_translation(v3(v2_expand(camera_pos), 0.0));
		
		// Draw the current world in the center of everything
		draw_image(world_images[current_world], v2(-world_size.x/2, -world_size.y/2), v2(world_size.x, world_size.y), COLOR_WHITE);
		
		Vector2 move_axes = v2(0, 0);
		if (is_key_down('A')) move_axes.x -= 1.0;
		if (is_key_down('D')) move_axes.x += 1.0;
		if (is_key_down('S')) move_axes.y -= 1.0;
		if (is_key_down('W')) move_axes.y += 1.0;
		
		move_axes = v2_normalize(move_axes);
		
		player_pos = v2_add(player_pos, v2_mulf(move_axes, 200*delta_time));
		
		// Draw the player
		draw_image(player_sprite, player_pos, v2(player_sprite->width*4, player_sprite->height*4), COLOR_WHITE);
		
	
		// Reset camera for rendering the minimaps, so they are always the same position in the window.
		draw_frame.camera_xform = m4_identity();
		
		///
		// Draw the minimaps
		//
		for (World world = WORLD1; world < WORLD_MAX; world += 1) {
			float32 minimap_w = 256;
			float32 minimap_h = 256;
			float32 minimap_x = -window.width/2+40 + world*(minimap_w+30);
			float32 minimap_y =  window.height/2-minimap_h-40;
			
			// Red Background (borders for current world)
			if (world == current_world) 
				draw_rect(v2(minimap_x-10, minimap_y-10), v2(minimap_w+20, minimap_h+20), COLOR_RED);
				
			// The minimap
			draw_image(world_images[world], v2(minimap_x, minimap_y), v2(minimap_w, minimap_h), COLOR_WHITE);
			
			// Draw a blip representing the player on the minimap for the current world
			if (world == current_world) {
				float32 minimap_center_x = minimap_x + minimap_w/2;
				float32 minimap_center_y = minimap_y + minimap_h/2;
				float32 normx = player_pos.x/((world_max.x-world_min.x)/2);
				float32 normy = player_pos.y/((world_max.y-world_min.y)/2);
				Vector2 pos_on_minimap = v2(minimap_center_x+normx*(minimap_w/2.0), minimap_center_y+normy*(minimap_h/2.0));
				draw_circle(v2_sub(pos_on_minimap, v2(3, 3)), v2(6, 6), COLOR_RED);
			}
		}
		
		
		os_update(); 
		gfx_update();
	}

	return 0;
}