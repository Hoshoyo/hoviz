#include "input.h"
#include <GLFW/glfw3.h>
#include "hoviz.h"

void
mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	switch (action) {
		case GLFW_PRESS: {
            hoviz_input_state.mouse_buttons[button].state = 1;
            hoviz_input_state.mouse_buttons[button].position = hoviz_input_state.mouse_position;
			hoviz_input_state.mouse_buttons[button].mods = mods;
		}break;
		case GLFW_RELEASE: {
			hoviz_input_state.mouse_buttons[button].state = 0;
            hoviz_input_state.mouse_buttons[button].position = hoviz_input_state.mouse_position;
			hoviz_input_state.mouse_buttons[button].mods = mods;
		}break;
		default: return;
	}
}

void
key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    hoviz_input_state.mods = mods;
	if(key >= 0 && key <= 1024) {
        switch (action) {
            case GLFW_PRESS: {
                hoviz_input_state.key_state[key] = 1;
            }break;
            case GLFW_RELEASE: {
                hoviz_input_state.key_state[key] = 0;
                hoviz_input_state.key_event[key] = 1;
            }break;
            case GLFW_REPEAT:{
                hoviz_input_state.key_state[key] = 1;
            }break;
            default: return;
        }
    }
}

void
cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	hoviz_input_state.mouse_position = (vec2){xpos, ypos};
}

void input_set_callbacks(void* glfw_window) {
    glfwSetMouseButtonCallback(glfw_window, mouse_button_callback);
    glfwSetCursorPosCallback(glfw_window, cursor_position_callback);
    glfwSetKeyCallback(glfw_window, key_callback);
}