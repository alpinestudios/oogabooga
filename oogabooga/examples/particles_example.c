

#if !OOGABOOGA_ENABLE_EXTENSIONS
	#error particles_example.c requires OOGABOOGA_ENABLE_EXTENSIONS to be enabled
#endif
#if !OOGABOOGA_EXTENSION_PARTICLES
	#error particles_example.c requires OOGABOOGA_EXTENSION_PARTICLES to be enabled
#endif


#if OOGABOOGA_ENABLE_EXTENSIONS && OOGABOOGA_EXTENSION_PARTICLES

Emission_Config emission_rain;
Emission_Config emission_poof;
void setup_emission_configs();

int entry(int argc, char **argv) {
	
	// This is how we (optionally) configure the window.
	// To see all the settable window properties, ctrl+f "struct Os_Window" in os_interface.c
	window.title = STR("Particles example");
	
	setup_emission_configs();
	
	Gfx_Font *font = load_font_from_disk(STR("C:/windows/fonts/arial.ttf"), get_heap_allocator());
	assert(font, "Failed loading arial.ttf");
	
	// Keep a stack of the looped emissions so we can pop them
	Emission_Handle *emission_stack = 0;
	growing_array_init((void**)&emission_stack, sizeof(Emission_Handle), get_heap_allocator());
	
	// Particles spawn in contiguously meaning vbo will initially grow a little bit at a time
	// as we start drawing more quads, so let's just reserve a lot of bytes now instead.
	gfx_reserve_vbo_bytes(MB(16));
	
	float64 last_time = os_get_elapsed_seconds();
	while (!window.should_close) {
		reset_temporary_storage();
		
		float64 now = os_get_elapsed_seconds();
		float64 delta_time = now-last_time;
		
		last_time = now;
		
		draw_frame.projection = m4_make_orthographic_projection(window.pixel_width * -0.5, window.pixel_width * 0.5, window.pixel_height * -0.5, window.pixel_height * 0.5, -1, 10);
		
		float mx = input_frame.mouse_x - window.width/2;
        float my = input_frame.mouse_y - window.height/2;
		
		// Left click: Emit a rain thingy and add to stack of rain emissions
		if (is_key_just_pressed(MOUSE_BUTTON_LEFT)) {
			Emission_Handle h = emit_particles(emission_rain, v2(mx, my));
			growing_array_add((void**)&emission_stack, &h);
		}
		
		u64 num_emissions = growing_array_get_valid_count(emission_stack);
		// Pop emissions on right click
		if (is_key_just_pressed(MOUSE_BUTTON_RIGHT) && num_emissions > 0) {
			Emission_Handle h = emission_stack[num_emissions-1];
			
			emission_release(h);
			growing_array_pop((void**)&emission_stack);
		}
		
		// Emit poof with spacebar, but just use one instance which is reset with a new seed.
		// It's completely fine to just call emit_particles() and do nothing else each time,
		// this is just to prove that you can have one resetting emission.
		if (is_key_just_pressed(KEY_SPACEBAR)) {
		
			local_persist Emission_Handle inst = ZERO(Emission_Handle);
			local_persist bool inst_set = false;
			
			if (!inst_set) {
				inst = emit_particles(emission_poof, v2(mx, my));
				inst_set = true;
			} else {
				emission_poof.seed = get_random();
				emission_set_config(inst, emission_poof);
				emission_reset(inst);
				emission_set_position(inst, v2(mx, my));
			}
		}
		
		
		
		ext_update(delta_time);
		ext_draw();
		
		draw_text(font, tprint("FPS: %.2f", 1.0/delta_time), 32, v2(-window.width/2+30, window.height/2-60), v2(1, 1), COLOR_WHITE);
		
		os_update(); 
		gfx_update();
	}

	return 0;
}


void setup_emission_configs() {

	Gfx_Image *img = load_image_from_disk(STR("oogabooga/examples/berry_bush.png"), get_heap_allocator());
	assert(img, "Failed loading berry_bush.png");

	//
	// Rain

	emission_rain.kind_pool[0] = PARTICLE_KIND_RECTANGLE;
	emission_rain.kind_pool[1] = PARTICLE_KIND_CIRCLE;
	emission_rain.kind_pool[2] = PARTICLE_KIND_IMAGE;
	emission_rain.number_of_kinds = 3;
	
	emission_rain.image_pool[0] = img;
	emission_rain.number_of_images = 1;
	
	emission_rain.loop = true; // Loop means first particle starts over after last particle is emitted
	// If we want to loop without pauses, then number_of_particles needs to be at least
	// life_time*emissions_per_second
	emission_rain.number_of_particles = 600;
	emission_rain.emissions_per_second = 100;
	
	emission_rain.life_time.flat_f32 = 0.6;
	
	// Color and size must be set for particles to be visible at all
	
	emission_rain.color.mode = EMISSION_PROPERTY_MODE_INTERPOLATE;
	emission_rain.color.interp_kind = EMISSION_INTERPOLATION_SMOOTH;
	emission_rain.color.min_v4 = v4(0.85, 0.5, 0.5, 1.0);
	emission_rain.color.max_v4 = v4(0.85, 0.5, 0.5, 0.0); // Fade out
	
	emission_rain.size.flat_v2 = v2(16, 16);
	
	// start_position is relative to the position which you emit at
	emission_rain.start_position.mode = EMISSION_PROPERTY_MODE_RANDOM;
	emission_rain.start_position.min_v2 = v2(-16, -16);
	emission_rain.start_position.max_v2 = v2(16, 16);
	
	
	emission_rain.velocity.mode = EMISSION_PROPERTY_MODE_RANDOM;
	emission_rain.velocity.min_v2 = v2(-1000, -1000);
	emission_rain.velocity.max_v2 = v2( 1000,  1000);
	
	emission_rain.acceleration.flat_v2 = v2(0, -2500);

	//
	// Poof
	emission_poof.kind_pool[0] = PARTICLE_KIND_RECTANGLE;
	emission_poof.number_of_kinds = 1;
	
	emission_poof.number_of_particles = 10; 
	emission_poof.emissions_per_second = 100;
	emission_poof.loop = false;
	
	// If we don't loop, the emission will get released when done.
	// Alternatilvely, we set persist to true so we can keep it around to reset
	// and potentially release manually with emission_release()
	emission_poof.persist = true; 
	
	emission_poof.life_time.flat_f32 = 1.6;
	
	emission_poof.velocity.mode = EMISSION_PROPERTY_MODE_RANDOM;
	emission_poof.velocity.min_v2 = v2(-300, -300);
	emission_poof.velocity.max_v2 = v2( 300,  300);
	
	emission_poof.rotational_acceleration.mode = EMISSION_PROPERTY_MODE_RANDOM;
	emission_poof.rotational_acceleration.min_f32 = -TAU32;
	emission_poof.rotational_acceleration.max_f32 = TAU32;
	
	// Fade out
	emission_poof.color.mode = EMISSION_PROPERTY_MODE_INTERPOLATE;
	emission_poof.color.min_v4 = COLOR_WHITE;
	emission_poof.color.max_v4 = v4_zero;
	
	// Shrink until death
	emission_poof.size.mode = EMISSION_PROPERTY_MODE_INTERPOLATE;
	emission_poof.size.interp_kind = EMISSION_INTERPOLATION_SMOOTH;
	emission_poof.size.min_v2 = v2(16, 16);
	emission_poof.size.max_v2 = v2(0, 0);
	
	// For centered rotations, the pivot should interpolate same as size (but half)
	emission_poof.pivot.mode = EMISSION_PROPERTY_MODE_INTERPOLATE;
	emission_poof.pivot.interp_kind = EMISSION_INTERPOLATION_SMOOTH;
	emission_poof.pivot.min_v2 = v2(8, 8);
	emission_poof.pivot.max_v2 = v2(0, 0);
}

#endif // OOGABOOGA_ENABLE_EXTENSIONS && OOGABOOGA_EXTENSION_PARTICLES
