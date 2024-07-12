
/*

	This is an example showcasing how we can make a custom logger to log both to stdout and
	and in-game logger.
	
	We also have log levels to be able to disable/enable the respective levels.
	
	BEWARE!!
	This logger is not thread-safe. If multiple threads call log(), then nobody knows
	what might happen. If you need to make it thread-safe, check out concurrency.c.

*/

// start all log levels enabled
bool log_level_enabled_flags[LOG_LEVEL_COUNT] = {1, 1, 1, 1};

// We delete old messages when this overflows
#define MAX_LOG_MESSAGES 50
typedef struct Log_Message {
	string message;
	Log_Level level;
} Log_Message;
Log_Message log_messages[MAX_LOG_MESSAGES];
s64 num_log_messages = 0;

string get_log_level_name(Log_Level level) {
	switch (level) {
		case LOG_VERBOSE: return STR("Verbose");
		case LOG_INFO:    return STR("Info");
		case LOG_WARNING: return STR("Warning");
		case LOG_ERROR:   return STR("Error");
		default: return STR("");
	}
}

void my_logger(Log_Level level, string s) {


	string prefix = STR("[INVALID LOG LEVEL]");
	if (level >= 0 && level < LOG_LEVEL_COUNT) {
		// if log level is disabled, we just leave
		if (!log_level_enabled_flags[level]) {
			return;
		}
	
		prefix = tprint("[%s]", get_log_level_name(level));
	}

	
	// Format the final string.
	// Since we will be storing it in log_message, we need to use "sprint" rather than
	// "tprint" because it tprint uses the temp allocator which gets reset each frame.
	// In a real world scenario we would probably have a dedicated allocator for these
	// strings rather than heap allocating all of them.
	string message = sprint(get_heap_allocator(), "%s %s\n", prefix, s);
	
	// Output the final string to stdout
	print(message);
	
	
	// Also add to in-game log messages
	Log_Message msg = (Log_Message){message, level};
	
	if (num_log_messages < MAX_LOG_MESSAGES) {
		log_messages[num_log_messages] = msg;
		num_log_messages += 1;
	} else {
		// Shift memory down by one to make space for the next message and deleting the first.
		memcpy(log_messages, &log_messages[1], sizeof(log_messages)-sizeof(Log_Message));
		log_messages[num_log_messages-1] = msg;
	}
}

#define FONT_HEIGHT 38
Gfx_Font *font;

void draw_log(float x, float y);

int entry(int argc, char **argv) {
	
	// Window setup
	window.title = STR("Minimal Game Example");
	window.scaled_width = 1280;
	window.scaled_height = 720;
	window.x = 200;
	window.y = 200;
	window.clear_color = hex_to_rgba(0x6495EDff);
	
	// Load a font to draw the logs with
	font = load_font_from_disk(STR("C:/windows/fonts/arial.ttf"), get_heap_allocator());
	assert(font, "Failed loading arial.ttf, %d", GetLastError());
	
	// This is where we set the logger we want all logs to go through
	context.logger = my_logger;

	while (!window.should_close) {
		reset_temporary_storage();
		
		os_update(); 
		gfx_update();
		
		// pixel-aligned projection
		draw_frame.projection = m4_make_orthographic_projection(window.pixel_width * -0.5, window.pixel_width * 0.5, window.pixel_height * -0.5, window.pixel_height * 0.5, -1, 10);
		
		float x = -window.width/2+60;
		float y = window.height/2-FONT_HEIGHT/2-30;
		
		draw_text(font, STR("Left-click to toggle, right-click to send log message"), FONT_HEIGHT, v2(x-1, y+1), v2(1, 1), COLOR_BLACK);
		draw_text(font, STR("Left-click to toggle, right-click to send log message"), FONT_HEIGHT, v2(x, y), v2(1, 1), COLOR_WHITE);
		
		y -= FONT_HEIGHT*1.3;

		// Loop through all levels to draw their state and act on input
		for (Log_Level level = 0; level < LOG_LEVEL_COUNT; level += 1) {
			bool enabled = log_level_enabled_flags[level];
			string s = tprint("%s: %s", get_log_level_name(level), enabled ? STR("on") : STR("off"));
			Gfx_Text_Metrics m = measure_text(font, s, FONT_HEIGHT, v2(1, 1));
			
			Vector4 color = COLOR_WHITE;
			
			Vector2 bottom_left = v2_sub(v2(x, y), m.functional_pos_min);
			
			float L = bottom_left.x;
			float R = L + m.visual_size.x;
			float B = bottom_left.y;
			float T = B + m.visual_size.y;
			
			float mx = input_frame.mouse_x - window.width/2;
			float my = input_frame.mouse_y - window.height/2;
			
			bool hovered = mx >= L && mx < R && my >= B && my < T;
			if (hovered) color = v4(.8, .8, .8, 1.0);
			if (hovered && (is_key_down(MOUSE_BUTTON_LEFT) || is_key_down(MOUSE_BUTTON_RIGHT))) 
				color = v4(.6, .6, .6, 1.0);
			
			if (hovered && is_key_just_released(MOUSE_BUTTON_LEFT))
				log_level_enabled_flags[level] = !log_level_enabled_flags[level];
			if (hovered && is_key_just_released(MOUSE_BUTTON_RIGHT)) {
				if (level == LOG_VERBOSE) log_verbose("This is a log message");
				if (level == LOG_INFO)    log_info("This is a log message");
				if (level == LOG_WARNING) log_warning("This is a log message");
				if (level == LOG_ERROR)   log_error("This is a log message");
			}
			
			draw_rect(v2_sub(bottom_left, v2(8, 8)), v2_add(m.functional_size, v2(16, 16)), v4_mul(v4(.3, .3, .3, 1), color));
			draw_text(font, s, FONT_HEIGHT, v2(x-1, y+1), v2(1, 1), COLOR_BLACK);
			draw_text(font, s, FONT_HEIGHT, v2(x, y), v2(1, 1), color);
			
			y -= FONT_HEIGHT*1.3;
		}
		
		y -= FONT_HEIGHT*1.3;
		
		draw_log(x, y);
	}

	return 0;
}

void draw_log(float x, float y) {


	draw_text(font, STR("In-game log:"), FONT_HEIGHT, v2(x-1, y+1), v2(1, 1), COLOR_BLACK);
	Gfx_Text_Metrics m = draw_text_and_measure(font, STR("In-game log:"), FONT_HEIGHT, v2(x, y), v2(1, 1), COLOR_WHITE);
	
	y -= m.functional_size.y+20;
	
	// Here we draw each entry in the log that we can fit on screen, starting from the top
	// so we see the last log message first.
	for (s64 i = num_log_messages-1; i >= 0; i--) {
			
		Log_Level level = log_messages[i].level;
		
		if (level >= 0 && level < LOG_LEVEL_COUNT && !log_level_enabled_flags[level]) {
			// If it's disabled, skip it
			continue;
		}
		
		// Set color reflecting log level
		Vector4 color = COLOR_WHITE;
		if (level == LOG_VERBOSE)      color = v4(.6, .6, 1, 1);
		else if (level == LOG_INFO)    color = v4(.3, 1, .4, 1);
		else if (level == LOG_WARNING) color = v4(.8, .8, 1, 1);
		else if (level == LOG_ERROR)   color = v4(1, .2, .2, 1);
		
		draw_text(font, log_messages[i].message, FONT_HEIGHT, v2(x-1, y+1), v2(1, 1), COLOR_BLACK);
		draw_text(font, log_messages[i].message, FONT_HEIGHT, v2(x, y), v2(1, 1), color);
		
		y -= FONT_HEIGHT * 1.3;
		
		if (y+FONT_HEIGHT < -window.height/2) break; // Occlude text outside of view
	}
}