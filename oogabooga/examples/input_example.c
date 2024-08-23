
// This example is kinda dumb for now, I just log stuff to console.


#define MAX_KEYS_PER_BINDING 3

typedef enum Action {
	ACTION_DASH,
	ACTION_SHOOT,
	
	ACTION_MAX
} Action;

typedef struct Key_Bind {
	Input_Key_Code codes[MAX_KEYS_PER_BINDING];
} Key_Bind;

// Index with action into key bind
Key_Bind key_binds[ACTION_MAX] = {0};

bool is_action_just_pressed(Action action) {
	for (u64 i = 0; i < MAX_KEYS_PER_BINDING; i++) {
		Input_Key_Code code = key_binds[action].codes[i];
		if (code == 0) continue;
		
		if (is_key_just_pressed(code)) return true;
	}
	return false;
}

int entry(int argc, char **argv) {
	
	window.title = STR("Input example");
	window.scaled_width = 1280;
	window.scaled_height = 720; 
	window.x = 200;
	window.y = 90;
	window.clear_color = hex_to_rgba(0x6495EDff);
	
	key_binds[ACTION_DASH].codes[0] = KEY_SPACEBAR;
	key_binds[ACTION_DASH].codes[1] = GAMEPAD_A;
	
	key_binds[ACTION_SHOOT].codes[0] = MOUSE_BUTTON_LEFT;
	key_binds[ACTION_SHOOT].codes[1] = GAMEPAD_RIGHT_BUMPER;

	float64 last_time = os_get_elapsed_seconds();
	while (!window.should_close) {
		reset_temporary_storage();
		
		if (is_key_just_pressed(GAMEPAD_LEFT_TRIGGER)) {
			log("Left trigger");
		}
		if (is_key_just_pressed(GAMEPAD_B)) {
			log("B");
		}
		
		if (is_action_just_pressed(ACTION_DASH)) log("DASH");
		if (is_action_just_pressed(ACTION_SHOOT)) log("PEW PEW");
		
		// Vibrate depending on how far pushed the triggers are
		set_gamepad_vibration(input_frame.left_trigger, input_frame.right_trigger);
		
		// Example to retrieve axes for multiple gamepads
		for (u64 i = 0; i < input_frame.number_of_events; i++) {
			Input_Event e = input_frame.events[i];
			
			switch (e.kind) {
				case INPUT_EVENT_GAMEPAD_AXIS: {
					
					if (e.axes_changed & INPUT_AXIS_LEFT_STICK) log("Gamepad %d left stick: %f %f", e.gamepad_index, e.left_stick.x, e.left_stick.y);
					if (e.axes_changed & INPUT_AXIS_RIGHT_STICK) log("Gamepad %d right stick: %f %f", e.gamepad_index, e.right_stick.x, e.right_stick.y);
					if (e.axes_changed & INPUT_AXIS_LEFT_TRIGGER)  log("Gamepad %d left trigger: %f", e.gamepad_index, e.left_trigger);
					if (e.axes_changed & INPUT_AXIS_RIGHT_TRIGGER) log("Gamepad %d right trigger: %f", e.gamepad_index, e.right_trigger);
					
					break;
				}
				default: break;
			}
		}
		
		os_update(); 
		gfx_update();
	}

	return 0;
}