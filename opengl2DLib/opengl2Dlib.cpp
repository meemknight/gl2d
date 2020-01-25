#include "opengl2Dlib.h"
#include <fstream>

namespace gl2d
{
#pragma region shaders

	static internal::ShaderProgram defaultShader = {};
	static Camera defaultCamera = cameraCreateDefault();

	static const char* defaultVertexShader =
		"#version 300 es\n"
		"precision mediump float;\n"
		"in vec2 quad_positions;\n"
		"in vec4 quad_colors;\n"
		"in vec2 texturePositions;\n"
		"out vec4 v_color;\n"
		"out vec2 v_texture;\n"
		"void main()\n"
		"{\n"
		"	gl_Position = vec4(quad_positions, 0, 1);\n"
		"	v_color = quad_colors;\n"
		"	v_texture = texturePositions;\n"
		"}\n";

	static const char* defaultFragmentShader =
		"#version 300 es\n"
		"precision mediump float;\n"
		"out vec4 color;\n"
		"in vec4 v_color;\n"
		"in vec2 v_texture;\n"
		"uniform sampler2D u_sampler;\n"
		"void main()\n"
		"{\n"
		"    color = v_color * texture(u_sampler, v_texture);\n"
		"}\n";

#pragma endregion

	static errorFuncType *errorFunc = defaultErrorFunc;

	void defaultErrorFunc(const char* msg) 
	{
		std::cout << msg << "\n";
	}

	errorFuncType *setErrorFuncCallback(errorFuncType *newFunc)
	{
		auto a = errorFunc;
		errorFunc = newFunc;
		return a;
	}

	namespace internal
	{
		float positionToScreenCoordsX(const float position, float w)
		{
			return (position / w) * 2 - 1;
		}

		float positionToScreenCoordsY(const float position, float h)
		{
			return -((-position / h) * 2 - 1);
		}

		stbtt_aligned_quad fontGetGlyphQuad(const Font font, const char c)
		{
			stbtt_aligned_quad quad = { 0 };
			float xoffset = 0;
			float yoffset = 0;

			stbtt_GetPackedQuad(font.packedCharsBuffer, font.size.x, font.size.y, c - ' ', &xoffset, &yoffset, &quad, 1);

			return quad;
		}

		glm::vec4 fontGetGlyphTextureCoords(const Font font, const char c)
		{
			float xoffset = 0;
			float yoffset = 0;

			const stbtt_aligned_quad quad = fontGetGlyphQuad(font, c);

			return glm::vec4{ quad.s0, quad.t0, quad.s1, quad.t1 };
		}

		GLuint loadShader(const char* source, GLenum shaderType)
		{
			GLuint id = glCreateShader(shaderType);

			glShaderSource(id, 1, &source, 0);
			glCompileShader(id);

			int result = 0;
			glGetShaderiv(id, GL_COMPILE_STATUS, &result);

			if (!result)
			{
				char* message = 0;
				int   l = 0;

				glGetShaderiv(id, GL_INFO_LOG_LENGTH, &l);

				message = new char[l];

				glGetShaderInfoLog(id, l, &l, message);

				message[l - 1] = 0;

				errorFunc(message);

				delete[] message;

			}

			return id;
		}

		internal::ShaderProgram createShaderProgram(const char* vertex, const char* fragment)
		{
			internal::ShaderProgram shader = { 0 };

			const GLuint vertexId = loadShader(vertex, GL_VERTEX_SHADER);
			const GLuint fragmentId = loadShader(fragment, GL_FRAGMENT_SHADER);

			shader.id = glCreateProgram();
			glAttachShader(shader.id, vertexId);
			glAttachShader(shader.id, fragmentId);

			glBindAttribLocation(shader.id, 0, "quad_positions");
			glBindAttribLocation(shader.id, 1, "quad_colors");
			glBindAttribLocation(shader.id, 2, "texturePositions");

			glLinkProgram(shader.id);

			glDeleteShader(vertexId);
			glDeleteShader(fragmentId);

			int info = 0;
			glGetProgramiv(shader.id, GL_LINK_STATUS, &info);

			if (info != GL_TRUE)
			{
				char* message = 0;
				int   l = 0;

				glGetProgramiv(shader.id, GL_INFO_LOG_LENGTH, &l);

				message = new char[l];

				glGetProgramInfoLog(shader.id, l, &l, message);

				errorFunc(message);

				delete[] message;
			}

			glValidateProgram(shader.id);

			shader.u_sampler = glGetUniformLocation(shader.id, "u_sampler");

			return shader;
		}
	}

	void init()
	{
		defaultShader = internal::createShaderProgram(defaultVertexShader, defaultFragmentShader);
		enableNecessaryGLFeatures();
	}

	glm::vec2 rotateAroundPoint(glm::vec2 vec, glm::vec2 point, const float degrees)
	{
		point.y = -point.y;
		float a = glm::radians(degrees);
		float s = sinf(a);
		float c = cosf(a);
		vec.x -= point.x;
		vec.y -= point.y;
		float newx = vec.x * c - vec.y * s;
		float newy = vec.x * s + vec.y * c;
		// translate point back:
		vec.x = newx + point.x;
		vec.y = newy + point.y;
		return vec;
	}

	glm::vec2 scaleAroundPoint(glm::vec2 vec, glm::vec2 point, float scale)
	{
		//vec.x = vec.x - point.x;
		//vec.y = vec.y - point.y;

		vec.x = vec.x * scale;
		vec.y = vec.y * scale;

		if (scale > 1)
		{
			scale = scale - 1;
		}
		else
		{
			scale = scale - 1;
		}

		vec.x = vec.x + point.x * scale;
		vec.y = vec.y + point.y * scale;

		return vec;
	}

	///////////////////// Texture /////////////////////
#pragma region Texture

	void convertFromRetardedCoordonates(int tSizeX, int tSizeY, int x, int y, int sizeX, int sizeY, int s1, int s2, int s3, int s4, Texture_Coords *outer, Texture_Coords *inner)
	{
		float newX = (float)tSizeX / (float)x;
		float newY = (float)tSizeY / (float)y;
		newY = 1 - newY;

		float newSizeX = (float)tSizeX / (float)sizeX;
		float newSizeY = (float)tSizeY / (float)sizeY;

		if (outer)
		{
			outer->x = newX;
			outer->y = newY;
			outer->z = newX + newSizeX;
			outer->w = newY - newSizeY;
		}

		if (inner)
		{
			inner->x = newX + ((float)s1 / tSizeX);
			inner->y = newY - ((float)s2 / tSizeY);
			inner->z = newX + newSizeX - ((float)s3 / tSizeX);
			inner->w = newY - newSizeY + ((float)s4 / tSizeY);
		}

	}

#pragma endregion

	///////////////////// Font /////////////////////
#pragma region Font

	void Font::createFromTTF(const unsigned char * ttf_data, const size_t ttf_data_size)
	{

		size.x = 2000,
		size.y = 2000,
		max_height = 0,
		packedCharsBufferSize = sizeof(stbtt_packedchar) * ('~' - ' ');

		//STB TrueType will give us a one channel buffer of the font that we then convert to RGBA for OpenGL
		const size_t fontMonochromeBufferSize = size.x * size.y;
		const size_t fontRgbaBufferSize = size.x * size.y * 4;

		unsigned char* fontMonochromeBuffer = new unsigned char[fontMonochromeBufferSize];
		unsigned char* fontRgbaBuffer = new unsigned char[fontRgbaBufferSize];

		packedCharsBuffer = new stbtt_packedchar[packedCharsBufferSize];

		stbtt_pack_context stbtt_context;
		stbtt_PackBegin(&stbtt_context, fontMonochromeBuffer, size.x, size.y, 0, 1, NULL);
		stbtt_PackSetOversampling(&stbtt_context, 2, 2);
		stbtt_PackFontRange(&stbtt_context, ttf_data, 0, 65, ' ', '~' - ' ', packedCharsBuffer);
		stbtt_PackEnd(&stbtt_context);

		for (int i = 0; i < fontMonochromeBufferSize; i++)
		{

			fontRgbaBuffer[(i * 4)] = fontMonochromeBuffer[i];
			fontRgbaBuffer[(i * 4) + 1] = fontMonochromeBuffer[i];
			fontRgbaBuffer[(i * 4) + 2] = fontMonochromeBuffer[i];

			if (fontMonochromeBuffer[i] > 1)
			{
				fontRgbaBuffer[(i * 4) + 3] = 255;
			}
			else
			{
				fontRgbaBuffer[(i * 4) + 3] = 0;
			}
		}

		//Init texture
		{
			glGenTextures(1, &texture.id);
			glBindTexture(GL_TEXTURE_2D, texture.id);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, fontRgbaBuffer);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}

		delete[] fontMonochromeBuffer;
		delete[] fontRgbaBuffer;

		for (char c = ' '; c <= '~'; c++)
		{
			const stbtt_aligned_quad  q = internal::fontGetGlyphQuad(*this, c);
			const float               m = q.y1 - q.y0;

			if (m > max_height)
			{
				max_height = m;
			}
		}
	}

	void Font::createFromFile(const char * file)
	{
		std::ifstream fileFont(file, std::ios::binary);

		if (!fileFont.is_open())
		{
			char c[256] = { 0 };
			strcat_s(c, "error openning: ");
			strcat_s(c+strlen(c), 200,file);
			errorFunc(c);
			return;
		}

		int fileSize = 0;
		fileFont.seekg(0, std::ios::end);
		fileSize = fileFont.tellg();
		fileFont.seekg(0, std::ios::beg);
		unsigned char * fileData = new unsigned char[fileSize];
		fileFont.read((char*)fileData, fileSize);
		fileFont.close();
		
		createFromTTF(fileData, fileSize);

		delete[] fileData;
	}


#pragma endregion

	///////////////////// Camera /////////////////////
#pragma region Camera
	Camera cameraCreateDefault()
	{
		Camera c = {};
		c.zoom = 1;
		return c;
	}	
#pragma endregion

	///////////////////// Renderer2D /////////////////////
#pragma region Renderer2D

	void gl2d::Renderer2D::flush()
	{
		if (spriteTexturesCount == 0)
		{
			return;
		}

		glUseProgram(currentShader.id);

		glViewport(0, 0, windowW, windowH);

		glBindVertexArray(vao);
		glUniform1i(currentShader.u_sampler, 0);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[Renderer2DBufferType::quadPositions]);
		glBufferData(GL_ARRAY_BUFFER, spritePositionsCount * sizeof(glm::vec2), spritePositions, GL_STREAM_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[Renderer2DBufferType::quadColors]);
		glBufferData(GL_ARRAY_BUFFER, spriteColorsCount * sizeof(glm::vec4), spriteColors, GL_STREAM_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[Renderer2DBufferType::texturePositions]);
		glBufferData(GL_ARRAY_BUFFER, texturePositionsCount * sizeof(glm::vec2), texturePositions, GL_STREAM_DRAW);

		//Instance render the textures
		{
			const int size = spriteTexturesCount;
			int pos = 0;
			unsigned int id = spriteTextures[0].id;

			spriteTextures[0].bind();

			for (int i = 1; i < size; i++)
			{
				if (spriteTextures[i].id != id)
				{
					glDrawArrays(GL_TRIANGLES, pos * 6, 6 * (i - pos));

					pos = i;
					id = spriteTextures[i].id;

					spriteTextures[i].bind();
				}

			}

			glDrawArrays(GL_TRIANGLES, pos * 6, 6 * (size - pos));

			glBindVertexArray(0);
		}

		spritePositionsCount = 0;
		spriteColorsCount = 0;
		spriteTexturesCount = 0;
		texturePositionsCount = 0;
	}

	void Renderer2D::flushFBO(FrameBuffer frameBuffer)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer.fbo);
		glBindTexture(GL_TEXTURE_2D, 0);

		flush();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void enableNecessaryGLFeatures()
	{
		glEnable(GL_BLEND);

		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	///////////////////// Renderer2D - render ///////////////////// 

	void Renderer2D::renderRectangle(const Rect transforms, const Color4f colors[4], const glm::vec2 origin, const float rotation, const Texture texture, const glm::vec4 textureCoords)
	{
		glm::vec2 newOrigin;
		newOrigin.x = origin.x + transforms.x + (transforms.z / 2);
		newOrigin.y = origin.y + transforms.y + (transforms.w / 2);
		renderRectangleAbsRotation(transforms, colors, newOrigin, rotation, texture, textureCoords);
	}

	void gl2d::Renderer2D::renderRectangleAbsRotation(const Rect transforms, const Color4f colors[4], const glm::vec2 origin, const float rotation, const Texture texture, const glm::vec4 textureCoords)
	{
		//We need to flip texture_transforms.y
		const float transformsY = transforms.y * -1;

		glm::vec2 v1 = { transforms.x,				  transformsY };
		glm::vec2 v2 = { transforms.x,				  transformsY - transforms.w };
		glm::vec2 v3 = { transforms.x + transforms.z, transformsY - transforms.w };
		glm::vec2 v4 = { transforms.x + transforms.z, transformsY };

		//Apply rotations
		if (rotation != 0)
		{
			v1 = rotateAroundPoint(v1, origin, rotation);
			v2 = rotateAroundPoint(v2, origin, rotation);
			v3 = rotateAroundPoint(v3, origin, rotation);
			v4 = rotateAroundPoint(v4, origin, rotation);
		}

		//Apply camera transformations
		v1.x -= currentCamera.position.x;
		v1.y += currentCamera.position.y;
		v2.x -= currentCamera.position.x;
		v2.y += currentCamera.position.y;
		v3.x -= currentCamera.position.x;
		v3.y += currentCamera.position.y;
		v4.x -= currentCamera.position.x;
		v4.y += currentCamera.position.y;

		//Apply camera rotation
		if (currentCamera.rotation != 0)
		{
			glm::vec2 cameraCenter;

			cameraCenter.x = windowW / 2.0f;
			cameraCenter.y = windowH / 2.0f;

			v1 = rotateAroundPoint(v1, cameraCenter, currentCamera.rotation);
			v2 = rotateAroundPoint(v2, cameraCenter, currentCamera.rotation);
			v3 = rotateAroundPoint(v3, cameraCenter, currentCamera.rotation);
			v4 = rotateAroundPoint(v4, cameraCenter, currentCamera.rotation);
		}

		//Apply camera zoom
		//if(renderer->currentCamera.zoom != 1)
		{
			glm::vec2 cameraCenter;
			cameraCenter.x = -currentCamera.position.x;
			cameraCenter.y = currentCamera.position.y;

			v1 = scaleAroundPoint(v1, cameraCenter, currentCamera.zoom);
			v2 = scaleAroundPoint(v2, cameraCenter, currentCamera.zoom);
			v3 = scaleAroundPoint(v3, cameraCenter, currentCamera.zoom);
			v4 = scaleAroundPoint(v4, cameraCenter, currentCamera.zoom);
		}

		v1.x = internal::positionToScreenCoordsX(v1.x, windowW);
		v2.x = internal::positionToScreenCoordsX(v2.x, windowW);
		v3.x = internal::positionToScreenCoordsX(v3.x, windowW);
		v4.x = internal::positionToScreenCoordsX(v4.x, windowW);
		v1.y = internal::positionToScreenCoordsY(v1.y, windowH);
		v2.y = internal::positionToScreenCoordsY(v2.y, windowH);
		v3.y = internal::positionToScreenCoordsY(v3.y, windowH);
		v4.y = internal::positionToScreenCoordsY(v4.y, windowH);

		spritePositions[spritePositionsCount++] = glm::vec2{ v1.x, v1.y };
		spritePositions[spritePositionsCount++] = glm::vec2{ v2.x, v2.y };
		spritePositions[spritePositionsCount++] = glm::vec2{ v4.x, v4.y };

		spritePositions[spritePositionsCount++] = glm::vec2{ v2.x, v2.y };
		spritePositions[spritePositionsCount++] = glm::vec2{ v3.x, v3.y };
		spritePositions[spritePositionsCount++] = glm::vec2{ v4.x, v4.y };

		spriteColors[spriteColorsCount++] = colors[0];
		spriteColors[spriteColorsCount++] = colors[1];
		spriteColors[spriteColorsCount++] = colors[3];
		spriteColors[spriteColorsCount++] = colors[1];
		spriteColors[spriteColorsCount++] = colors[2];
		spriteColors[spriteColorsCount++] = colors[3];

		texturePositions[texturePositionsCount++] = glm::vec2{ textureCoords.x, textureCoords.y }; //1
		texturePositions[texturePositionsCount++] = glm::vec2{ textureCoords.x, textureCoords.w }; //2
		texturePositions[texturePositionsCount++] = glm::vec2{ textureCoords.z, textureCoords.y }; //4
		texturePositions[texturePositionsCount++] = glm::vec2{ textureCoords.x, textureCoords.w }; //2
		texturePositions[texturePositionsCount++] = glm::vec2{ textureCoords.z, textureCoords.w }; //3
		texturePositions[texturePositionsCount++] = glm::vec2{ textureCoords.z, textureCoords.y }; //4

		spriteTextures[spriteTexturesCount++] = texture;	
	}

	void Renderer2D::renderRectangle(const Rect transforms, const glm::vec2 origin, const float rotation, const Texture texture, const glm::vec4 textureCoords)
	{
		gl2d::Color4f colors[4] = { Colors_White, Colors_White, Colors_White, Colors_White };
		renderRectangle(transforms, colors, origin, rotation, texture, textureCoords);
	}

	void Renderer2D::renderRectangleAbsRotation(const Rect transforms, const glm::vec2 origin, const float rotation, const Texture texture, const glm::vec4 textureCoords)
	{
		gl2d::Color4f colors[4] = { Colors_White, Colors_White, Colors_White, Colors_White };
		renderRectangleAbsRotation(transforms, colors, origin, rotation, texture, textureCoords);
	}

	void Renderer2D::renderRectangle(const Rect transforms, const Color4f colors[4], const glm::vec2 origin, const float rotation)
	{
		renderRectangle(transforms, colors, origin, rotation, this->white1pxSquareTexture);
	}

	void Renderer2D::renderRectangleAbsRotation(const Rect transforms, const Color4f colors[4], const glm::vec2 origin, const float rotation)
	{
		renderRectangleAbsRotation(transforms, colors, origin, rotation, this->white1pxSquareTexture);
	}

	void Renderer2D::render9Patch(const Rect position, const int borderSize, const Color4f color, const glm::vec2 origin, const float rotation, const Texture texture, const Texture_Coords textureCoords, const Texture_Coords inner_texture_coords)
	{
		glm::vec4 colorData[4] = { color, color, color, color };

		//inner
		Rect innerPos = position;
		innerPos.x += borderSize;
		innerPos.y += borderSize;
		innerPos.z -= borderSize * 2;
		innerPos.w -= borderSize * 2;
		renderRectangle(innerPos, colorData, Position2D{ 0, 0 }, 0, texture, inner_texture_coords);

		//top
		Rect topPos = position;
		topPos.x += borderSize;
		topPos.z -= (float)borderSize * 2;
		topPos.w = (float)borderSize;
		glm::vec4 upperTexPos;
		upperTexPos.x = inner_texture_coords.x;
		upperTexPos.y = textureCoords.y;
		upperTexPos.z = inner_texture_coords.z;
		upperTexPos.w = inner_texture_coords.y;
		renderRectangle(topPos, colorData, Position2D{ 0, 0 }, 0, texture, upperTexPos);

		//bottom
		Rect bottom = position;
		bottom.x += (float)borderSize;
		bottom.y += (float)position.w - borderSize;
		bottom.z -= (float)borderSize * 2;
		bottom.w = (float)borderSize;
		glm::vec4 bottomTexPos;
		bottomTexPos.x = inner_texture_coords.x;
		bottomTexPos.y = inner_texture_coords.w;
		bottomTexPos.z = inner_texture_coords.z;
		bottomTexPos.w = textureCoords.w;
		renderRectangle(bottom, colorData, Position2D{ 0, 0 }, 0, texture, bottomTexPos);

		//left
		Rect left = position;
		left.y += borderSize;
		left.z = (float)borderSize;
		left.w -= (float)borderSize * 2;
		glm::vec4 leftTexPos;
		leftTexPos.x = textureCoords.x;
		leftTexPos.y = inner_texture_coords.y;
		leftTexPos.z = inner_texture_coords.x;
		leftTexPos.w = inner_texture_coords.w;
		renderRectangle(left, colorData, Position2D{ 0, 0 }, 0, texture, leftTexPos);

		//right
		Rect right = position;
		right.x += position.z - borderSize;
		right.y += borderSize;
		right.z = (float)borderSize;
		right.w -= (float)borderSize * 2;
		glm::vec4 rightTexPos;
		rightTexPos.x = inner_texture_coords.z;
		rightTexPos.y = inner_texture_coords.y;
		rightTexPos.z = textureCoords.z;
		rightTexPos.w = inner_texture_coords.w;
		renderRectangle(right, colorData, Position2D{ 0, 0 }, 0, texture, rightTexPos);

		//topleft
		Rect topleft = position;
		topleft.z = (float)borderSize;
		topleft.w = (float)borderSize;
		glm::vec4 topleftTexPos;
		topleftTexPos.x = textureCoords.x;
		topleftTexPos.y = textureCoords.y;
		topleftTexPos.z = inner_texture_coords.x;
		topleftTexPos.w = inner_texture_coords.y;
		renderRectangle(topleft, colorData, Position2D{ 0, 0 }, 0, texture, topleftTexPos);

		//topright
		Rect topright = position;
		topright.x += position.z - borderSize;
		topright.z = (float)borderSize;
		topright.w = (float)borderSize;
		glm::vec4 toprightTexPos;
		toprightTexPos.x = inner_texture_coords.z;
		toprightTexPos.y = textureCoords.y;
		toprightTexPos.z = textureCoords.z;
		toprightTexPos.w = inner_texture_coords.y;
		renderRectangle(topright, colorData, Position2D{ 0, 0 }, 0, texture, toprightTexPos);

		//bottomleft
		Rect bottomleft = position;
		bottomleft.y += position.w - borderSize;
		bottomleft.z = (float)borderSize;
		bottomleft.w = (float)borderSize;
		glm::vec4 bottomleftTexPos;
		bottomleftTexPos.x = textureCoords.x;
		bottomleftTexPos.y = inner_texture_coords.w;
		bottomleftTexPos.z = inner_texture_coords.x;
		bottomleftTexPos.w = textureCoords.w;
		renderRectangle(bottomleft, colorData, Position2D{ 0, 0 }, 0, texture, bottomleftTexPos);

		//bottomright
		Rect bottomright = position;
		bottomright.y += position.w - borderSize;
		bottomright.x += position.z - borderSize;
		bottomright.z = (float)borderSize;
		bottomright.w = (float)borderSize;
		glm::vec4 bottomrightTexPos;
		bottomrightTexPos.x = inner_texture_coords.z;
		bottomrightTexPos.y = inner_texture_coords.w;
		bottomrightTexPos.z = textureCoords.z;
		bottomrightTexPos.w = textureCoords.w;
		renderRectangle(bottomright, colorData, Position2D{ 0, 0 }, 0, texture, bottomrightTexPos);

	}

	void Renderer2D::render9Patch2(const Rect position, const int borderSize, const Color4f color, const glm::vec2 origin, const float rotation, const Texture texture, const Texture_Coords textureCoords, const Texture_Coords inner_texture_coords)
	{
		glm::vec4 colorData[4] = { color, color, color, color };

		int w;
		int h;
		glBindTexture(GL_TEXTURE_2D, texture.id);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

		float textureSpaceW = textureCoords.z - textureCoords.x;
		float textureSpaceH = textureCoords.y - textureCoords.w;

		float topBorder = (textureCoords.y - inner_texture_coords.y) / textureSpaceH * position.w;
		float bottomBorder = (inner_texture_coords.w - textureCoords.w) / textureSpaceH * position.w;
		float leftBorder = (inner_texture_coords.x - textureCoords.x) / textureSpaceW * position.z;
		float rightBorder = (textureCoords.z - inner_texture_coords.z) / textureSpaceW * position.z;

		//inner
		Rect innerPos = position;
		innerPos.x += leftBorder;
		innerPos.y += topBorder;
		innerPos.z -= leftBorder + rightBorder;
		innerPos.w -= topBorder + bottomBorder;
		renderRectangle(innerPos, colorData, Position2D{ 0, 0 }, 0, texture, inner_texture_coords);

		//top
		Rect topPos = position;
		topPos.x += leftBorder;
		topPos.z -= leftBorder + rightBorder;
		topPos.w = topBorder;
		glm::vec4 upperTexPos;
		upperTexPos.x = inner_texture_coords.x;
		upperTexPos.y = textureCoords.y;
		upperTexPos.z = inner_texture_coords.z;
		upperTexPos.w = inner_texture_coords.y;
		renderRectangle(topPos, colorData, Position2D{ 0, 0 }, 0, texture, upperTexPos);

		//bottom
		Rect bottom = position;
		bottom.x += leftBorder;
		bottom.y += (float)position.w - bottomBorder;
		bottom.z -= leftBorder + rightBorder;
		bottom.w = bottomBorder;
		glm::vec4 bottomTexPos;
		bottomTexPos.x = inner_texture_coords.x;
		bottomTexPos.y = inner_texture_coords.w;
		bottomTexPos.z = inner_texture_coords.z;
		bottomTexPos.w = textureCoords.w;
		renderRectangle(bottom, colorData, Position2D{ 0, 0 }, 0, texture, bottomTexPos);

		//left
		Rect left = position;
		left.y += topBorder;
		left.z = leftBorder;
		left.w -= topBorder + bottomBorder;
		glm::vec4 leftTexPos;
		leftTexPos.x = textureCoords.x;
		leftTexPos.y = inner_texture_coords.y;
		leftTexPos.z = inner_texture_coords.x;
		leftTexPos.w = inner_texture_coords.w;
		renderRectangle(left, colorData, Position2D{ 0, 0 }, 0, texture, leftTexPos);

		//right
		Rect right = position;
		right.x += position.z - rightBorder;
		right.y += topBorder;
		right.z = rightBorder;
		right.w -= topBorder + bottomBorder;
		glm::vec4 rightTexPos;
		rightTexPos.x = inner_texture_coords.z;
		rightTexPos.y = inner_texture_coords.y;
		rightTexPos.z = textureCoords.z;
		rightTexPos.w = inner_texture_coords.w;
		renderRectangle(right, colorData, Position2D{ 0, 0 }, 0, texture, rightTexPos);

		//topleft
		Rect topleft = position;
		topleft.z = leftBorder;
		topleft.w = topBorder;
		glm::vec4 topleftTexPos;
		topleftTexPos.x = textureCoords.x;
		topleftTexPos.y = textureCoords.y;
		topleftTexPos.z = inner_texture_coords.x;
		topleftTexPos.w = inner_texture_coords.y;
		renderRectangle(topleft, colorData, Position2D{ 0, 0 }, 0, texture, topleftTexPos);

		//topright
		Rect topright = position;
		topright.x += position.z - rightBorder;
		topright.z = rightBorder;
		topright.w = topBorder;
		glm::vec4 toprightTexPos;
		toprightTexPos.x = inner_texture_coords.z;
		toprightTexPos.y = textureCoords.y;
		toprightTexPos.z = textureCoords.z;
		toprightTexPos.w = inner_texture_coords.y;
		renderRectangle(topright, colorData, Position2D{ 0, 0 }, 0, texture, toprightTexPos);

		//bottomleft
		Rect bottomleft = position;
		bottomleft.y += position.w - bottomBorder;
		bottomleft.z = leftBorder;
		bottomleft.w = bottomBorder;
		glm::vec4 bottomleftTexPos;
		bottomleftTexPos.x = textureCoords.x;
		bottomleftTexPos.y = inner_texture_coords.w;
		bottomleftTexPos.z = inner_texture_coords.x;
		bottomleftTexPos.w = textureCoords.w;
		renderRectangle(bottomleft, colorData, Position2D{ 0, 0 }, 0, texture, bottomleftTexPos);

		//bottomright
		Rect bottomright = position;
		bottomright.y += position.w - bottomBorder;
		bottomright.x += position.z - rightBorder;
		bottomright.z = rightBorder;
		bottomright.w = bottomBorder;
		glm::vec4 bottomrightTexPos;
		bottomrightTexPos.x = inner_texture_coords.z;
		bottomrightTexPos.y = inner_texture_coords.w;
		bottomrightTexPos.z = textureCoords.z;
		bottomrightTexPos.w = textureCoords.w;
		renderRectangle(bottomright, colorData, Position2D{ 0, 0 }, 0, texture, bottomrightTexPos);

	}

	void Renderer2D::create()
	{
		white1pxSquareTexture.create1PxSquare();

		spritePositionsCount = 0;
		spriteColorsCount = 0;
		texturePositionsCount = 0;
		spriteTexturesCount = 0;

		windowW;
		windowH;

		currentShader = defaultShader;
		currentCamera = defaultCamera;

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(Renderer2DBufferType::bufferSize, buffers);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[Renderer2DBufferType::quadPositions]);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[Renderer2DBufferType::quadColors]);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[Renderer2DBufferType::texturePositions]);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindVertexArray(0);
	}

	void Renderer2D::renderText(const glm::vec2 position, const char * text, const int text_length, const Font font, const Color4f color, const float size, const float spacing, const float line_space)
	{
		Rect rectangle;
		rectangle.x = position.x;

		//This is the y position we render at because it advances when we encounter newlines
		float linePositionY = position.y;

		for (int i = 0; i < text_length; i++)
		{
			if (text[i] == '\n')
			{
				rectangle.x = position.x;
				linePositionY += (font.max_height + line_space) * size;
			}
			else if (text[i] == '\t')
			{
				rectangle.x += spacing * 3 * size * 4;
			}
			else if (text[i] == ' ')
			{
				rectangle.x += spacing * 3 * size;
			}
			else if (text[i] >= ' ' && text[i] <= '~')
			{
				const stbtt_aligned_quad quad = internal::fontGetGlyphQuad(font, text[i]);

				rectangle.z = quad.x1 - quad.x0;
				rectangle.w = quad.y1 - quad.y0;

				rectangle.z *= size;
				rectangle.w *= size;

				rectangle.y = linePositionY - rectangle.w;

				glm::vec4 colorData[4] = { color, color, color, color };
				renderRectangle(rectangle, colorData, glm::vec2{ 0, 0 }, 0, font.texture, glm::vec4{ quad.s0, quad.t0, quad.s1, quad.t1 });
				rectangle.x += rectangle.z + spacing * size;
			}
		}

	}

	void Renderer2D::clearScreen(const Color4f color)
	{
		glClearBufferfv(GL_COLOR, 0, &color[0]);
	}

	void Renderer2D::setShaderProgram(const internal::ShaderProgram shader)
	{
		currentShader = shader;
	}

	void Renderer2D::setCamera(const Camera camera)
	{
		currentCamera = camera;
	}

	void Renderer2D::resetCameraAndShader()
	{
		currentCamera = defaultCamera;
		currentShader = defaultShader;
	}

#pragma endregion

	glm::ivec2 Texture::GetSize()
	{
		glm::ivec2 s;
		glBindTexture(GL_TEXTURE_2D, id);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &s.x);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &s.y);
		return s;
	}

	void Texture::createFromBuffer(const char * image_data, const int width, const int height)
	{
		GLuint id = 0;

		glActiveTexture(GL_TEXTURE0);

		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);

		this->id = id;
	}

	void Texture::create1PxSquare()
	{
		const char buff[] =
		{
			0xff,
			0xff,
			0xff,
			0xff
		};

		createFromBuffer(buff, 1, 1);
	}

	void Texture::createFromFileData(const unsigned char * image_file_data, const size_t image_file_size)
	{
		stbi_set_flip_vertically_on_load(true);

		int width = 0;
		int height = 0;
		int channels = 0;

		const unsigned char* decodedImage = stbi_load_from_memory(image_file_data, image_file_size, &width, &height, &channels, 4);
		
		createFromBuffer((const char*)decodedImage, width, height);

		//Replace stbi allocators
		free((void*)decodedImage);
	}

	void Texture::loadFromFile(const char * fileName)
	{
		std::ifstream file(fileName, std::ios::binary);

		if (!file.is_open())
		{
			char c[256] = {0};
			strcat_s(c, "error openning: ");
			strcat_s(c + strlen(c), 200, fileName);
			errorFunc(c);
			return;
		}

		int fileSize = 0;
		file.seekg(0, std::ios::end);
		fileSize = file.tellg();
		file.seekg(0, std::ios::beg);
		unsigned char * fileData = new unsigned char[fileSize];
		file.read((char*)fileData, fileSize);
		file.close();

		createFromFileData(fileData, fileSize);

		delete[] fileData;

	}

	void Texture::bind(const unsigned int sample)
	{
		glActiveTexture(GL_TEXTURE0 + sample);
		glBindTexture(GL_TEXTURE_2D, id);
	}

	void Texture::unbind()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Texture::cleanup()
	{
		glDeleteTextures(1, &id);
	}

	glm::mat3 Camera::getMatrix()
	{
		glm::mat3 m;
		m = { zoom, 0, position.x ,
			 0, zoom, position.y,
			0, 0, 1,
		};
		return m; //todo could have problems
	}

	void FrameBuffer::create(unsigned int w, unsigned int h)
	{
		FrameBuffer fb;
		glGenFramebuffers(1, &fb.fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);

		glGenTextures(1, &fb.texture);
		glBindTexture(GL_TEXTURE_2D, fb.texture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb.texture, 0);

		//glDrawBuffer(GL_COLOR_ATTACHMENT0);

		glGenTextures(1, &fb.depthtTexture);
		glBindTexture(GL_TEXTURE_2D, fb.depthtTexture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fb.depthtTexture, 0);

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		*this = fb;
	}

	//todo vlod: will probasbly also clear the fbo
	void FrameBuffer::resize(unsigned int w, unsigned int h)
	{
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		glBindTexture(GL_TEXTURE_2D, depthtTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	}

	void FrameBuffer::cleanup()
	{
		glDeleteFramebuffers(1, &fbo);
		fbo = 0;

		glDeleteTextures(1, &texture);
		texture = 0;

		glDeleteTextures(1, &depthtTexture);
		depthtTexture = 0;
	}

	void FrameBuffer::clear()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

}