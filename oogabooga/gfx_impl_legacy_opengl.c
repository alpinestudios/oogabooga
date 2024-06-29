
// #Temporary #Cleanup
// #Temporary #Cleanup
// #Temporary #Cleanup
// #Temporary #Cleanup
#include "GL/gl.h"
HDC hdc;
typedef BOOL (APIENTRY *PFNWGLSWAPINTERVALEXTPROC) (int interval);

void gfx_init() {
	// #Temporary #Cleanup
    // #Temporary #Cleanup
    // #Temporary #Cleanup
    // #Temporary #Cleanup
    
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0,
        0, 0,
        0, 0, 0, 0, 0,
        24,
        8,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    hdc = GetDC(window._os_handle);
    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pixelFormat, &pfd);

    HGLRC hglrc = wglCreateContext(hdc);
    wglMakeCurrent(hdc, hglrc);
    
    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC) wglGetProcAddress("wglSwapIntervalEXT");
    
    assert(wglSwapIntervalEXT, "Could not load wglSwapIntervalEXT");
    
    wglSwapIntervalEXT(0);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
}

void gfx_update() {
	
	Draw_Quad_Block *block = &draw_frame.first_block;
	glBegin(GL_QUADS);
	while (block != 0) {
		
		for (u64 i = 0; i < block->num_quads; i++) {
			Draw_Quad q = block->quad_buffer[i];
			
			glColor4f(q.color.r, q.color.g, q.color.b, q.color.a);
			glVertex2f(v2_expand(q.bottom_left));
			glVertex2f(v2_expand(q.top_left));
			glVertex2f(v2_expand(q.top_right));
			glVertex2f(v2_expand(q.bottom_right));
		}
		
		block = block->next;
	}
	glEnd();
	
	SwapBuffers(hdc);
	glClearColor(window.clear_color.r, window.clear_color.g, window.clear_color.b, window.clear_color.a);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, window.width, window.height);
	
	draw_frame = ZERO(Draw_Frame);
}