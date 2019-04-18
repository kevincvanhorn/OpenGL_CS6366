/*
 * @author Kevin VanHorn - kcv150030
 * Loads and displays an obj file, allowing for local transformation of the camear using a model view matrix.
 * Gui is displayed using nanogui.
*/

#include <iostream>
#include <fstream>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other includes
#include "Shader.h"
#include "nanogui/nanogui.h"
#include "nanogui/slider.h"
#include "nanogui/layout.h"
#include "nanogui/textbox.h"
#include <glm/glm.hpp>
#include <SOIL.h>

#include "PointLight.h"
#include "Camera.h"


// Pre-define gui variable handles:
#define GUI_DOUBLE nanogui::detail::FormWidget<double, std::integral_constant<bool, true>>
#define GUI_STRING nanogui::detail::FormWidget<std::string, std::true_type>
#define GUI_COLOR nanogui::detail::FormWidget<nanogui::Color, std::true_type>
#define GUI_BOOL nanogui::detail::FormWidget<bool, std::true_type>
#define GUI_FLOAT nanogui::detail::FormWidget<float, std::integral_constant<bool, true>> 
#define GUI_INT nanogui::detail::FormWidget<int, std::integral_constant<bool, true>> 

using namespace nanogui;

void GLAPIENTRY
MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
}

// Active local camera rotation type.
enum ERotType {
	Right_Pos,
	Right_Neg,
	Up_Pos,
	Up_Neg,
	Front_Pos,
	Front_Neg
};

// Method to render obj file.
enum ERenderType {
	Line,
	Point,
	Triangle
};

// Pre-loaded model names.
enum EModelName {
	HEAD,
	TEAPOT,
	BUCKY,
	BONSAI
};

// Holds data read from the obj file.
struct Vertex {
	glm::vec3 Position;
	//glm::vec3 Normal;
	//glm::vec2 TexCoords;
	//glm::vec3 Tangent;
	//glm::vec3 BiTangent;

	//glm::vec3 Color;
};

// Window dimensions
const GLuint width = 1200, height = 700;
Camera camera(width, height); // Create camera w/ screen aspect ratio.
PointLight pointLight(camera.GetCameraPos());

// Gui variable handles to edit fields in the gui:
nanogui::detail::FormWidget<ERenderType, std::integral_constant<bool, true>>* gui_RenderType;
nanogui::detail::FormWidget<EModelName, std::integral_constant<bool, true>>* gui_ModelName;

GUI_COLOR* gui_ColObject;
GUI_STRING* gui_ImageBar;
GUI_DOUBLE* gui_RotValue;
GUI_DOUBLE* gui_PositionX;
GUI_DOUBLE* gui_PositionY;
GUI_DOUBLE* gui_PositionZ;

GUI_BOOL* gui_bTransferFunctionSign;
GUI_INT* gui_SamplingRate;

// Variable defaults modified in the gui:
Color colObject(1.0f, 0.53f, 0.29f, 1.0f); // color of the object
double dPositionX = 0.0;
double dPositionY = 0.0;
double dPositionZ = -10.0;
double dRotValue = 0.0;
double dNearPlane = camera.NEAR_PLANE;
double dFarPlane = camera.FAR_PLANE;
ERenderType renderType = ERenderType::Triangle;

std::string strObjectFile2;
EModelName modelName = EModelName::TEAPOT;
std::string strImageBar = "colorbar.png";
bool bTransferFunctionSign = true;
int samplingRate = 100;
float viewSlider = 1.0f;
float slider0 = 0.161290f;
float slider1 = 0.564516f;
float slider2 = 0.677419f;
float slider3 = 0.612903f;
float slider4 = 0.790323f;
float slider5 = 0.677419f;
float slider6 = 0.790323f;
float slider7 = 0.887097f;

VectorXf gVector(8);
Graph *graph;
unsigned int texmapID;
unsigned int colorMapID;
Color emmisiveColor(1.0f, 1.0f, 1.0f, 1.0f);
const int MAX_SLICES = 512;

// resolution of the color bar
float imageX = 256;
float imageY = 10;

std::string strObjectFile;

GLfloat cube_vertices[24] = {
0.0, 0.0, 0.0,  0.0, 0.0, 1.0,  0.0, 1.0, 0.0,  0.0, 1.0, 1.0,
1.0, 0.0, 0.0,  1.0, 0.0, 1.0,  1.0, 1.0, 0.0,  1.0, 1.0, 1.0 };
GLuint cube_indices[36] = {
	1,5,7,  7,3,1,  0,2,6,  6,4,0,  0,1,3,   3,2,0,  7,5,4,  4,6,7,  2,3,7,  7,6,2,  1,0,4,  4,5,1 };
GLuint cube_edges[24]{ 1,5,  5,7,  7,3,  3,1,  0,4,  4,6,  6,2,  2,0,  0,1,  2,3,  4,5,  6,7 };

glm::vec3 vertexList[8] = { glm::vec3(0.0f,0.0f,0.0f),
						   glm::vec3(1.0f,0.0f,0.0f),
						   glm::vec3(1.0f, 1.0f,0.0f),
						   glm::vec3(0.0f, 1.0f,0.0f),
						   glm::vec3(0.0f,0.0f, 1.0f),
						   glm::vec3(1.0f,0.0f, 1.0f),
						   glm::vec3(1.0f, 1.0f, 1.0f),
						   glm::vec3(0.0f, 1.0f, 1.0f) };
//unit cube edges
int edgeList[8][12] = {
	{ 0,1,5,6,   4,8,11,9,  3,7,2,10 }, // v0 is front
	{ 0,4,3,11,  1,2,6,7,   5,9,8,10 }, // v1 is front
	{ 1,5,0,8,   2,3,7,4,   6,10,9,11}, // v2 is front
	{ 7,11,10,8, 2,6,1,9,   3,0,4,5  }, // v3 is front
	{ 8,5,9,1,   11,10,7,6, 4,3,0,2  }, // v4 is front
	{ 9,6,10,2,  8,11,4,7,  5,0,1,3  }, // v5 is front
	{ 9,8,5,4,   6,1,2,0,   10,7,11,3}, // v6 is front
	{ 10,9,6,5,  7,2,3,1,   11,4,8,0 }  // v7 is front
};
const int edges[12][2] = { {0,1},{1,2},{2,3},{3,0},{0,4},{1,5},{2,6},{3,7},{4,5},{5,6},{6,7},{7,4} };

// Second Window:

// Render Variables:
Screen *screen = nullptr;
std::vector<Vertex> Vertices;
GLuint VBO, VAO;

// Forwardly declared functions:
void RotateByVal(ERotType rotType);
void ReloadObjectModel();
void ResetGui();
bool LoadModel(std::vector<Vertex> &vertices);
void SetupGUI(GLFWwindow* window);
void InitModel();
void SetColor();
void SetViewLoc(float min, float max);

void UpdateModelName();
void OnSetSamplingRate(); // TODO: Implement SamplingRate function.
void AddSlider(FormHelper *gui, ref<Window> nanoguiWindow2, const std::string label, const std::string value, float& callbackVar);

void SetGraphValues();
void LoadTextures();
bool LoadCube(std::vector<Vertex> &vertices);
void SliceVolume();
void LoadImage(std::string img_path, int width, int height, unsigned int ID);
void LoadColorMap();

GLubyte * load_3d_raw_data(std::string texture_path, glm::vec3 dimension);

// Main function with intialization and game loop.
int main()
{
	// Init GLFW
	glfwInit();

	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	// Create a GLFWwindow object
	GLFWwindow* window = glfwCreateWindow(width, height, "Assignment 1", nullptr, nullptr);
	if (window == nullptr) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
}
	glfwMakeContextCurrent(window);

#if defined(NANOGUI_GLAD)
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		throw std::runtime_error("Could not initialize GLAD!");
	glGetError(); // pull and ignore unhandled errors like GL_INVALID_ENUM
#endif

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	SetupGUI(window);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	glewInit();

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);

	// Build and compile our shader program
	Shader ourShader("shader/basic.vert", "shader/basic.frag");

	GLuint MatrixID = glGetUniformLocation(ourShader.program, "MVP");

	ReloadObjectModel();

	// Game Loop:
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		// Clear the colorbuffer
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Draw the Obj:
		ourShader.use();
		ourShader.setVec3("modelColor", colObject[0], colObject[1], colObject[2]); // Set the color of the object
		ourShader.setVec3("emmisiveColor", emmisiveColor[0], emmisiveColor[1], emmisiveColor[2]); // TODO: Calculate emmisive color.
		//ourShader.setVec3("camPos", camera.GetCameraPos());

		ourShader.setVec2("resolution", imageX, imageY);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Volume Rendering
		//glGenTextures(1, &texmapID);
		//glBindTexture(GL_TEXTURE_3D, texmapID);

		ourShader.setFloat("s0",slider0);
		ourShader.setFloat("s1",slider1);
		ourShader.setFloat("s2",slider2);
		ourShader.setFloat("s3",slider3);
		ourShader.setFloat("s4",slider4);
		ourShader.setFloat("s5",slider5);
		ourShader.setFloat("s6",slider6);
		ourShader.setFloat("s7",slider7);

		ourShader.setFloat("sREL",15/samplingRate);

		SliceVolume();
		ourShader.setInt("texMap", 0);
		ourShader.setInt("texColorMap", 1);

		glBindVertexArray(VAO);

		if (Vertices.size() > 0) {
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &camera.GetMVPMatrix()[0][0]);

			// Set draw type:
			if (renderType == ERenderType::Triangle) { // TODO: Implement triangle draw type. Currently the previous iteration of solid.
				glDrawArrays(GL_TRIANGLES, 0, Vertices.size());
			}
			else if (renderType == ERenderType::Point) {
				glEnable(GL_PROGRAM_POINT_SIZE);
				glDrawArrays(GL_POINTS, 0, Vertices.size());
			}
			else if (renderType == ERenderType::Line) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glDrawArrays(GL_TRIANGLES, 0, Vertices.size());
			}
		}

		glBindVertexArray(0);

		SliceVolume();

		// Set draw mode back to fill & draw gui
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		screen->drawWidgets();

		// Swap the screen buffers
		glfwSwapBuffers(window);
	}

	// De-allocate all resources once they've outlived their purpose
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
	return 0;
}

/* Commnunicates with the camera object to adjust the view matrix rotational values. 
* @rotType the type of rotation to apply the rotation amount "dRotValue" to.
*/
void RotateByVal(ERotType rotType)
{
	if (rotType == ERotType::Front_Neg) {
		camera.RotZ += dRotValue;
	}
	else if (rotType == ERotType::Front_Pos) {
		camera.RotZ += -1*dRotValue;
	}
	else if (rotType == ERotType::Right_Neg) {
		camera.RotX += dRotValue;
	}
	else if (rotType == ERotType::Right_Pos) {
		camera.RotX += -1*dRotValue;
	}
	else if (rotType == ERotType::Up_Neg) {
		camera.RotY += (dRotValue);
	}
	else if (rotType == ERotType::Up_Pos) {
		camera.RotY += -1 *(dRotValue);
	}
	camera.OnUpdateRotation();
}

/* Covers overhead for each time a model is loaded. */
void ReloadObjectModel()
{
	Vertices.clear(); // Refresh vertex buffer.
	// Load the model from the file path & add colors:

	LoadCube(Vertices);
	LoadTextures();
	InitModel();

	/*if (LoadModel(Vertices)) {
		SetColor();
		//LoadTextures();
	}*/
}

/* Reinitializes buffer for drawing, handling OpenGL configuration. */
void InitModel() {
	if (Vertices.size() <= 0) return;

	std::cout << "Init Model. \n";
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * Vertices.size(), &Vertices[0], GL_DYNAMIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	// Normal attribute
	//glEnableVertexAttribArray(1);
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

	// Texture Coords attribute:
	//glEnableVertexAttribArray(2);
	//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

	// Tangent attribute:
	//glEnableVertexAttribArray(3);
	//glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));

	// BiTangent attribute:
	//glEnableVertexAttribArray(4);
	//glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, BiTangent));

	glBindVertexArray(0); // Unbind VAO
}

/* Set the color value for each vertex. */
void SetColor()
{
	/*for (int i = 0; i < Vertices.size(); i++) {
		Vertices[i].Color = glm::vec3(colObject.r(), colObject.g(), colObject.b());
	}
	if (Vertices.size() > 0) {
		InitModel();
	}*/
}

/* Centers the object in the screen when loaded. */
void SetViewLoc(float min, float max)
{
	float center, dist;

	center = (max + min) / 2;
	dist = abs(max - min) * 2;
	
	camera.SetModelCenter(center, -dist);

	// Round to 2 decimal places for gui 
	dPositionX = 0;
	dPositionY = ((double)((int)(center*100))) /100;
	dPositionZ = ((double)((int)(((double)dist) * 100))) / 100;

	// Update gui handles:
	gui_PositionX->setValue(dPositionX);
	gui_PositionY->setValue(dPositionY);
	gui_PositionZ->setValue(dPositionZ);
}

// Update the file for a given model name;
void UpdateModelName()
{
	if (modelName == EModelName::BONSAI) {
		strObjectFile;
	}
	else if (modelName == EModelName::BUCKY) {
		strObjectFile;
	}
	else if (modelName == EModelName::HEAD) {
		strObjectFile;
	}
	if (modelName == EModelName::TEAPOT) {
		strObjectFile;
	}
}

void OnSetSamplingRate()
{
}

void AddSlider(FormHelper *gui, ref<Window> nanoguiWindow2, const std::string label, const std::string value, float& callbackVar) {
	Widget* panel = new Widget(nanoguiWindow2);
	panel->setLayout(new BoxLayout(Orientation::Horizontal, Alignment::Middle, 0, 20));

	gui->addWidget(label, panel);
	
	Slider *slider = new Slider(panel);
	slider->setValue(callbackVar);
	slider->setFixedWidth(80);

	TextBox *textBox = new TextBox(panel);
	textBox->setFixedSize(Vector2i(60, 25));
	textBox->setValue(value);
	slider->setCallback([textBox](float value) { textBox->setValue(std::to_string(value)); });
	slider->setFinalCallback([&](float value) { callbackVar = value; SetGraphValues(); });
	textBox->setFixedSize(Vector2i(100, 25));
	textBox->setFontSize(20);
	textBox->setAlignment(TextBox::Alignment::Center);
}

bool LoadCube(std::vector<Vertex> &vertices) {
	GLuint i = 0;

	while (i < 36) {
		Vertex vertex;

		vertex.Position = glm::vec3(
			cube_vertices[3*cube_indices[i]], 
			cube_vertices[3 * cube_indices[i] +1], 
			cube_vertices[3 * cube_indices[i] + 2]);
	
		i += 1;
		Vertices.push_back(vertex);
	}
	return true;
}

/* Configure Nanogui settings with a given window. */
void SetupGUI(GLFWwindow* window)
{
	// Create a nanogui screen and pass the glfw pointer to initialize
	screen = new Screen();
	screen->initialize(window, true);

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	glfwSwapInterval(0);
	glfwSwapBuffers(window);

	// Create nanogui gui
	bool enabled = true;
	FormHelper *gui = new FormHelper(screen);

	ref<Window> nanoguiWindow = gui->addWindow(Eigen::Vector2i(10, 10), "Nanogui Control Bar"); // Gui Header

	// Set vars for camera position:
	gui->addGroup("Position");
	gui_PositionX = gui->addVariable("X", dPositionX);
	gui_PositionY = gui->addVariable("Y", dPositionY);
	gui_PositionZ = gui->addVariable("Z", dPositionZ);

	gui_PositionX->setSpinnable(true);
	gui_PositionY->setSpinnable(true);
	gui_PositionZ->setSpinnable(true);

	// Camera local rotation:
	gui->addGroup("Rotate");
	gui_RotValue = gui->addVariable("Rotate Value", dRotValue);
	gui_RotValue->setSpinnable(true);

	gui->addButton("Rotate Right+", []() { RotateByVal(ERotType::Right_Pos); });
	gui->addButton("Rotate Right-", []() { RotateByVal(ERotType::Right_Neg); });
	gui->addButton("Rotate Up+", []() { RotateByVal(ERotType::Up_Pos); });
	gui->addButton("Rotate Up-", []() { RotateByVal(ERotType::Up_Neg); });
	gui->addButton("Rotate Front+", []() { RotateByVal(ERotType::Front_Pos); });
	gui->addButton("Rotate Front-", []() { RotateByVal(ERotType::Front_Neg); });

	// Camera viewing plane & render settings:
	gui->addGroup("Configuration");

	gui_ModelName = gui->addVariable("Model Name", modelName, enabled);
	gui_ModelName->setItems({ "HEAD", "TEAPOT", "BUCKY", "BONSAI" });
	UpdateModelName();

	gui_RenderType = gui->addVariable("Render Type", renderType, enabled);
	gui_RenderType->setItems({ "Line", "Point", "Triangle" });

	gui_ImageBar = gui->addVariable("Colorbar Image Path:", strImageBar);

	gui->addButton("Reload Model", &ReloadObjectModel);
	gui->addButton("Reset Camera", &ResetGui);

	// Volume Rendering settings:
	gui->addGroup("Volume Rendering");
	gui_ColObject = gui->addVariable("Object Color", colObject);
	gui_bTransferFunctionSign = gui->addVariable("Transfer Function Sign", bTransferFunctionSign);

	gui_SamplingRate = gui->addVariable("Sampling Rate s (unit slice number)", samplingRate);
	gui_SamplingRate->setSpinnable(true);

	// Callbacks to set global variables when changed in the gui:
	gui_ImageBar->setCallback([](const std::string &str) { strImageBar = str; });
	gui_RotValue->setCallback([](double val) { dRotValue = val; });
	gui_PositionX->setCallback([](double val) { dPositionX = val; camera.localPos.x = dPositionX; });
	gui_PositionY->setCallback([](double val) { dPositionY = val; camera.localPos.y = dPositionY; });
	gui_PositionZ->setCallback([](double val) { dPositionZ = val; camera.localPos.z = dPositionZ; });

	gui_RenderType->setCallback([](const ERenderType &val) {renderType = val;});
	gui_ModelName->setCallback([](const EModelName &val) {modelName = val; UpdateModelName(); });

	gui_ColObject->setCallback([](const Color &c) {colObject = c; });
	gui_SamplingRate->setCallback([](int val) { samplingRate = val; OnSetSamplingRate(); });

	// SECOND WINDOW ---------------------------------------------------
	ref<Window> nanoguiWindow2 = gui->addWindow(Eigen::Vector2i(1000, 10), "Nanogui Control Bar 2"); // Gui Header

	AddSlider(gui, nanoguiWindow2, "View Slider", "1.000000", viewSlider);

	// GRAPH
	Widget* panel = new Widget(nanoguiWindow2);
	panel->setLayout(new BoxLayout(Orientation::Horizontal, Alignment::Middle, 20, 20));
	gui->addWidget("Transfer Function", panel);
	graph = new Graph(panel);

	graph->setFixedSize(Vector2i(196, 196)); // 7 * 28
	graph->setHeader("Alpha: (0,1)");
	graph->setCaption("Intensity: (0:255)");
	graph->setFontSize(20);
	graph->setTextColor(nanogui::Color(1.0f, 1.0f, 1.0f, 1.0f));

	SetGraphValues();

	AddSlider(gui, nanoguiWindow2, "Slider 0", "0.161290", slider0);
	AddSlider(gui, nanoguiWindow2, "Slider 1", "0.564516", slider1);
	AddSlider(gui, nanoguiWindow2, "Slider 2", "0.677419", slider2);
	AddSlider(gui, nanoguiWindow2, "Slider 3", "0.612903", slider3);
	AddSlider(gui, nanoguiWindow2, "Slider 4", "0.790323", slider4);
	AddSlider(gui, nanoguiWindow2, "Slider 5", "0.677419", slider5);
	AddSlider(gui, nanoguiWindow2, "Slider 6", "0.790323", slider6);
	AddSlider(gui, nanoguiWindow2, "Slider 7", "0.887097", slider7);
	// END GRAPH

	// Init screen:
	screen->setVisible(true);
	screen->performLayout();

	// Set additional callbacks:

	glfwSetCursorPosCallback(window,
		[](GLFWwindow *, double x, double y) {
		screen->cursorPosCallbackEvent(x, y);
	}
	);

	glfwSetMouseButtonCallback(window,
		[](GLFWwindow *, int button, int action, int modifiers) {
		screen->mouseButtonCallbackEvent(button, action, modifiers);
	}
	);

	glfwSetKeyCallback(window,
		[](GLFWwindow *, int key, int scancode, int action, int mods) {
		screen->keyCallbackEvent(key, scancode, action, mods);
	}
	);

	glfwSetCharCallback(window,
		[](GLFWwindow *, unsigned int codepoint) {
		screen->charCallbackEvent(codepoint);
	}
	);

	glfwSetDropCallback(window,
		[](GLFWwindow *, int count, const char **filenames) {
		screen->dropCallbackEvent(count, filenames);
	}
	);

	glfwSetScrollCallback(window,
		[](GLFWwindow *, double x, double y) {
		screen->scrollCallbackEvent(x, y);
	}
	);

	glfwSetFramebufferSizeCallback(window,
		[](GLFWwindow *, int width, int height) {
		screen->resizeCallbackEvent(width, height);
	}
	);
}

void SetGraphValues()
{
	gVector(0) = slider0;
	gVector(1) = slider1;
	gVector(2) = slider2;
	gVector(3) = slider3;
	gVector(4) = slider4;
	gVector(5) = slider5;
	gVector(6) = slider6;
	gVector(7) = slider7;
	if (graph) {
		graph->setValues(gVector);
	}
}

/* Reset variables used in the gui to their default values. */
void SetDefaults() {
	colObject = nanogui::Color(1.0f, 1.0f, 1.0f, 1.0f);
	dPositionX = 0.0;
	dPositionY = 0.0;
	dPositionZ = -10.0;
	dRotValue = 0.0;
	dNearPlane = camera.NEAR_PLANE;
	dFarPlane = camera.FAR_PLANE;
	renderType = ERenderType::Triangle;
	strObjectFile = "";
	strImageBar = "";
	modelName = EModelName::TEAPOT;
	UpdateModelName();

	camera.Reset();
}

/* Update displayed values via gui variable handles. */
void ResetGui()
{
	SetDefaults();

	gui_ImageBar->setValue(strImageBar);
	gui_RotValue->setValue(dRotValue);
	gui_PositionX->setValue(dPositionX);
	gui_PositionY->setValue(dPositionY);
	gui_PositionZ->setValue(dPositionZ);
	gui_ColObject->setValue(colObject);
	gui_RenderType->setValue(renderType);
}

void LoadTextures() {
	std::string texture_path = "BostonTeapot_256_256_178.raw";
	glm::vec3 dimension = glm::vec3(256,256,178);

	GLubyte* pData = load_3d_raw_data(texture_path, dimension);

	if (!pData) return;

	//glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &texmapID);
	glBindTexture(GL_TEXTURE_3D, texmapID);

	LoadColorMap();

	//glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &colorMapID);
	glBindTexture(GL_TEXTURE_2D, colorMapID);
	
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, dimension.x, dimension.y, dimension.z, 0, GL_RED,GL_UNSIGNED_BYTE, pData);
}

// @ref https://github.com/mmmovania/opengl33_dev_cookbook_2013/tree/master/Chapter7/3DTextureSlicing
void SliceVolume() {
	int num_slices = samplingRate;
	Vertices.clear();

	glm::vec3 viewDir = camera.GetCameraDir();

	//get the max and min distance of each vertex of the unit cube
	//in the viewing direction
	//float max_dist = glm::dot(viewDir, glm::vec3(cube_vertices[0], cube_vertices[1], cube_vertices[2]));
	float max_dist = glm::dot(viewDir, vertexList[0]);
	float min_dist = max_dist;
	int max_index = 0;
	int count = 0;

	for (int i = 1; i < 8; i++) {
		//get the distance between the current unit cube vertex and 
		//the view vector by dot product
		//float dist = glm::dot(viewDir, glm::vec3(cube_vertices[i], cube_vertices[i+1], cube_vertices[i+2]));
		float dist = glm::dot(viewDir, vertexList[i]);

		//if distance is > max_dist, store the value and index
		if (dist > max_dist) {
			max_dist = dist;
			max_index = i;
		}

		//if distance is < min_dist, store the value 
		if (dist < min_dist)
			min_dist = dist;
	}

	//find tha abs maximum of the view direction vector
	int max_dim = 0;
	glm::vec3 v = glm::abs(v);
	float val = v.x;
	if (v.y > val) {
		val = v.y;
		max_dim = 1;
	}
	if (v.z > val) {
		val = v.z;
		max_dim = 2;
	}

	//expand it a little bit
	min_dist -= 0.0001f;
	max_dist += 0.0001f;

	//local variables to store the start, direction vectors, 
	//lambda intersection values
	glm::vec3 vecStart[12];
	glm::vec3 vecDir[12];
	float lambda[12];
	float lambda_inc[12];
	float denom = 0;

	//set the minimum distance as the plane_dist
	//subtract the max and min distances and divide by the 
	//total number of slices to get the plane increment
	float plane_dist = min_dist;
	float plane_dist_inc = (max_dist - min_dist) / float(num_slices);

	//for all edges
	for (int i = 0; i < 12; i++) {
		//get the start position vertex by table lookup
		// vecStart[i] = vertexList[edges[edgeList[max_index][i]][0]];

		/*
		vecStart[i] = glm::vec3(
			cube_vertices[cube_edges[cube_indices[max_index * 3 + i] + 0]],
			cube_vertices[cube_edges[cube_indices[max_index * 3 + i] + 0] + 1],
			cube_vertices[cube_edges[cube_indices[max_index * 3 + i] + 0] + 2]);

		//get the direction by table lookup
		glm::vec3 e2 = glm::vec3(
			cube_vertices[cube_edges[cube_indices[max_index * 3 + i] + 1]],
			cube_vertices[cube_edges[cube_indices[max_index * 3 + i] + 1] + 1],
			cube_vertices[cube_edges[cube_indices[max_index * 3 + i] + 1] + 2]);

		vecDir[i] = e2 - vecStart[i];
		*/

		vecStart[i] = vertexList[edges[edgeList[max_index][i]][0]];

		//get the direction by table lookup
		vecDir[i] = vertexList[edges[edgeList[max_index][i]][1]] - vecStart[i];

		//do a dot of vecDir with the view direction vector
		denom = glm::dot(vecDir[i], viewDir);

		//determine the plane intersection parameter (lambda) and 
		//plane intersection parameter increment (lambda_inc)
		if (1.0 + denom != 1.0) {
			lambda_inc[i] = plane_dist_inc / denom;
			lambda[i] = (plane_dist - glm::dot(vecStart[i], viewDir)) / denom;
		}
		else {
			lambda[i] = -1.0;
			lambda_inc[i] = 0.0;
		}
	}

	// local variables to store the intesected points
		//note that for a plane and sub intersection, we can have 
		//a minimum of 3 and a maximum of 6 vertex polygon
	glm::vec3 intersection[6];
	float dL[12];

	//loop through all slices
	for (int i = num_slices - 1; i >= 0; i--) {

		//determine the lambda value for all edges
		for (int e = 0; e < 12; e++)
		{
			dL[e] = lambda[e] + i * lambda_inc[e];
		}

		//if the values are between 0-1, we have an intersection at the current edge
		//repeat the same for all 12 edges
		if ((dL[0] >= 0.0) && (dL[0] < 1.0)) {
			intersection[0] = vecStart[0] + dL[0] * vecDir[0];
		}
		else if ((dL[1] >= 0.0) && (dL[1] < 1.0)) {
			intersection[0] = vecStart[1] + dL[1] * vecDir[1];
		}
		else if ((dL[3] >= 0.0) && (dL[3] < 1.0)) {
			intersection[0] = vecStart[3] + dL[3] * vecDir[3];
		}
		else continue;

		if ((dL[2] >= 0.0) && (dL[2] < 1.0)) {
			intersection[1] = vecStart[2] + dL[2] * vecDir[2];
		}
		else if ((dL[0] >= 0.0) && (dL[0] < 1.0)) {
			intersection[1] = vecStart[0] + dL[0] * vecDir[0];
		}
		else if ((dL[1] >= 0.0) && (dL[1] < 1.0)) {
			intersection[1] = vecStart[1] + dL[1] * vecDir[1];
		}
		else {
			intersection[1] = vecStart[3] + dL[3] * vecDir[3];
		}

		if ((dL[4] >= 0.0) && (dL[4] < 1.0)) {
			intersection[2] = vecStart[4] + dL[4] * vecDir[4];
		}
		else if ((dL[5] >= 0.0) && (dL[5] < 1.0)) {
			intersection[2] = vecStart[5] + dL[5] * vecDir[5];
		}
		else {
			intersection[2] = vecStart[7] + dL[7] * vecDir[7];
		}
		if ((dL[6] >= 0.0) && (dL[6] < 1.0)) {
			intersection[3] = vecStart[6] + dL[6] * vecDir[6];
		}
		else if ((dL[4] >= 0.0) && (dL[4] < 1.0)) {
			intersection[3] = vecStart[4] + dL[4] * vecDir[4];
		}
		else if ((dL[5] >= 0.0) && (dL[5] < 1.0)) {
			intersection[3] = vecStart[5] + dL[5] * vecDir[5];
		}
		else {
			intersection[3] = vecStart[7] + dL[7] * vecDir[7];
		}
		if ((dL[8] >= 0.0) && (dL[8] < 1.0)) {
			intersection[4] = vecStart[8] + dL[8] * vecDir[8];
		}
		else if ((dL[9] >= 0.0) && (dL[9] < 1.0)) {
			intersection[4] = vecStart[9] + dL[9] * vecDir[9];
		}
		else {
			intersection[4] = vecStart[11] + dL[11] * vecDir[11];
		}

		if ((dL[10] >= 0.0) && (dL[10] < 1.0)) {
			intersection[5] = vecStart[10] + dL[10] * vecDir[10];
		}
		else if ((dL[8] >= 0.0) && (dL[8] < 1.0)) {
			intersection[5] = vecStart[8] + dL[8] * vecDir[8];
		}
		else if ((dL[9] >= 0.0) && (dL[9] < 1.0)) {
			intersection[5] = vecStart[9] + dL[9] * vecDir[9];
		}
		else {
			intersection[5] = vecStart[11] + dL[11] * vecDir[11];
		}

		//after all 6 possible intersection vertices are obtained,
		//we calculated the proper polygon indices by using indices of a triangular fan
		int indices[] = { 0,1,2, 0,2,3, 0,3,4, 0,4,5 };

		//Using the indices, pass the intersection vertices to the vTextureSlices vector
		for (int i = 0; i < 12; i++) {
			Vertex vertex;
			vertex.Position = intersection[indices[i]];
			Vertices.push_back(vertex);
		}		
	}

	int numVertsToRemove = (float)(1 - viewSlider) *  (Vertices.size());
	numVertsToRemove -= numVertsToRemove % 3;

	Vertices.erase(Vertices.begin(), Vertices.begin()+numVertsToRemove);

	InitModel();
}

GLubyte * load_3d_raw_data(std::string texture_path, glm::vec3 dimension) {
	size_t size = dimension[0] * dimension[1] * dimension[2];

	FILE *fp;
	GLubyte *data = new GLubyte[size];			  // 8bit
	if (!(fp = fopen(texture_path.c_str(), "rb"))) {
		std::cout << "Error: opening .raw file failed" << std::endl;
		//exit(EXIT_FAILURE);
		return nullptr;
	}
	else {
		std::cout << "OK: open .raw file successed" << std::endl;
	}
	if (fread(data, sizeof(char), size, fp) != size) {
		std::cout << "Error: read .raw file failed" << std::endl;
		//exit(1);
		return nullptr;
	}
	else {
		std::cout << "OK: read .raw file successed" << std::endl;
	}
	fclose(fp);
	return data;
}


void LoadColorMap()
{
	std::string path = strObjectFile;
	int imgX = 256, imgY = 1;

	if (true) {
		std::cout << strImageBar << "\n";
		LoadImage(strImageBar, imgX, imgY, colorMapID);
	}
}

void LoadImage(std::string img_path, int width, int height, unsigned int ID)
{
	unsigned char* image = SOIL_load_image(img_path.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
	if (image) {
		glBindTexture(GL_TEXTURE_2D, ID);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		SOIL_free_image_data(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind
	}
	else {
		printf("ERROR: Texture not found. %s", img_path);
	}

}