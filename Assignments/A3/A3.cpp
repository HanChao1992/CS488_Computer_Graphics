#include "A3.hpp"
#include "scene_lua.hpp"
using namespace std;

#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/MathUtils.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

static bool show_gui = true;

const size_t CIRCLE_PTS = 48;

//----------------------------------------------------------------------------------------
// Constructor
A3::A3(const std::string & luaSceneFile)
	: m_luaSceneFile(luaSceneFile),
	  m_positionAttribLocation(0),
	  m_normalAttribLocation(0),
	  m_vao_meshData(0),
	  m_vbo_vertexPositions(0),
	  m_vbo_vertexNormals(0),
	  m_vao_arcCircle(0),
	  m_vbo_arcCircle(0)
{

}

//----------------------------------------------------------------------------------------
// Destructor
A3::~A3()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A3::init()
{
	// Set the background colour.
	glClearColor(0.35, 0.35, 0.35, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao_arcCircle);
	glGenVertexArrays(1, &m_vao_meshData);
	enableVertexShaderInputSlots();

	processLuaSceneFile(m_luaSceneFile);

	// Load and decode all .obj files at once here.  You may add additional .obj files to
	// this list in order to support rendering additional mesh types.  All vertex
	// positions, and normals will be extracted and stored within the MeshConsolidator
	// class.
	unique_ptr<MeshConsolidator> meshConsolidator (new MeshConsolidator{
			getAssetFilePath("cube.obj"),
			getAssetFilePath("sphere.obj"),
			getAssetFilePath("suzanne.obj")
	});


	// Acquire the BatchInfoMap from the MeshConsolidator.
	meshConsolidator->getBatchInfoMap(m_batchInfoMap);

	// Take all vertex data within the MeshConsolidator and upload it to VBOs on the GPU.
	uploadVertexDataToVbos(*meshConsolidator);

	mapVboDataToVertexShaderInputLocations();

	initPerspectiveMatrix();

	initViewMatrix();

	initLightSources();

	positionMat = mat4();
	rotationMat = mat4();
	factor = 200.0f;
	zBuffer = true;
	backfaceCulling = false;
	frontfaceCulling = false;
	if (m_framebufferWidth <= m_framebufferHeight) {
		ballDia = m_framebufferWidth / 2;
	}
	else {
		ballDia = m_framebufferHeight / 2;
	}

	// Exiting the current scope calls delete automatically on meshConsolidator freeing
	// all vertex data resources.  This is fine since we already copied this data to
	// VBOs on the GPU.  We have no use for storing vertex data on the CPU side beyond
	// this point.
}

//----------------------------------------------------------------------------------------
void A3::processLuaSceneFile(const std::string & filename) {
	// This version of the code treats the Lua file as an Asset,
	// so that you'd launch the program with just the filename
	// of a puppet in the Assets/ directory.
	// std::string assetFilePath = getAssetFilePath(filename.c_str());
	// m_rootNode = std::shared_ptr<SceneNode>(import_lua(assetFilePath));

	// This version of the code treats the main program argument
	// as a straightforward pathname.
	m_rootNode = std::shared_ptr<SceneNode>(import_lua(filename));
	if (!m_rootNode) {
		std::cerr << "Could not open " << filename << std::endl;
	}
}

//----------------------------------------------------------------------------------------
void A3::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();

	m_shader_arcCircle.generateProgramObject();
	m_shader_arcCircle.attachVertexShader( getAssetFilePath("arc_VertexShader.vs").c_str() );
	m_shader_arcCircle.attachFragmentShader( getAssetFilePath("arc_FragmentShader.fs").c_str() );
	m_shader_arcCircle.link();
}

//----------------------------------------------------------------------------------------
void A3::enableVertexShaderInputSlots()
{
	//-- Enable input slots for m_vao_meshData:
	{
		glBindVertexArray(m_vao_meshData);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_positionAttribLocation = m_shader.getAttribLocation("position");
		glEnableVertexAttribArray(m_positionAttribLocation);

		// Enable the vertex shader attribute location for "normal" when rendering.
		m_normalAttribLocation = m_shader.getAttribLocation("normal");
		glEnableVertexAttribArray(m_normalAttribLocation);

		CHECK_GL_ERRORS;
	}


	//-- Enable input slots for m_vao_arcCircle:
	{
		glBindVertexArray(m_vao_arcCircle);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_arc_positionAttribLocation = m_shader_arcCircle.getAttribLocation("position");
		glEnableVertexAttribArray(m_arc_positionAttribLocation);

		CHECK_GL_ERRORS;
	}

	// Restore defaults
	glBindVertexArray(0);
}

//----------------------------------------------------------------------------------------
void A3::uploadVertexDataToVbos (
		const MeshConsolidator & meshConsolidator
) {
	// Generate VBO to store all vertex position data
	{
		glGenBuffers(1, &m_vbo_vertexPositions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexPositionBytes(),
				meshConsolidator.getVertexPositionDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store all vertex normal data
	{
		glGenBuffers(1, &m_vbo_vertexNormals);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexNormalBytes(),
				meshConsolidator.getVertexNormalDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store the trackball circle.
	{
		glGenBuffers( 1, &m_vbo_arcCircle );
		glBindBuffer( GL_ARRAY_BUFFER, m_vbo_arcCircle );

		float *pts = new float[ 2 * CIRCLE_PTS ];
		for( size_t idx = 0; idx < CIRCLE_PTS; ++idx ) {
			float ang = 2.0 * M_PI * float(idx) / CIRCLE_PTS;
			pts[2*idx] = cos( ang );
			pts[2*idx+1] = sin( ang );
		}

		glBufferData(GL_ARRAY_BUFFER, 2*CIRCLE_PTS*sizeof(float), pts, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A3::mapVboDataToVertexShaderInputLocations()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_meshData);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexPositions" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);
	glVertexAttribPointer(m_positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexNormals" into the
	// "normal" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);
	glVertexAttribPointer(m_normalAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;

	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_arcCircle);

	// Tell GL how to map data from the vertex buffer "m_vbo_arcCircle" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_arcCircle);
	glVertexAttribPointer(m_arc_positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A3::initPerspectiveMatrix()
{
	float aspect = ((float)m_windowWidth) / m_windowHeight;
	m_perpsective = glm::perspective(degreesToRadians(60.0f), aspect, 0.1f, 100.0f);
}


//----------------------------------------------------------------------------------------
void A3::initViewMatrix() {
	m_view = glm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f),
			vec3(0.0f, 1.0f, 0.0f));
}

//----------------------------------------------------------------------------------------
void A3::initLightSources() {
	// World-space position
	m_light.position = vec3(-2.0f, 5.0f, 0.5f);
	m_light.rgbIntensity = vec3(0.8f); // White light
}

//----------------------------------------------------------------------------------------
void A3::uploadCommonSceneUniforms() {
	m_shader.enable();
	{
		//-- Set Perpsective matrix uniform for the scene:
		GLint location = m_shader.getUniformLocation("Perspective");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perpsective));
		CHECK_GL_ERRORS;


		//-- Set LightSource uniform for the scene:
		{
			location = m_shader.getUniformLocation("light.position");
			glUniform3fv(location, 1, value_ptr(m_light.position));
			location = m_shader.getUniformLocation("light.rgbIntensity");
			glUniform3fv(location, 1, value_ptr(m_light.rgbIntensity));
			CHECK_GL_ERRORS;
		}

		//-- Set background light ambient intensity
		{
			location = m_shader.getUniformLocation("ambientIntensity");
			vec3 ambientIntensity(0.05f);
			glUniform3fv(location, 1, value_ptr(ambientIntensity));
			CHECK_GL_ERRORS;
		}
	}
	m_shader.disable();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A3::appLogic()
{
	// Place per frame, application logic here ...

	uploadCommonSceneUniforms();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A3::guiLogic()
{
	if( !show_gui ) {
		return;
	}

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

			if (ImGui::BeginMenu("Application")) {
				if (ImGui::Button("Reset Position(I)")) {
					resetPosition();
				}
				if (ImGui::Button("Reset Orientation(O)")) {
					resetRotation();
				}
				if (ImGui::Button("Reset All(A)")) {
					resetAll();
				}
				if( ImGui::Button( "Quit(Q)" ) ) {
					glfwSetWindowShouldClose(m_window, GL_TRUE);
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Options")) {
				if (ImGui::Checkbox("Circle(C)", &circle)) {
				}
				if (ImGui::Checkbox("Z-Buffer(Z)", &zBuffer)) {
				}
				if (ImGui::Checkbox("Backface Culling(B)", &backfaceCulling)) {
				}
				if (ImGui::Checkbox("Frontface Culling(F)", &frontfaceCulling)) {
				}
				ImGui::EndMenu();
			}

		// Add more gui elements here here ...


		// Create Button, and check if it was clicked:


		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();
}

//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms:
static void updateShaderUniforms(
		const ShaderProgram & shader,
		const GeometryNode & node,
		const glm::mat4 & viewMatrix
) {

	shader.enable();
	{
		//-- Set ModelView matrix:
		GLint location = shader.getUniformLocation("ModelView");
		mat4 modelView = viewMatrix * node.trans;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
		CHECK_GL_ERRORS;

		//-- Set NormMatrix:
		location = shader.getUniformLocation("NormalMatrix");
		mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
		glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
		CHECK_GL_ERRORS;


		//-- Set Material values:
		location = shader.getUniformLocation("material.kd");
		vec3 kd = node.material.kd;
		glUniform3fv(location, 1, value_ptr(kd));
		CHECK_GL_ERRORS;
		location = shader.getUniformLocation("material.ks");
		vec3 ks = node.material.ks;
		glUniform3fv(location, 1, value_ptr(ks));
		CHECK_GL_ERRORS;
		location = shader.getUniformLocation("material.shininess");
		glUniform1f(location, node.material.shininess);
		CHECK_GL_ERRORS;

	}
	shader.disable();

}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A3::draw() {

	if (zBuffer) {
		glEnable( GL_DEPTH_TEST );
	}
	if (backfaceCulling) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}
	if (frontfaceCulling) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
	}
	else {
		glDisable(GL_CULL_FACE);
	}
	renderSceneGraph(*m_rootNode);


	glDisable( GL_DEPTH_TEST );
	if (circle) {
		renderArcCircle();
	}
}

/*
 * Traverse through the tree
 */
void A3::traverse(SceneNode *node, mat4 M ) {
	mat4 T = node->get_transform();
	node->set_transform(M * T);

	for (SceneNode *child: node->children) {
		traverse(child, node->get_transform());
	}

	if (node->m_nodeType == NodeType::GeometryNode) {
		const GeometryNode * geometryNode = static_cast<const GeometryNode *>(node);

		updateShaderUniforms(m_shader, *geometryNode, m_view);

		BatchInfo batchInfo = m_batchInfoMap[geometryNode->meshId];
		m_shader.enable();
		glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
		m_shader.disable();
	}
	node->set_transform(T);
}
//----------------------------------------------------------------------------------------
void A3::renderSceneGraph(SceneNode & root) {

	// Bind the VAO once here, and reuse for all GeometryNode rendering below.
	glBindVertexArray(m_vao_meshData);

	// This is emphatically *not* how you should be drawing the scene graph in
	// your final implementation.  This is a non-hierarchical demonstration
	// in which we assume that there is a list of GeometryNodes living directly
	// underneath the root node, and that we can draw them in a loop.  It's
	// just enough to demonstrate how to get geometry and materials out of
	// a GeometryNode and onto the screen.

	// You'll want to turn this into recursive code that walks over the tree.
	// You can do that by putting a method in SceneNode, overridden in its
	// subclasses, that renders the subtree rooted at every node.  Or you
	// could put a set of mutually recursive functions in this class, which
	// walk down the tree from nodes of different types.
	traverse(&root, mat4());
	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
// Draw the trackball circle.
void A3::renderArcCircle() {
	glBindVertexArray(m_vao_arcCircle);

	m_shader_arcCircle.enable();
		GLint m_location = m_shader_arcCircle.getUniformLocation( "M" );
		float aspect = float(m_framebufferWidth)/float(m_framebufferHeight);
		glm::mat4 M;
		if( aspect > 1.0 ) {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5/aspect, 0.5, 1.0 ) );
		} else {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5, 0.5*aspect, 1.0 ) );
		}
		glUniformMatrix4fv( m_location, 1, GL_FALSE, value_ptr( M ) );
		glDrawArrays( GL_LINE_LOOP, 0, CIRCLE_PTS );
	m_shader_arcCircle.disable();

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A3::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A3::cursorEnterWindowEvent (
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
bool A3::mouseMoveEvent (
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
		mat4 translator;
		mat4 rotator;
 		if (mouseL) {
			vec3 tran = vec3(mousexDiff/factor, -mouseyDiff/factor, 0);
			translator = translate(mat4(), tran);
			// Stores each translation so that later on we can reset it
			// by reversing it
			positionMat *= translator;
			m_rootNode->set_transform(translator * m_rootNode->get_transform());
		}
		if (mouseM) {
			vec3 tran = vec3(0, 0, mouseyDiff/factor);
			translator = translate(mat4(), tran);
			positionMat *= translator;
			m_rootNode->set_transform(translator * m_rootNode->get_transform());
		}
		if (mouseR) {
			vec3 w = virtualTrackBallMapping(preMousexPos, preMouseyPos);
			//cout << w << endl;
			vec3 v = virtualTrackBallMapping(curMousexPos, curMouseyPos);
			vec3 a = virtualTrackBallAxis(v, w);
			rotator = virtualTrackBallMat(a);
			//cout << rotator << endl;
			rotationMat *= rotator;
			m_rootNode->set_transform(m_rootNode->get_transform() * rotator);
		}
	}
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A3::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if (!ImGui::IsMouseHoveringAnyWindow()) {
 		if (actions == GLFW_PRESS) {
 			double xpos, ypos;
 			glfwGetCursorPos(m_window, &xpos, &ypos);
 			preMousexPos = xpos;
			preMouseyPos = ypos;
 			if (button == GLFW_MOUSE_BUTTON_LEFT) {
				mouseL = true;
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
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A3::mouseScrollEvent (
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
bool A3::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);
	initPerspectiveMatrix();
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A3::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	if( action == GLFW_PRESS ) {
		if( key == GLFW_KEY_M ) {
			show_gui = !show_gui;
			eventHandled = true;
		}
		if (key == GLFW_KEY_I) {
			resetPosition();
			eventHandled = true;
		}
		if (key == GLFW_KEY_O) {
			resetRotation();
			eventHandled = true;
		}
		if (key == GLFW_KEY_A) {
			resetAll();
			eventHandled = true;
		}
		if (key == GLFW_KEY_Q) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}
		if (key == GLFW_KEY_C) {
			if (circle) {
				circle = false;
			}
			else {
				circle = true;
			}
			eventHandled = true;
		}
		if (key == GLFW_KEY_Z) {
			if (zBuffer) {
				zBuffer = false;
			}
			else {
				zBuffer = true;
			}
			eventHandled = true;
		}
		if (key == GLFW_KEY_B) {
			if (backfaceCulling) {
				backfaceCulling = false;
			}
			else {
				backfaceCulling = true;
			}
			eventHandled = true;
		}
		if (key == GLFW_KEY_F) {
			if (frontfaceCulling) {
				frontfaceCulling = false;
			}
			else {
				frontfaceCulling = true;
			}
			eventHandled = true;
		}
	}
	// Fill in with event handling code...

	return eventHandled;
}

/*
 * reverse the translation process to reset the position
 * of the puppet
 */
void A3::resetPosition() {
	//cout << positionMat << endl;
	m_rootNode->set_transform(inverse(positionMat) * m_rootNode->get_transform());
	positionMat = mat4();
}

/*
 * reverse the  rotation process to reset the orientation
 * of the puppet
 */
void A3::resetRotation() {
	m_rootNode->set_transform(m_rootNode->get_transform() * inverse(rotationMat));
	rotationMat = mat4();
}

/*
 * reset both position and rotation
 */
void A3::resetAll() {
	resetPosition();
	resetRotation();
	circle = false;
	zBuffer = true;
	backfaceCulling = false;
	frontfaceCulling = false;
}

/*
 * mapping mouse coordinates to sphere
 */
vec3 A3::virtualTrackBallMapping(float xpos, float ypos) {
	vec3 v;

	v.x = (xpos - m_windowWidth / 2.0) * 2.0 / ballDia;
	v.y = (ypos - m_windowHeight / 2.0) * 2.0 / ballDia;
	float zhelp = 1.0 - v.x * v.x - v.y * v.y;
	if(zhelp >= 0.0) {
		v.z = sqrt(zhelp);
	}
	else { // if the mouse goes outside the trackball, regard it as on
			   // the diameter of the ball.
		v.z = 0.0;
	}
	return v;
}

/*
 * generate the rotation axis;
 */
vec3 A3::virtualTrackBallAxis(vec3 v, vec3 w) {
	vec3 a = cross(v, w);
	return a;
}

/*
 * generate the rotation matrix.
 */
mat4 A3::virtualTrackBallMat(vec3 v) {
	mat4 ballMat;
	float vlen = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	if (vlen < 0.00000001) { // if the length of the vector is ZERO
		return mat4();
	}
	float x = v.x / vlen;
	float y = v.y / vlen;
	float z = v.z / vlen;
	ballMat[0] = vec4(cos(vlen) + x*x*(1-cos(vlen)),
									  x*y*(1-cos(vlen)) - sin(vlen)*z,
										z*x*(1-cos(vlen)) + sin(vlen)*y,
										0);
	ballMat[1] = vec4(x*y*(1-cos(vlen)) + z*sin(vlen),
										cos(vlen) + y*y*(1-cos(vlen)),
										z*y*(1-cos(vlen)) - sin(vlen)*x,
										0);
	ballMat[2] = vec4(x*z*(1-cos(vlen)) - y*sin(vlen),
										y*z*(1-cos(vlen)) + sin(vlen)*x,
										cos(vlen) + z*z*(1-cos(vlen)),
										0);
	ballMat[3] = vec4(0, 0, 0, 1);
	return ballMat;

}
