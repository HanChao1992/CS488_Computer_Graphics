#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include <glm/glm.hpp>

#include <vector>

// Set a global maximum number of vertices in order to pre-allocate VBO data
// in one shot, rather than reallocating each frame.
const GLsizei kMaxVertices = 1000;


// Convenience class for storing vertex data in CPU memory.
// Data should be copied over to GPU memory via VBO storage before rendering.
class VertexData {
public:
	VertexData();

	std::vector<glm::vec2> positions;
	std::vector<glm::vec3> colours;
	GLuint index;
	GLsizei numVertices;
};


class A2 : public CS488Window {
public:
	A2();
	virtual ~A2();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

	void createShaderProgram();
	void enableVertexAttribIndices();
	void generateVertexBuffers();
	void mapVboDataToVertexAttributeLocation();
	void uploadVertexDataToVbos();

	void initLineData();

	void setLineColour(const glm::vec3 & colour);

	void drawLine (
			const glm::vec2 & v0,
			const glm::vec2 & v1
	);

	glm::mat4 rotation(double angle, char axis);
	glm::mat4 translation(glm::vec3 tran);
	glm::mat4 scaling(glm::vec3 scale);
	void reset();
	void drawViewport();
	void drawMCoord();
	void drawWCoord();
	void drawCube();
	void clipping(glm::vec4 A, glm::vec4 B, glm::vec4 P[], glm::vec4 n[]);
	void perspective(float &fov, float aspect, float near, float far);
  void setViewPort();
	void setPlanes();
	ShaderProgram m_shader;


	glm::mat4 projMat; // Project Matrix
	glm::mat4 viewMat; // viewing Matrix
	glm::mat4 modelMat; // Modelling Matrix
	glm::mat4 scalingMat; // Scaling Matrix
	glm::vec4 viewPort[3]; // Viewport data
	glm::vec4 myCube[8]; // Cube data
	glm::vec4 myCubeDraw[8];
	glm::vec4 myWCoord[4]; // World Coordinates data
	glm::vec4 myMCoord[4]; // Model Coordinates data
	glm::vec4 planes[6]; // Store planes
	glm::vec4 norms[6]; // Store nomrs

	GLuint m_vao;            // Vertex Array Object
	GLuint m_vbo_positions;  // Vertex Buffer Object
	GLuint m_vbo_colours;    // Vertex Buffer Object

	VertexData m_vertexData;

	// modes
	enum modes {
		rotateView,
		transView,
		perspect,
		rotateModel,
		transModel,
		scaleModel,
		vp
	};

	glm::vec4 viewPortBegin = glm::vec4(-0.9f, -0.9f, 0 ,1);
	glm::vec4 viewPortBeginHelp = glm::vec4(-0.9f, -0.9f, 0 ,1);
	float viewPortWidth = 0.0f;
	float viewPortHeight = 0.0f;
	float factor;
	float aspect;
	float fov;
	float preMousexPos;
	float curMousexPos;
	float preMouseyPos;
	float curMouseyPos;
	bool initial = true;
	bool mouseL = false;
	bool mouseM = false;
	bool mouseR = false;
	float nearP;
	float farP;
	int mode;

	glm::vec3 m_currentLineColour;

};
