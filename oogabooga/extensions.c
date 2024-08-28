
#if OOGABOOGA_EXTENSION_PARTICLES
	#include "ext_particles.c"
#endif

void ext_init() {
#if OOGABOOGA_EXTENSION_PARTICLES
	particles_init();
#endif
}

void ext_update(float32 delta_time) {
#if OOGABOOGA_EXTENSION_PARTICLES
	particles_update();
#endif
}

void ext_draw() {
#if OOGABOOGA_EXTENSION_PARTICLES
	particles_draw();
#endif
}