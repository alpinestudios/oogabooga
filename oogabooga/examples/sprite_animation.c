


int entry(int argc, char **argv) {
	
	window.title = STR("Sprite animation example");
	
	Gfx_Image *anim_sheet = load_image_from_disk(STR("oogabooga/examples/male_animation.png"), get_heap_allocator());
	assert(anim_sheet, "Could not open oogabooga/examples/male_animation.png");
	
	// Configure information about the whole image as a sprite sheet
	u32 number_of_columns = 10;
	u32 number_of_rows = 6;
	u32 total_number_of_frames = number_of_rows * number_of_columns;
	
	u32 anim_frame_width  = anim_sheet->width  / number_of_columns;
	u32 anim_frame_height = anim_sheet->height / number_of_rows;
	
	// Configure the animation by setting the start & end frames in the grid of frames
	// (Inspect sheet image and count the frame indices you want)
	// In sprite sheet animations, it usually goes down. So Y 0 is actuall the top of the
	// sprite sheet, and +Y is down on the sprite sheet.
	u32 anim_start_frame_x = 2;
	u32 anim_start_frame_y = 1;
	u32 anim_end_frame_x = 6;
	u32 anim_end_frame_y = 2;
	u32 anim_start_index = anim_start_frame_y * number_of_columns + anim_start_frame_x;
	u32 anim_end_index   = anim_end_frame_y   * number_of_columns + anim_end_frame_x;
	u32 anim_number_of_frames = max(anim_end_index, anim_start_index)-min(anim_end_index, anim_start_index)+1;
	
	// Sanity check configuration
	assert(anim_end_index > anim_start_index, "The last frame must come before the first frame");
	assert(anim_start_frame_x < number_of_columns, "anim_start_frame_x is out of bounds");
	assert(anim_start_frame_y < number_of_rows, "anim_start_frame_y is out of bounds");
	assert(anim_end_frame_x < number_of_columns, "anim_end_frame_x is out of bounds");
	assert(anim_end_frame_y < number_of_rows, "anim_end_frame_y is out of bounds");
	
	// Calculate duration per frame in seconds
	float32 playback_fps = 4;
	float32 anim_time_per_frame = 1.0 / playback_fps;
	float32 anim_duration = anim_time_per_frame * (float32)anim_number_of_frames;
	
	float32 anim_start_time = os_get_elapsed_seconds();
	
	float64 last_time = os_get_elapsed_seconds();
	while (!window.should_close) {
		reset_temporary_storage();
		
		
		float64 now = os_get_elapsed_seconds();
		float64 delta = now-last_time;
		last_time = now;
		
		draw_frame.projection = m4_make_orthographic_projection(window.pixel_width * -0.5, window.pixel_width * 0.5, window.pixel_height * -0.5, window.pixel_height * 0.5, -1, 10);
		
		// Float modulus to "loop" around the timer over the anim duration
		float32 anim_elapsed = fmodf(now - anim_start_time, anim_duration);
		
		// Get current progression in animation from 0.0 to 1.0
		float32 anim_progression_factor = anim_elapsed / anim_duration;
		
		u32 anim_current_index = anim_number_of_frames * anim_progression_factor;
		u32 anim_absolute_index_in_sheet = anim_start_index + anim_current_index;
		
		u32 anim_index_x = anim_absolute_index_in_sheet % number_of_columns;
		u32 anim_index_y = anim_absolute_index_in_sheet / number_of_columns + 1;
		
		u32 anim_sheet_pos_x = anim_index_x * anim_frame_width;
		u32 anim_sheet_pos_y = (number_of_rows - anim_index_y) * anim_frame_height; // Remember, Y inverted.
		
		// Draw the sprite sheet, with the uv box for the current frame.
		// Uv box is a Vector4 of x1, y1, x2, y2 where each value is a percentage value 0.0 to 1.0
		// from left to right / bottom to top in the texture.
		Draw_Quad *quad = draw_image(anim_sheet, v2(0, 0), v2(anim_frame_width*4, anim_frame_height*4), COLOR_WHITE);
		quad->uv.x1 = (float32)(anim_sheet_pos_x)/(float32)anim_sheet->width;
		quad->uv.y1 = (float32)(anim_sheet_pos_y)/(float32)anim_sheet->height;
		quad->uv.x2 = (float32)(anim_sheet_pos_x+anim_frame_width) /(float32)anim_sheet->width;
		quad->uv.y2 = (float32)(anim_sheet_pos_y+anim_frame_height)/(float32)anim_sheet->height;
		
		
		// Visualize sprite sheet animation
		Vector2 sheet_pos = v2(-window.width/2+40, -window.height/2+40);
		Vector2 sheet_size = v2(anim_sheet->width, anim_sheet->height);
		Vector2 frame_pos_in_sheet = v2(anim_sheet_pos_x, anim_sheet_pos_y);
		Vector2 frame_size = v2(anim_frame_width, anim_frame_height);
		draw_rect(sheet_pos, sheet_size, COLOR_BLACK); // Draw black background
		draw_rect(v2_add(sheet_pos, frame_pos_in_sheet), frame_size, COLOR_WHITE); // Draw white rect on current frame
		draw_image(anim_sheet, sheet_pos, sheet_size, COLOR_WHITE); // Draw the seet
		
		os_update(); 
		gfx_update();
	}

	return 0;
}