#include <gl/glew.h>
#include <glfw/glfw3.h>

#include <iostream>
#include <fstream>
#include <Windows.h>
#include <string>

#include "gl2d.h"

int main() 
{
#pragma region glfw

	glfwInit();
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	GLFWwindow *wind = glfwCreateWindow(840, 640, "window", 0, 0);
	glfwMakeContextCurrent(wind);

	glewInit();

#pragma endregion

	gl2d::init();

	gl2d::Renderer2D renderer;
	renderer.create();
	
	gl2d::Font f("roboto_black.ttf");
	gl2d::Texture texture("test.jpg");

	//std::cout << texture.GetSize().x;

	float speed = 400;

	long lastTime = GetTickCount();

	std::string text = {};

	while (!glfwWindowShouldClose(wind))
	{

#pragma region movement

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
		if (GetAsyncKeyState('A'))
		{
			renderer.currentCamera.position.x -= speed * deltaTime;
		}
		if (GetAsyncKeyState('D'))
		{
			renderer.currentCamera.position.x += speed * deltaTime;
		}
#pragma endregion


		int w = 0; int h = 0;
		glfwGetWindowSize(wind, &w, &h);
		renderer.updateWindowMetrics(w, h);


		renderer.clearScreen();

		//necessary only if you modify gl settings
		//gl2d::enableNecessaryGLFeatures();

		renderer.renderRectangle({ 100,350, 100, 100 }, { 0,0 }, 0, texture);

		glm::vec4 colors[4] = { Colors_Orange,Colors_Orange ,Colors_Orange ,Colors_Orange };
		renderer.renderRectangle({ 10,10, 100, 100 }, colors, {}, 30);

		renderer.renderText({ 0,100 }, "text Text", f, Colors_Red);
		renderer.renderText({ 0,200 }, "text Vlad", f, Colors_Blue );
		renderer.renderText({ 0,300 }, "text Mihai", f, Colors_Green);

		renderer.renderRectangle({ 100,150, 100, 100 }, {1,0,0,0.5}, { 0,0 }, 0);


		renderer.renderRectangle({ 300,300,10,10 }, Colors_Orange);

		glm::vec2 textSize = renderer.getTextSize(text.c_str(), f);
		renderer.renderText({ 300,300 }, text.c_str(), f, Colors_Gray,
			1.5, 4.0, 3, false);


		for(int i=8;i<255;i++)
		{
			if(GetAsyncKeyState(i) == -32767 )
			{
			
				if(i == VK_BACK)
				{
					if(text.length())
					text = std::string(text.begin(), text.end() - 1);
				}
				else if (i == VK_SPACE)
				{
					text += ' ';
				}else if (i == VK_RETURN)
				{
					text += '\n';
				}else if(i == VK_OEM_3)
				{
					text += '`';
				}
				else
				{
					text += char(i);
				}

			}
		
		}

		renderer.flush();

#pragma region glfw

		glfwSwapBuffers(wind);
		glfwPollEvents();

#pragma endregion

	}

	return 0;
}