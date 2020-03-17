#pragma once
#include <GL/glew.h>
#include <iostream>
#include <glm/glm.hpp>
#include "stb_image.h"
#include "stb_truetype.h"

namespace gl2d
{
	void init();

	void defaultErrorFunc(const char* msg);

	using errorFuncType = decltype(defaultErrorFunc);

	errorFuncType *setErrorFuncCallback(errorFuncType *newFunc);

	struct Font;

	namespace internal
	{
		struct ShaderProgram
		{
			GLuint id;
			int u_sampler;
		};

		float positionToScreenCoordsX(const float position, float w);
		float positionToScreenCoordsY(const float position, float h);

		stbtt_aligned_quad fontGetGlyphQuad(const Font font, const char c);
		glm::vec4 fontGetGlyphTextureCoords(const Font font, const char c);

	}

	///////////////////// COLOR ///////////////////
#pragma region Color

	using Color4f = glm::vec4;
#define Colors_Red (gl2d::Color4f{ 1, 0, 0, 1 })
#define Colors_Green (gl2d::Color4f{ 0, 1, 0, 1 })
#define Colors_Blue (gl2d::Color4f{ 0, 0, 1, 1 })
#define Colors_Black (gl2d::Color4f{ 0, 0, 0, 1 })
#define Colors_White (gl2d::Color4f{ 1, 1, 1, 1 })
#define Colors_Yellow (gl2d::Color4f{ 1, 1, 0, 1 })
#define Colors_Magenta (gl2d::Color4f{ 1, 0, 1, 1 })
#define Colors_Turqoise (gl2d::Color4f{ 0, 1, 1, 1 })
#define Colors_Orange (gl2d::Color4f{ 1, (float)0x7F / 255.0f, 0, 1 })
#define Colors_Purple (gl2d::Color4f{ 101.0f / 255.0f, 29.0f / 255.0f, 173.0f / 255.0f, 1 })
#define Colors_Gray (gl2d::Color4f{ (float)0x7F / 255.0f, (float)0x7F / 255.0f, (float)0x7F / 255.0f, 1 })

#pragma endregion

	///////////////////// MATH ////////////////////
#pragma region math

	using Rect = glm::vec4;
	glm::vec2 rotateAroundPoint(glm::vec2 vec, glm::vec2 point, const float degrees);
	glm::vec2 scaleAroundPoint(glm::vec2 vec, glm::vec2 point, float scale);

#pragma endregion

	///////////////////// Texture /////////////////////
#pragma region Texture

	struct Texture
	{
		GLuint id = 0;

		Texture() {};
		Texture(const char *file) { loadFromFile(file); }

		glm::ivec2 GetSize();

		//Note: This function expects a buffer of bytes in GL_RGBA format
		void createFromBuffer(const char* image_data, const int width, const int height);
		void create1PxSquare();
		void createFromFileData(const unsigned char* image_file_data, const size_t image_file_size);

		void loadFromFile(const char *fileName);

		void bind(const unsigned int sample = 0);
		void unbind();

		void cleanup();
	};

	struct TextureRegion
	{
		Texture texture;
		glm::vec4 textureCoords;
	};

#pragma endregion

	///////////////////// Font /////////////////////
#pragma region Font
#define Default_Font_Characters_Range_Begin cast(char, ' ')
#define Default_Font_Characters_Range_End cast(char, '~')
#define Default_Font_Characters_Range_Size cast(isize, Default_Font_Characters_Range_End - Default_Font_Characters_Range_Begin)

	typedef float Font_Size;

	typedef struct Font Font;
	struct Font
	{
		Texture           texture;
		glm::ivec2        size;
		stbtt_packedchar* packedCharsBuffer;
		int               packedCharsBufferSize;
		float             max_height;

		Font() {}
		explicit Font(const char* file) { createFromFile(file); }

		void createFromTTF(const unsigned char* ttf_data, const size_t ttf_data_size);
		void createFromFile(const char *file);
	};

#pragma endregion

	///////////////////// Camera /////////////////////
#pragma region Camera

	struct Camera;
	Camera cameraCreateDefault();

	struct Camera
	{
		glm::vec2  position;
		glm::vec2  offset;   // Camera offset (displacement from target)
		glm::vec2  target;   // Camera target (rotation and zoom origin)
		float rotation; // Camera rotation in degrees
		float zoom;     // Camera zoom (scaling), should be 1.0f by default

		void setDefault() { *this = cameraCreateDefault(); }
		glm::mat3 getMatrix();
	};


#pragma endregion

	///////////////////// Renderer2d /////////////////////
#pragma region Renderer2d

	typedef glm::vec2 Position2D;
	typedef glm::vec4 Texture_Coords;

	struct FrameBuffer
	{
		unsigned int fbo;
		Texture texture;
		//todo remove
		unsigned int depthtTexture;

		void create(unsigned int w, unsigned int h);
		void resize(unsigned int w, unsigned int h);

		//clears resources
		void cleanup();

		//clears colors
		void clear();
	};


#define Renderer2D_Max_Buffer_Capacity 1000
#define DefaultTextureCoords (glm::vec4{ 0, 1, 1, 0 })

	enum Renderer2DBufferType
	{
		quadPositions,
		quadColors,
		texturePositions,

		bufferSize
	};

	typedef struct Renderer2D Renderer2D;
	struct Renderer2D
	{
		Renderer2D() {};

		void create();

		GLuint buffers[Renderer2DBufferType::bufferSize];
		GLuint vao;

		//Note: Just for testing purposes
		//4 elements each component
		glm::vec2 spritePositions[Renderer2D_Max_Buffer_Capacity];
		glm::vec4 spriteColors[Renderer2D_Max_Buffer_Capacity];
		glm::vec2 texturePositions[Renderer2D_Max_Buffer_Capacity];
		Texture   spriteTextures[Renderer2D_Max_Buffer_Capacity];

		int spritePositionsCount = 0;
		int spriteColorsCount = 0;
		int texturePositionsCount = 0;
		int spriteTexturesCount = 0;

		Texture white1pxSquareTexture;

		internal::ShaderProgram currentShader;
		Camera currentCamera;

		//window metrics, should be up to date at all times
		int windowW = 0;
		int windowH = 0;
		void updateWindowMetrics(int w, int h) { windowW = w; windowH = h; }

		// The origin will be the bottom left corner since it represents the line for the text to be drawn
		//Pacing and lineSpace are influenced by size
		void renderText(const glm::vec2 position, const char* text, const Font font, const Color4f color, const float size = 1.5f, const float spacing = 4, const float line_space = 3);

		//todo color overloads
		void renderRectangle(const Rect transforms, const Color4f colors[4], const glm::vec2 origin, const float rotation, const Texture texture, const glm::vec4 textureCoords = DefaultTextureCoords);
		inline void renderRectangle(const Rect transforms, const Color4f colors, const glm::vec2 origin, const float rotation, const Texture texture, const glm::vec4 textureCoords = DefaultTextureCoords)
		{
			Color4f c[4] = { colors,colors,colors,colors };
			renderRectangle(transforms, c, origin, rotation, texture, textureCoords);
		}

		void renderRectangleAbsRotation(const Rect transforms, const Color4f colors[4], const glm::vec2 origin, const float rotation, const Texture texture, const glm::vec4 textureCoords = DefaultTextureCoords);
		inline void renderRectangleAbsRotation(const Rect transforms, const Color4f colors, const glm::vec2 origin, const float rotation, const Texture texture, const glm::vec4 textureCoords = DefaultTextureCoords)
		{
			Color4f c[4] = { colors,colors,colors,colors };
			renderRectangleAbsRotation(transforms, c, origin, rotation, texture, textureCoords);
		}

		void renderRectangle(const Rect transforms, const glm::vec2 origin, const float rotation, const Texture texture, const glm::vec4 textureCoords = DefaultTextureCoords);
		void renderRectangleAbsRotation(const Rect transforms, const glm::vec2 origin, const float rotation, const Texture texture, const glm::vec4 textureCoords = DefaultTextureCoords);

		void renderRectangle(const Rect transforms, const Color4f colors[4], const glm::vec2 origin = { 0,0 }, const float rotation = 0);
		inline void renderRectangle(const Rect transforms, const Color4f colors, const glm::vec2 origin = { 0,0 }, const float rotation = 0)
		{
			Color4f c[4] = { colors,colors,colors,colors };
			renderRectangle(transforms, c, origin, rotation);
		}

		void renderRectangleAbsRotation(const Rect transforms, const Color4f colors[4], const glm::vec2 origin = { 0,0 }, const float rotation = 0);
		inline void renderRectangleAbsRotation(const Rect transforms, const Color4f colors, const glm::vec2 origin = { 0,0 }, const float rotation = 0)
		{
			Color4f c[4] = { colors,colors,colors,colors };
			renderRectangleAbsRotation(transforms, c, origin, rotation);
		}

		void render9Patch(const Rect position, const int borderSize, const Color4f color, const glm::vec2 origin, const float rotation, const Texture texture, const Texture_Coords textureCoords, const Texture_Coords inner_texture_coords);
		void render9Patch2(const Rect position, const int borderSize, const Color4f color, const glm::vec2 origin, const float rotation, const Texture texture, const Texture_Coords textureCoords, const Texture_Coords inner_texture_coords);

		void clearScreen(const Color4f color = Colors_Black);

		void setShaderProgram(const internal::ShaderProgram shader);
		void setCamera(const Camera camera);

		void resetCameraAndShader();

		//draws to the screen
		void flush();
		void flushFBO(FrameBuffer frameBuffer);
	};

	//this should be called if the user changes the gl state by hand
	void enableNecessaryGLFeatures();

#pragma endregion


};