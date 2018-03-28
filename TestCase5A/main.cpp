#include "Model.h"
#include "CTestData.h"
#include "Camera.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>


#include "BVHAccel.h"
#include "shader.h"

#include <vector>
#include <iostream>
#include <stack>
#include <queue>
#include<time.h>

//for statics
int drawMeshes = 0;
/*	Target:
*		0->cube
*		1->IFC Model
*		2->PLY Model
*		3->Obj
*/
const int testData = 3;
/*
* OBJ DATA
*/
//Texture
//const std::string objPath = "../../scene/Paris/Paris2010_0.obj";
//const std::string objPath = "../../scene/The City/The City.obj";
const std::string objPath = "../../scene/bunny.obj";
//const std::string objPath = "../../scene/Medieval/Medieval_City.obj";
//None Texture
//std::string objPath = "../../scene/serpertine city/serpentine city.obj";

/*
* PLY DATA(Not implement)
*/
std::string plyPath = "../../scene/complete power plant Sorted/";

/*
* IFC DATA
*/
//std::string path = "../../scene/20160124OTC-Conference Center.ifc";
const std::string path = "../../scene/20160125Autodesk_Hospital_Parking Garage_2015.ifc";
//const std::string path = "../../scene/20160125WestRiverSide Hospital-Ifc2x3-Autodesk_Hospital_Sprinkle_2015.ifc";
//const std::string path = "../../scene/0912107-01stair_geometry_ben_1.ifc";

const int testCubeSeaWidth = 1000;
const int testCubeSeaHeight = 100;

const float companionWindowVertices[] = {
	//2 position,2 texture
	-1, -1, 0, 0, //0
	0, -1, 1, 0, //1
	-1, 1, 0, 1, //2
	0, 1, 1, 1, //3
	0, -1, 0, 0, //4
	1, -1, 1, 0, //5
	0, 1, 0, 1, //6
	1, 1, 1, 1 //7
};
const unsigned int companionWindowIndex[] = { 0, 1, 3, 0, 3, 2, 4, 5, 7, 4, 7, 6 };
const GLuint boundIndex[36] = {
	0, 1, 2, 0, 2, 3, 0, 5, 1, 0, 6, 5, 0, 3, 7, 0, 7, 6, 4, 5, 6, 4, 6, 7, 4, 2, 3, 4, 3, 7, 4, 5, 1, 4, 1, 2
};

const int renderWidth = 512;
const int renderHeight = 512;
const int companionWindowWidth = renderWidth*2;
const int companionWindowHeight = renderHeight;

int frameID = 0;
GLFWwindow* window = NULL;
GLuint cubeVAO = 0, cubeVBO = 0, cubeFBOForInstanceDrawing = 0, drawingLeftFbo = 0,
drawingRightFbo = 0;
GLuint companionWindowVAO = 0, companionWindowVBO = 0, companionWindowIBO = 0;
Shader* companionShader = NULL;
Shader* gBoundsShader = NULL;
Shader* gDrawingWithPhoneShader = NULL;

GLuint cubeTexLeft = 0, cubeTexRight = 0;
Model* model;
BVHAccel* bvhAccel;
std::vector<Mesh*> Objects;

//fbo
GLuint gFBO;
GLuint gCompanionTexture;
GLuint gSplitFBO;
GLuint gQuadVAO;

ofstream out;

// camera
glm::vec3 cameraPos = glm::vec3(0, 0.1, 0.5);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
Camera camera = Camera(cameraPos);
glm::mat4 leftViewMatrix;
glm::mat4 leftProjectionMatrix;
glm::mat4 rightViewMatrix;
glm::mat4 rightProjectionMatrix;
glm::mat4 leftHMDMatrix;
glm::mat4 rightHMDMatrix;

bool firstMouse = true;
float lastX = 512.0f / 2.0;
float lastY = 512.0f / 2.0;
// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;


void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
							const GLchar* message, const void* userParam);

void mouse_callback(GLFWwindow* window, double xpos, double ypos);

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

ofstream initOutput();

bool setUpDll() {
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	// glfw window creation
	// --------------------
	window = glfwCreateWindow(companionWindowWidth, companionWindowHeight, "LearnOpenVR-Cube", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//init glew
	glewExperimental = GL_TRUE;
	GLenum nGlewError = glewInit();
	if (nGlewError != GLEW_OK) {
		printf("%s - Error initializing GLEW! %s\n", __FUNCTION__, glewGetErrorString(nGlewError));
		return false;
	}

	GLint flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // makes sure errors are displayed synchronously
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_HIGH, 0, NULL, GL_TRUE);
	} else
		return false;

	GLint bitsSupported;
	// check to make sure functionality is supported
	glGetQueryiv(GL_SAMPLES_PASSED, GL_QUERY_COUNTER_BITS, &bitsSupported);
	if (bitsSupported == 0) {
		throw "ARB_QUERY doesn't surpport";
	}

	return true;
}

void setupShaders() {
	//BOUNDS
	gBoundsShader = new Shader("shaders/bounding.vs", "shaders/bounding.fs");
	//
	gDrawingWithPhoneShader = new Shader("shaders/cubeNormalDrawingWithPhone.vs",
											"shaders/cubeNormalDrawingWithPhone.fs");
	companionShader = new Shader("shaders/companion.vs", "shaders/companion.fs");
}

bool setUpBuffer() {
	glGenVertexArrays(1, &companionWindowVAO);
	glGenBuffers(1, &companionWindowVBO);
	glBindVertexArray(companionWindowVAO);
	glBindBuffer(GL_ARRAY_BUFFER, companionWindowVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(companionWindowVertices), &companionWindowVertices[0], GL_STATIC_DRAW);
	glGenBuffers(1, &companionWindowIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, companionWindowIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(companionWindowIndex), &companionWindowIndex[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (companionWindowVAO == 0 || companionWindowVBO == 0)
		return false;

	glGenFramebuffers(1, &drawingLeftFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, drawingLeftFbo);

	glGenTextures(1, &cubeTexLeft);
	glBindTexture(GL_TEXTURE_2D, cubeTexLeft);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, renderWidth, renderHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cubeTexLeft, 0);

	unsigned int rboLeft;
	glGenRenderbuffers(1, &rboLeft);
	glBindRenderbuffer(GL_RENDERBUFFER, rboLeft);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, renderWidth, renderHeight);
	// use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboLeft);
	// now actually attach it

	glGenFramebuffers(1, &drawingRightFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, drawingRightFbo);

	glGenTextures(1, &cubeTexRight);
	glBindTexture(GL_TEXTURE_2D, cubeTexRight);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, renderWidth, renderHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cubeTexRight, 0);

	unsigned int rboRight;
	glGenRenderbuffers(1, &rboRight);
	glBindRenderbuffer(GL_RENDERBUFFER, rboRight);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, renderWidth, renderHeight);
	// use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboRight);
	// now actually attach it

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throw "frameBuffer init failed";
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

void handleInput(GLFWwindow* window) {

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

void drawNode(LinearBVHNode* curNode, bool vIsLeft) {

	for (int i = curNode->objectOffset; i < curNode->objectOffset + curNode->nObject; i++) {
		gDrawingWithPhoneShader->use();
		gDrawingWithPhoneShader->setMat4("uProjectionMatrix", vIsLeft?leftProjectionMatrix:rightProjectionMatrix);
		gDrawingWithPhoneShader->setMat4("uViewMatrix", vIsLeft ? leftViewMatrix : rightViewMatrix);
		gDrawingWithPhoneShader->setMat4("uHMDMatrix", vIsLeft ? leftHMDMatrix : rightHMDMatrix);

		gDrawingWithPhoneShader->setVec3("viewPos", camera.Position);

		Objects[i]->Draw(gDrawingWithPhoneShader);
		glUseProgram(0);

		drawMeshes++;
	}

}

bool resultAvailable(LinearBVHNode* node) {
	GLint available;
	glGetQueryObjectivARB(node->queryID, GL_QUERY_RESULT_AVAILABLE_ARB, &available);
	//	glGetOcclusionQueryuivNV(node->queryID, GL_PIXEL_COUNT_AVAILABLE_NV,&available);
	return available;
}

GLuint getOcculusionResult(LinearBVHNode* curNode) {
	GLuint sampleCount;
	glGetQueryObjectuivARB(curNode->queryID, GL_QUERY_RESULT_ARB, &sampleCount);
	//	glGetOcclusionQueryuivNV(curNode->queryID, GL_PIXEL_COUNT_NV, &sampleCount);

	return sampleCount;
}

void pullUpVisibility(LinearBVHNode* root, LinearBVHNode* curNode) {
	while (!curNode->visible) {
		curNode->visible = true;
		curNode = &root[curNode->parentOffset];
	}
}

void traversalNode(LinearBVHNode* curNode, int& curVisitOffset, stack<int>& toVisit) {
	if (curNode->isLeaf) {
		drawNode(curNode,true);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, drawingRightFbo);
		drawNode(curNode,false);
		glBindFramebuffer(GL_FRAMEBUFFER, drawingLeftFbo);

	}
		
	else {
		glm::vec3 EyeToCentroid = curNode->bounds.getCentroid() - camera.Position;
		int DirisNeg[3]{ EyeToCentroid.x < 0, EyeToCentroid.y < 0, EyeToCentroid.z < 0 };
		if (DirisNeg[curNode->axis]) {
			toVisit.push(curVisitOffset + 1);
			curVisitOffset = curNode->secondChildOffset;
		} else {
			toVisit.push(curNode->secondChildOffset);
			++curVisitOffset;
		}
	}
}

void drawBound(LinearBVHNode* vCurNode, bool isLeft) {
	gBoundsShader->use();
	gBoundsShader->setMat4("matrix", isLeft ? leftProjectionMatrix*leftViewMatrix*leftHMDMatrix 
						   : rightProjectionMatrix*rightViewMatrix*rightHMDMatrix);
	GLuint VBO, EBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vCurNode->boundVertices), &vCurNode->boundVertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(boundIndex), &boundIndex[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteVertexArrays(1, &VAO);
}

void issueOcculusionQuery(LinearBVHNode* vCurNode, bool isLeft) {
	glDepthMask(GL_FALSE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	//glGenOcclusionQueriesNV(1,&(vCurNode->queryID));
	//glBeginOcclusionQueryNV(vCurNode->queryID);
	glGenQueriesARB(1, &(vCurNode->queryID));
	glBeginQueryARB(GL_SAMPLES_PASSED_ARB, vCurNode->queryID);

	drawBound(vCurNode, isLeft);

	glEndQueryARB(GL_SAMPLES_PASSED_ARB);
	//glEndOcclusionQueryNV();
	glDepthMask(GL_TRUE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

}

void initFrame(GLuint vFrameBuffer) {
	glBindFramebuffer(GL_FRAMEBUFFER, vFrameBuffer);
	glViewport(0, 0, renderWidth, renderHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
}

void renderWithOptimizeCHC() {
	out << "=================Frame" << frameID << "================" << "\n";

	drawMeshes = 0;
	//1.main
	//cout << "start rendering" << endl;
	initFrame(drawingLeftFbo);
	initFrame(drawingRightFbo);

	glBindFramebuffer(GL_FRAMEBUFFER, drawingLeftFbo);


	leftProjectionMatrix = glm::perspective(glm::radians(camera.Zoom), (float)renderWidth / (float)renderHeight,
										0.1f, 1000000.0f);
	leftViewMatrix = camera.GetViewMatrix();
	//leftHMDMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 10.0f));
	leftHMDMatrix = glm::mat4(1.0f);


	rightProjectionMatrix = glm::perspective(glm::radians(camera.Zoom), (float)renderWidth / (float)renderHeight,
											0.1f, 1000000.0f);
	rightViewMatrix = camera.GetViewMatrix();
	//rightHMDMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 10.0f));
	rightHMDMatrix = glm::mat4(1.0f);


	Objects = bvhAccel->getOrderedMesh();
	//std::priority_queue<LinearBVHNode*> traversalPriorityQueue;
	std::queue<LinearBVHNode*> queryQueue; //for occlusion query
	std::queue<LinearBVHNode*> queryQueueForNextFrame; //for occlusion query
	LinearBVHNode* curNode;
	int curVisitOffset = 0; //for travel in front-back order
	std::stack<int> toVisit;
	//travel
	LinearBVHNode* root = bvhAccel->getLinearNodes();
	bool doneTravel = false;
	while (true) {
		//cout << curVisitOffset << endl;
		//PART 1:Process finished queries
		while (!queryQueue.empty()
			   || doneTravel == true) {
			if (!queryQueue.empty()
				&& resultAvailable(queryQueue.front())) {
				//previous invisible is ready
				curNode = queryQueue.front();
				queryQueue.pop();
				if (getOcculusionResult(curNode) > 1) {
					traversalNode(curNode, curVisitOffset, toVisit);
					pullUpVisibility(root, curNode);
					glDeleteQueriesARB(1, &curNode->queryID);
				} else {
					issueOcculusionQuery(curNode, false);
					if (getOcculusionResult(curNode) > 1) {
						traversalNode(curNode, curVisitOffset, toVisit);
						pullUpVisibility(root, curNode);
						glDeleteQueriesARB(1, &curNode->queryID);
					}
					else {
						if (toVisit.empty() && curVisitOffset == 0) {
							doneTravel = true;
							break;
						} else if (toVisit.empty() && !curNode->isLeaf) {
							//the last invisible node
							doneTravel = true;
							break;
						} else if (!curNode->isLeaf) {
							//invisible node
							curVisitOffset = toVisit.top();
							toVisit.pop();
						}
					}
				}
			} else if (!queryQueueForNextFrame.empty()
					   && resultAvailable(queryQueueForNextFrame.front())) {
				//previous visible is ready
				curNode = queryQueueForNextFrame.front();
				queryQueueForNextFrame.pop();
				if (getOcculusionResult(curNode) > 1) {
					pullUpVisibility(root, curNode);
					glDeleteQueriesARB(1, &curNode->queryID);
				}
				else {
					issueOcculusionQuery(curNode, false);
					if (getOcculusionResult(curNode) > 1) {
						pullUpVisibility(root, curNode);
						glDeleteQueriesARB(1, &curNode->queryID);
					}
				}
			}
			if (queryQueue.empty() && queryQueueForNextFrame.empty()) break;
		}


		if (queryQueue.empty() && queryQueueForNextFrame.empty() && doneTravel) {
			break;
		}
		//PART 2:traversal
		curNode = &root[curVisitOffset];
		bool wasVisible = curNode->visible && (curNode->lastVisited == frameID - 1);
		bool opened = wasVisible && !curNode->isLeaf;
		curNode->visible = false;
		curNode->lastVisited = frameID;
		//===================version1========================
		if (!opened) {
			issueOcculusionQuery(curNode,true);
			wasVisible ? queryQueueForNextFrame.push(curNode) : queryQueue.push(curNode);
		}
		if (wasVisible)
			traversalNode(curNode, curVisitOffset, toVisit);

		if (curNode->isLeaf) {
			if (!toVisit.empty()) {
				curVisitOffset = toVisit.top();
				toVisit.pop();
			} else {
				doneTravel = true;
				curVisitOffset = 0;
			}
		}
	}
	++frameID; //count the frames
	glBindVertexArray(0);
	glUseProgram(0);

	out << "DrawCalls(/Meshes):"<< drawMeshes/2 <<"\n";

}

void renderCompanionWindow() {
	glViewport(0, 0, companionWindowWidth, companionWindowHeight);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	companionShader->use();
	companionShader->setInt("mytexture", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cubeTexLeft);
	glBindVertexArray(companionWindowVAO);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glDrawElements(GL_TRIANGLES, sizeof(companionWindowIndex) / 6, GL_UNSIGNED_INT, 0);
	glBindTexture(GL_TEXTURE_2D, cubeTexRight);
	glDrawElements(GL_TRIANGLES, sizeof(companionWindowIndex) / 6, GL_UNSIGNED_INT
				   , (const void *)(uintptr_t)(sizeof(companionWindowIndex) / 2));
	glBindVertexArray(0);
	glUseProgram(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Render() {
	while (!glfwWindowShouldClose(window)) {
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		handleInput(window);

		renderWithOptimizeCHC();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
			
		renderCompanionWindow();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void initTestData() {
	cout << "Target:" << testData << endl;
	switch (testData) {
		case 0:
		{
			CTestData testData(3);
			model = new Model(testData.getModel());
			break;
		}
		case 1:
		{
			//ifc
			model = new Model(path, false);
			break;
		}
		case 2:
		{
			model = new Model(plyPath + "0.ply", false);
			for (int i = 0; i < 4; ++i) {
				model->loadModel(plyPath + to_string(i) + ".ply");
			}
			break;
		}
		case 3:
		{
			model = new Model(objPath, false);
			break;
		}
		default:
			break;
	}
	std::cout << "read successful" << std::endl;

	//build tree
	bvhAccel = new BVHAccel(model->meshes, 500, SAH);
	std::cout << "build successful" << std::endl;

	out << "=================Properties================\n";
	out << "Total Meshes:" << model->meshes.size() << "\n";
	out << "Total Triangles:" << model->totalTriangles << "\n";
	out << "Total Nodes for BVHAccel:" << bvhAccel->totalLinearNodes << "\n";
	out << "===========================================\n";

}

int main() {
	out = initOutput();
	if (!setUpDll()) {
		std::cout << "init dll error" << std::endl;
		return 0;
	}
	initTestData();
	setUpBuffer();
	setupShaders();
	//fill the test data
	


	Render();

	out.close();
	return 0;
}

void APIENTRY glDebugOutput(GLenum source,
							GLenum type,
							GLuint id,
							GLenum severity,
							GLsizei length,
							const GLchar* message,
							const void* userParam) {
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return; // ignore these non-significant error codes

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source) {
		case GL_DEBUG_SOURCE_API: std::cout << "Source: API";
			break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: std::cout << "Source: Window System";
			break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler";
			break;
		case GL_DEBUG_SOURCE_THIRD_PARTY: std::cout << "Source: Third Party";
			break;
		case GL_DEBUG_SOURCE_APPLICATION: std::cout << "Source: Application";
			break;
		case GL_DEBUG_SOURCE_OTHER: std::cout << "Source: Other";
			break;
	}
	std::cout << std::endl;

	switch (type) {
		case GL_DEBUG_TYPE_ERROR: std::cout << "Type: Error";
			break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour";
			break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: std::cout << "Type: Undefined Behaviour";
			break;
		case GL_DEBUG_TYPE_PORTABILITY: std::cout << "Type: Portability";
			break;
		case GL_DEBUG_TYPE_PERFORMANCE: std::cout << "Type: Performance";
			break;
		case GL_DEBUG_TYPE_MARKER: std::cout << "Type: Marker";
			break;
		case GL_DEBUG_TYPE_PUSH_GROUP: std::cout << "Type: Push Group";
			break;
		case GL_DEBUG_TYPE_POP_GROUP: std::cout << "Type: Pop Group";
			break;
		case GL_DEBUG_TYPE_OTHER: std::cout << "Type: Other";
			break;
	}
	std::cout << std::endl;

	switch (severity) {
		case GL_DEBUG_SEVERITY_HIGH: std::cout << "Severity: high";
			break;
		case GL_DEBUG_SEVERITY_MEDIUM: std::cout << "Severity: medium";
			break;
		case GL_DEBUG_SEVERITY_LOW: std::cout << "Severity: low";
			break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification";
			break;
	}
	std::cout << std::endl;
	std::cout << std::endl;
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(yoffset);
}

ofstream initOutput() {
	ofstream out;
	out.open("log.txt", ios::out|ios::trunc);
	if (!out.is_open()) {
		cout << "Open log File Wrong" << endl;
	}
	time_t now;
	time(&now);
	struct tm tmTmp;
	char stTmp[32];
	localtime_s(&tmTmp, &now);
	asctime_s(stTmp, &tmTmp);
	out << stTmp << endl;
	return out;
}
