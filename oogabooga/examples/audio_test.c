#define FONT_HEIGHT 48

Gfx_Font *font;

bool button(string label, Vector2 pos, Vector2 size, bool enabled);



int entry(int argc, char **argv) {
	
	window.title = STR("Audio test");
	window.scaled_width = 1280; // We need to set the scaled size if we want to handle system scaling (DPI)
	window.scaled_height = 720; 
	window.x = 200;
	window.y = 90;
	window.clear_color = hex_to_rgba(0x6495EDff);
	
	Allocator heap = get_heap_allocator();
	
	font = load_font_from_disk(STR("C:/windows/fonts/arial.ttf"), heap);
	assert(font, "Failed loading arial.ttf");
	
	Audio_Source bruh, song;
	
	bool bruh_ok = audio_open_source_load(&bruh, STR("oogabooga/examples/bruh.wav"), heap);
	assert(bruh_ok, "Could not load bruh.wav");

	bool song_ok = audio_open_source_stream(&song, STR("oogabooga/examples/song.ogg"),  heap);
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
	
	audio_player_set_source(clip_player, bruh, false);
	audio_player_set_source(song_player, song, false);
	
	audio_player_set_state(clip_player, AUDIO_PLAYER_STATE_PAUSED);
	audio_player_set_state(song_player, AUDIO_PLAYER_STATE_PLAYING);
	
	audio_player_set_looping(clip_player, true);
	//play_one_audio_clip(STR("oogabooga/examples/block.wav"));
	
	while (!window.should_close) {
		reset_temporary_storage();
		
		draw_frame.projection = m4_make_orthographic_projection(window.pixel_width * -0.5, window.pixel_width * 0.5, window.pixel_height * -0.5, window.pixel_height * 0.5, -1, 10);
		
		if (is_key_just_pressed(MOUSE_BUTTON_RIGHT)) {
			float mx = input_frame.mouse_x;
			float my = input_frame.mouse_y;
			// Easy mode (when you don't care and just want to play a clip)
			Vector3 p = v3(mx/(f32)window.width*2.0-1, my/(f32)window.height*2.0-1, 0);
			log("%f, %f", p.x, p.y);
			play_one_audio_clip_at_position(STR("oogabooga/examples/block.wav"), p);
			// Or just play_one_audio_clip if you don't care about spacialization
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
			play_one_audio_clip(STR("oogabooga/examples/bruh.wav"));
		}
		rect.y -= FONT_HEIGHT*3;
		if (button(STR("Reset song"), rect.xy, rect.zw, false)) {
			audio_player_set_progression_factor(song_player, 0);
		}
		
		rect.y = window.height/2-FONT_HEIGHT-40;
		rect.x += rect.z + FONT_HEIGHT;
		if (button(STR("Song vol up"), rect.xy, rect.zw, false)) {
			song_player->volume += 0.05;
		}
		rect.y -= FONT_HEIGHT*1.8;
		if (button(STR("Song vol down"), rect.xy, rect.zw, false)) {
			song_player->volume -= 0.05;
		}
		song_player->volume = clamp(song_player->volume, 0, 20);
		rect.x += rect.z + FONT_HEIGHT;
		draw_text(font, tprint("Song volume: %d%%", (s64)round(song_player->volume*100)), FONT_HEIGHT, v2_sub(rect.xy, v2(2, -2)), v2(1, 1), COLOR_BLACK);
		draw_text(font, tprint("Song volume: %d%%", (s64)round(song_player->volume*100)), FONT_HEIGHT, rect.xy, v2(1, 1), COLOR_WHITE);
		
		
		rect.y -= FONT_HEIGHT*3;
		
		draw_text(font, STR("Right-click for thing"), FONT_HEIGHT, v2_sub(rect.xy, v2(2, -2)), v2(1, 1), COLOR_BLACK);
		draw_text(font, STR("Right-click for thing"), FONT_HEIGHT, rect.xy, v2(1, 1), COLOR_WHITE);
		
		os_update(); 
		gfx_update();
	}

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