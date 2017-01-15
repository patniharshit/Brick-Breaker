#include <iostream>
#include <cmath>
#include <fstream>
#include <bits/stdc++.h>
#include <time.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

#define LASERRAYSPEED 20
#define LASERGUNVELOCITY 6

struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;

	GLenum PrimitiveMode;
	GLenum FillMode;
	int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

struct COLOR {
	float r;
	float g;
	float b;
};
typedef struct COLOR COLOR;

COLOR grey = {168.0/255.0,168.0/255.0,168.0/255.0};
COLOR gold = {218.0/255.0,165.0/255.0,32.0/255.0};
COLOR coingold = {255.0/255.0,223.0/255.0,0.0/255.0};
COLOR red = {255.0/255.0,51.0/255.0,51.0/255.0};
COLOR lightgreen = {57/255.0,230/255.0,0/255.0};
COLOR darkgreen = {51/255.0,102/255.0,0/255.0};
COLOR black = {30/255.0,30/255.0,21/255.0};
COLOR blue = {0,0,1};
COLOR darkbrown = {46/255.0,46/255.0,31/255.0};
COLOR lightbrown = {95/255.0,63/255.0,32/255.0};
COLOR brown1 = {117/255.0,78/255.0,40/255.0};
COLOR brown2 = {134/255.0,89/255.0,40/255.0};
COLOR brown3 = {46/255.0,46/255.0,31/255.0};
COLOR cratebrown = {153/255.0,102/255.0,0/255.0};
COLOR cratebrown1 = {121/255.0,85/255.0,0/255.0};
COLOR cratebrown2 = {102/255.0,68/255.0,0/255.0};
COLOR skyblue2 = {113/255.0,185/255.0,209/255.0};
COLOR skyblue1 = {123/255.0,201/255.0,227/255.0};
COLOR skyblue = {132/255.0,217/255.0,245/255.0};
COLOR cloudwhite = {229/255.0,255/255.0,255/255.0};
COLOR cloudwhite1 = {204/255.0,255/255.0,255/255.0};
COLOR lightpink = {255/255.0,122/255.0,173/255.0};
COLOR darkpink = {255/255.0,51/255.0,119/255.0};
COLOR white = {255/255.0,255/255.0,255/255.0};
COLOR score = {117/255.0,78/255.0,40/255.0};

struct Sprite {
	string name;
	COLOR color;
	float x,y;
	VAO* object;
	int status;
	float height,width;
	float x_speed,y_speed;
	float angle; //Current Angle (Actual rotated angle of the object)
	int inAir;
	float radius;
	int fixed;
	float friction; //Value from 0 to 1
	int health;
	int isRotating;
	int direction; //0 for clockwise and 1 for anticlockwise for animation
	float remAngle; //the remaining angle to finish animation
	int isMovingAnim;
	int dx;
	int dy;
	float weight;
};
typedef struct Sprite Sprite;

map<string, Sprite> laserObjects;
vector<Sprite> brickObjects;
map<string, Sprite> mirrorObjects;
map<string, Sprite> bucketObjects;
map<string, Sprite> otherObjects;

int windowWidth = 1200, windowHeight = 1200;
int redBucketX = -windowWidth/4 - 40, greenBucketX = windowWidth/4 + 40;
int GRAVITY = 2;
float ctBrick, lutBrick, launchAngle;
int current_brick = 0;
bool sKeyPressed = false, fKeyPressed = false;
bool spaceKeyPressed = false;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
	//    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
			0,                  // attribute 0. Vertices
			3,                  // size (x,y,z)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
			1,                  // attribute 1. Color
			3,                  // size (r,g,b)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
	GLfloat* color_buffer_data = new GLfloat [3*numVertices];
	for (int i=0; i<numVertices; i++) {
		color_buffer_data [3*i] = red;
		color_buffer_data [3*i + 1] = green;
		color_buffer_data [3*i + 2] = blue;
	}

	return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Enable Vertex Attribute 1 - Color
	glEnableVertexAttribArray(1);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Function is called first on GLFW_PRESS.

	if (action == GLFW_RELEASE) {
		switch (key) {
			case GLFW_KEY_S:
				sKeyPressed = false;
				break;
			case GLFW_KEY_F:
				fKeyPressed = false;
				break;
			case GLFW_KEY_SPACE:
				spaceKeyPressed = true;
				break;
			default:
				break;
		}
	}
	else if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_S:
				sKeyPressed = true;
				break;
			case GLFW_KEY_F:
				fKeyPressed = true;
				break;
			case GLFW_KEY_ESCAPE:
				quit(window);
				break;
			default:
				break;
		}
	}
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
			quit(window);
			break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
	switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			if (action == GLFW_RELEASE) {
				spaceKeyPressed = true;
			}
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			if (action == GLFW_RELEASE) {
			}
			break;
		default:
			break;
	}
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
	int fbwidth=width, fbheight=height;
	/* With Retina display on Mac OS X, GLFW's FramebufferSize
	   is different from WindowSize */
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
	// Perspective projection for 3D views
	// Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

	// Ortho projection for 2D views
	Matrices.projection = glm::ortho(-400.0f, 400.0f, -300.0f,300.0f, 0.1f, 500.0f);
}

VAO *triangle, *rectangle;

// Creates the triangle object used in this sample code
void createTriangle ()
{
	/* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

	/* Define vertex array as used in glBegin (GL_TRIANGLES) */
	GLfloat vertex_buffer_data [] = {
		0, 1,0, // vertex 0
		-1,-1,0, // vertex 1
		1,-1,0, // vertex 2
	};

	GLfloat color_buffer_data [] = {
		1,0,0, // color 0
		0,1,0, // color 1
		0,0,1, // color 2
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

// Creates the rectangle object used in this sample code
void createRectangle (string name, float weight, COLOR colorA, COLOR colorB, COLOR colorC, COLOR colorD, float x, float y, float height, float width, string component)
{
	// GL3 accepts only Triangles. Quads are not supported
	float w=width/2,h=height/2;
	GLfloat vertex_buffer_data [] = {
		-w,-h,0, // vertex 1
		-w,h,0, // vertex 2
		w,h,0, // vertex 3

		w,h,0, // vertex 3
		w,-h,0, // vertex 4
		-w,-h,0  // vertex 1
	};

	GLfloat color_buffer_data [] = {
		colorA.r,colorA.g,colorA.b, // color 1
		colorB.r,colorB.g,colorB.b, // color 2
		colorC.r,colorC.g,colorC.b, // color 3

		colorC.r,colorC.g,colorC.b, // color 4
		colorD.r,colorD.g,colorD.b, // color 5
		colorA.r,colorA.g,colorA.b // color 6
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	VAO *rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

	Sprite rectangleSprite = {};
	rectangleSprite.color = colorA;
	rectangleSprite.name = name;
	rectangleSprite.object = rectangle;
	rectangleSprite.x=x;
	rectangleSprite.y=y;
	rectangleSprite.height=height;
	rectangleSprite.width=width;
	rectangleSprite.status=1;
	rectangleSprite.inAir=0;
	rectangleSprite.x_speed=0;
	rectangleSprite.y_speed=0;
	rectangleSprite.fixed=0;
	rectangleSprite.radius=(sqrt(height*height+width*width))/2;
	rectangleSprite.friction=0.4;
	rectangleSprite.health=100;
	rectangleSprite.weight=weight;
	//All the different layers
	if(component=="laser")
		laserObjects[name]=rectangleSprite;
	else if(component=="brick")
		brickObjects.push_back(rectangleSprite);
	else if(component=="bucket")
		bucketObjects[name]=rectangleSprite;
	else if(component=="mirror")
		mirrorObjects[name]=rectangleSprite;
	else
		otherObjects[name]=rectangleSprite;
}


void keyPressed(glm::mat4 VP) {
	if(sKeyPressed) {
		for(map<string,Sprite>::iterator it=laserObjects.begin();it!=laserObjects.end();it++){
			string current = it->first;
			if((current != "laserray") || (laserObjects["laserray"].x == laserObjects["laserbarrel"].x)) {
				if(laserObjects[current].y < 280) {
					laserObjects[current].y += LASERGUNVELOCITY;
				}
			}
		}
	}
	if(fKeyPressed) {
		for(map<string,Sprite>::iterator it=laserObjects.begin();it!=laserObjects.end();it++){
			string current = it->first;
			if((current != "laserray") || (laserObjects["laserray"].x == laserObjects["laserbarrel"].x)) {
				if(laserObjects[current].y > -180) {
					laserObjects[current].y -= LASERGUNVELOCITY;
				}
			}
		}
	}
}

void iterateOnMap(map<string,Sprite> objectMap, glm::mat4 VP, GLFWwindow* window )
{
	//  Don't change unless you are sure!!
	glm::mat4 MVP;	// MVP = Projection * View * Model

	// Load identity to model matrix
	Matrices.model = glm::mat4(1.0f);

	for(map<string,Sprite>::iterator it=objectMap.begin();it!=objectMap.end();it++){
		string current = it->first; //The name of the current object
		if(objectMap[current].status==0)
			continue;
		glm::mat4 MVP;  // MVP = Projection * View * Model

		Matrices.model = glm::mat4(1.0f);

		glm::mat4 ObjectTransform;
		glm::mat4 translateObject = glm::translate (glm::vec3(objectMap[current].x, objectMap[current].y, 0.0f));

		glm::mat4 rotateGun;
		double mouse_x, mouse_y;
		glfwGetCursorPos(window,&mouse_x,&mouse_y);
		float angle = -(atan((mouse_y-400)/(mouse_x)));
		if(current == "laserbarrel") {
			rotateGun = glm::rotate((float)(angle), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
		}
		if((current == "laserray") && (laserObjects["laserray"].x == laserObjects["laserbarrel"].x))
			launchAngle = angle;

		if(spaceKeyPressed && current == "laserray") {
				rotateGun = glm::rotate((float)(launchAngle), glm::vec3(0,0,1));
		}

		ObjectTransform=translateObject * rotateGun;
		Matrices.model *= ObjectTransform;
		MVP = VP * Matrices.model; // MVP = p * V * M

		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

		if(spaceKeyPressed && current == "laserray" &&
				(laserObjects["laserray"].x > laserObjects["lasergun"].x + 40)) {
			draw3DObject(objectMap[current].object);
		}
		else if(current != "laserray")
			draw3DObject(objectMap[current].object);

		//glPopMatrix ();
	}
}

void iterateOnBrickObjects(vector<Sprite> objectMap, glm::mat4 VP)
{
	int iter = 0;
	ctBrick = glfwGetTime();
	if(current_brick < brickObjects.size() && (ctBrick - lutBrick) > 1) {
		brickObjects[current_brick].status = 1;
		current_brick++;
		lutBrick = glfwGetTime();
	}
	//  Don't change unless you are sure!!
	glm::mat4 MVP;	// MVP = Projection * View * Model

	// Load identity to model matrix
	Matrices.model = glm::mat4(1.0f);

	for(int i=0; i<brickObjects.size() && i<current_brick; i++) {
		if(brickObjects[i].status==0)
			continue;
		else
			brickObjects[i].y -= GRAVITY;

		if(brickObjects[i].y <= -windowHeight/6) {
			brickObjects[i].status = 0;
		}

		glm::mat4 MVP;  // MVP = Projection * View * Model

		Matrices.model = glm::mat4(1.0f);

		glm::mat4 ObjectTransform;
		glm::mat4 translateObject = glm::translate (glm::vec3(brickObjects[i].x, brickObjects[i].y, 0.0f)); // glTranslatef
		ObjectTransform=translateObject;
		Matrices.model *= ObjectTransform;
		MVP = VP * Matrices.model; // MVP = p * V * M

		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(objectMap[i].object);

		//glPopMatrix ();
	}
}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw (GLFWwindow* window )
{
	// clear the color and depth in the frame buffer
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// use the loaded shader program
	// Don't change unless you know what you are doing
	glUseProgram (programID);

	// Eye - Location of camera. Don't change unless you are sure!!
	glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
	// Target - Where is the camera looking at.  Don't change unless you are sure!!
	glm::vec3 target (0, 0, 0);
	// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
	glm::vec3 up (0, 1, 0);

	// Compute Camera matrix (view)
	//Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
	//  Don't change unless you are sure!!F
	Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
	//  Don't change unless you are sure!!
	glm::mat4 VP = Matrices.projection * Matrices.view;

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// For each model you render, since the MVP will be different (at least the M part)

	iterateOnMap(bucketObjects, VP, window);
	iterateOnMap(laserObjects, VP, window);
	iterateOnMap(mirrorObjects, VP, window);
	iterateOnBrickObjects(brickObjects, VP);
	iterateOnMap(otherObjects, VP, window);

	keyPressed(VP);

	if(spaceKeyPressed) {
		laserObjects["laserray"].x += cos(launchAngle) * LASERRAYSPEED;
		laserObjects["laserray"].y += sin(launchAngle) * LASERRAYSPEED;
	}
	int posx = laserObjects["laserray"].x;
	int posy = laserObjects["laserray"].y;
	if(posx > windowWidth/2 || posy > windowHeight/2 || posx < -windowWidth/2 || posy < -windowHeight/2) {
		spaceKeyPressed = false;
		laserObjects["laserray"].x = laserObjects["laserbarrel"].x;
		laserObjects["laserray"].y = laserObjects["laserbarrel"].y;
	}
	// Increment angles
	float increments = 1;

	//camera_rotation_angle++; // Simulating camera rotation
	//triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
	//rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
	GLFWwindow* window; // window desciptor/handle

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		//        exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

	if (!window) {
		glfwTerminate();
		//        exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval( 1 );

	/* --- register callbacks with GLFW --- */

	/* Register function to handle window resizes */
	/* With Retina display on Mac OS X GLFW's FramebufferSize
	   is different from WindowSize */
	glfwSetFramebufferSizeCallback(window, reshapeWindow);
	glfwSetWindowSizeCallback(window, reshapeWindow);

	/* Register function to handle window close */
	glfwSetWindowCloseCallback(window, quit);

	/* Register function to handle keyboard input */
	glfwSetKeyCallback(window, keyboard);      // general keyboard input
	glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

	/* Register function to handle mouse click */
	glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

	return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	createRectangle("separator",100,white,white,white,white,0, -windowHeight/6,0.5,windowWidth,"other");
	createRectangle("redBucket",100,red,red,red,red, -windowWidth/4 -40, -windowHeight/5 - 10,windowHeight/12,windowWidth/10,"bucket");
	createRectangle("greenBucket",100,darkgreen,darkgreen,lightgreen,lightgreen, windowWidth/4 +40, -windowHeight/5 -10,windowHeight/12,windowWidth/10,"bucket");
	createRectangle("laserbody",100,white,white,white,white,-410, 40, 40, 80,"laser");
	createRectangle("lasergun",100,white,white,white,white,-380, 40, 25, 40, "laser");
	createRectangle("laserbarrel",100,white,white,white,white, -365, 40, 5, 40, "laser");
	createRectangle("laserray",100,red,red,red,red,laserObjects["laserbarrel"].x, laserObjects["laserbarrel"].y, 5, 40, "laser");

	COLOR brickcolor = red;
	for(int i =0; i < 1000; i++) {
		if(i % 3 == 0)
			brickcolor = red;
		else if(i % 3 == 1)
			brickcolor = lightgreen;
		else
			brickcolor = skyblue1;
		int randnum = rand() % 500 - 200;
		createRectangle("indivBrick",100,brickcolor,brickcolor,brickcolor,brickcolor, randnum, windowHeight/4, 30, 20, "brick");
	}

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

	// Background color of the scene
	glClearColor (0.0f, 0.0f, 0.0f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	srand(time(NULL));
	GLFWwindow* window = initGLFW(windowWidth, windowHeight);

	initGL (window, windowWidth, windowHeight);

	double last_update_time = glfwGetTime(), current_time;

	/* Draw in loop */
	while (!glfwWindowShouldClose(window)) {

		// OpenGL Draw commands
		draw(window);

		// Swap Frame Buffer in double buffering
		glfwSwapBuffers(window);

		// Poll for Keyboard and mouse events
		glfwPollEvents();

		// Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
		current_time = glfwGetTime(); // Time in seconds
		if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
			// do something every 0.5 seconds ..
			last_update_time = current_time;
		}
	}

	glfwTerminate();
	//    exis(EXIT_SUCCESS);
}
