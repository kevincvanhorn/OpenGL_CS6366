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
	Solid
};

// Specifies direction of culling.
enum ECullingType {
	CW,
	CCW
};

// Model shading type.
enum EShadingType {
	SMOOTH,
	FLAT
};

// For depth test against camera Z value.
enum EDepthType{
	LESS,
	ALWAYS
};


// Holds data read from the obj file.
struct Vertex {
	// Position
	glm::vec3 Position;
	// Normal
	glm::vec3 Normal;
	// TexCoords
	glm::vec2 TexCoords;
	// Color
	glm::vec3 Color;
};

// Window dimensions
const GLuint width = 1200, height = 700;
Camera camera(width, height); // Create camera w/ screen aspect ratio.
PointLight pointLight(camera.GetCameraPos());

// Gui variable handles to edit fields in the gui:
nanogui::detail::FormWidget<ERenderType, std::integral_constant<bool, true>>* gui_RenderType;
nanogui::detail::FormWidget<ECullingType, std::integral_constant<bool, true>>* gui_CullingType;
nanogui::detail::FormWidget<EShadingType, std::integral_constant<bool, true>>* gui_ShadingType;
nanogui::detail::FormWidget<EDepthType, std::integral_constant<bool, true>>* gui_DepthType;

GUI_COLOR* gui_ColObject;
GUI_STRING* gui_ObjectFile;
GUI_DOUBLE* gui_RotValue;
GUI_DOUBLE* gui_PositionX;
GUI_DOUBLE* gui_PositionY;
GUI_DOUBLE* gui_PositionZ;
GUI_DOUBLE* gui_NearPlane;
GUI_DOUBLE* gui_FarPlane;

GUI_FLOAT* gui_ObjectShininess;
GUI_BOOL* gui_DLightStatus;
GUI_COLOR* gui_DLightAmbient;
GUI_COLOR* gui_DLightDiffuse;
GUI_COLOR* gui_DLightSpecular;
GUI_BOOL* gui_PointLightStatus;
GUI_COLOR* gui_PLightAmbient;
GUI_COLOR* gui_PLightDiffuse;
GUI_COLOR* gui_PLightSpecular;
GUI_BOOL* gui_PLightRotateX;
GUI_BOOL* gui_PLightRotateY;
GUI_BOOL* gui_PLightRotateZ;

GUI_FLOAT* gui_DLightX;
GUI_FLOAT* gui_DLightY;
GUI_FLOAT* gui_DLightZ;

GUI_BOOL* gui_TextureStatus;
GUI_BOOL* gui_NormalMapStatus;

// Variable defaults modified in the gui:
Color colObject(1.0f, 1.0f, 1.0f, 1.0f); // color of the object
double dPositionX = 0.0;
double dPositionY = 0.0;
double dPositionZ = -10.0;
double dRotValue = 0.0;
double dNearPlane = camera.NEAR_PLANE;
double dFarPlane = camera.FAR_PLANE;
ERenderType renderType = ERenderType::Solid;
ECullingType cullingType = ECullingType::CCW;
EShadingType shadingType = EShadingType::SMOOTH;
EDepthType depthType = EDepthType::LESS;

// Second Window:
float fObjectShininess = 32;
bool bDirectionLightStatus = true;
bool bPointLightStatus = false;
bool bPointLightRotateX = false;
bool bPointLightRotateY = false;
bool bPointLightRotateZ = false;
Color cDLightAmbient(0.0f, 0.0f, 0.0f, 1.f);
Color cDLightDiffuse(0.0f, 0.0f, 0.0f, 1.f);
Color cDLightSpecular(0.0f, 0.0f, 0.0f, 1.f);
Color cPLightAmbient(0.0f, 0.0f, 0.0f, 1.f);
Color cPLightDiffuse(0.0f, 0.0f, 0.0f, 1.f);
Color cPLightSpecular(0.0f, 0.0f, 0.0f, 1.f);

float fDLightX = 0;
float fDLightY = -1;
float fDLightZ = -1;
bool bTextureStatus = true;
bool bNormalMapStatus = false;

unsigned int diffuseID;
unsigned int normalID;

std::string strObjectFile = "";

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
void SetCulling();
void SetViewLoc(float min, float max);

void ResetPointLight();

void LoadImage(std::string img_path, int width, int height);
void LoadTextures();

void InitTexturesGL();

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

	strObjectFile = "cyborg.obj";
	ReloadObjectModel();

	// Game Loop:
	while (!glfwWindowShouldClose(window))
	{
		SetCulling(); // Specify culling variables.
		glfwPollEvents();

		// Clear the colorbuffer
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (depthType == EDepthType::ALWAYS) {
		}
		else {
			glEnable(GL_DEPTH_TEST);
		}

		// Draw the Obj
		ourShader.use();
		ourShader.setVec3("modelColor", colObject[0], colObject[1], colObject[2]); // Set the color of the object
		ourShader.setVec3("camPos", camera.GetCameraPos());
		pointLight.Loc = camera.GetCameraPos();

		pointLight.UpdatePos(bPointLightRotateX, bPointLightRotateY, bPointLightRotateZ);
		
		ourShader.setFloat("shininess", fObjectShininess);

		if (shadingType == EShadingType::FLAT) {
			ourShader.setBool("bUseSmooth", false);
		}
		else {
			ourShader.setBool("bUseSmooth", true);
		}

		// Use Diffuse texture
		if (bTextureStatus) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, diffuseID);
		}

		if (bTextureStatus) {
			ourShader.setInt("TexDiffuse", 0);
			ourShader.setBool("bDiffuseTex", true);
		}
		else {
			ourShader.setBool("bDiffuseTex", false);

		}

		// Direction Light
		if (bDirectionLightStatus) {
			ourShader.setVec3("dLightDiffuse", cDLightDiffuse[0], cDLightDiffuse[1], cDLightDiffuse[2]);
			ourShader.setVec3("dLightSpecular", cDLightSpecular[0], cDLightSpecular[1], cDLightSpecular[2]);
			ourShader.setVec3("dLightAmbient", cDLightAmbient[0], cDLightAmbient[1], cDLightAmbient[2]);
		}
		else {
			ourShader.setVec3("dLightDiffuse", 0, 0, 0);
			ourShader.setVec3("dLightSpecular", 0, 0, 0);
			ourShader.setVec3("dLightAmbient", 0, 0, 0);
		}

		// Point Light
		if (bPointLightStatus) {
			ourShader.setVec3("pLightDiffuse", cPLightDiffuse[0], cPLightDiffuse[1], cPLightDiffuse[2]);
			ourShader.setVec3("pLightSpecular", cPLightSpecular[0], cPLightSpecular[1], cPLightSpecular[2]);
			ourShader.setVec3("pLightAmbient", cPLightAmbient[0], cPLightAmbient[1], cPLightAmbient[2]);
			ourShader.setVec3("pLightloc", pointLight.getLoc());

			//printf("%f \n",pointLight.getLoc().x);
		}
		else {
			ourShader.setVec3("pLightDiffuse", 0, 0, 0);
			ourShader.setVec3("pLightSpecular", 0, 0, 0);
			ourShader.setVec3("pLightAmbient", 0, 0, 0);
			ourShader.setVec3("pLightLoc", pointLight.getLoc());
		}
		
		glBindVertexArray(VAO);

		if (Vertices.size() > 0) {
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &camera.GetMVPMatrix()[0][0]);
			
			// Set draw type:
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

	InitTexturesGL();
	LoadTextures();

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

/* Configures OpenGl vars for culling. */
void SetCulling()
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	if (cullingType == ECullingType::CCW) {
		glFrontFace(GL_CCW);
	}
	else{
		glFrontFace(GL_CW);
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

void ResetPointLight()
{
	pointLight.MovedLoc = pointLight.Loc;
}

void InitTexturesGL()
{
	glGenTextures(1, &diffuseID);
	//glGenTextures(1, &normalID);

	//glActiveTexture(GL_TEXTURE0);

	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, normalID);
}

void LoadImage(std::string img_path, int width, int height)
{
	unsigned char* image = SOIL_load_image(img_path.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
	if (image) {
		glBindTexture(GL_TEXTURE_2D, diffuseID);
		
		//printf("FOUND: Texture found.");
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else {
		printf("ERROR: Texture not found. %s", img_path);
	}

}

void LoadTextures()
{
	std::string path = strObjectFile;
	int imgX, imgY;

	if (strObjectFile.length() > 0) {
		const std::string obj(".obj");
		if (path.size() > obj.size() &&
			path.substr(path.size() - obj.size()) == ".obj") {
			path = path.substr(0, path.length() - obj.length());
			std::cout << path;
			if (path.compare("cyborg") !=0 && path.compare("cube") !=0) {
				printf("ERROR: Texture maps not provided for this object:  _%s_ ", path);
				return;
			}
			if (path == "cube") {
				imgX = imgY = 256;
			}
			else { imgX = imgY = 1024; }
			printf("%d",imgX);
		}
		else {
			printf("ERROR: Invalid object name for texture loading.");
			return;
		}
	}

	if (true){//bTextureStatus) {
		std::string texPath = path.append("_diffuse.png");
		std::cout << texPath << "\n";
		LoadImage(texPath, imgX, imgY);
	}
	if (bNormalMapStatus) {
		std::string normPath = path.append("_normal.png");
		LoadImage(normPath, imgX, imgY);
	}
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

	SetViewLoc(Min_Z, Max_Z); // Position object in center.

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
	gui_NearPlane = gui->addVariable("Z Near", dNearPlane);
	gui_FarPlane = gui->addVariable("Z Far", dFarPlane);

	gui_NearPlane->setSpinnable(true);
	gui_FarPlane->setSpinnable(true);

	gui_RenderType = gui->addVariable("Render Type", renderType, enabled);
	gui_RenderType->setItems({ "Line", "Point", "Solid" });

	gui_CullingType = gui->addVariable("Culling Type", cullingType, enabled);
	gui_CullingType->setItems({ "CW", "CCW" });

	gui_ShadingType = gui->addVariable("Shading Type", shadingType, enabled);
	gui_ShadingType->setItems({ "SMOOTH", "FLAT" });

	gui_DepthType = gui->addVariable("Depth Type", depthType, enabled);
	gui_DepthType->setItems({ "LESS", "ALWAYS" });

	gui_ObjectFile = gui->addVariable("Model Name", strObjectFile);

	gui->addButton("Reload Model", &ReloadObjectModel);
	gui->addButton("Reset Camera", &ResetGui);

	// Callbacks to set global variables when changed in the gui:
	gui_ObjectFile->setCallback([](const std::string &str) { strObjectFile = str; });
	gui_RotValue->setCallback([](double val) { dRotValue = val; });
	gui_PositionX->setCallback([](double val) { dPositionX = val; camera.localPos.x = dPositionX; });
	gui_PositionY->setCallback([](double val) { dPositionY = val; camera.localPos.y = dPositionY; });
	gui_PositionZ->setCallback([](double val) { dPositionZ = val; camera.localPos.z = dPositionZ; });
	gui_FarPlane->setCallback([](double val) { dFarPlane = val; camera.farPlane = dFarPlane; });
	gui_NearPlane->setCallback([](double val) { dNearPlane = val; camera.nearPlane = dNearPlane; });

	gui_RenderType->setCallback([](const ERenderType &val) {renderType = val;});
	gui_CullingType->setCallback([](const ECullingType &val) {cullingType = val; SetCulling(); });
	gui_ShadingType->setCallback([](const EShadingType &val) {shadingType = val; });
	gui_DepthType->setCallback([](const EDepthType &val) {depthType = val; });

	// SECOND WINDOW ---------------------------------------------------
	ref<Window> nanoguiWindow2 = gui->addWindow(Eigen::Vector2i(230, 10), "Nanogui Control Bar 2"); // Gui Header

	// Object color:
	gui->addGroup("Lighting");
	gui_ColObject = gui->addVariable("Object Color", colObject);

	gui_ObjectShininess = gui->addVariable("Object Shininess", fObjectShininess);

	gui_DLightStatus = gui->addVariable("Direction Light Status", bDirectionLightStatus);

	gui_DLightX = gui->addVariable("Direction Light Direction X", fDLightX);
	gui_DLightY = gui->addVariable("Direction Light Direction Y", fDLightY);
	gui_DLightZ = gui->addVariable("Direction Light Direction Z", fDLightZ);


	gui_DLightAmbient = gui->addVariable("Direction Light Ambient Color", cDLightAmbient);
	gui_DLightDiffuse = gui->addVariable("Direction Light Ambient Diffuse", cDLightDiffuse);
	gui_DLightSpecular = gui->addVariable("Direction Light Ambient Specular", cDLightSpecular);

	gui_PointLightStatus = gui->addVariable("Point Light Status", bPointLightStatus);
	gui_PLightAmbient = gui->addVariable("Point  Light Ambient Color", cPLightAmbient);
	gui_PLightDiffuse = gui->addVariable("Point Light Ambient Diffuse", cPLightDiffuse);
	gui_PLightSpecular = gui->addVariable("Point Light Ambient Specular", cPLightSpecular);

	gui_PLightRotateX = gui->addVariable("Point Light Rotate on X", bPointLightRotateX);
	gui_PLightRotateY = gui->addVariable("Point Light Rotate on Y", bPointLightRotateY);
	gui_PLightRotateZ = gui->addVariable("Point Light Rotate on Z", bPointLightRotateZ);

	gui->addButton("Reset Point Light", &ResetPointLight);

	// Set Callbacks
	//gui_ColObject->setFinalCallback([](const Color &c) { colObject = c; SetColor(); }); // TODO: Change this from final to normal callback
	gui_ColObject->setCallback([](const Color &c) {colObject = c; });
	gui_DLightAmbient->setCallback([](const Color &c) {cDLightAmbient = c; });
	gui_PLightSpecular->setCallback([](const Color &c) {cPLightSpecular = c; });
	gui_PLightRotateX->setCallback([](bool b) {bPointLightRotateX = b; });

	// ASSIGNMENT 3 - -----------------------------------------------
	gui_TextureStatus = gui->addVariable("Texture Status", bTextureStatus);
	gui_NormalMapStatus = gui->addVariable("Normal Map Status", bNormalMapStatus);

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

/* Reset variables used in the gui to their default values. */
void SetDefaults() {
	colObject = nanogui::Color(1.0f, 1.0f, 1.0f, 1.0f);
	dPositionX = 0.0;
	dPositionY = 0.0;
	dPositionZ = -10.0;
	dRotValue = 0.0;
	dNearPlane = camera.NEAR_PLANE;
	dFarPlane = camera.FAR_PLANE;
	renderType = ERenderType::Line;
	cullingType = ECullingType::CCW;
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
