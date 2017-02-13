#include "A2.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>
using namespace std;

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
using namespace glm;



#define PI 3.1415926535897

//----------------------------------------------------------------------------------------
// Constructor
VertexData::VertexData()
	: numVertices(0),
	  index(0)
{
	positions.resize(kMaxVertices);
	colours.resize(kMaxVertices);
}


//----------------------------------------------------------------------------------------
// Constructor
A2::A2()
	: m_currentLineColour(vec3(0.0f))
{

}

//----------------------------------------------------------------------------------------
// Destructor
A2::~A2()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A2::init()
{
	// Set the background colour.
	glClearColor(0.3, 0.5, 0.7, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao);

	enableVertexAttribIndices();

	generateVertexBuffers();

	mapVboDataToVertexAttributeLocation();

	reset();
}

/*
 * Reset to initial state
 */
void A2::reset() {
	projMat = mat4();
	viewMat = mat4();
	modelMat = mat4();
	scalingMat = mat4();
	factor = 70.0f;
	fov = 30.0f;
	nearP = 1.0f;
	farP = 30.0f;
	mode  = 3;
	initial = true;
	int winWidth, winHeight;
	viewPortBegin = glm::vec4(-0.9f, -0.9f, 0 ,1);
	viewPortBeginHelp = glm::vec4(-0.9f, -0.9f, 0 ,1);
	glfwGetWindowSize(m_window, &winWidth, &winHeight);
	aspect = winWidth / winHeight;
	viewMat[3] = vec4(0, 0, 10, 1);
	perspective(fov, aspect, nearP, farP);
}
//----------------------------------------------------------------------------------------
void A2::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();
}

//----------------------------------------------------------------------------------------
void A2::enableVertexAttribIndices()
{
	glBindVertexArray(m_vao);

	// Enable the attribute index location for "position" when rendering.
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray(positionAttribLocation);

	// Enable the attribute index location for "colour" when rendering.
	GLint colourAttribLocation = m_shader.getAttribLocation( "colour" );
	glEnableVertexAttribArray(colourAttribLocation);

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A2::generateVertexBuffers()
{
	// Generate a vertex buffer to store line vertex positions
	{
		glGenBuffers(1, &m_vbo_positions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	// Generate a vertex buffer to store line colors
	{
		glGenBuffers(1, &m_vbo_colours);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A2::mapVboDataToVertexAttributeLocation()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao);

	// Tell GL how to map data from the vertex buffer "m_vbo_positions" into the
	// "position" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glVertexAttribPointer(positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_colours" into the
	// "colour" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
	GLint colorAttribLocation = m_shader.getAttribLocation( "colour" );
	glVertexAttribPointer(colorAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

/*
 * Rotation Matrix
 */
mat4 A2::rotation(double angle, char axis) {
	mat4 rotMat;
	float co = cos(angle);
	float si = sin(angle);
	if (axis == 'x') {
		rotMat[0] = vec4(1, 0, 0, 0);
		rotMat[1] = vec4(0, co, -si, 0);
		rotMat[2] = vec4(0, si, co, 0);
		rotMat[3] = vec4(0, 0, 0, 1);
	}
	else if (axis == 'y') {
		rotMat[0] = vec4(co, 0, -si, 0);
		rotMat[1] = vec4(0, 1, 0, 0);
		rotMat[2] = vec4(si, 0, co, 0);
		rotMat[3] = vec4(0, 0, 0, 1);
	}
	if (axis == 'z') {
		rotMat[0] = vec4(co, si, 0, 0);
		rotMat[1] = vec4(-si, co, 0, 0);
		rotMat[2] = vec4(0, 0, 1, 0);
		rotMat[3] = vec4(0, 0, 0, 1);
	}
	return rotMat;
}

/*
 * Translation Matrix
 */
mat4 A2::translation(glm::vec3 tran) {
	mat4 transMat;


	transMat[0] = vec4(1, 0, 0, 0);
	transMat[1] = vec4(0, 1, 0, 0);
	transMat[2] = vec4(0, 0, 1, 0);
	transMat[3] = vec4(tran.x, tran.y, tran.z, 1);

	return transMat;
}

/*
 * Scaling Matrix
 */
mat4 A2::scaling(glm::vec3 scale) {
	mat4 scaleMat;
	scaleMat[0] = vec4(scale.x, 0, 0, 0);
	scaleMat[1] = vec4(0, scale.y, 0, 0);
	scaleMat[2] = vec4(0, 0, scale.z, 0);
	scaleMat[3] = vec4(0, 0, 0, 1);
	return scaleMat;
}

/*
 * Modify Projection Matrix
 */
void A2::perspective(float &fov, float aspect, float near, float far) {
	if (fov < 5) {
		fov = 5;
	}
	else if (fov > 160) {
		fov = 160;
	}
  float nfov = 1.0f / tan (fov * PI / (180.0f * 2.0f));
	float a = (far + near) / (far - near);
	float b = -2 * far * near / (far - near);
	projMat[0] = vec4(nfov/aspect, 0, 0, 0);
	projMat[1] = vec4(0, nfov, 0, 0);
	projMat[2] = vec4(0, 0, a, 1);
	projMat[3] = vec4(0, 0, b, 0);
}

//---------------------------------------------------------------------------------------
void A2::initLineData()
{
	m_vertexData.numVertices = 0;
	m_vertexData.index = 0;
}

//---------------------------------------------------------------------------------------
void A2::setLineColour (
		const glm::vec3 & colour
) {
	m_currentLineColour = colour;
}

//---------------------------------------------------------------------------------------
void A2::drawLine(
		const glm::vec2 & v0,   // Line Start (NDC coordinate)
		const glm::vec2 & v1    // Line End (NDC coordinate)
) {

	m_vertexData.positions[m_vertexData.index] = v0;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;
	m_vertexData.positions[m_vertexData.index] = v1;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;

	m_vertexData.numVertices += 2;
}

/*
 * Set all the planes to be clipped
 */
void A2::setPlanes() {
	if (initial) { // default
		// near
		planes[0] = vec4(0.0f, 0.0f, nearP, 1.0f);
		norms[0] = vec4(0.0f, 0.0f, -1.0f, 0.0f);
		// far
		planes[1] = vec4(0.0f, 0.0f, farP, 1.0f);
		norms[1] = vec4(0.0f, 0.0f, -1.0f, 0.0f);
		// left
		planes[2] = vec4(-0.9f, 0.0f, 0.0f, 1.0f);
		norms[2] = vec4(1.0f, 0.0f, 0.0f, 0.0f);
		// right
		planes[3] = vec4(0.9f, 0.0f, 0.0f, 1.0f);
		norms[3] = vec4(-1.0f, 0.0f, 0.0f, 0.0f);
		// top
		planes[4] = vec4(0.0f, 0.9f, 0.0f, 1.0f);
		norms[4] = vec4(0.0f, -1.0f, 0.0f, 0.0f);
		// bottom
		planes[5] = vec4(0.0f, -0.9f, 0.0f, 1.0f);
		norms[5] = vec4(0.0f, 1.0f, 0.0f, 0.0f);
	}
	else {
		planes[0] = vec4(0.0f, 0.0f, nearP, 1.0f);
		norms[0] = vec4(0.0f, 0.0f, -1.0f, 0.0f);

		planes[1] = vec4(0.0f, 0.0f, farP, 1.0f);
		norms[1] = vec4(0.0f, 0.0f, -1.0f, 0.0f);

		planes[2] = vec4(viewPort[0].x, 0.0f, 0.0f, 1.0f);
		norms[2] = vec4(1.0f, 0.0f, 0.0f, 0.0f);

		planes[3] = vec4(viewPort[2].x, 0.0f, 0.0f, 1.0f);
		norms[3] = vec4(-1.0f, 0.0f, 0.0f, 0.0f);

		planes[4] = vec4(0.0f, viewPort[1].y, 0.0f, 1.0f);
		norms[4] = vec4(0.0f, -1.0f, 0.0f, 0.0f);

		planes[5] = vec4(0.0f, viewPort[3].y, 0.0f, 1.0f);
		norms[5] = vec4(0.0f, 1.0f, 0.0f, 0.0f);
	}
}

/*
 * Set the viewport data
 */
void A2::setViewPort(){
	if (initial) {// default
		viewPort[0] = vec4(-0.9f, -0.9f, 0, 1);
		viewPort[1] = vec4(-0.9f, 0.9f, 0, 1);
		viewPort[2] = vec4(0.9f, 0.9f, 0, 1);
		viewPort[3] = vec4(0.9f, -0.9f, 0, 1);
	}
	else {
	viewPort[0] = viewPortBegin;
	viewPort[1] = vec4(viewPortBegin.x, viewPortBegin.y + viewPortHeight, 0, 1);
	viewPort[2] = vec4(viewPortBegin.x + viewPortWidth, viewPortBegin.y + viewPortHeight, 0, 1);
	viewPort[3] = vec4(viewPortBegin.x + viewPortWidth, viewPortBegin.y, 0, 1);
	}
}

/*
 * Draw the viewport
 */
void A2::drawViewport() {
	setLineColour(vec3(0.0f, 0.0f, 0.0f));

	drawLine(vec2(viewPort[0].x, viewPort[0].y), vec2(viewPort[1].x, viewPort[1].y));
	drawLine(vec2(viewPort[1].x, viewPort[1].y), vec2(viewPort[2].x, viewPort[2].y));
	drawLine(vec2(viewPort[2].x, viewPort[2].y), vec2(viewPort[3].x, viewPort[3].y));
	drawLine(vec2(viewPort[0].x, viewPort[0].y), vec2(viewPort[3].x, viewPort[3].y));
}

/*
 * Draw the model coordinate system
 */
void A2::drawMCoord() {
	myMCoord[0] = vec4(0.25f, 0.0f, 0.0f, 1);
	myMCoord[1] = vec4(0.0f, 0.25f, 0.0f, 1);
	myMCoord[2] = vec4(0.0f, 0.0f, 0.25f, 1);
	myMCoord[3] = vec4(0.0f, 0.0f, 0.0f, 1);

	for (int i = 0; i < 4; i++) {
		myMCoord[i] = projMat * viewMat * modelMat * myMCoord[i];
	}

	setLineColour(vec3(1.0f, 0.0f, 0.0f));
	clipping(myMCoord[3], myMCoord[0], planes, norms);
	clipping(myMCoord[3], myMCoord[1], planes, norms);
	clipping(myMCoord[3], myMCoord[2], planes, norms);
}

/*
 * Draw the world coordinate systems
 */
void A2::drawWCoord() {
	myWCoord[0] = vec4(0.5f, 0.0f, 0.0f, 1);
	myWCoord[1] = vec4(0.0f, 0.5f, 0.0f, 1);
	myWCoord[2] = vec4(0.0f, 0.0f, 0.5f, 1);
	myWCoord[3] = vec4(0.0f, 0.0f, 0.0f, 1);

  for (int i = 0; i < 4; i++) {
		myWCoord[i] = projMat * viewMat * myWCoord[i];
	}

	setLineColour(vec3(0.0f, 1.0f, 0.0f));
	clipping(myWCoord[3], myWCoord[0], planes, norms);
	clipping(myWCoord[3], myWCoord[1], planes, norms);
	clipping(myWCoord[3], myWCoord[2], planes, norms);
}

/*
 * Draw the cube
 */
void A2::drawCube () {
	// cube data
	myCube[0] = (vec4(1.0f, -1.0f, -1.0f, 1));
	myCube[1] = (vec4(-1.0f, -1.0f, -1.0f, 1));
	myCube[2] = (vec4(-1.0f, 1.0f, -1.0f, 1));
	myCube[3] = (vec4(1.0f, 1.0f, -1.0f, 1));
	myCube[4] = (vec4(-1.0f, -1.0f, 1.0f, 1));
	myCube[5] = (vec4(1.0f, -1.0f, 1.0f, 1));
	myCube[6] = (vec4(1.0f, 1.0f, 1.0f, 1));
	myCube[7] = (vec4(-1.0f, 1.0f, 1.0f, 1));

	for (int i = 0; i < 8; i++) {
		myCubeDraw[i] = projMat * viewMat * modelMat * scalingMat * myCube[i];
		//cout << myCubeDraw[i] << endl;
	}

	setLineColour(vec3(1.0f, 1.0f, 1.0f));
	clipping(myCubeDraw[0], myCubeDraw[1], planes, norms);
	clipping(myCubeDraw[0], myCubeDraw[3], planes, norms);
	clipping(myCubeDraw[0], myCubeDraw[5], planes, norms);
	clipping(myCubeDraw[1], myCubeDraw[2], planes, norms);
	clipping(myCubeDraw[1], myCubeDraw[4], planes, norms);
	clipping(myCubeDraw[2], myCubeDraw[3], planes, norms);
	clipping(myCubeDraw[2], myCubeDraw[7], planes, norms);
	clipping(myCubeDraw[3], myCubeDraw[6], planes, norms);
	clipping(myCubeDraw[4], myCubeDraw[5], planes, norms);
	clipping(myCubeDraw[4], myCubeDraw[7], planes, norms);
	clipping(myCubeDraw[5], myCubeDraw[6], planes, norms);
	clipping(myCubeDraw[6], myCubeDraw[7], planes, norms);

}

/*
 * Clipping
 */
void A2::clipping (vec4 A, vec4 B, vec4 P[], vec4 n[]) {
	for (int i = 0; i < 6; i++ ) {
		A = A / A.w;
		B = B / B.w;

		float wecA = dot(A - P[i], n[i]);
		float wecB = dot(B - P[i], n[i]);
		if (wecA < 0.0001 && wecB < 0.0001) {
			return;
		}
	  if (wecA >= 0.0001 && wecB >= 0.0001) {
			continue;
		}
		float t = wecA / (wecA - wecB);
		if (wecA < 0.0001) {
			A = A + t * (B - A);
		}
		else {
			B = A + t * (B - A);
		}
	}

	drawLine(vec2(A.x, A.y), vec2(B.x, B.y));
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A2::appLogic()
{

	// Place per frame, application logic here ..
	initLineData();
	setViewPort();
	setPlanes();
  drawViewport();
	drawMCoord();
	drawWCoord();
	drawCube();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A2::guiLogic()
{
	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);

		// Add more gui elements here here ...
		ImGui::PushID(0);
		if (ImGui::RadioButton("##RotateView", &mode, 0)) {

		}
		ImGui::PopID();
		ImGui::SameLine();
		ImGui::Text("Rotate View [O]");
		ImGui::PushID(1);
		if (ImGui::RadioButton("##TranslateView", &mode, 1)) {

		}
		ImGui::PopID();
		ImGui::SameLine();
		ImGui::Text("Translate View [N]");

		ImGui::PushID(2);
		if (ImGui::RadioButton("##Perspective", &mode, 2)) {

		}
		ImGui::PopID();
		ImGui::SameLine();
		ImGui::Text("Perspective [P]");

		ImGui::PushID(3);
		if (ImGui::RadioButton("##RotateModel", &mode, 3)) {

		}
		ImGui::PopID();
		ImGui::SameLine();
		ImGui::Text("Rotate Model [R]");

		ImGui::PushID(4);
		if (ImGui::RadioButton("##TranslateModel", &mode, 4)) {

		}
		ImGui::PopID();
		ImGui::SameLine();
		ImGui::Text("Translate Model [T]");

		ImGui::PushID(5);
		if (ImGui::RadioButton("##ScaleModel", &mode, 5)) {

		}
		ImGui::PopID();
		ImGui::SameLine();
		ImGui::Text("Scale Model [S]");

		ImGui::PushID(6);
		if (ImGui::RadioButton("##Viewport", &mode, 6)) {

		}
		ImGui::PopID();
		ImGui::SameLine();
		ImGui::Text("Viewport [V]");

		ImGui::Text("Near Plane: %f", nearP);
		ImGui::Text("Far Plane: %f", farP);

		// Create Button, and check if it was clicked:
		if( ImGui::Button( "Reset" ) ) {
			reset();
		}

		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();
}

//----------------------------------------------------------------------------------------
void A2::uploadVertexDataToVbos() {

	//-- Copy vertex position data into VBO, m_vbo_positions:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2) * m_vertexData.numVertices,
				m_vertexData.positions.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	//-- Copy vertex colour data into VBO, m_vbo_colours:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * m_vertexData.numVertices,
				m_vertexData.colours.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A2::draw()
{
	uploadVertexDataToVbos();

	glBindVertexArray(m_vao);

	m_shader.enable();
		glDrawArrays(GL_LINES, 0, m_vertexData.numVertices);
	m_shader.disable();

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A2::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A2::cursorEnterWindowEvent (
		int entered
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A2::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if (mouseL || mouseR || mouseM) {
		curMousexPos = preMousexPos;
		curMouseyPos = preMouseyPos;
		preMouseyPos = yPos;
		preMousexPos = xPos;
		float mousexDiff = preMousexPos - curMousexPos;
		float mouseyDiff = preMouseyPos - curMouseyPos;
		switch (mode) {
			case rotateView:
				if (mouseL) {
					viewMat *= rotation(mousexDiff / factor,  'x');
				}
				if (mouseM) {
					viewMat *= rotation(mousexDiff / factor, 'y');
				}
				if (mouseR) {
					viewMat *= rotation(mousexDiff / factor, 'z');
				}
				break;
			case transView:
				if (mouseL) {
					viewMat *= translation(vec3(mousexDiff / factor , 0, 0));
				}
				if (mouseM) {
					viewMat *= translation(vec3(0, mousexDiff / factor, 0));
				}
				if (mouseR) {
					viewMat *= translation(vec3(0, 0, mousexDiff / factor));
				}
				break;
			case perspect:
				if (mouseL) {
					fov = fov - mousexDiff / factor;
					perspective(fov, aspect, nearP, farP);
				}
				if (mouseM) {
					nearP = nearP - mousexDiff / factor;
					perspective(fov, aspect, nearP, farP);
				}
				if (mouseR) {
					farP = farP - mousexDiff / factor;
					perspective(fov, aspect, nearP, farP);
				}
				break;
			case rotateModel:
				if (mouseL) {
					modelMat *= rotation(mousexDiff / factor,  'x');
				}
				if (mouseM) {
					modelMat *= rotation(mousexDiff / factor,  'y');
				}
				if (mouseR) {
					modelMat *= rotation(mousexDiff / factor,  'z');
				}
				break;
			case transModel:
				if (mouseL) {
					modelMat *= translation(vec3(mousexDiff / factor, 0, 0));
				}
				if (mouseM) {
					modelMat *= translation(vec3(0, mousexDiff / factor, 0));
				}
				if (mouseR) {
					modelMat *= translation(vec3(0, 0, mousexDiff / factor));
				}
				break;
			case scaleModel:
				if (mouseL) {
					scalingMat *= scaling(vec3(1 + mousexDiff / factor, 1, 1));
				}
				if (mouseM) {
					scalingMat *= scaling(vec3(1, 1+ mousexDiff / factor, 1));
				}
				if (mouseR) {
					scalingMat *= scaling(vec3(1, 1, 1+ mousexDiff / factor));
				}
				break;
		  case vp:
				// Check boundries
				if (curMousexPos > (1.95 * (m_windowWidth / 2))) {
					curMousexPos = (1.95 * (m_windowWidth / 2));
				}
				if (curMousexPos < (0.05 * (m_windowWidth / 2))) {
					curMousexPos = (0.05 * (m_windowWidth / 2));
				}
				if (curMouseyPos > (1.95 * (m_windowWidth / 2))) {
					curMouseyPos = (1.95 * (m_windowWidth / 2));
				}
				if (curMouseyPos < (0.05 * (m_windowWidth / 2))) {
					curMouseyPos = (0.05 * (m_windowWidth / 2));
				}

				viewPortWidth = curMousexPos - viewPortBeginHelp.x;

				viewPortHeight = curMouseyPos - viewPortBeginHelp.y;

				aspect = viewPortHeight/viewPortWidth;
				if (aspect < 0) {
					aspect *= -1;
				}
				// Check boundries
				viewPortBegin.x = (viewPortBeginHelp.x - (m_windowWidth / 2) ) / (m_windowWidth / 2);
				if (viewPortBegin.x > 0.95) {
					viewPortBegin.x = 0.95;
				}
				if (viewPortBegin.x < -0.95) {
					viewPortBegin.x = -0.95;
				}
				if (viewPortBegin.y > 0.95) {
					viewPortBegin.y = 0.95;
				}
				if (viewPortBegin.y < -0.95) {
					viewPortBegin.y = -0.95;
				}
				viewPortBegin.y = ((m_windowHeight / 2) - viewPortBeginHelp.y) / (m_windowHeight / 2);
				viewPortWidth = viewPortWidth/m_windowWidth * 2;
				viewPortHeight = -viewPortHeight/m_windowHeight * 2;
				perspective(fov, aspect, nearP, farP);
				break;
			}


	return eventHandled;
}}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
 bool A2::mouseButtonInputEvent (
 		int button,
 		int actions,
 		int mods
 ) {
 	bool eventHandled(false);

 	if (!ImGui::IsMouseHoveringAnyWindow()) {
 		if (actions == GLFW_PRESS) {
 			double xpos, ypos;
 			glfwGetCursorPos(m_window, &xpos, &ypos);
 			preMousexPos = xpos;
			preMouseyPos = ypos;
 			if (button == GLFW_MOUSE_BUTTON_LEFT) {
				mouseL = true;
				if (mode == vp) {
					initial = false;
					viewPortWidth = 0.0f;
					viewPortHeight = 0.0f;
					if (xpos )
					viewPortBeginHelp = vec4(xpos, ypos, 0.0f, 1.0f);
				}
			}
			if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
				mouseM = true;
			}
 			if (button == GLFW_MOUSE_BUTTON_RIGHT) {
				mouseR = true;
 		 	}
	 	}
 		if (actions == GLFW_RELEASE) {
 			if (button == GLFW_MOUSE_BUTTON_LEFT) {
 				mouseL = false;

 			}
 			if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
				mouseM = false;
			}
 			if (button == GLFW_MOUSE_BUTTON_RIGHT) {
				mouseR = false;
			}
 		}
	}

 	eventHandled = true;

 	return eventHandled;
 }

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A2::mouseScrollEvent (
		double xOffSet,
		double yOffSet
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A2::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A2::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if( action == GLFW_PRESS ) {
		// Respond to some key events.
		switch (key) {
			case GLFW_KEY_Q:
				glfwSetWindowShouldClose(m_window, GL_TRUE);
				break;
			case GLFW_KEY_A:
			  reset();
				eventHandled = true;
				break;
			case GLFW_KEY_O:
				mode = rotateView;
				eventHandled = true;
				break;
			case GLFW_KEY_N:
				mode = transView;
				eventHandled = true;
				break;
			case GLFW_KEY_P:
				mode = perspect;
				eventHandled = true;
				break;
			case GLFW_KEY_R:
				mode = rotateModel;
				eventHandled = true;
				break;
			case GLFW_KEY_T:
				mode = transModel;
				eventHandled = true;
				break;
			case GLFW_KEY_S:
				mode = scaleModel;
				eventHandled = true;
				break;
			case GLFW_KEY_V:
				mode = vp;
				eventHandled = true;
				break;
		}
	}
  return eventHandled;
}
