#include <gl/glew.h>
#include <glfw/glfw3.h>

#include <iostream>
#include <fstream>
#include <Windows.h>

#include "opengl2Dlib.h"

int main() 
{
	glfwInit();
	GLFWwindow *wind = glfwCreateWindow(620, 420, "window", 0, 0);
	glfwMakeContextCurrent(wind);
	glewInit();

	gl2d::init();

	gl2d::Renderer2D renderer;
	renderer.create();
	
	gl2d::Font f("roboto_black.ttf");

	while (!glfwWindowShouldClose(wind))
	{
		int w = 0; int h = 0;
		glfwGetWindowSize(wind, &w, &h);
		renderer.updateWindowMetrics(w, h);
		renderer.clearScreen();

		glfwPollEvents();

		gl2d::enableNecessaryGLFeatures();
		renderer.resetCameraAndShader();

		glm::vec4 colors[4] = { Colors_Orange,Colors_Orange ,Colors_Orange ,Colors_Orange };
		renderer.renderRectangle({ 10,10, 100, 100 }, colors, {}, 30);

		renderer.renderText({ 0,100 }, "text Text", 9, f, Colors_Red);

		renderer.flush();

		glfwSwapBuffers(wind);
	}

	return 0;
}