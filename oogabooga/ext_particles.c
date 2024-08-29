
/*

	This is an oogabooga extension, so it's not compiled by default.
	Do the following before including oogabooga.c to enable this extension:
	#define OOGABOOGA_ENABLE_EXTENSIONS   1
	#define OOGABOOGA_EXTENSION_PARTICLES 1

	Usage:
	
		Create a Emission_Config, configure
		
		call emit_particles(config, pos);
		
		
		Emission_Config my_emission = ZERO(Emission_Config);
		
		// For randomized properties:
		my_emission.size.mode = EMISSION_PROPERTY_MODE_RANDOM;
		my_emission.size.min_v2 = v2(...);
		my_emission.size.max_v2 = v2(...);
		
		// For flat properties:
		my_emission.rotation.mode = EMISSION_PROPERTY_MODE_FLAT; // You can remove this line if my_emission is already zeroed
		my_emission.rotation.flat_f32 = 6.9;
		
		// For interpolated properties (from min to max over birth to death):
		my_emission.color.mode = EMISSION_PROPERTY_MODE_INTERPOLATE;
		my_emission.color.min_v4 = COLOR_WHITE;
		my_emission.color.max_v4 = COLOR_BLACK;
		
		
		NOTE:
			To understand whether to use flat_f32, flat_v2, flat_v3, flat_v4 etc, see struct Emission_Config where
			I have placed a comment describing the intended data type for the property
			
		
		emit_particles() returns a Emission_Handle, which you can use for the following:
		
		void emission_reset(Emission_Handle h);
		void emission_set_config(Emission_Handle h, Emission_Config config);
		void emission_set_position(Emission_Handle h, Vector2 pos);
		void emission_release(Emission_Handle h);
		
		NOTE:
			Emission instances will, by default,  be released and their handles invalidated after the last particle
			in the emission has died. UNLESS: config.loop is true OR config.persist is true.
			
			
		
*/

#define PARTICLE_KIND_POOL_MAX 128
#define PARTICLE_IMAGE_POOL_MAX 128

#define PARTICLES_MEM_PER_BLOCK MB(1)
#define PARTICLES_PER_BLOCK (PARTICLES_MEM_PER_BLOCK / sizeof(Particle))

typedef enum Particle_Kind {
	PARTICLE_KIND_RECTANGLE,
	PARTICLE_KIND_CIRCLE,
	PARTICLE_KIND_IMAGE,
} Particle_Kind;


typedef enum Emission_Property_Mode {
	EMISSION_PROPERTY_MODE_FLAT,
	EMISSION_PROPERTY_MODE_RANDOM,
	EMISSION_PROPERTY_MODE_INTERPOLATE,
} Emission_Property_Mode;
typedef enum Emission_Interpolation_Kind {
	EMISSION_INTERPOLATION_LINEAR,
	EMISSION_INTERPOLATION_SMOOTH,
	EMISSION_INTERPOLATION_SINE_WAVE,
} Emission_Interpolation_Kind;
typedef struct Emission_Property {
	
	union {
		union {
			float32 min_f32;
			Vector2 min_v2;
			Vector3 min_v3;
			Vector4 min_v4;
		};
		union {
			float32 flat_f32;
			Vector2 flat_v2;
			Vector3 flat_v3;
			Vector4 flat_v4;
		};
	};
	union {
		float32 max_f32;
		Vector2 max_v2;
		Vector3 max_v3;
		Vector4 max_v4;
	};
	
	Emission_Property_Mode mode;
	Emission_Interpolation_Kind interp_kind;
	
} Emission_Property;

typedef struct Particle {
	Particle_Kind kind;
	float32 rotation;
	Vector4 color;
	Vector2 position;
	Vector2 pivot;
	Vector2 size;
	u32 index;
} Particle;

typedef struct Emission_Config {
	u32 number_of_particles;
	float32 emissions_per_second;
	
	Particle_Kind kind_pool[PARTICLE_KIND_POOL_MAX];
	u32 number_of_kinds;
	
	Gfx_Image *image_pool[PARTICLE_IMAGE_POOL_MAX];
	u32 number_of_images;
	
	bool persist; // Set to true to not release when done playing
	
	bool loop;
	
	u64 seed;
	
	// Vector2
	Emission_Property start_position;
	// Vector2
	Emission_Property pivot;
	
	// Float32
	Emission_Property life_time;
	
	// Vector2
	Emission_Property velocity;
	// Vector2
	Emission_Property acceleration;
	
	// float32
	Emission_Property rotation;
	Emission_Property rotational_acceleration;
	
	// Vector4
	Emission_Property color;
	
	// Vector2
	Emission_Property size;
	
} Emission_Config;



typedef struct Emission_Instance {
	Emission_Config config;
	Vector2 pos;
	float32 start_time;
	bool allocated;
	u32 generation;
} Emission_Instance;

typedef struct Emission_Handle {
	u32 index;
	u32 generation;
} Emission_Handle;

// #Global
#if OOGABOOGA_LINK_EXTERNAL_INSTANCE
ogb_instance Emission_Instance *emissions;
#else
Emission_Instance *emissions;
#endif

float32 sample_interp_one(Emission_Interpolation_Kind interp, float32 min, float32 max, float t) {
	switch (interp) {
		case EMISSION_INTERPOLATION_LINEAR: {
			return lerpf(min, max, t);
		}
		case EMISSION_INTERPOLATION_SMOOTH: {
			return smerpf(min, max, t);
		}
		case EMISSION_INTERPOLATION_SINE_WAVE: {
			return sine_oscillate_n_waves_normalized(t, 1);
		}
	}
}

float32 sample_emission_property_f32(Emission_Property p, u64 seed, float32 t) {
	
	switch (p.mode) {
		case EMISSION_PROPERTY_MODE_FLAT:
			return p.flat_f32;
		case EMISSION_PROPERTY_MODE_RANDOM:
			float32 v = get_random_float32_in_range(p.min_f32, p.max_f32);
			return v;
		case EMISSION_PROPERTY_MODE_INTERPOLATE:
			return sample_interp_one(p.interp_kind, p.min_f32, p.max_f32, t);
	}
	return 0.0;
}
Vector2 sample_emission_property_v2(Emission_Property p, u64 seed, float32 t) {
	
	switch (p.mode) {
		case EMISSION_PROPERTY_MODE_FLAT:
			return p.flat_v2;
		case EMISSION_PROPERTY_MODE_RANDOM:
			Vector2 v;
			v.x = get_random_float32_in_range(p.min_v2.x, p.max_v2.x);
			v.y = get_random_float32_in_range(p.min_v2.y, p.max_v2.y);
			return v;
		case EMISSION_PROPERTY_MODE_INTERPOLATE:
			return v2(
				sample_interp_one(p.interp_kind, p.min_v2.x, p.max_v2.x, t),
				sample_interp_one(p.interp_kind, p.min_v2.y, p.max_v2.y, t)
			);
	}
	return v2(0, 0);
}
Vector3 sample_emission_property_v3(Emission_Property p, u64 seed, float32 t) {
	
	switch (p.mode) {
		case EMISSION_PROPERTY_MODE_FLAT:
			return p.flat_v3;
		case EMISSION_PROPERTY_MODE_RANDOM:
			Vector3 v;
			v.x = get_random_float32_in_range(p.min_v3.x, p.max_v3.x);
			v.y = get_random_float32_in_range(p.min_v3.y, p.max_v3.y);
			v.z = get_random_float32_in_range(p.min_v3.z, p.max_v3.z);
			return v;
		case EMISSION_PROPERTY_MODE_INTERPOLATE:
			return v3(
				sample_interp_one(p.interp_kind, p.min_v3.x, p.max_v3.x, t),
				sample_interp_one(p.interp_kind, p.min_v3.y, p.max_v3.y, t),
				sample_interp_one(p.interp_kind, p.min_v3.z, p.max_v3.z, t)
			);
	}
	return v3(0, 0, 0);
}
Vector4 sample_emission_property_v4(Emission_Property p, u64 seed, float32 t) {
	
	switch (p.mode) {
		case EMISSION_PROPERTY_MODE_FLAT:
			return p.flat_v4;
		case EMISSION_PROPERTY_MODE_RANDOM:
			Vector4 v;
			v.x = get_random_float32_in_range(p.min_v4.x, p.max_v4.x);
			v.y = get_random_float32_in_range(p.min_v4.y, p.max_v4.y);
			v.z = get_random_float32_in_range(p.min_v4.z, p.max_v4.z);
			v.w = get_random_float32_in_range(p.min_v4.w, p.max_v4.w);
			return v;
		case EMISSION_PROPERTY_MODE_INTERPOLATE:
			return v4(
				sample_interp_one(p.interp_kind, p.min_v4.x, p.max_v4.x, t),
				sample_interp_one(p.interp_kind, p.min_v4.y, p.max_v4.y, t),
				sample_interp_one(p.interp_kind, p.min_v4.z, p.max_v4.z, t),
				sample_interp_one(p.interp_kind, p.min_v4.w, p.max_v4.w, t)
			);
	}
	return v4(0, 0, 0, 0);
}

Emission_Handle emit_particles(Emission_Config config, Vector2 pos) {

	config.number_of_particles = max(config.number_of_particles, 1);
	config.emissions_per_second = max(config.emissions_per_second, 1);
	if (config.seed == 0) config.seed = get_random();

	for (u64 i = 0; i < growing_array_get_valid_count(emissions); i += 1) {
		if (!emissions[i].allocated) {
			emissions[i] = ZERO(Emission_Instance);
			emissions[i].config = config;
			emissions[i].pos = pos;
			emissions[i].start_time = os_get_elapsed_seconds();
			emissions[i].allocated = true;
			emissions[i].generation += 1;
			
			return (Emission_Handle) { i, emissions[i].generation };
		}
	}

	Emission_Instance inst = ZERO(Emission_Instance);
	inst.config = config;
	inst.pos = pos;
	inst.start_time = os_get_elapsed_seconds();
	inst.allocated = true;	
	inst.generation = 0;	
	growing_array_add((void**)&emissions, &inst);
	
	return (Emission_Handle){ growing_array_get_valid_count(emissions) - 1, 0 };
}

void emission_reset(Emission_Handle h) {
	assert(h.index < growing_array_get_valid_count(emissions), "Invalid Emission_Handle");
	assert(h.generation == emissions[h.index].generation, "Invalid Emission_Handle; emission has been released");
	
	Emission_Instance *e = &emissions[h.index];
	e->start_time = os_get_elapsed_seconds();
}

void emission_set_config(Emission_Handle h, Emission_Config config) {
	assert(h.index < growing_array_get_valid_count(emissions), "Invalid Emission_Handle");
	assert(h.generation == emissions[h.index].generation, "Invalid Emission_Handle; emission has been released");
	
	Emission_Instance *e = &emissions[h.index];
	
	e->config = config;
}
void emission_set_position(Emission_Handle h, Vector2 pos) {
	assert(h.index < growing_array_get_valid_count(emissions), "Invalid Emission_Handle");
	assert(h.generation == emissions[h.index].generation, "Invalid Emission_Handle; emission has been released");
	
	Emission_Instance *e = &emissions[h.index];
	
	e->pos = pos;
}
void emission_release(Emission_Handle h) {
	assert(h.index < growing_array_get_valid_count(emissions), "Invalid Emission_Handle");
	
	Emission_Instance *e = &emissions[h.index];
	
	if (e->generation == h.generation) e->allocated = false;
}

void particles_init() {
	growing_array_init_reserve((void**)&emissions, sizeof(Emission_Instance), 16, get_heap_allocator());
}

void particles_update() {

	// I ended up not needing this 
}

void particles_draw() {
	// We compute each particle each frame depending on now vs then
	
	float32 now = os_get_elapsed_seconds();

	u64 backup_seed = seed_for_random;
	
	for (u64 i = 0; i < growing_array_get_valid_count(emissions); i += 1) {
		Emission_Instance *e = &emissions[i];
		if (!e->allocated) continue;
		
		float32 passed = now - e->start_time;
		
		float32 sample_life_time = sample_emission_property_f32(e->config.life_time, 69, 0.0);
		
		// Duration until last particle is emitted
		float32 last_emit_duration  = (float32)e->config.number_of_particles/e->config.emissions_per_second;
		// Duration until last particle dies
		float32 last_death_duration = last_emit_duration + sample_life_time;
		
		float32 emission_interval = last_emit_duration / (float32)e->config.number_of_particles;
		
		u64 max_emitted = (u64)(passed/emission_interval);
		max_emitted = min(max_emitted, e->config.number_of_particles);
		
		if (!e->config.persist && !e->config.loop && passed > last_death_duration) {
			e->allocated = false;
			continue;
		}
		
		seed_for_random = e->config.seed;
			
		for (u64 j = 0; j < max_emitted; j += 1) {
			Particle p = ZERO(Particle);
			
			float32 emission_time = (float32)j*emission_interval;
			
			bool is_emitted = passed >= emission_time;
			if (!is_emitted) continue;
			
			if (e->config.number_of_kinds <= 1) {
				p.kind = e->config.kind_pool[0];
			} else {
				p.kind = e->config.kind_pool[get_random_int_in_range(0, e->config.number_of_kinds-1)];
			}
			
			
			float32 life_time = sample_emission_property_f32(e->config.life_time, e->config.seed, 0.0);
			
			float32 age = passed - emission_time;
			
			if (e->config.loop) age = fmodf(age, last_emit_duration);
			
			float32 t = age/life_time;
			
			Vector2 origin = e->pos;
			origin = v2_add(origin, sample_emission_property_v2(e->config.start_position, e->config.seed, t));
			
			p.pivot = sample_emission_property_v2(e->config.pivot, e->config.seed, t);
			
			Vector2 velocity = sample_emission_property_v2(e->config.velocity, e->config.seed, t);
			Vector2 acceleration = sample_emission_property_v2(e->config.acceleration, e->config.seed, t);
			
			velocity = v2_add(velocity, v2_mulf(acceleration, age));
			p.position = v2_add(origin, v2_mulf(velocity, age));
			
			
			p.rotation = sample_emission_property_f32(e->config.rotation, e->config.seed, t);
			p.rotation += sample_emission_property_f32(e->config.rotational_acceleration, e->config.seed, t) * age;
			
			p.color = sample_emission_property_v4(e->config.color, e->config.seed, t);
			
			p.size = sample_emission_property_v2(e->config.size, e->config.seed, t);

			p.index = j;
			
			// deth
			// Must happen after computation for seed consistency
			// #Speed
			if (age > life_time) {
				continue;
			}
			
			
			Matrix3 xform = m3_identity();
			xform = m3_translate(xform, p.position);
			xform = m3_rotate(xform, p.rotation);
			xform = m3_translate(xform, v2_mulf(p.pivot, -1));
			switch (p.kind) {
				case PARTICLE_KIND_RECTANGLE: {
					draw_rect_xform(m3_to_m4(xform), p.size, p.color);
					break;
				}
				case PARTICLE_KIND_CIRCLE: {
					draw_circle_xform(m3_to_m4(xform), p.size, p.color);
					break;
				}
				case PARTICLE_KIND_IMAGE: {
					Gfx_Image *image = 0;
					assert(e->config.number_of_images > 0, "Particle is PARTICLE_KIND_IMAGE but config.number_of_images is <= 0");
					if (e->config.number_of_images == 1) {
						image = e->config.image_pool[0];
					} else {
						image = e->config.image_pool[get_random_int_in_range(0, e->config.number_of_images-1)];
					}
					draw_image_xform(image, m3_to_m4(xform), p.size, p.color);
					break;
				}
			}
		}
		
		
	}
	
	seed_for_random = backup_seed;
}

