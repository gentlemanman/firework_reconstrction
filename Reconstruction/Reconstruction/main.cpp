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
//Զ���ü���
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
	Shader insertShader("insert.vs", "insert.fs");

	// load models
	// -----------
	Model ourModel("obj/ball/3.obj");//ģ��
	// ��ȡλ�ú���ɫ��Ϣ
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, CAM_NEAR, CAM_FAR);
	glm::mat4 inverse_projection = glm::inverse(projection);
	const int total_frames = 1; // ��֡��
	const int start_idx = 170; // �ؽ�֡�Ŀ�ʼ	
	vector<vector<point_3D>> frames_points;
	vector<Mat> rgbsMat;
	vector<ProcessPoints> frames_pps;
	
	for (int i = start_idx; i < total_frames + start_idx; ++i) {
		vector<glm::vec3> m_pos;
		vector<glm::vec3> m_color;
		get_3D_param(inverse_projection, i, m_pos, m_color);// ��ȡ��ǰ֡�ĵ����Ϣ
		ProcessPoints pps(m_pos, m_color);
		frames_pps.push_back(pps);
		// ��ȡ��ǰ֡
		Mat rgbMat = get_rgbMat(i);
		rgbsMat.push_back(rgbMat);
	}
	//��ȡ��ǰ֡�Ĳ�ֵ��
	vector<vector<glm::vec3>> insert_pos;
	vector<vector<glm::vec3>> insert_color;
	for (int i = 0; i < total_frames; ++i) {
		vector<vector<int>> insert_idx = frames_pps[i].insert_idx(); // ��ȡ��ǰ֡�Ĳ�ֵ������
		vector<glm::vec3> frame_pos = frames_pps[i].frame_pos();
		vector<glm::vec3> frame_color = frames_pps[i].frame_color();
		//��ȡ��ֵ��λ��
		vector<glm::vec3> frame_insert_pos = Interpolation(insert_idx, frame_pos);
		insert_pos.push_back(frame_insert_pos);
		//��ȡ��ֵ����ɫ
		vector<glm::vec3> frame_insert_color = Interpolation(insert_idx, frame_color);
		insert_color.push_back(frame_insert_color);
	}

	///����������д��
	// ��ȡ��һ֡��ԭʼ����
	ProcessPoints pps0 = frames_pps[0];
	vector<glm::vec3> pos0 = pps0.frame_pos(); // ��ȡ��ǰ֡ȫ��λ��
	vector<glm::vec3> color0 = pps0.frame_color(); // ��ȡ��ǰ֡ȫ����ɫ
	// ��ȡ��һ֡��ֵ����
	vector<glm::vec3> insert_pos0 = insert_pos[0];
	vector<glm::vec3> insert_color0 = insert_color[0];
	insert_pos0.clear();
	// �ܹ���Ҫʵ����������
	int amount = pos0.size() + insert_pos0.size();
	glm::mat4* modelMatrices = new glm::mat4[amount];
	glm::vec3* colors = new glm::vec3[amount];
	// ԭʼ����
	for (int i = 0; i < pos0.size(); i++) {
		// λ����Ϣ
		glm::mat4 tmp_model;
		tmp_model = glm::translate(tmp_model, pos0[i]);
		tmp_model = glm::scale(tmp_model, glm::vec3(0.15f, 0.15f, 0.15f));
		modelMatrices[i] = tmp_model;
		// ��ɫ��Ϣ
		colors[i] = color0[i];
	}
	// ��ֵ����
	for (int i = pos0.size(); i < amount; ++i) {
		int j = i - pos0.size();
		// ��ֵλ����Ϣ
		glm::mat4 tmp_model;
		tmp_model = glm::translate(tmp_model, insert_pos0[j]);
		tmp_model = glm::scale(tmp_model, glm::vec3(0.15f, 0.15f, 0.15f));
		modelMatrices[i] = tmp_model;
		// ��ֵ��ɫ��Ϣ
		colors[i] = insert_color0[j];
	}

	// �����µ�buffer_pos
	unsigned int buffer_pos;
	glGenBuffers(1, &buffer_pos);
	glBindBuffer(GL_ARRAY_BUFFER, buffer_pos);
	glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);
	// ͨ���������Դ������ݣ�����vec4�Ĵ�С��Ҫʹ���·�ʽ��
	for (unsigned int i = 0; i < ourModel.meshes.size(); ++i) {
		unsigned int VAO = ourModel.meshes[i].VAO;
		glBindVertexArray(VAO);
		GLsizei vec4Size = sizeof(glm::vec4);
		// λ������
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));
		// ���û���һ��ʵ�����������
		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
		glBindVertexArray(0);
	}

	unsigned int buffer_color;
	glGenBuffers(1, &buffer_color);
	glBindBuffer(GL_ARRAY_BUFFER, buffer_color);
	glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::vec3), &colors[0], GL_STATIC_DRAW);
	for (unsigned int i = 0; i < ourModel.meshes.size(); ++i) {
		unsigned int VAO = ourModel.meshes[i].VAO;
		glBindVertexArray(VAO);
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
		glVertexAttribDivisor(7, 1);
		glBindVertexArray(0);
	}

	cout << "amount:" << amount << endl;
	cout << "meshes size:" << ourModel.meshes.size() << endl;
	int num_frame = 0; // ��ǰ֡��������ʾÿһ֡�͵�ǰ֡�����е�
	// render loop
	while (!glfwWindowShouldClose(window))
	{
		//cout << "FPS:" << 1 / deltaTime<< endl;
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
		//����
		for (unsigned int i = 0; i < ourModel.meshes.size(); i++) {
			glBindVertexArray(ourModel.meshes[i].VAO);
			glDrawElementsInstanced(GL_TRIANGLES, ourModel.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, amount);
			glBindVertexArray(0);
		}		
		
		// opencv: ��ʾ�ؽ��ĵ�ǰ֡
		imshow("window", rgbsMat[num_frame]);
		// glfw:��Ⱦ����ǰ֡
		glfwSwapBuffers(window);
		glfwPollEvents();
		// �������λ�ôӲ�ͬ�Ƕȹ۲��ؽ���Ч��
		// updateCamera(camera);
		// ���µ�ǰ֡idx
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