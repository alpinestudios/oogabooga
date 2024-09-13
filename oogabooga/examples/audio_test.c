#define FONT_HEIGHT 48

Gfx_Font *font;

bool button(string label, Vector2 pos, Vector2 size, bool enabled);

/*
	NOTE:
	
		In most cases you will probably just want to call play_one_clip().
		Using Audio Players is for when you need more control over the playback.

*/

int entry(int argc, char **argv) {
	
	window.title = STR("Audio test");
	window.point_width = 1280; // We need to set the scaled size if we want to handle system scaling (DPI)
	window.point_height = 720; 
	window.x = 200;
	window.y = 90;
	window.clear_color = hex_to_rgba(0x6495EDff);
	
	Allocator heap = get_heap_allocator();
	
	font = load_font_from_disk(STR("C:/windows/fonts/arial.ttf"), heap);
	assert(font, "Failed loading arial.ttf");
	
	Audio_Source bruh, song;
	
	bool bruh_ok = audio_open_source_load(&bruh, STR("oogabooga/examples/bruh.wav"), heap);
	assert(bruh_ok, "Could not load bruh.wav");

	bool song_ok = audio_open_source_stream(&song, STR("oogabooga/examples/song.ogg"), heap);
	assert(song_ok, "Could not load song.ogg");
	
	// By default, audio sources will be converted to the same format as the output buffer.
	// However, if you want it to be a specific format (or something smaller than the
	// output format), then you can call:
	// audio_open_source_load_format()
	// audio_open_source_stream_format()
	
	Audio_Player *clip_player = audio_player_get_one();
	Audio_Player *song_player = audio_player_get_one();
	
	// If you ever need it, you can give the player back to be reused somewhere else.
	// audio_player_release(clip_player);
	// But this is probably only something you would need to care about if you had a very
	// complicated audio system.
	
	audio_player_set_source(clip_player, bruh);
	audio_player_set_source(song_player, song);
	
	audio_player_set_state(clip_player, AUDIO_PLAYER_STATE_PAUSED);
	audio_player_set_state(song_player, AUDIO_PLAYER_STATE_PAUSED);
	
	audio_player_set_looping(clip_player, true);
	//play_one_audio_clip(STR("oogabooga/examples/block.wav"));
	
	while (!window.should_close) {
		reset_temporary_storage();
		
		draw_frame.projection = m4_make_orthographic_projection(window.pixel_width * -0.5, window.pixel_width * 0.5, window.pixel_height * -0.5, window.pixel_height * 0.5, -1, 10);
		
		if (is_key_just_pressed(MOUSE_BUTTON_RIGHT)) {
			// Easy mode (when you don't care and just want to play a clip)
			play_one_audio_clip(STR("oogabooga/examples/block.wav"));
		}
		
		
		
		Vector4 rect;
		rect.x = -window.width/2+40;
		rect.y = window.height/2-FONT_HEIGHT-40;
		rect.z = FONT_HEIGHT*8;
		rect.w = FONT_HEIGHT*1.5;
		
		bool clip_playing = clip_player->state == AUDIO_PLAYER_STATE_PLAYING;
		bool song_playing = song_player->state == AUDIO_PLAYER_STATE_PLAYING;
		
		if (button(STR("Song"), rect.xy, rect.zw, song_playing)) {
			if (song_playing) audio_player_set_state(song_player, AUDIO_PLAYER_STATE_PAUSED);
			else              audio_player_set_state(song_player, AUDIO_PLAYER_STATE_PLAYING);
		}
		
		rect.y -= FONT_HEIGHT*1.8;
		if (button(STR("Loop Bruh"), rect.xy, rect.zw, clip_playing)) {
			if (clip_playing) audio_player_set_state(clip_player, AUDIO_PLAYER_STATE_PAUSED);
			else              audio_player_set_state(clip_player, AUDIO_PLAYER_STATE_PLAYING);
		}
		rect.y -= FONT_HEIGHT*1.8;
		if (button(STR("One Bruh"), rect.xy, rect.zw, false)) {
			Audio_Playback_Config config = {0};
			config.volume                = 1.0;
			config.playback_speed        = get_random_float32_in_range(0.8, 1.2);
			config.enable_spacialization = true;
			config.position_ndc          = v3(get_random_float32_in_range(-1, 1), get_random_float32_in_range(-1, 1), 0);
			play_one_audio_clip_with_config(STR("oogabooga/examples/bruh.wav"), config);
		}
		rect.y -= FONT_HEIGHT*3;
		if (button(STR("Reset song"), rect.xy, rect.zw, false)) {
			audio_player_set_progression_factor(song_player, 0);
		}
		
		rect.y = window.height/2-FONT_HEIGHT-40;
		rect.x += rect.z + FONT_HEIGHT;
		if (button(STR("Song vol up"), rect.xy, rect.zw, false)) {
			song_player->config.volume += 0.05;
		}
		rect.y -= FONT_HEIGHT*1.8;
		if (button(STR("Song vol down"), rect.xy, rect.zw, false)) {
			song_player->config.volume -= 0.05;
		}
		song_player->config.volume = clamp(song_player->config.volume, 0, 20);
		rect.x += rect.z + FONT_HEIGHT;
		draw_text(font, tprint("Song volume: %d%%", (s64)round(song_player->config.volume*100)), FONT_HEIGHT, v2_sub(rect.xy, v2(2, -2)), v2(1, 1), COLOR_BLACK);
		draw_text(font, tprint("Song volume: %d%%", (s64)round(song_player->config.volume*100)), FONT_HEIGHT, rect.xy, v2(1, 1), COLOR_WHITE);
		rect.x -= rect.z + FONT_HEIGHT;
		
		rect.y -= FONT_HEIGHT*5;
		if (button(STR("Speed up"), rect.xy, rect.zw, false)) {
			song_player->config.playback_speed += 0.05;
		}
		rect.y -= FONT_HEIGHT*1.8;
		if (button(STR("Speed down"), rect.xy, rect.zw, false)) {
			song_player->config.playback_speed -= 0.05;
		}
		song_player->config.playback_speed = clamp(song_player->config.playback_speed, 0, 20);
		rect.x += rect.z + FONT_HEIGHT;
		draw_text(font, tprint("Speed: %d%%", (s64)round(song_player->config.playback_speed*100)), FONT_HEIGHT, v2_sub(rect.xy, v2(2, -2)), v2(1, 1), COLOR_BLACK);
		draw_text(font, tprint("Speed: %d%%", (s64)round(song_player->config.playback_speed*100)), FONT_HEIGHT, rect.xy, v2(1, 1), COLOR_WHITE);
		
		
		rect.y -= FONT_HEIGHT*3;
		
		draw_text(font, STR("Right-click for thing"), FONT_HEIGHT, v2_sub(rect.xy, v2(2, -2)), v2(1, 1), COLOR_BLACK);
		draw_text(font, STR("Right-click for thing"), FONT_HEIGHT, rect.xy, v2(1, 1), COLOR_WHITE);
		
		os_update(); 
		gfx_update();
	}
	
	// Don't actually do this on exit!!
	// Your OS will clean up everything when program exits, so this is only slowing down the time it takes for the program to exit.
	// This is just for testing purposes.
	audio_source_destroy(&bruh);
	audio_source_destroy(&song);

	return 0;
}

bool button(string label, Vector2 pos, Vector2 size, bool enabled) {

	Vector4 color = v4(.25, .25, .25, 1);
	
	float L = pos.x;
	float R = L + size.x;
	float B = pos.y;
	float T = B + size.y;
	
	float mx = input_frame.mouse_x - window.width/2;
	float my = input_frame.mouse_y - window.height/2;

	bool pressed = false;

	if (mx >= L && mx < R && my >= B && my < T) {
		color = v4(.15, .15, .15, 1);
		if (is_key_down(MOUSE_BUTTON_LEFT)) {
			color = v4(.05, .05, .05, 1);
		}
		
		pressed = is_key_just_released(MOUSE_BUTTON_LEFT);
	}
	
	if (enabled) {
		color = v4_add(color, v4(.1, .1, .1, 0));
	}

	draw_rect(pos, size, color);
	
	Gfx_Text_Metrics m = measure_text(font, label, FONT_HEIGHT, v2(1, 1));
	
	Vector2 bottom_left = v2_sub(pos, m.functional_pos_min);
	bottom_left.x += size.x/2;
	bottom_left.x -= m.functional_size.x/2;
	
	bottom_left.y += size.y/2;
	bottom_left.y -= m.functional_size.y/2;
	
	draw_text(font, label, FONT_HEIGHT, bottom_left, v2(1, 1), COLOR_WHITE);
	
	return pressed;
}