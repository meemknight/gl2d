#include <glad/glad.h>
#include <glfw/glfw3.h>

#include <iostream>
#include <fstream>
#include <Windows.h>
#include <string>

#include "gl2d/gl2d.h"

int main() 
{
#pragma region glfw

	glfwInit();
	//glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	GLFWwindow *wind = glfwCreateWindow(840, 640, "window", 0, 0);
	glfwMakeContextCurrent(wind);

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

#pragma endregion

	gl2d::init();

	gl2d::setVsync(1);

	gl2d::Renderer2D renderer; //please just make an instance of the object don't allocate it on heap for no reason.
	renderer.create();
	
	gl2d::Font f(RESOURCES_PATH "roboto_black.ttf");
	
	
	gl2d::Texture texture(RESOURCES_PATH "test.jpg");
	gl2d::Texture button(RESOURCES_PATH "button.png", true, true);

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

		if (GetAsyncKeyState('W'))
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
		if (GetAsyncKeyState('Q'))
		{
			renderer.currentCamera.zoom += 1 * deltaTime;
		}
		if (GetAsyncKeyState('E'))
		{
			renderer.currentCamera.zoom -= 1 * deltaTime;
		}
#pragma endregion


		int w = 0; int h = 0;
		glfwGetWindowSize(wind, &w, &h);
		renderer.updateWindowMetrics(w, h);


		renderer.clearScreen({0.1,0.2,0.6,1});


		
		static float pos;
		if (GetAsyncKeyState('O'))
		{
			pos += deltaTime * 20;;
		}
		if (GetAsyncKeyState('P'))
		{
			pos -= deltaTime * 20;
		}
		float t = pos;

		renderer.renderRectangle({100,350, 100, 100}, texture, {1,1,1,1}, {}, t);
		renderer.renderRectangleOutline({100,350, 100, 100}, {1,1,1,0.5}, 10, {}, t);
		renderer.renderRectangle({150,400, 2, 2}, Colors_Orange, {}, t);

		renderer.renderRectangle({200,150, 100, 100}, texture, {1,1,1,1}, {}, t);
		renderer.renderRectangleOutline({200,150, 100, 100}, {1,1,1,0.5}, 10, {}, t);
		renderer.renderRectangle({-1,-1, 2, 2}, Colors_Orange, {}, t);

		renderer.renderCircleOutline({500,200}, Colors_Orange, 100);


		//renderer.renderLine({201, 350}, 0, 100, Colors_White, 2.f);
		if(0)
		{
			double x=0, y = 0;
			glfwGetCursorPos(wind, &x, &y);
			renderer.renderLine({201, 350}, {x,y}, Colors_White, 10.f);
		}

		if (0)
		{
			renderer.renderLine({201, 350}, {300, 650}, Colors_White, 10.f);
			renderer.renderRectangle({201, 350, 2,2}, Colors_Orange);
			renderer.renderRectangle({300, 650, 2,2}, Colors_Orange);
		}

		//glm::vec4 colors[4] = { Colors_Orange,Colors_Orange ,Colors_Orange ,Colors_Orange };
		//renderer.renderRectangle({ 10,10, 100, 100 }, colors, {}, 30);
		
		//renderer.renderRectangle({ 100,150, 100, 100 }, { 1,0,0,0.5 });
		
		//glm::vec4 c[4] = {Colors_Orange, Colors_Orange, Colors_Green, Colors_Blue};
		//renderer.renderRectangle({ 300,300,10,10 }, c);
		
		//renderer.render9Patch2({400, 50, 100, 300}, Colors_White, {}, 0, button, GL2D_DefaultTextureCoords, {0.2,0.8,0.8,0.2});
		//renderer.render9Patch2({600, 150, 300, 100}, Colors_White, {}, 0, button, GL2D_DefaultTextureCoords, {0.2,0.8,0.8,0.2});
		
		if(0) //keyboard
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
		
		//renderer.renderText({0,0}, text.c_str(), f, Colors_White);
		
		if(0) //test view rect
		{
			auto rect = renderer.getViewRect();
		
			std::cout << rect.x << " " << rect.y << " " << rect.z << " " << rect.w << "\n";
		
			rect.x += 5;
			rect.y += 5;
			rect.z -= 10;
			rect.w -= 10;
		
			renderer.renderRectangle(rect, {0.5,0.9,0.6,0.2});
		}

		renderer.flush();

#pragma region glfw

		glfwSwapBuffers(wind);
		glfwPollEvents();

#pragma endregion

	}

	return 0;
}