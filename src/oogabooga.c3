module engine;

import std::os::win32;

def Win32_HANDLE = Win32_PVOID;
def Win32_HWND = Win32_HANDLE;

const float SPRITE_PIXEL_SIZE = 16;
const Vector4 COLOR_WHITE = {{ 1, 1, 1, 1 }};
// const Matrix4 IDENTITY = 

// define m4_identity m4_make_scale(v3(1, 1, 1))

struct CString {
	ulong count;
	char *data;
}


// def Vector2 = float[<2>];
// def Vector3 = float[<3>];
// def Vector4 = float[<4>];

struct OsWindow {	
	CString title; 
	union { int width;  int pixel_width;  }
	union { int height; int pixel_height; }
	
	int scaled_width;
	int scaled_height;
	int x;
	int y;
	Vector4 clear_color;
	bool enable_vsync;
	bool should_close;
	// Read Only
	bool _initialized;
	Win32_HWND _os_handle;
}


enum InputKeyCode : (int value) {
	KEY_UNKNOWN = 0,

    // Non-textual keys that have placements in the ASCII table
    // (and thus in Unicode):

    KEY_BACKSPACE   = 8,
    KEY_TAB         = 9,
    KEY_ENTER       = 13,
    KEY_ESCAPE      = 27,
    KEY_SPACEBAR    = 32,

    // The letters A-Z live in here as well and may be returned
    // by keyboard events.

    KEY_DELETE      = 127,

    KEY_ARROW_UP    = 128,
    KEY_ARROW_DOWN  = 129,
    KEY_ARROW_LEFT  = 130,
    KEY_ARROW_RIGHT = 131,

    KEY_PAGE_UP     = 132,
    KEY_PAGE_DOWN   = 133,

    KEY_HOME        = 134,
    KEY_END         = 135,

    KEY_INSERT      = 136,

    KEY_PAUSE       = 137,
    KEY_SCROLL_LOCK = 138,

    KEY_ALT = 139,
    KEY_CTRL = 140,
    KEY_SHIFT = 141,
    KEY_CMD = 142,
    KEY_META = 142, // should be same as KEY_CMD

    KEY_F1 = 143,
    KEY_F2 = 144,
    KEY_F3 = 145,
    KEY_F4 = 146,
    KEY_F5 = 147,
    KEY_F6 = 148,
    KEY_F7 = 149,
    KEY_F8 = 150,
    KEY_F9 = 151,
    KEY_F10 = 152,
    KEY_F11 = 153,
    KEY_F12 = 154,

    KEY_PRINT_SCREEN = 155,

    MOUSE_BUTTON_LEFT = 156,
    MOUSE_BUTTON_MIDDLE = 157,
    MOUSE_BUTTON_RIGHT = 158,
    
    MOUSE_FIRST = 156,
    MOUSE_LAST = 158, 
    
    INPUT_KEY_CODE_COUNT = 161
}


union Vector2 {
	float[2] data;
	struct {float  x, y;}
}

fn Vector2 Vector2.add(self, Vector2 b)
{
	return v2_add(self, b);
}

fn bool Vector2.equals(self, Vector2 b)
{
	return self.x == b.x && self.y == b.y;
}

fn Vector2 Vector2.mulf(self, float b)
{
	return v2_mulf(self, b);
}

fn Vector2 Vector2.sub(self, Vector2 b) {
	return v2_sub(self, b);
}

fn float Vector2.length(self) {
	return v2_length(self);
}

fn float Vector2.distance(self, Vector2 b) {
	return v2_length(v2_sub(self, b));
}

fn Vector2 Vector2.normalize(&self)
{
    return v2_normalize(*self);
}

union Vector3 {
	float[3] data;
	struct  {float  x, y, z;}
	struct  {float  r, g, b;}
	struct  {float  xy;}
	struct  {float  _x; Vector2 yz;}
}

 union Vector4 @align(16) {
	float[4] data;
	struct {float  x, y, z, w;}
	struct {Vector2  xy; Vector2 zw;}
	struct {float  x1, y1, x2, y2;}
	struct {float  r, g, b, a;}
	struct {float  left, bottom, right, top;}
	struct {Vector3  xyz;}
	struct {float  _x; Vector3 yzw;}
}

struct StbBuff
{
   char *data;
   int cursor;
   int size;
}

struct StbttFontinfo
{
   void           * userdata;
   char  * data;              // pointer to .ttf file
   int              fontstart;         // offset of start of font

   int numGlyphs;                     // number of glyphs, needed for range checking

   int loca,head,glyf,hhea,hmtx,kern,gpos,svg; // table locations as offset from start of .ttf
   int index_map;                     // a cmap mapping for our chosen character encoding
   int indexToLocFormat;              // format needed to map from glyph index to glyph

   StbBuff cff;                    // cff font data
   StbBuff charOoggaStrings;            // the charOoggaString index
   StbBuff gsubrs;                 // global charOoggaString subroutines index
   StbBuff subrs;                  // private charOoggaString subroutines index
   StbBuff fontdicts;              // array of font dicts
   StbBuff fdselect;               // map from glyph to fontdict
}

enum Allocator_Message {
	ALLOCATOR_ALLOCATE,
	ALLOCATOR_DEALLOCATE,
	ALLOCATOR_REALLOCATE,
}

def AllocatorProc = fn void(long, void*, Allocator_Message, void*);

struct Allocator {
	AllocatorProc proc;
	void *data;	
}

struct HashTable {
	// Each entry is hash-key-value
	// Hash is sizeof(u64) bytes, key is _key_size bytes and value is _value_size bytes
	void *entries; 
	ulong count; // Number of valid entries
	ulong capacity_count; // Number of allocated entries
	ulong _key_size;
	ulong _value_size;
	Allocator allocator;
}

 struct GfxFontMetrics {
	float latin_ascent;
	float latin_descent;
	
	float max_ascent;
	float max_descent;

	float line_spacing;	
}

struct GfxFontVariation {
	GfxFont *font;
	uint height;
	GfxFontMetrics metrics;
	float scale;
	uint codepoint_range_per_atlas;
	HashTable atlases; // u32 atlas_index, Gfx_Font_Atlas
	bool initted;
}

const int MAX_FONT_HEIGHT = 512;
struct GfxFont {
	StbttFontinfo stbtt_handle;
	CString raw_font_data;
	GfxFontVariation[MAX_FONT_HEIGHT] variations; // Variation per font height
	Allocator allocator;
}

struct GfxImage {
	uint width, height, channels;
	int gfx_handle;
	Allocator allocator;
}


fn ulong length_of_null_terminated_string(char* cstring) {
	ulong len = 0;
	while (*cstring != 0) {
		len += 1;
		cstring += 1;
	}
	return len;
}

enum GfxFilterMode {
	GFX_FILTER_MODE_NEAREST,
	GFX_FILTER_MODE_LINEAR,
}

struct DrawQuad {
	// BEWARE !! These are in ndc
	Vector2 bottom_left, top_left, top_right, bottom_right;
	// r, g, b, a
	Vector4 color;
	GfxImage *image;
	GfxFilterMode image_min_filter;
	GfxFilterMode image_mag_filter;
	int z;
	char type;
	bool has_scissor;
	// x1, y1, x2, y2
	Vector4 uv;
	Vector4 scissor;
	
	Vector4[1] userdata; // #Volatile do NOT change this to a pointer	
}

struct Matrix4 {
    union {float[4][4] m; float[16] data; }
}

struct DrawFrame {
	ulong num_quads;
	
	Matrix4 projection;
	Matrix4 view;
	
	bool enable_z_sorting;
	int[4096] z_stack;
	ulong z_count;

	Vector4[4096] scissor_stack;
	int scissor_count;
	
	void *cbuffer;
	
}

distinct Input_State_Flags = int;
const Input_State_Flags INPUT_STATE_DOWN = 1<<0;
const Input_State_Flags INPUT_STATE_JUST_PRESSED = 1 << 1;
const Input_State_Flags INPUT_STATE_JUST_RELEASED = 1 << 2;
const Input_State_Flags INPUT_STATE_REPEAT = 1 << 3;

enum Input_Event_Kind {
	INPUT_EVENT_KEY,
	INPUT_EVENT_SCROLL,
	INPUT_EVENT_TEXT,
}

struct Input_Event {
	Input_Event_Kind kind;
	
	// For INPUT_EVENT_KEY
	InputKeyCode key_code;
	Input_State_Flags key_state;
	
	// For INPUT_EVENT_SCROLL
	double xscroll; // float64
	double yscroll;
	
	// For INPUT_EVENT_TEXT_INPUT
	union { uint utf32; char ascii; } // u32; char;
	
}

struct Input_Frame {
	Input_Event[10000] events;
	ulong number_of_events; //u64
	
	float mouse_x;
	float mouse_y;
	
	Input_State_Flags[InputKeyCode.INPUT_KEY_CODE_COUNT.value] key_states;
	
}



extern fn void push_z_layer(int z);
extern fn void pop_z_layer();
extern fn void push_window_scissor(Vector2 min, Vector2 max);
extern fn void pop_window_scissor();


extern fn DrawQuad *draw_image(GfxImage *image, Vector2 position, Vector2 size, Vector4 color);
extern fn DrawQuad *draw_image_xform(GfxImage *image, Matrix4, Vector2 size, Vector4 color);
extern fn DrawQuad *draw_rect(Vector2 position, Vector2 size, Vector4 color);
extern fn DrawQuad *draw_rect_xform(Matrix4 xform, Vector2 size, Vector4 color);
extern fn DrawQuad *draw_circle_xform(Matrix4, Vector2, Vector4);
extern fn DrawQuad *draw_circle(Vector2, Vector2, Vector4);
extern fn void draw_line(Vector2 p0, Vector2 p1, float line_width, Vector4 color);

extern fn GfxImage *load_image_from_disk(CString path, Allocator allocator);

extern fn void oogabooga_init(ulong);
extern fn void os_update();
extern fn void gfx_update();
extern fn bool os_read_entire_file_s(CString path, CString *result, Allocator allocator);
extern fn bool shader_recompile_with_extension(CString, ulong);

extern fn void dealloc_string(Allocator allocator, CString s);

extern fn double os_get_current_time_in_seconds();
extern fn void os_high_precision_sleep(double);
extern fn ulong os_get_current_cycle_count();

extern fn float get_random_float32_in_range(float, float);
extern fn float get_random_float32();

// GLOBALS
extern engine::OsWindow window;
extern engine::DrawFrame draw_frame;
extern engine::Input_Frame input_frame;
extern ulong seed_for_random;

extern fn Vector4 hex_to_rgba(long);

extern fn void reset_temporary_storage();
extern fn Allocator get_heap_allocator();
extern fn Allocator get_temp_storage();



extern fn bool is_key_down(int);
extern fn bool is_key_just_released(int);
extern fn bool is_key_just_pressed(int);

extern fn Matrix4 m4_scalar(float);
extern fn Matrix4 m4_mul(Matrix4 a, Matrix4 b);
extern fn Matrix4 m4_make_orthographic_projection(float, float, float, float, float, float);
extern fn Matrix4 m4_make_scale(Vector3);
extern fn Matrix4 m4_make_translation(Vector3);
extern fn Matrix4 m4_translate(Matrix4, Vector3);
extern fn Matrix4 m4_scale(Matrix4, Vector3);
extern fn Matrix4 m4_rotate_z(Matrix4, float);
extern fn Vector4 m4_transform(Matrix4, Vector4);
extern fn Matrix4 m4_inverse(Matrix4);

extern fn Vector2 v2_normalize(Vector2);
extern fn Vector2 v2_add(Vector2, Vector2);
extern fn Vector2 v2_sub(Vector2, Vector2);
extern fn float v2_length(Vector2);
extern fn Vector2 v2_mulf(Vector2, float);

extern fn double fmod(double, double);
extern fn double fabs(double);

extern fn double lerpf(double from, double to, double x);
extern fn long lerpi(long from, long to, double x);
extern fn double smerpf(double from, double to, double t);
extern fn long smerpi(long from, long to, double t);