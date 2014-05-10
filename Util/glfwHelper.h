
#ifndef GLFWHELPER_H_
#define GLFWHELPER_H_

#include <GL/gl.h>
#include <GL/glext.h>

#include <GLFW/glfw3.h>

#include <iostream>

typedef void (*RenderCallBack)(void);

bool glfwHelperInit(GLFWerrorfun ecbFun)
{
	//Error Handler for GLFW
	glfwSetErrorCallback(ecbFun);
	//Attempt to initialize or exit
	if (!glfwInit())
		return false;

	return true;
}

void glfwHelperTerminate()
{
	glfwTerminate();
}

class glfwWindow
{
	private:
		GLFWwindow* window;
		GLFWkeyfun keyCB;

		RenderCallBack renderCB;

		int wWidth, wHeight;

	public:
		glfwWindow(const char *title, int wWidth, int wHeight);

		void resize(int nWidth, int nHeight);

		void setKeyboardCallback(GLFWkeyfun kcb);
		void setRenderCallback(RenderCallBack rcb);

		void setCurrentContext();

		void render();

		void close();
};

glfwWindow::glfwWindow(const char *title, int width, int height)
{
	keyCB = NULL;
	renderCB = NULL;

	wWidth = width;
	wHeight = height;

	if (!(window = glfwCreateWindow(width, height, title, NULL, NULL)))
	{
		std::cout << "Failed to create Window" << std::endl;
	}
}

void glfwWindow::resize(int nWidth, int nHeight)
{
	wWidth = nWidth;
	wHeight = nHeight;

	glfwSetWindowSize(window, nWidth, nHeight);
}


void glfwWindow::setKeyboardCallback(GLFWkeyfun kcb)
{
	keyCB = kcb;
}

void glfwWindow::setRenderCallback(RenderCallBack rcb)
{
	renderCB = rcb;
}

void glfwWindow::setCurrentContext()
{
	glfwMakeContextCurrent(window);
}

void glfwWindow::render()
{

	while (!glfwWindowShouldClose(window))
	{
		if(renderCB == NULL)
			break;
		renderCB();
		//Swap windows and check for new events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	//Cleanup
	glfwDestroyWindow(window);
	glfwTerminate();

}

void glfwWindow::close()
{
	glfwSetWindowShouldClose(window, GL_TRUE);
}

#endif
