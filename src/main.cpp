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
	gl2d::Font font(RESOURCES_PATH "roboto_black.ttf", false);
	gl2d::Texture texture(RESOURCES_PATH "test.jpg");

	//you can manually change the font spacing
	//font.spaceSize = 20;

	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		//very important, don't forget to call renderer.updateWindowMetrics, 
		//this is probably the thing that I forget most often
		int w = 0; int h = 0;
		glfwGetWindowSize(window, &w, &h);
		renderer.updateWindowMetrics(w, h);


		// Handle input and update

		// Clear screen
		renderer.clearScreen({0.1, 0.2, 0.6, 1});


		// Render objects
		//renderer.renderRectangle({100, 250, 100, 100}, Colors_Orange, {}, 0);
		renderer.renderRectangle({100, 100, 100, 100}, texture, Colors_White, {}, 0);

		//renderer.renderText({500, 500}, "render text test !\n# # # #\n####", font, Colors_White, 64, 4, 0, true);
		//renderer.renderRectangle({500, 500, 4, 4}, Colors_Red, {}, 0);

		// Add more rendering here...

		


		// Flush renderer (dump your rendering into the screen)
		renderer.flush();

		// Swap buffers and poll events
		glfwSwapBuffers(window);
		glfwPollEvents();
	} 

	//cleanup if you want, no need for it here tho.
	return 0;
}