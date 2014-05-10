
#include <gl/gl.h>
#include <glfw/glfw3.h>


#include <cstdlib>
#include <cstdio>
#include <iostream>

static void error_callback(int error, const char* description)
{
	std::cerr << "ERROR: " << description << std::endl;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}
