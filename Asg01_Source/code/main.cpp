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
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Tangent;
	glm::vec3 BiTangent;

	glm::vec3 Color;
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
std::string strImageBar = "./objs/colorbar.png";
bool bTransferFunctionSign = true;
int samplingRate = 2000;
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

std::string strObjectFile;

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
		ourShader.setVec3("camPos", camera.GetCameraPos());
		pointLight.Loc = camera.GetCameraPos();

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
	if (LoadModel(Vertices)) {
		SetColor();
		//LoadTextures();
	}
}

/* Reinitializes buffer for drawing, handling OpenGL configuration. */
void InitModel() {
	std::cout << "Init Model. \n";
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	// Normal attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

	// Texture Coords attribute:
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

	// Tangent attribute:
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));

	// BiTangent attribute:
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, BiTangent));

	glBindVertexArray(0); // Unbind VAO
}

/* Set the color value for each vertex. */
void SetColor()
{
	for (int i = 0; i < Vertices.size(); i++) {
		Vertices[i].Color = glm::vec3(colObject.r(), colObject.g(), colObject.b());
	}
	if (Vertices.size() > 0) {
		InitModel();
	}
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

/*
 * Load the obj model from a file.
 * @ref Assignment Tips in assignment writeup.
 */
bool LoadModel(std::vector<Vertex> &vertices) {
	std::vector<glm::vec3> positions;
	std::vector< glm::vec3> normals;
	std::vector<glm::vec2> tex_coords;

	std::ifstream file(strObjectFile.c_str(), std::ios::in);

	float Min_Z = std::numeric_limits<float>::max();
	float Max_Z = std::numeric_limits<float>::min();
	
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

				// Construct bounds of model:
				if (vert_pos[1] < Min_Z) {
					Min_Z = vert_pos[1];
				}
				if (vert_pos[1] > Max_Z) {
					Max_Z = vert_pos[1];
				}
				
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
				vertex_0.TexCoords.y *= -1;
				vertex_0.Normal = normals[norm_idx - 1];
				sscanf(s_vertex_1.c_str(), "%d/%d/%d", &pos_idx, &tex_idx, &norm_idx);

				Vertex vertex_1;
				vertex_1.Position = positions[pos_idx - 1];
				vertex_1.TexCoords = tex_coords[tex_idx - 1];
				vertex_1.TexCoords.y *= -1;
				vertex_1.Normal = normals[norm_idx - 1];
				sscanf(s_vertex_2.c_str(), "%d/%d/%d", &pos_idx, &tex_idx, &norm_idx);

				Vertex vertex_2;
				vertex_2.Position = positions[pos_idx - 1];
				vertex_2.TexCoords = tex_coords[tex_idx - 1];
				vertex_2.TexCoords.y *= -1;
				vertex_2.Normal = normals[norm_idx - 1];

				// Compute Triangle tangent/bi-tangent:
				glm::vec2 UV1 = vertex_1.TexCoords - vertex_0.TexCoords;
				glm::vec2 UV2 = vertex_2.TexCoords - vertex_0.TexCoords;
				glm::vec3 V1 = vertex_1.Position - vertex_0.Position;
				glm::vec3 V2 = vertex_2.Position - vertex_0.Position;
				float d = 1.0f / (UV1.x * UV2.y - UV1.y * UV2.x);
				glm::vec3 tangent;// = (V1 * UV2.y - V2 * UV1.y)*d;
				tangent.x = d * (UV2.y * V1.x - UV1.y * V2.x);
				tangent.y = d * (UV2.y * V1.y - UV1.y * V2.y);
				tangent.z = d * (UV2.y * V1.z - UV1.y * V2.z);
				tangent = glm::normalize(tangent);
				
				glm::vec3 bitangent;// = (V2 * UV1.x - V1 * UV2.x)*d;
				bitangent.x = d * (-UV2.x * V1.x + UV1.x * V2.x);
				bitangent.y = d * (-UV2.x * V1.y + UV1.x * V2.y);
				bitangent.z = d * (-UV2.x * V1.z + UV1.x * V2.z);
				bitangent = glm::normalize(bitangent);

				vertex_0.Tangent = tangent;
				vertex_1.Tangent = tangent;
				vertex_2.Tangent = tangent;
				vertex_0.BiTangent = bitangent;
				vertex_1.BiTangent = bitangent;
				vertex_2.BiTangent = bitangent;

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

	camera.Reset();

	if (strObjectFile == "DNE.obj") {
		SetViewLoc(Min_Z, Max_Z); // Position object in center.
		camera.RotX = 30;
		camera.localPos.z = 2.3;
		dPositionZ = 2.3f;
		gui_PositionZ->setValue(dPositionZ);
		camera.localPos.y = 1.3;
		dPositionY = 1.3f;
		gui_PositionY->setValue(dPositionY);
	}
	else {
		SetViewLoc(Min_Z, Max_Z); // Position object in center.
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

GLubyte * load_3d_raw_data(std::string texture_path, glm::vec3 dimension) {
	size_t size = dimension[0] * dimension[1] * dimension[2];

	FILE *fp;
	GLubyte *data = new GLubyte[size];			  // 8bit
	if (!(fp = fopen(texture_path.c_str(), "rb"))) {
		std::cout << "Error: opening .raw file failed" << std::endl;
		exit(EXIT_FAILURE);
	}
	else {
		std::cout << "OK: open .raw file successed" << std::endl;
	}
	if (fread(data, sizeof(char), size, fp) != size) {
		std::cout << "Error: read .raw file failed" << std::endl;
		exit(1);
	}
	else {
		std::cout << "OK: read .raw file successed" << std::endl;
	}
	fclose(fp);
	return data;
}
