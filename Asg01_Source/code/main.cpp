#include <iostream>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other includes
#include "Shader.h"
#include "nanogui/nanogui.h"

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
double dPositionZ = 0.0;
double dRotValue = 0.0;
double dNearPlane = 0.0;
double dFarPlane = 0.0;
ERenderType renderType = ERenderType::Line;
ECullingType cullingType = ECullingType::CW;
std::string strObjectFile = "";

Screen *screen = nullptr;

// Forward declared functions:
void RotateByVal(ERotType rotType);
void ReloadObjectModel();
void ResetGui();

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
	gui_PositionY = gui->addVariable("Y", dPositionZ);
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
	gui_ColObject->setFinalCallback([](const Color &c) { colObject = c; });
	gui_PositionX->setCallback([](double val) { dPositionX = val; });
	gui_PositionY->setCallback([](double val) { dPositionY = val; });
	gui_PositionZ->setCallback([](double val) { dPositionZ = val; });
	gui_FarPlane->setCallback([](double val) { dFarPlane = val; });
	gui_NearPlane->setCallback([](double val) { dNearPlane = val; });
	gui_RenderType->setCallback([](const ERenderType &val) {renderType = val;});
	gui_CullingType->setCallback([](const ECullingType &val) {cullingType = val;});

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

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	glewInit();


	// Build and compile our shader program
	Shader ourShader("shader/basic.vert", "shader/basic.frag");


	// Set up vertex data (and buffer(s)) and attribute pointers
	GLfloat vertices[] = {
		// Positions         // Colors
		0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f,  // Bottom Right
		-0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,  // Bottom Left
		0.0f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f   // Top 
	};
	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// Color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0); // Unbind VAO


	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();

		// Render
		// Clear the colorbuffer
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Draw the triangle
		ourShader.use();
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);

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

}

void ReloadObjectModel()
{
}

/* Reset variables used in the gui to their default values. */
void SetDefaults() {
	colObject = nanogui::Color(0.5f, 0.5f, 0.7f, 1.f);
	dPositionX = 0.0;
	dPositionY = 0.0;
	dPositionZ = 0.0;
	dRotValue = 0.0;
	dNearPlane = 0.0;
	dFarPlane = 0.0;
	renderType = ERenderType::Line;
	cullingType = ECullingType::CW;
	strObjectFile = "";
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
