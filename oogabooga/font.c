
// See oogabooga/examples/text_rendering.c for usage



/*
TODO:
	- Justify rows in walk_glyphs
*/


// #Memory #Speed
// This is terruble  and huge waste of video memory. We should have a constant codepoint range
// with arbitrary atlas sizes instead.
#define FONT_ATLAS_WIDTH  2048
#define FONT_ATLAS_HEIGHT 2048
#define MAX_FONT_HEIGHT 512

typedef struct Gfx_Font Gfx_Font;
typedef struct Gfx_Text_Metrics {
	
	
	/*
	
		FUNCTIONAL BOX:
			x0: left start of text box
			x1: right end of text box
			y0: The baseline for the bottom line of text
			y1: The baseline for the top line of text + latin_ascent
			
		VISUAL BOX:
			x0: The minimum X of any pixels
			x1: The maximum X of any pixels
			y0: The minimum Y of any pixels
			y1: The maximum Y of any pixels
			
			
		Usage:
		
			For a single piece of static text, it might look better to use the visual box for aligning it somewhere.
			I might be stupid and that might be useless tho.
			
			Most of the time, you are probably going to want to use the functional box.
			
			For example, to center:
			
				Gfx_Text_Metrics m = measure_text(...);
				
				// This is the point where the center of the text box will be
				Vector2 draw_pos = v2(...);
				
				// First justify for the bottom-left to be at the draw point
				Vector2 justified = v2_sub(draw_pos, m.functional_pos_min);
				
				// Then move text backwards by functional_size/2 to align its center to the draw point
				justified = v2_sub(justified, v2_divf(m.functional_size, 2));
	
	*/
	Vector2 functional_pos_min;
	Vector2 functional_pos_max;
	Vector2 functional_size;
	
	// The visual bounds for the text.
	Vector2 visual_pos_min;
	Vector2 visual_pos_max;
	Vector2 visual_size;
	
} Gfx_Text_Metrics;
typedef struct Gfx_Font_Metrics {
	float latin_ascent;
	float latin_descent;
	
	float max_ascent;
	float max_descent;
	
	float line_spacing;
	
} Gfx_Font_Metrics;
typedef struct Gfx_Glyph {
	u32 codepoint;
	float xoffset, yoffset;
	float advance;
	float width, height;
	Vector4 uv;
} Gfx_Glyph;
typedef struct Gfx_Font_Atlas {
	Gfx_Image *image;
	u32 first_codepoint;
	Gfx_Glyph *glyphs; // first_codepoint + index == the codepoint
} Gfx_Font_Atlas;
typedef struct Gfx_Font_Variation {
	Gfx_Font *font;
	u32 height;
	Gfx_Font_Metrics metrics;
	float scale;
	u32 codepoint_range_per_atlas;
	Hash_Table atlases; // u32 atlas_index, Gfx_Font_Atlas
	bool initted;
} Gfx_Font_Variation;
typedef struct Gfx_Font {
	stbtt_fontinfo stbtt_handle;
	string raw_font_data;
	Gfx_Font_Variation variations[MAX_FONT_HEIGHT]; // Variation per font height
	Allocator allocator;
} Gfx_Font;

Gfx_Font *load_font_from_disk(string path, Allocator allocator) {
	
	string font_data;
	bool read_ok = os_read_entire_file(path, &font_data, allocator);
	
	if (!read_ok) return 0;
	
	third_party_allocator = allocator;
	
	stbtt_fontinfo stbtt_handle;
	int result = stbtt_InitFont(&stbtt_handle, font_data.data, stbtt_GetFontOffsetForIndex(font_data.data, 0));
	
	if (result == 0) return 0;
	
	Gfx_Font *font = alloc(allocator, sizeof(Gfx_Font));
	memset(font, 0, sizeof(Gfx_Font));
	font->stbtt_handle = stbtt_handle;
	font->raw_font_data = font_data;
	font->allocator = allocator;
	
	third_party_allocator = ZERO(Allocator);
	
	return font;
}
void destroy_font(Gfx_Font *font) {

	third_party_allocator = font->allocator;

	for (u64 i = 0; i < MAX_FONT_HEIGHT; i++) {
		Gfx_Font_Variation *variation = &font->variations[i];
		if (!variation->initted) continue;
		
		for (u64 j = 0; j < variation->atlases.count; j++) {
			Gfx_Font_Atlas *atlas = (Gfx_Font_Atlas*)hash_table_get_nth_value(&variation->atlases, j);
			delete_image(atlas->image);
			dealloc(font->allocator, atlas->glyphs);
		}
		
		hash_table_destroy(&variation->atlases);
		
	}

	dealloc_string(font->allocator, font->raw_font_data);
	dealloc(font->allocator, font);
	
	third_party_allocator = ZERO(Allocator);
}

void font_variation_init(Gfx_Font_Variation *variation, Gfx_Font *font, u32 font_height) {

	variation->font = font;
	variation->height = font_height;
	
	u32 x_range = FONT_ATLAS_WIDTH / font_height;
	u32 y_range = FONT_ATLAS_HEIGHT / font_height;
	
	variation->codepoint_range_per_atlas = x_range*y_range;
	
	variation->atlases = make_hash_table(u32, Gfx_Font_Atlas, font->allocator);
	
	variation->scale = stbtt_ScaleForPixelHeight(&font->stbtt_handle, (float)font_height);
	
	// Unscaled !
	int ascent, descent, line_gap;
	stbtt_GetFontVMetrics(&font->stbtt_handle, &ascent, &descent, &line_gap);
	
	variation->metrics.max_ascent = ((float)ascent*variation->scale);
	variation->metrics.max_descent = ((float)descent*variation->scale);
	variation->metrics.line_spacing = ((float)line_gap*variation->scale);
	
	variation->metrics.latin_descent = 9999999;
	
	for (u32 c = 'a'; c <= 'z'; c++) {
		// This one is bottom-top as opposed to normally in stbtt where it's top-bottom
		int x0, y0, x1, y1;
		stbtt_GetCodepointBitmapBox(&font->stbtt_handle, (int)c, variation->scale, variation->scale, &x0, &y0, &x1, &y1);
		
		float c_descent = -(float)y1;
		if (c_descent < variation->metrics.latin_descent)  {
			variation->metrics.latin_descent = c_descent;
		}
	}
	
	for (u32 c = 'A'; c <= 'Z'; c++) {
		// This one is bottom-top as opposed to normally in stbtt where it's top-bottom
		int x0, y0, x1, y1;
		stbtt_GetCodepointBitmapBox(&font->stbtt_handle, (int)c, variation->scale, variation->scale, &x0, &y0, &x1, &y1);
		float c_ascent = (float)abs(y0);
		if (c_ascent > variation->metrics.latin_ascent) 
			variation->metrics.latin_ascent = c_ascent;
	}
	
	variation->initted = true;
}

void font_atlas_init(Gfx_Font_Atlas *atlas, Gfx_Font_Variation *variation, u32 first_codepoint) {
	stbtt_fontinfo stbtt_handle = variation->font->stbtt_handle;
	atlas->first_codepoint = first_codepoint;
	
	atlas->image = make_image(FONT_ATLAS_WIDTH, FONT_ATLAS_HEIGHT, 1, 0, variation->font->allocator);
	atlas->glyphs = alloc(variation->font->allocator, variation->codepoint_range_per_atlas*sizeof(Gfx_Glyph));
	
	u32 cursor_x = 0;
	u32 cursor_y = 0;
	
	third_party_allocator = variation->font->allocator;
	// Used for flipping bitmaps
	u8 *temp_row = (u8 *)talloc(variation->height);
	for (u32 c = first_codepoint; c < first_codepoint + variation->codepoint_range_per_atlas; c++) {
		u32 i = c-first_codepoint;
		Gfx_Glyph *glyph = &atlas->glyphs[i];
		glyph->codepoint = c;
		
		int w, h, x, y;
		void *bitmap = stbtt_GetCodepointBitmap(&stbtt_handle, variation->scale, variation->scale, (int)c, &w, &h, &x, &y);
		
		if (cursor_x+w > FONT_ATLAS_WIDTH) {
			cursor_x = 0;
			cursor_y += variation->height;
		}
		
		if (bitmap) {
			// #Speed #Loadtimes
			for (int row = 0; row < h; ++row) {
                gfx_set_image_data(atlas->image, cursor_x, cursor_y + (h - 1 - row), w, 1, bitmap + (row * w));
            }
			stbtt_FreeBitmap(bitmap, 0);
		}
		
		glyph->xoffset = (float)x;
		glyph->yoffset = variation->height - (float)y - (float)h - variation->metrics.max_ascent+variation->metrics.max_descent;  // Adjusted yoffset for bottom-up rendering
		glyph->width   = (float)w;
		glyph->height  = (float)h;
		
		int advance, left_side_bearing;
		stbtt_GetCodepointHMetrics(&stbtt_handle, c, &advance, &left_side_bearing);
		
		glyph->advance = (float)advance*variation->scale;
		//glyph->xoffset += (float)left_side_bearing*variation->scale;
		
		glyph->uv.x1 = ((float)cursor_x)/(float)FONT_ATLAS_WIDTH;
		glyph->uv.y1 = ((float)cursor_y)/(float)FONT_ATLAS_HEIGHT;
		glyph->uv.x2 = ((float)cursor_x+glyph->width)/(float)FONT_ATLAS_WIDTH;
		glyph->uv.y2 = ((float)cursor_y+glyph->height)/(float)FONT_ATLAS_HEIGHT;
		
		cursor_x += w;
	}
	
	third_party_allocator = ZERO(Allocator);
}

void render_atlas_if_not_yet_rendered(Gfx_Font *font, u32 font_height, u32 codepoint) {
	assert(font_height <= MAX_FONT_HEIGHT, "Font height too large; maximum of %d is allowed.", MAX_FONT_HEIGHT);
	Gfx_Font_Variation *variation = &font->variations[font_height];
	
	if (!variation->initted) {
		font_variation_init(variation, font, font_height);
	}
	
	u32 atlas_index = codepoint / variation->codepoint_range_per_atlas;
	
	if (!hash_table_contains(&variation->atlases, atlas_index)) {
		Gfx_Font_Atlas atlas = ZERO(Gfx_Font_Atlas);
		font_atlas_init(&atlas, variation, atlas_index*variation->codepoint_range_per_atlas);
		hash_table_add(&variation->atlases, atlas_index, atlas);
	}
}

typedef bool(*Walk_Glyphs_Callback_Proc)(Gfx_Glyph glyph, Gfx_Font_Atlas *atlas, float glyph_x, float glyph_y, void *ud);

typedef struct {
	Gfx_Font *font;
	string text;
	u32 raster_height;
	Vector2 scale;
	bool ignore_control_codes;
	void *ud;
} Walk_Glyphs_Spec;
void walk_glyphs(Walk_Glyphs_Spec spec, Walk_Glyphs_Callback_Proc proc) {
	
	Gfx_Font_Variation *variation = &spec.font->variations[spec.raster_height];
	
	float x = 0;
	float y = 0;
	
	u32 last_c = 0;
	u32 c = next_utf8(&spec.text);
	while (c != 0) {
		
		render_atlas_if_not_yet_rendered(spec.font, spec.raster_height, c);
		
		if (c == '\n') {
			x = 0;
			y -= (variation->metrics.latin_ascent-variation->metrics.latin_descent+variation->metrics.line_spacing)*spec.scale.y;
			last_c = 0;
		}
		
		if (c < 32 && spec.ignore_control_codes) {
			c = next_utf8(&spec.text);
			continue;
		}
		
		u32 atlas_index = c/variation->codepoint_range_per_atlas;
		
		Gfx_Font_Atlas *atlas = (Gfx_Font_Atlas*)hash_table_find(&variation->atlases, atlas_index);
		Gfx_Glyph glyph = atlas->glyphs[c-atlas->first_codepoint];
		
		float glyph_x = x+glyph.xoffset*spec.scale.x;
		float glyph_y = y+(glyph.yoffset)*spec.scale.y;
		bool should_continue = proc(glyph, atlas, glyph_x, glyph_y, spec.ud);
		
		if (!should_continue) break;
		
		// #Incomplete kerning
		x += glyph.advance*spec.scale.x;
		if (last_c != 0) {
			int kerning_unscaled = stbtt_GetCodepointKernAdvance(&spec.font->stbtt_handle, last_c, c);
			float kerning_scaled_to_font_height = kerning_unscaled * variation->scale;
			x += kerning_scaled_to_font_height*spec.scale.x;
		}
		
		last_c = c;
		c = next_utf8(&spec.text);
	}
}

Gfx_Font_Metrics get_font_metrics(Gfx_Font *font, u32 raster_height) {
	Gfx_Font_Variation *variation = &font->variations[raster_height];
	
	if (!variation->initted) {
		font_variation_init(variation, font, raster_height);
	}
	
	return variation->metrics;
}

Gfx_Font_Metrics get_font_metrics_scaled(Gfx_Font *font, u32 raster_height, Vector2 scale) {
	Gfx_Font_Metrics metrics = get_font_metrics(font, raster_height);
	
	metrics.latin_ascent *= scale.x;
	metrics.latin_descent *= scale.x;
	metrics.max_ascent *= scale.x;
	metrics.max_descent *= scale.x;
	metrics.line_spacing *= scale.x;
	
	return metrics;
}

typedef struct {
	Gfx_Text_Metrics m;
	Gfx_Font *font;
	u32 raster_height;
	Vector2 scale;
} Measure_Text_Walk_Glyphs_Context;

bool measure_text_glyph_callback(Gfx_Glyph glyph, Gfx_Font_Atlas *atlas, float glyph_x, float glyph_y, void *ud) {

	Measure_Text_Walk_Glyphs_Context *c = (Measure_Text_Walk_Glyphs_Context*)ud;
	
	Gfx_Font_Metrics m = get_font_metrics_scaled(c->font, c->raster_height, c->scale);
	
	float functional_left = glyph_x-glyph.xoffset*c->scale.x;
	float functional_bottom = glyph_y-glyph.yoffset*c->scale.y; // baseline
	float functional_right = functional_left + (glyph.width+glyph.xoffset)*c->scale.x;
	float functional_top = functional_bottom + (m.latin_ascent+glyph.yoffset)*c->scale.y;
	
	c->m.functional_pos_min.x = min(c->m.functional_pos_min.x, functional_left);
	c->m.functional_pos_min.y = min(c->m.functional_pos_min.y, functional_bottom);
	c->m.functional_pos_max.x = max(c->m.functional_pos_max.x, functional_right);
	c->m.functional_pos_max.y = max(c->m.functional_pos_max.y, functional_top);
	
	float visual_left = glyph_x;
	float visual_bottom = glyph_y;
	float visual_right = visual_left + glyph.width*c->scale.x;
	float visual_top = visual_bottom + glyph.height*c->scale.y;
	
	c->m.visual_pos_min.x = min(c->m.visual_pos_min.x, visual_left);
	c->m.visual_pos_min.y = min(c->m.visual_pos_min.y, visual_bottom);
	c->m.visual_pos_max.x = max(c->m.visual_pos_max.x, visual_right);
	c->m.visual_pos_max.y = max(c->m.visual_pos_max.y, visual_top);
	
	return true;
}
Gfx_Text_Metrics measure_text(Gfx_Font *font, string text, u32 raster_height, Vector2 scale) {
	Measure_Text_Walk_Glyphs_Context c = ZERO(Measure_Text_Walk_Glyphs_Context);
	
	c.scale = scale;
	c.font = font;
	c.raster_height = raster_height;
	
	walk_glyphs((Walk_Glyphs_Spec){font, text, raster_height, scale, true, &c}, measure_text_glyph_callback);
	
	c.m.functional_size = v2_sub(c.m.functional_pos_max, c.m.functional_pos_min);
	c.m.visual_size = v2_sub(c.m.visual_pos_max, c.m.visual_pos_min);
	
	return c.m;
}

