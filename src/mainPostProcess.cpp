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
	
	//I set the version to test if my library still works with older versions, but you don't
	//need to care about that
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Initialize gl2d
	gl2d::init();


	gl2d::Renderer2D renderer;
	renderer.create();

	// Load resources example
	//gl2d::Font font(RESOURCES_PATH "roboto_black.ttf");
	gl2d::Texture texture(RESOURCES_PATH "test.jpg");
	gl2d::Texture background(RESOURCES_PATH "background.png");

	glm::ivec2 backgroundSize = background.GetSize();

	
	//postProcess = gl2d::createPostProcessShaderFromFile(RESOURCES_PATH "defaultPostProcess.frag");
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


		renderer.flushFBO(fbo);

		renderer.renderPostProcessSameSize(blur, fbo.texture, fbo2);

		renderer.renderPostProcessSameSize(removeColors, fbo2.texture, fbo);
		
		renderer.renderFrameBufferToTheEntireScreen(fbo);


		//renderer.renderRectangle({0,0,w,h},fbo.texture);
		//renderer.flush();

		// Swap buffers and poll events
		glfwSwapBuffers(window);
		glfwPollEvents();
	} 

	//cleanup if you want, no need for it here tho.
	return 0;
}