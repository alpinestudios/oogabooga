

int entry(int argc, char **argv) {
	
	window.title = fixed_string("My epic game");
	//window.width = 400;
	//window.height = 400;
	//window.x = 200;
	//window.y = 200;
	
	window.x = 0;
	window.y = 0;
	
	while (!window.should_close) {
		reset_temporary_storage();
		context.allocator = temp;
		
		print("%d, %d, %d, %d\n", window.x, window.y, window.width, window.height);
		
		glBegin(GL_QUADS);
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex2f(-0.5f, -0.5f);
		glVertex2f(0.5f, -0.5f);
		glVertex2f(0.5f, 0.5f);
		glVertex2f(-0.5f, 0.5f);
		glEnd();
		
		os_update();
	}

	return 0;
}

