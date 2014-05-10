#include <GL/glew.h>
#include <GL/gl.h>
//#include <GL/glext.h>

#include <glm/glm.hpp>

#include <GLFW/glfw3.h>

#include "util/glfwHelper.h"

#include <cstdlib>
#include <cstdio>
#include <iostream>

#include <ctime>
#include <cstring>

#include <string>
#include <fstream>
#include <sstream>

#include "glfwCallbacks.h"
#include "util/Scene.h"

using namespace std;
using namespace glm;

GLuint program;
GLuint vertShad;
GLuint geomShad;
GLuint fragShad;

GLuint cmpProg;
GLuint cmpShad;

GLuint outTex;

GLuint vao;
GLuint vbo;

GLuint vao_sbo;
GLuint ssb_struct;

vec2 square[4] = { vec2(-1, -1), vec2(1, -1), vec2(-1, 1), vec2(1, 1) };

Scene s;

p_struct *p_buf;

//Read Data and prepare buffers
void prepareScene(void)
{
	p_buf = new p_struct[s.primIndex];

	for (int x = 0; x < s.primIndex; x++)
	{
		p_buf[x] = s.primList[x]->getStruct();
	}

	glGenBuffers(1, &ssb_struct);

	glBindBuffer(GL_ARRAY_BUFFER, ssb_struct);
	glBufferData(GL_ARRAY_BUFFER, sizeof(p_struct) * (s.primIndex), p_buf, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}
//Read an input file
string ReadFile(const char * fName)
{
	ifstream input(fName);
	if (!input)
	{
		cout << "Unable to read:" << fName << endl;
		exit(1);
	}
	stringstream temp;
	temp << input.rdbuf();
	return temp.str();

}
//Print Compiler Log
void printLog(GLuint handle)
{
	int logLen, actLen;
	glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logLen);

	GLchar log[logLen];
	glGetShaderInfoLog(handle, logLen, &actLen, log);

	if (actLen < 0)
		return;
	cout << "CompLog:" << log << endl;

}
//Create the program
void setup_programs()
{
	string source;
	const GLchar * sSource;

	//Program / shaders for the square
	program = glCreateProgram();

	vertShad = glCreateShader(GL_VERTEX_SHADER);
	source = ReadFile("shaders/vtxSquare.glsl");
	sSource = source.c_str();
	glShaderSource(vertShad, 1, &sSource, NULL);
	glCompileShader(vertShad);
	printLog(vertShad);

	fragShad = glCreateShader(GL_FRAGMENT_SHADER);
	source = ReadFile("shaders/frgSquare.glsl");
	sSource = source.c_str();
	glShaderSource(fragShad, 1, &sSource, NULL);
	glCompileShader(fragShad);
	printLog(fragShad);

	glAttachShader(program, vertShad);
	glAttachShader(program, fragShad);

	glLinkProgram(program);

	//Computer shader
	cmpProg = glCreateProgram();

	cmpShad = glCreateShader(GL_COMPUTE_SHADER);
	source = ReadFile("shaders/cmpShad.glsl");
	sSource = source.c_str();
	glShaderSource(cmpShad, 1, &sSource, NULL);
	glCompileShader(cmpShad);
	printLog(cmpShad);

	glAttachShader(cmpProg, cmpShad);

	glLinkProgram(cmpProg);

}

//Init GL
void initGl()
{
	//set up the window
	glViewport(0, 0, 800, 800);

	//Create the buffer
	glGenBuffers(1, &vbo);
	//Load the square verticies
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(square), &square[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Generate the VAO
	glGenVertexArrays(1, &vao);

	//Bind the VAO
	glBindVertexArray(vao);

	//Attach the buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glGenTextures(1, &outTex);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, outTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, s.viewPort.pxWidth, s.viewPort.pxHeight, 0, GL_RGB, GL_FLOAT, NULL);

}

void setUniforms()
{
	GLuint loc;

	loc = glGetUniformLocation(cmpProg, "pxWidth");
	glUniform1i(loc, s.viewPort.pxWidth);

	loc = glGetUniformLocation(cmpProg, "pxHeight");
	glUniform1i(loc, s.viewPort.pxHeight);

	loc = glGetUniformLocation(cmpProg, "primCount");
	glUniform1i(loc, (s.primIndex));

	loc = glGetUniformLocation(cmpProg, "eyeLoc");
	glUniform3fv(loc, 1, &s.viewPort.eyeLoc[0]);

	loc = glGetUniformLocation(cmpProg, "llCorner");
	glUniform3fv(loc, 1, &s.viewPort.llCorner[0]);

	loc = glGetUniformLocation(cmpProg, "sWidth");
	glUniform3fv(loc, 1, &s.viewPort.screenWidth[0]);

	loc = glGetUniformLocation(cmpProg, "sHeight");
	glUniform3fv(loc, 1, &s.viewPort.screenHeight[0]);

	loc = glGetUniformLocation(cmpProg, "lLoc");
	glUniform3fv(loc, 1, &s.viewPort.lightSource.loc[0]);

	loc = glGetUniformLocation(cmpProg, "I");
	glUniform1f(loc, s.viewPort.lightSource.I);

	loc = glGetUniformLocation(cmpProg, "aI");
	glUniform1f(loc, s.viewPort.lightSource.ambientI);

}

//Call the compute shader
void rayTrace(void)
{
	//Bind the output image
	glBindImageTexture(0, outTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glUseProgram(cmpProg);
	setUniforms();
	//Attach the buffer
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ssb_struct);

	//Sub divide the render based on prim count
	int sqCount = 1;

	if (s.primIndex > 75000)
	{
		sqCount = 32;
	} else if (s.primIndex > 25000)
	{
		sqCount = 16;
	} else if (s.primIndex > 10000)
	{
		sqCount = 8;
	} else if (s.primIndex > 50000)
	{
		sqCount = 4;
	} else if (s.primIndex > 2000)
	{
		sqCount = 2;
	}

	for (int x = 0; x < sqCount; x++)
	{
		for (int y = 0; y < sqCount; y++)
		{
			ivec2 subStart = ivec2(x * (512 / sqCount), y * (512 / sqCount));

			GLuint loc = glGetUniformLocation(cmpProg, "subStart");
			glUniform2iv(loc, 1, &subStart[0]);

			glDispatchCompute((512 / sqCount) / 16, (512 / sqCount) / 16, 1);

			//glMemoryBarrier(GL_ALL_BARRIER_BITS);
		}
	}

	//glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void render(void)
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDisable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);

	glUseProgram(program);

	glBindVertexArray(vao);

	//Send triangles to pipeline
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindVertexArray(0);

}

int main(int argc, char **argv)
{
	if (argc == 1)
	{
		read_input_file(s, "input.txt");
	} else
	{
		read_input_file(s, argv[1]);
	}

	if (argc == 3 && strcmp(argv[2], "-CPU") == 0)
	{
		cout << "CPU Render Begin" << endl;
		clock_t begin, end;
		double elapsed_secs;

		begin = clock();

		Image img(s.viewPort.pxWidth, s.viewPort.pxHeight);

		s.render(img);

		end = clock();
		elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
		cout << "CPU Render End" << endl;
		cout << "CPU Render Time: " << elapsed_secs << endl;

		img.saveToPPM((char*) "output.ppm");
	}

	if (!glfwHelperInit(error_callback))
	{
		std::cout << "Init Failed\n";
		exit(EXIT_FAILURE);
	}

	glfwWindow win("GPU Raytrace", 800, 800);
	win.setCurrentContext();

	//GLEW Init Stuff
	glewInit();
	if (glewIsSupported("GL_VERSION_4_3"))
		std::cout << "Ready for OpenGL 4.3" << std::endl;
	else
	{
		std::cerr << "OpenGL 4.3 not supported" << std::endl;
		exit(1);
	}

	win.setKeyboardCallback(key_callback);
	win.setRenderCallback(render);

	//win.resize(s.viewPort.pxWidth, s.viewPort.pxHeight);

	prepareScene();
	setup_programs();
	initGl();

	rayTrace();

	win.render();

	glfwHelperTerminate();
}
