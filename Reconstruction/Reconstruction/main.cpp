#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "model.h"
#include "camera.h"
#include "utils.h"
#include "process_points.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 900;
const unsigned int SCR_HEIGHT = 720;
//远近裁剪面
const float CAM_NEAR = 1.0f;
const float CAM_FAR = 20.0f;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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

	// build and compile shaders
	// -------------------------
	Shader ourShader("1.model_loading.vs", "1.model_loading.fs");

	// load models
	// -----------
	Model ourModel("obj/ball/3.obj");//模型
	// 获取位置和颜色信息
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, CAM_NEAR, CAM_FAR);
	glm::mat4 inverse_projection = glm::inverse(projection);
	const int total_frames = 1; // 总帧数
	const int start_idx = 170; // 重建帧的开始	
	vector<vector<point_3D>> frames_points;
	vector<Mat> rgbsMat;
	vector<ProcessPoints> frames_pps;
	
	for (int i = start_idx; i < total_frames + start_idx; ++i) {
		vector<MyPoint> m_pos;
		vector<MyPoint> m_color;
		get_3D_param(inverse_projection, i, m_pos, m_color);// 获取当前帧的点的信息
		ProcessPoints pps(m_pos, m_color);
		frames_pps.push_back(pps);
		Mat rgbMat = get_rgbMat(i);// 获取当前帧
		rgbsMat.push_back(rgbMat);
	}
	cout << frames_pps[0].points_num() << endl;
	// 开始处理点云

	int num_frame = 0; // 当前帧，用来显示每一帧和当前帧的所有点
	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// don't forget to enable shader before setting uniforms
		ourShader.use();

		// view/projection transformations
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, CAM_NEAR, CAM_FAR);
		glm::mat4 view = camera.GetViewMatrix();
		ourShader.setMat4("projection", projection); 
		ourShader.setMat4("view", view);

		ProcessPoints pps = frames_pps[num_frame];
		// render the loaded model
		for (int i = 0; i < pps.points_num(); ++i) {
			// point3d 位置
			glm::mat4 model;
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
			model = glm::translate(model, pps.pos(i));
			model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));	// it's a bit too big for our scene, so scale it down
			ourShader.setMat4("model", model);
			// point 颜色
			ourShader.setVec3("pointColor", pps.color(i));
			ourModel.Draw(ourShader);
		}

		// opencv: 显示重建的当前帧
		imshow("window", rgbsMat[num_frame]);
		// glfw:渲染出当前帧
		glfwSwapBuffers(window);
		glfwPollEvents();
		// 更新相机位置从不同角度观察重建的效果
		// updateCamera(camera);
		// 更新当前帧idx
		if (++num_frame == total_frames) num_frame = 0;
		//cout << "frame:" << num_frame << endl;
			
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to tops

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}