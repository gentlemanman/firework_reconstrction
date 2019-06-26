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
#include <fstream>
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
Camera camera(glm::vec3(0.0f, 0.0f, -1.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
// 是否暂停当前帧
bool is_pause = false;

int main()
{
	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
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
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	// configure global opengl state
	glEnable(GL_DEPTH_TEST);
	// 背面剔除
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// build and compile shaders
	Shader ourShader("firework.vs", "firework.fs");
	Shader castleShader("castle.vs", "castle.fs");

	// load models
	Model ourModel("obj/ball/1.obj"); // 模型
	Model castleModel("obj/castle/my.obj");

	// 一、开始处理原始数据，获取位置和颜色
	int wid = 480, height = 480;
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)wid / (float)height, CAM_NEAR, CAM_FAR);
	glm::mat4 inverse_projection = glm::inverse(projection);
	const int total_frames = 5; // 总帧数
	const int start_idx = 0; // 重建帧的开始	
	cout << "开始的索引:" << start_idx << "  总共的帧数:" << total_frames << endl;
	vector<vector<point_3D>> frames_points;
	vector<Mat> rgbsMat;
	vector<ProcessPoints> frames_pps;
	
	for (int i = start_idx; i < total_frames + start_idx; ++i) {
		cout << "处理原始rgb和depth中：" << i - start_idx << "/" << total_frames<< endl;
		vector<glm::vec3> m_pos;
		vector<glm::vec3> m_color;
		get_3D_param(inverse_projection, i, m_pos, m_color);// 获取当前帧的点的信息
		ProcessPoints pps(m_pos, m_color, is_Interpolation);
		frames_pps.push_back(pps);
		// 获取当前帧
		Mat rgbMat = get_rgbMat(i);
		rgbsMat.push_back(rgbMat);
	}
	// 二、获取所有帧的插值点
	vector<vector<glm::vec3>> insert_pos;
	vector<vector<glm::vec3>> insert_color;
	if(is_Interpolation)
		Interpolation(frames_pps, insert_pos, insert_color);// 计算每一帧的插值点
	
	// 三、将点信息保存至文件(开辟一个新线程来写文件)
	if (is_SavePointsToTxt) {
		thread thread_read(SavePointsToTxt, ref(frames_pps), ref(insert_pos), ref(insert_color), start_idx); thread_read.detach();
		//SavePointsToTxt(frames_pps, insert_pos, insert_color, start_idx);
	}
	
	// 四、形成buffer数据
	vector<glm::mat4*> frames_modelMatrices;
	vector<glm::vec3*> frames_colors;
	vector<int> frames_amount;
	GenBufferData(frames_pps, insert_pos, insert_color, frames_modelMatrices, frames_colors, frames_amount);
	
	// 在渲染循环外，定义数据的buffer
	unsigned int buffer_pos;
	glGenBuffers(1, &buffer_pos);
	
	unsigned int buffer_color;
	glGenBuffers(1, &buffer_color);
	
	int cur_frame = 0; // 当前帧，用来显示每一帧和当前帧的所有点
	bool first_frame = true;
	// render loop
	while (!glfwWindowShouldClose(window))
	{
		// Sleep(100);
		// 由于每一帧的数据传递不一样，需要把数据传递放到渲染循环中
		int cur_amount = frames_amount[cur_frame];
		glm::mat4* cur_modelMatrices = frames_modelMatrices[cur_frame];
		glm::vec3* cur_colors = frames_colors[cur_frame];
		// 定义新的buffer_pos
		glBindBuffer(GL_ARRAY_BUFFER, buffer_pos);
		glBufferData(GL_ARRAY_BUFFER, cur_amount * sizeof(glm::mat4), &cur_modelMatrices[0], GL_STATIC_DRAW);
		// 通过顶点属性传递数据，超过vec4的大小，要使用新方式。
		for (unsigned int i = 0; i < ourModel.meshes.size(); ++i) {
			unsigned int VAO = ourModel.meshes[i].VAO;
			glBindVertexArray(VAO);
			GLsizei vec4Size = sizeof(glm::vec4);
			// 位置数据
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
			glEnableVertexAttribArray(4);
			glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size));
			glEnableVertexAttribArray(5);
			glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
			glEnableVertexAttribArray(6);
			glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));
			// 设置绘制一个实例后更新数据
			glVertexAttribDivisor(3, 1);
			glVertexAttribDivisor(4, 1);
			glVertexAttribDivisor(5, 1);
			glVertexAttribDivisor(6, 1);
			glBindVertexArray(0);
		}

		glBindBuffer(GL_ARRAY_BUFFER, buffer_color);
		glBufferData(GL_ARRAY_BUFFER, cur_amount * sizeof(glm::vec3), &cur_colors[0], GL_STATIC_DRAW);
		for (unsigned int i = 0; i < ourModel.meshes.size(); ++i) {
			unsigned int VAO = ourModel.meshes[i].VAO;
			glBindVertexArray(VAO);
			glEnableVertexAttribArray(7);
			glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
			glVertexAttribDivisor(7, 1);
			glBindVertexArray(0);
		}
		// 更新相机位置从不同角度观察重建的效果
		if (first_frame) {
			// first_frame = false;
			// updateCamera(camera);
		}
		//cout << "FPS:" << 1 / deltaTime<< endl;
		// per-frame time logic
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		processInput(window);

		// render
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// don't forget to enable shader before setting uniforms
		
		ourShader.use();
		
		
		// view/projection transformations
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, CAM_NEAR, CAM_FAR);
		glm::mat4 view = camera.GetViewMatrix();
		ourShader.setMat4("projection", projection); 
		ourShader.setMat4("view", view);

		//绘制烟花
		for (unsigned int i = 0; i < ourModel.meshes.size(); i++) {
			glBindVertexArray(ourModel.meshes[i].VAO);
			glDrawElementsInstanced(GL_TRIANGLES, ourModel.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, cur_amount);
			glBindVertexArray(0);
		}
				
		castleShader.use();
		//绘制城堡
		castleShader.setMat4("projection", projection);
		castleShader.setMat4("view", view);
		glm::mat4 model;
		model = glm::translate(model, glm::vec3(-1, -4, 4)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.7f, 0.7f, 0.7f));	// it's a bit too big for our scene, so scale it down
		model = glm::rotate(model, glm::radians(150.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		castleShader.setMat4("model", model);
		castleModel.Draw(castleShader);

		// opencv: 显示重建的当前帧
		// imshow("window", rgbsMat[cur_frame]);
		// glfw:渲染出当前帧
		glfwSwapBuffers(window);
		glfwPollEvents();
		// 是否保存当前帧截图
		if (is_Screenshot) {
			SaveScreenshot(cur_frame + start_idx);
			if (cur_frame + 1 == total_frames) is_Screenshot = !is_Screenshot;
		}
		// 更新当前帧idx
		if (!is_pause) {
			if (++cur_frame == total_frames) cur_frame = 0;
		}	
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
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		is_pause = !is_pause;
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