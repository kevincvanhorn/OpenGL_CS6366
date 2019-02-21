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
#include <glm/glm.hpp>

#include "Camera.h"

using namespace nanogui;

// Shorthand for gui variable handles:
#define GUI_DOUBLE nanogui::detail::FormWidget<double, std::integral_constant<bool, true>>
#define GUI_STRING nanogui::detail::FormWidget<std::string, std::true_type>
#define GUI_COLOR nanogui::detail::FormWidget<nanogui::Color, std::true_type>

enum test_enum {
	Item1 = 0,
	Item2,
	Item3
};

enum ERotType {
	Right_Pos,
	Right_Neg,
	Up_Pos,
	Up_Neg,
	Front_Pos,
	Front_Neg
};

enum ERenderType {
	Line,
	Point,
	Solid
};

enum ECullingType {
	CW,
	CCW
};

struct Vertex {
	// Position
	glm::vec3 Position;
	glm::vec3 Color;
	// Normal
	glm::vec3 Normal;
	// TexCoords
	glm::vec2 TexCoords;
};

// Window dimensions
const GLuint width = 1200, height = 700;

// Gui variable handles:
nanogui::detail::FormWidget<ERenderType, std::integral_constant<bool, true>>* gui_RenderType;
nanogui::detail::FormWidget<ECullingType, std::integral_constant<bool, true>>* gui_CullingType;
GUI_COLOR* gui_ColObject;
GUI_STRING* gui_ObjectFile;
GUI_DOUBLE* gui_RotValue;
GUI_DOUBLE* gui_PositionX;
GUI_DOUBLE* gui_PositionY;
GUI_DOUBLE* gui_PositionZ;
GUI_DOUBLE* gui_NearPlane;
GUI_DOUBLE* gui_FarPlane;

// Variables modified in the gui:
Color colObject(0.5f, 0.5f, 0.7f, 1.f);
double dPositionX = 0.0;
double dPositionY = 0.0;
double dPositionZ = -10.0;
double dRotValue = 0.0;
double dNearPlane = 0.0;
double dFarPlane = 0.0;
ERenderType renderType = ERenderType::Line;
ECullingType cullingType = ECullingType::CW;
std::string strObjectFile = "";

// Render Variables
Screen *screen = nullptr;
std::vector<Vertex> Vertices;
GLuint VBO, VAO;
Camera camera(width, height);

// Forward declared functions:
void RotateByVal(ERotType rotType);
void ReloadObjectModel();
void ResetGui();
bool LoadModel(std::vector<Vertex> &vertices);

void SetupGUI(GLFWwindow* window);
void InitModel();

void SetColor();
void SetCulling();

// The MAIN function, from here we start the application and run the game loop
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
	glClear(GL_COLOR_BUFFER_BIT);

	SetupGUI(window);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	glewInit();

	// Build and compile our shader program
	Shader ourShader("shader/basic.vert", "shader/basic.frag");
	
	strObjectFile = "rock.obj";
	LoadModel(Vertices);

	glEnable(GL_CULL_FACE);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0); // Unbind VAO

	GLuint MatrixID = glGetUniformLocation(ourShader.program, "MVP");

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();

		// Clear the colorbuffer
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Draw the triangle
		ourShader.use();
		glBindVertexArray(VAO);

		if (Vertices.size() > 0) {
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &camera.GetMVPMatrix()[0][0]);

			if (renderType == ERenderType::Solid) {
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

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		screen->drawWidgets();

		// Swap the screen buffers
		glfwSwapBuffers(window);
	}
	// Properly de-allocate all resources once they've outlived their purpose
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
	return 0;
}

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

void ReloadObjectModel()
{
	Vertices.clear();
	if (LoadModel(Vertices)) {
		SetColor();
		//InitModel();
	}
}

// Reinitialize buffer for drawing:
void InitModel() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Color));

	glBindVertexArray(0); // Unbind VAO
}

void SetColor()
{
	for (int i = 0; i < Vertices.size(); i++) {
		Vertices[i].Color = glm::vec3(colObject.r(), colObject.g(), colObject.b());
	}
	if (Vertices.size() > 0) {
		InitModel();
	}
}

void SetCulling()
{
	glCullFace(GL_BACK);

	if (cullingType == ECullingType::CCW) {
		glFrontFace(GL_CCW);
		printf("1");
	}
	else{
		glFrontFace(GL_CW);
		printf("2");
	}
}

/*
 * Read 
 * @ref Assignment Tips.
 */
bool LoadModel(std::vector<Vertex> &vertices) {
	std::vector<glm::vec3> positions;
	std::vector< glm::vec3> normals;
	std::vector<glm::vec2> tex_coords;

	std::ifstream file(strObjectFile.c_str(), std::ios::in);
	
	// Check for valid file
	if (!file) {
		printf("ERROR: Invalid file path.\n");
		return false;
	}
	try {
		std::string curLine;

		// Read each line
		while (getline(file, curLine)) {
			std::stringstream ss(curLine);
			std::string firstWord;
			ss >> firstWord;

			// Vertex:
			if (firstWord == "v") {
				glm::vec3 vert_pos;
				ss >> vert_pos[0] >> vert_pos[1] >> vert_pos[2];
				positions.push_back(vert_pos);
			}
			// Texture Coordinate
			else if (firstWord == "vt") {
				glm::vec2 tex_coord;
				ss >> tex_coord[0] >> tex_coord[1];
				tex_coords.push_back(tex_coord);
			}
			// Vertex Normal:
			else if (firstWord == "vn") {
				glm::vec3 vert_norm;
				ss >> vert_norm[0] >> vert_norm[1] >> vert_norm[2];
				normals.push_back(vert_norm);
			}
			// Face:
			else if (firstWord == "f") {
				std::string s_vertex_0, s_vertex_1, s_vertex_2;
				ss >> s_vertex_0 >> s_vertex_1 >> s_vertex_2;
				int pos_idx, tex_idx, norm_idx;
				std::sscanf(s_vertex_0.c_str(), "%d/%d/%d", &pos_idx, &tex_idx, &norm_idx);
				// We have to use index - 1 because the obj index starts at 1
				Vertex vertex_0;
				vertex_0.Position = positions[pos_idx - 1];
				vertex_0.TexCoords = tex_coords[tex_idx - 1];
				vertex_0.Normal = normals[norm_idx - 1];
				sscanf(s_vertex_1.c_str(), "%d/%d/%d", &pos_idx, &tex_idx, &norm_idx);

				Vertex vertex_1;
				vertex_1.Position = positions[pos_idx - 1];
				vertex_1.TexCoords = tex_coords[tex_idx - 1];
				vertex_1.Normal = normals[norm_idx - 1];
				sscanf(s_vertex_2.c_str(), "%d/%d/%d", &pos_idx, &tex_idx, &norm_idx);

				Vertex vertex_2;
				vertex_2.Position = positions[pos_idx - 1];
				vertex_2.TexCoords = tex_coords[tex_idx - 1];
				vertex_2.Normal = normals[norm_idx - 1];
				vertices.push_back(vertex_0);
				vertices.push_back(vertex_1);
				vertices.push_back(vertex_2);
			}
		}
	}
	catch (const std::exception&) {
		std::cout << "ERROR: Obj file cannot be read.\n";
		return false;
	}
	return true;
}

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

	gui->addGroup("Color");
	gui_ColObject = gui->addVariable("Object Color", colObject);

	gui->addGroup("Position");

	gui_PositionX = gui->addVariable("X", dPositionX);
	gui_PositionY = gui->addVariable("Y", dPositionY);
	gui_PositionZ = gui->addVariable("Z", dPositionZ);

	gui_PositionX->setSpinnable(true);
	gui_PositionY->setSpinnable(true);
	gui_PositionZ->setSpinnable(true);

	gui->addGroup("Rotate");
	gui_RotValue = gui->addVariable("Rotate Value", dRotValue);
	gui_RotValue->setSpinnable(true);

	gui->addButton("Rotate Right+", []() { RotateByVal(ERotType::Right_Pos); });
	gui->addButton("Rotate Right-", []() { RotateByVal(ERotType::Right_Neg); });
	gui->addButton("Rotate Up+", []() { RotateByVal(ERotType::Up_Pos); });
	gui->addButton("Rotate Up-", []() { RotateByVal(ERotType::Up_Neg); });
	gui->addButton("Rotate Front+", []() { RotateByVal(ERotType::Front_Pos); });
	gui->addButton("Rotate Front-", []() { RotateByVal(ERotType::Front_Neg); });

	gui->addGroup("Configuration");
	gui_NearPlane = gui->addVariable("Z Near", dNearPlane);
	gui_FarPlane = gui->addVariable("Z Far", dFarPlane);

	gui_NearPlane->setSpinnable(true);
	gui_FarPlane->setSpinnable(true);

	gui_RenderType = gui->addVariable("Render Type", renderType, enabled);
	gui_RenderType->setItems({ "Line", "Point", "Solid" });

	gui_CullingType = gui->addVariable("Culling Type", cullingType, enabled);
	gui_CullingType->setItems({ "CW", "CCW" });

	gui_ObjectFile = gui->addVariable("Model Name", strObjectFile);

	gui->addButton("Reload Model", &ReloadObjectModel);
	gui->addButton("Reset", &ResetGui);

	// Callbacks to set global variables when changed in the gui:
	gui_ObjectFile->setCallback([](const std::string &str) { strObjectFile = str; });
	gui_RotValue->setCallback([](double val) { dRotValue = val; });
	gui_ColObject->setFinalCallback([](const Color &c) { colObject = c; SetColor(); });
	gui_PositionX->setCallback([](double val) { dPositionX = val; camera.localPos.x = dPositionX; });
	gui_PositionY->setCallback([](double val) { dPositionY = val; camera.localPos.y = dPositionY; });
	gui_PositionZ->setCallback([](double val) { dPositionZ = val; camera.localPos.z = dPositionZ; });
	gui_FarPlane->setCallback([](double val) { dFarPlane = val; camera.farPlane = dFarPlane; });
	gui_NearPlane->setCallback([](double val) { dNearPlane = val; camera.nearPlane = dNearPlane; });
	gui_RenderType->setCallback([](const ERenderType &val) {renderType = val;});
	gui_CullingType->setCallback([](const ECullingType &val) {cullingType = val; SetCulling(); });

	screen->setVisible(true);
	screen->performLayout();

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

/* Reset variables used in the gui to their default values. */
void SetDefaults() {
	colObject = nanogui::Color(0.5f, 0.5f, 0.7f, 1.f);
	dPositionX = 0.0;
	dPositionY = 0.0;
	dPositionZ = -10.0;
	dRotValue = 0.0;
	dNearPlane = 0.0;
	dFarPlane = 0.0;
	renderType = ERenderType::Line;
	cullingType = ECullingType::CW;
	strObjectFile = "";

	camera.Reset();
}

/* Update displayed values via gui variable handles. */
void ResetGui()
{
	SetDefaults();

	gui_ObjectFile->setValue(strObjectFile);
	gui_RotValue->setValue(dRotValue);
	gui_PositionX->setValue(dPositionX);
	gui_PositionY->setValue(dPositionY);
	gui_PositionZ->setValue(dPositionZ);
	gui_NearPlane->setValue(dNearPlane);
	gui_FarPlane->setValue(dFarPlane);
	gui_ColObject->setValue(colObject);
	gui_RenderType->setValue(renderType);
	gui_CullingType->setValue(cullingType);
}
