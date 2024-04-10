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

	gl2d::ShaderProgram colorShader = gl2d::createShaderFromFile(RESOURCES_PATH "removeColors.frag");
	GLuint u_strength = glGetUniformLocation(colorShader.id, "u_strength");

	//just for example
	gl2d::ShaderProgram defaultShader = gl2d::createShaderFromFile(RESOURCES_PATH "defaultRenderShader.frag");


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
		

		renderer.renderRectangle({100, 100, 100, 100}, texture);
		
		
		//flush everything before
		renderer.flush();

		//IT'S NOT GOOD TO CHANGE THE SHADER TOO MANY TIMES
		//IF YOU NEED TO RENDER AN EFFECT FOR 100 OBJECTS FOR EXAMPLE,
		//RENDER ALL OF THE NORMAL OBJECTS THAN THE EFFECTS OBJECTS, OR SOMETHING SIMILAR.
		//IF THE NUMBER OF FLUSHES OR SHADER BINDS IS INSIDE A BIG LOOP THAT WILL QUICKLY
		//KILL YOUR FPS. IF IT IS A SENSIBLE NUMBER LIKE 20 THERE IS NO PROBLEM.
		renderer.pushShader(colorShader);
		//custom unfiorm
		colorShader.bind();
		glUniform1i(u_strength, 5);
		renderer.renderRectangle({300, 100, 100, 100}, 
			texture);
		renderer.flush();
		renderer.popShader();
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