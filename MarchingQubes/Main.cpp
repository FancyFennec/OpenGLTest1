#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Headers/Constants.h"
#include "Headers/Window.h"
#include "Headers/Model.h"
#include "Headers/Light.h"
#include "Headers/MarchingCube.h"
#include "Headers/pNoise.h"

#include "Imgui/imgui.h"
#include "Imgui/imgui_impl_glfw.h"
#include "Imgui/imgui_impl_opengl3.h"

void updateDeltaTime();

// camera
Window window(Camera(glm::vec3(0.0f, 0.0f, 3.0f)), SCR_WIDTH, SCR_HEIGHT);
PerlinNoise pNoise = PerlinNoise(3);

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

const char* glsl_version = "#version 130";

std::vector<float> gridValues = {};

int main()
{
	window.initialise();

	// Setup Dear ImGui context
	bool err = gladLoadGL() == 0;
	if (err)
	{
		fprintf(stderr, "Failed to initialize OpenGL loader!\n");
		return 1;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings

	ImGui_ImplGlfw_InitForOpenGL(window.getWindow(), true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile our shader program
	// ------------------------------------
	Shader lampShader("Shaders/lampShader.vs", "Shaders/lampShader.fs");
	Shader simpleShader("Shaders/simpleShader.vs", "Shaders/simpleShader.fs");

	// set up vertex data (and buffer(s)) and configure vertex attributes
   // ------------------------------------------------------------------
	
	MarchingCube mc;

	int x = 300, y = 300, z = 300;

	for (int i = -1; i < x + 1; ++i) {
		for (int j = -1; j < y + 1; ++j) {
			for (int k = -1; k < z + 1; ++k) {
				if (i == -1 || j == -1 || k == -1 || i == x || j == y || k == z) {
					gridValues.push_back(0);
				} else {
					gridValues.push_back((float)pNoise.noise((double)i / ((double)x), (double)j / ((double)y), (double)k / ((double)z)));
				}
				
			}
		}
	}

	std::vector<float> mesh = mc.generateMesh(gridValues.data(), x + 2, y + 2, z + 2);

	Mesh cubeMesh;
	//cubeMesh.CreateMesh(mc.lookupMesh[45].data(), mc.lookupMesh[45].size() / 9);
	cubeMesh.CreateMesh(mesh.data(), mesh.size() / 9);
	Light light(&simpleShader);

	Model simpleModel(cubeMesh, simpleShader);
	Model lampModel(lampShader);
	lampModel.useCubeMesh();

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	glm::mat4 projection = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 lsmodel = glm::mat4(1.0f);

	// render loop
   // -----------
	while (!window.getShouldClose())
	{
		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// per-frame time logic and process input
		// --------------------
		updateDeltaTime();
		window.processInput(deltaTime); //TODO:Mybe we can solve this different, such that we don't have to put in delta time here
		// world transformation
		projection = glm::perspective(glm::radians(window.getCamera().Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		view = window.getCamera().GetViewMatrix();
		lsmodel = glm::mat4(1.0f);
		// clear screen and buffers
		// ------
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Set Ligh properties
		lsmodel = glm::rotate(lsmodel, (float)glfwGetTime(), glm::vec3(0, 1, 0));
		lsmodel = glm::translate(lsmodel, lightPos);
		lsmodel = glm::scale(lsmodel, glm::vec3(0.2f)); // a smaller cube

		static float a = 0.5f;
		static float d = 0.8f;
		static float s = 0.3f;

		light.setAmbient(glm::vec3(a));
		light.setDiffuse(glm::vec3(d));
		light.setSpecular(glm::vec3(s));
		light.setPosition(lightPos);
		light.setViewPos(window.getCamera().Position);

		//draw the lamp object
		lampModel.shader.useMVP(lsmodel, view, projection);
		lampModel.renderModel();


		//Draw simple cube
		simpleModel.shader.useLsMVP(lsmodel, view, projection);
		// light properties
		light.updateShader();
		simpleModel.updateColourShader();
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0, 0, -3));
		model = glm::scale(model, glm::vec3(4 / ((float)x), 4 / ((float)y), 4 / ((float)z)));
		simpleModel.shader.setMat4("model", model);
		simpleModel.renderModel();

		//Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{

			ImGui::Begin("Set light properties");                          // Create a window called "Hello, world!" and append into it.
			ImGui::SliderFloat("Ambient", &a, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::SliderFloat("Diffuse", &d, 0.0f, 1.0f);
			ImGui::SliderFloat("Specular", &s, 0.0f, 1.0f);

			ImGui::End();
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		window.swapBuffers();
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	cubeMesh.~Mesh();
	window.~Window();
	return 0;
}

void updateDeltaTime()
{
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
}