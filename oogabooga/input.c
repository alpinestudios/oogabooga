/*

	Main input juice:
	
	bool is_key_down(Input_Key_Code code);
	bool is_key_up(Input_Key_Code code);
	bool is_key_just_pressed(Input_Key_Code code);
	bool is_key_just_released(Input_Key_Code code);
	
	bool consume_key_down(Input_Key_Code code);
	bool consume_key_just_pressed(Input_Key_Code code);
	bool consume_key_just_released(Input_Key_Code code);

	// To loop through events this frame
	for (u64 i = 0; i < input_frame.number_of_events; i++) {
		Input_Event e = input_frame.events[i];
		
		switch (e.kind) {
			case INPUT_EVENT_KEY:    ...; break;
			case INPUT_EVENT_SCROLL: ...; break;
			case INPUT_EVENT_TEXT:   ...; break;
		}
	}
*/

typedef enum Input_Event_Kind {
	INPUT_EVENT_KEY,
	INPUT_EVENT_SCROLL,
	INPUT_EVENT_TEXT,
} Input_Event_Kind;

typedef enum Input_Key_Code {
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

    KEY_ALT,
    KEY_CTRL,
    KEY_SHIFT,
    KEY_CMD,
    KEY_META = KEY_CMD,

    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,

    KEY_PRINT_SCREEN,

    MOUSE_BUTTON_LEFT,
    MOUSE_BUTTON_MIDDLE,
    MOUSE_BUTTON_RIGHT,
    
    MOUSE_FIRST = MOUSE_BUTTON_LEFT,
    MOUSE_LAST  = MOUSE_BUTTON_RIGHT,
    
    INPUT_KEY_CODE_COUNT
} Input_Key_Code;

typedef enum Input_State_Flags {
	INPUT_STATE_DOWN          = 1<<0,
	INPUT_STATE_JUST_PRESSED  = 1<<1,
	INPUT_STATE_JUST_RELEASED = 1<<2,
	INPUT_STATE_REPEAT        = 1<<3,
} Input_State_Flags;

typedef struct Input_Event {
	Input_Event_Kind kind;
	
	// For INPUT_EVENT_KEY
	Input_Key_Code key_code;
	Input_State_Flags key_state;
	
	// For INPUT_EVENT_SCROLL
	float64 xscroll;
	float64 yscroll;
	
	// For INPUT_EVENT_TEXT_INPUT
	union { u32 utf32; char ascii; };
	
} Input_Event;

Input_Key_Code os_key_to_key_code(void* os_key);
void* key_code_to_os_key(Input_Key_Code key_code);

#define MAX_EVENTS_PER_FRAME 10000
typedef struct Input_Frame {
	Input_Event events[MAX_EVENTS_PER_FRAME];
	u64 number_of_events;
	
	float32 mouse_x;
	float32 mouse_y;
	
	Input_State_Flags key_states[INPUT_KEY_CODE_COUNT];
	
} Input_Frame;

// #Global
ogb_instance Input_Frame input_frame;

#if !OOGABOOGA_LINK_EXTERNAL_INSTANCE
Input_Frame input_frame = ZERO(Input_Frame);
#endif

bool has_key_state(Input_Key_Code code, Input_State_Flags flags) {
	assert(code > 0 && code < INPUT_KEY_CODE_COUNT, "Invalid key code %d!", code);
	Input_State_Flags state = input_frame.key_states[code];
	
#if CONFIGURATION == DEBUG
	{
		Input_State_Flags impossible = (INPUT_STATE_JUST_RELEASED | INPUT_STATE_DOWN);
		assert((impossible & state) != impossible, "Key state for key '%d' is corrupt!", code);
		impossible = (INPUT_STATE_JUST_RELEASED | INPUT_STATE_JUST_PRESSED);
		assert((impossible & state) != impossible, "Key state for key '%d' is corrupt!", code);
	}
#endif
	return (state & flags) == flags;
}
bool is_key_down(Input_Key_Code code) {
	return has_key_state(code, INPUT_STATE_DOWN);
}
bool is_key_up(Input_Key_Code code) {
	return input_frame.key_states[code] == 0 || has_key_state(code, INPUT_STATE_JUST_RELEASED);
}
bool is_key_just_pressed(Input_Key_Code code) {
	return has_key_state(code, INPUT_STATE_JUST_PRESSED);
}
bool is_key_just_released(Input_Key_Code code) {
	return has_key_state(code, INPUT_STATE_JUST_RELEASED);
}

bool consume_key_down(Input_Key_Code code) {
	bool result = is_key_down(code);
	input_frame.key_states[code] &= ~(INPUT_STATE_DOWN);
	return result;
}
bool consume_key_just_pressed(Input_Key_Code code) {
	bool result = is_key_just_pressed(code);
	input_frame.key_states[code] &= ~(INPUT_STATE_JUST_PRESSED);
	return result;
}
bool consume_key_just_released(Input_Key_Code code) {
	bool result = is_key_just_released(code);
	input_frame.key_states[code] &= ~(INPUT_STATE_JUST_RELEASED);
	return result;
}