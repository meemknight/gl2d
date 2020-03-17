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
	gl2d::Texture texture("test.jpg");

	std::cout << texture.GetSize().x;

	float speed = 40;

	renderer.resetCameraAndShader();

	long lastTime = GetTickCount();

	while (!glfwWindowShouldClose(wind))
	{
		long newTime = GetTickCount();
		float deltaTime = (float)(newTime - lastTime) / 1000.f;
		lastTime = GetTickCount();

		if(GetAsyncKeyState('W'))
		{
			renderer.currentCamera.position.y -= speed * deltaTime;
		}
		if (GetAsyncKeyState('S'))
		{
			renderer.currentCamera.position.y += speed * deltaTime;
		}

		int w = 0; int h = 0;
		glfwGetWindowSize(wind, &w, &h);
		renderer.updateWindowMetrics(w, h);
		renderer.clearScreen();

		glfwPollEvents();

		gl2d::enableNecessaryGLFeatures();

		renderer.renderRectangle({ 100,350, 100, 100 }, { 0,0 }, 0, texture);

		glm::vec4 colors[4] = { Colors_Orange,Colors_Orange ,Colors_Orange ,Colors_Orange };
		renderer.renderRectangle({ 10,10, 100, 100 }, colors, {}, 30);

		renderer.renderText({ 0,100 }, "text Text", f, Colors_Red);
		renderer.renderText({ 0,200 }, "text Vlad", f, Colors_Blue );
		renderer.renderText({ 0,300 }, "text Mihai", f, Colors_Green);

		renderer.renderRectangle({ 100,150, 100, 100 }, {0,0}, 0, texture);

		renderer.flush();

		glfwSwapBuffers(wind);
	}

	return 0;
}