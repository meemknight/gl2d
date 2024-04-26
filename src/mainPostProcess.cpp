#include <glad/glad.h>
#include <glfw/glfw3.h>
#include "gl2d/gl2d.h"

int main()
{
	// Initialize GLFW
	glfwInit();
	GLFWwindow *window = glfwCreateWindow(840, 640, "Window", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)(glfwGetProcAddress));
	
	// Initialize gl2d
	gl2d::init();


	gl2d::Renderer2D renderer;
	renderer.create();

	// Load resources example
	//gl2d::Font font(RESOURCES_PATH "roboto_black.ttf");
	gl2d::Texture texture(RESOURCES_PATH "test.jpg");
	gl2d::Texture background(RESOURCES_PATH "background.png");

	glm::ivec2 backgroundSize = background.GetSize();


	//auto default = gl2d::createPostProcessShaderFromFile(RESOURCES_PATH "defaultPostProcess.frag");
	auto blur = gl2d::createPostProcessShaderFromFile(RESOURCES_PATH "blur.frag");
	auto removeColors = gl2d::createPostProcessShaderFromFile(RESOURCES_PATH "removeColors.frag");
	gl2d::FrameBuffer fbo;
	gl2d::FrameBuffer fbo2;

	fbo.create(1, 1);
	fbo2.create(1, 1);

	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		//very important, don't forget to call renderer.updateWindowMetrics, 
		//this is probably the thing that I forget most often
		int w = 0; int h = 0;
		glfwGetWindowSize(window, &w, &h);
		renderer.updateWindowMetrics(w, h);
		fbo.resize(w, h);
		fbo2.resize(w, h);



		// Handle input and update

		// Clear screen
		renderer.clearScreen({0, 0, 0, 1});
		fbo.clear();
		fbo2.clear();

		// Render objects
		renderer.renderRectangle({0, 0, backgroundSize}, background);
		
		renderer.renderRectangle({100, 250, 100, 100}, Colors_Orange, {}, 0);
		renderer.renderRectangle({100, 100, 100, 100}, texture, Colors_White, {}, 0);
		renderer.renderRectangle({400, 200, 100, 100}, texture, Colors_White, {}, 0);
		// Add more rendering here...


		//let the library handle it for you
		renderer.flushPostProcess({blur, removeColors});
		

		//you can also post process a texture and render it onto another fbo or the screen!
		//renderer.flushFBO(fbo);
		//renderer.postProcessOverATexture({blur, removeColors}, fbo.texture);


		//manually doing it version 1
		//renderer.flushFBO(fbo);
		//renderer.renderPostProcess(blur, fbo.texture, fbo2);
		//renderer.renderPostProcess(removeColors, fbo2.texture, {});

		//manually doing it version 2
		//renderer.flushFBO(fbo);
		//renderer.renderPostProcess(blur, fbo.texture, fbo2);
		//renderer.renderPostProcess(removeColors, fbo2.texture, fbo);
		//renderer.renderFrameBufferToTheEntireScreen(fbo);

		//manually doing it version 3
		//renderer.flushFBO(fbo);
		//renderer.renderPostProcess(blur, fbo.texture, fbo2);
		//renderer.renderPostProcess(removeColors, fbo2.texture, fbo);
		//renderer.renderRectangle({0,0,w,h},fbo.texture);
		//renderer.flush();

		// Swap buffers and poll events
		glfwSwapBuffers(window);
		glfwPollEvents();
	} 

	//cleanup if you want, no need for it here tho.
	return 0;
}