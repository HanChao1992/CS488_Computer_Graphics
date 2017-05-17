#include "A1.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;
using namespace std;

static const size_t DIM = 16;

Cell * Cells[DIM][DIM];

// The default color for indicator
// is Red(255, 0, 0)
float indicatorColour[3] = {
    1.0f,
    0.0f,
    0.0f
};

// Zoom helper
float zoomer = 45.0f;

// Rotation helper
float rotator = 0.0f;
float rotateX = 0.0f;
float rotateZ = 0.0f; //float(DIM)*2.0*M_SQRT1_2;


//----------------------------------------------------------------------------------------
// Constructor
A1::A1()
	: current_col( 0 )
{
	colour = new float * [9];
	for (int i = 0; i < 8; i++) {
		colour[i] = new float[3];
		for (int j = 0; j < 3; j++) {
			colour[i][j] = 0.0f;
		}
		// colour[8] is white and it is
		// exclusively used for indicateHelp
		colour[8] = new float[3];
                for (int j = 0; j < 3; j++) {
                    colour[8][j] = 1.0f;
                }
	}
  // Setup Cell objects for all the cells
	for (int i = 0; i < DIM; i++) {
		for (int j = 0; j < DIM; j++) {
			Cells[i][j] = new Cell(i, j);
		}
	}

	// The default starting position is (0,0)
	curX = 0;
	curZ = 0;
	curCell = Cells[curX][curZ];
}

//----------------------------------------------------------------------------------------
// Destructor
A1::~A1()
{}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A1::init()
{
	// Set the background colour.
	glClearColor( 0.3, 0.5, 0.7, 1.0 );

	// Build the shader
	m_shader.generateProgramObject();
	m_shader.attachVertexShader(
		getAssetFilePath( "VertexShader.vs" ).c_str() );
	m_shader.attachFragmentShader(
		getAssetFilePath( "FragmentShader.fs" ).c_str() );
	m_shader.link();

	// Set up the uniforms
	P_uni = m_shader.getUniformLocation( "P" );
	V_uni = m_shader.getUniformLocation( "V" );
	M_uni = m_shader.getUniformLocation( "M" );
	col_uni = m_shader.getUniformLocation( "colour" );

	initGrid();

	// Set up initial view and projection matrices (need to do this here,
	// since it depends on the GLFW window being set up correctly).
	view = glm::lookAt(
		glm::vec3( 0.0f, float(DIM)*2.0*M_SQRT1_2, float(DIM)*2.0*M_SQRT1_2 ),
		glm::vec3( 0.0f, 0.0f, 0.0f ),
		glm::vec3( 0.0f, 1.0f, 0.0f ) );

	proj = glm::perspective(
		glm::radians( 45.0f ),
		float( m_framebufferWidth ) / float( m_framebufferHeight ),
		1.0f, 1000.0f );
}

void A1::initGrid()
{
	size_t sz = 3 * 2 * 2 * (DIM+3);

	float *verts = new float[ sz ];
	size_t ct = 0;
	for( int idx = 0; idx < DIM+3; ++idx ) {
		verts[ ct ] = -1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = idx-1;
		verts[ ct+3 ] = DIM+1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = idx-1;
		ct += 6;

		verts[ ct ] = idx-1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = -1;
		verts[ ct+3 ] = idx-1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = DIM+1;
		ct += 6;
	}

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_grid_vao );
	glBindVertexArray( m_grid_vao );

	// Create the cube vertex buffer
	glGenBuffers( 1, &m_grid_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_grid_vbo );
	glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float),
		verts, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my*
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	// OpenGL has the buffer now, there's no need for us to keep a copy.
	delete [] verts;

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A1::appLogic()
{
	// Place per frame, application logic here ...
}

/*
 * Push one vertex into the buffer
 */
void A1::one_vertex_to_cube(GLfloat * ver, GLfloat * cCube) {
	for (int i = 0; i <= 2; i++) {
		cCube[cubeIndex] = ver[i];
		cubeIndex++;
	}
}

/*
 * Push three vertices (a triangle) into the buffer
 */
void A1::one_triangle_to_cube(GLint a, GLint b, GLint c, GLfloat *cCube, GLfloat** cCell) {
	one_vertex_to_cube (cCell[a], cCube);
	one_vertex_to_cube (cCell[b], cCube);
	one_vertex_to_cube (cCell[c], cCube);
}

/*
 * Setup the cube
 */
void A1::cube(int x, int z) {
	// 12 * 3 = 36
	GLfloat * curCube = new GLfloat[36 * 3];
	if (Cells[x][z]->getHeight() > 0) {
		cubeIndex = 0;
		GLfloat ** curCell = Cells[x][z]->getCellVertex();

		// A cube is made from 12 triangles
		one_triangle_to_cube(0, 1, 2, curCube, curCell);
		one_triangle_to_cube(0, 2, 3, curCube, curCell);
		one_triangle_to_cube(0, 1, 4, curCube, curCell);
		one_triangle_to_cube(1, 4, 5, curCube, curCell);
		one_triangle_to_cube(1, 2, 5, curCube, curCell);
		one_triangle_to_cube(2, 5, 6, curCube, curCell);
		one_triangle_to_cube(2, 3, 6, curCube, curCell);
		one_triangle_to_cube(3, 6, 7, curCube, curCell);
		one_triangle_to_cube(0, 3, 4, curCube, curCell);
		one_triangle_to_cube(3, 4, 7, curCube, curCell);
		one_triangle_to_cube(4, 5, 6, curCube, curCell);
		one_triangle_to_cube(4, 6, 7, curCube, curCell);

		// Create the vertex array to record buffer assignments.
		glGenVertexArrays( 1, &m_cube_vao );
		glBindVertexArray( m_cube_vao );

		// Create the cube vertex buffer
		glGenBuffers( 1, &m_cube_vbo );
		glBindBuffer( GL_ARRAY_BUFFER, m_cube_vbo );
		glBufferData( GL_ARRAY_BUFFER, 3*36*sizeof(float),
			curCube, GL_STATIC_DRAW );

		// Specify the means of extracting the position values properly.
		GLint posAttrib = m_shader.getAttribLocation( "position" );
		glEnableVertexAttribArray( posAttrib );
		glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

		// Reset state to prevent rogue code from messing with *my*
		// stuff!
		glBindVertexArray(m_cube_vao);
                GLint colourIndex = Cells[x][z]->getColour();
                glUniform3f(col_uni, colour[colourIndex][0], colour[colourIndex][1], colour[colourIndex][2]);
	        glDrawArrays(GL_TRIANGLES, 0, 36);

		// OpenGL has the buffer now, there's no need for us to keep a copy.
		delete [] curCube;

		CHECK_GL_ERRORS;
	}
}

/*
 * Setup the indicator, the indicator is an upside down pyramid shape
 */
void A1::indicators(int x, int z){
  // 3 * 6 = 18
	GLfloat * indicator = new GLfloat[18 * 3];
	cubeIndex = 0;
	GLint indicatorHeight = Cells[x][z]->getHeight() + 2;
  // 5 vertices
	GLfloat ** indicator_vertex = new GLfloat *[5];
	for (int i = 0; i < 5; i++) {
		indicator_vertex[i] = new GLfloat[3];
	}

	// It can be seem as constructed in side a cube
	// We take the four middle points of the four sides the square on top and
	// connect with the centre point of the square on the bottom
	indicator_vertex[0][0] = x;
	indicator_vertex[0][1] = indicatorHeight;
	indicator_vertex[0][2] = z + 0.5;
	indicator_vertex[1][0] = x + 0.5;
	indicator_vertex[1][1] = indicatorHeight;
	indicator_vertex[1][2] = z;
	indicator_vertex[2][0] = x + 1;
	indicator_vertex[2][1] = indicatorHeight;
	indicator_vertex[2][2] = z + 0.5;
	indicator_vertex[3][0] = x + 0.5;
	indicator_vertex[3][1] = indicatorHeight;
	indicator_vertex[3][2] = z + 1;
	indicator_vertex[4][0] = x + 0.5;
	indicator_vertex[4][1] = indicatorHeight - 1;
	indicator_vertex[4][2] = z + 0.5;

  // Consists  of 6 triangles
	one_triangle_to_cube(0, 1, 2, indicator, indicator_vertex);
	one_triangle_to_cube(0, 2, 3, indicator, indicator_vertex);
	one_triangle_to_cube(0, 1, 4, indicator, indicator_vertex);
	one_triangle_to_cube(1, 2, 4, indicator, indicator_vertex);
	one_triangle_to_cube(2, 3, 4, indicator, indicator_vertex);
	one_triangle_to_cube(0, 3, 4, indicator, indicator_vertex);

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_indicator_vao );
	glBindVertexArray( m_indicator_vao );

	// Create the cube vertex buffer
	glGenBuffers( 1, &m_indicator_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_indicator_vbo );
	glBufferData( GL_ARRAY_BUFFER, 3*18*sizeof(float),
		indicator, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my*
	// stuff!
	glBindVertexArray(m_indicator_vao);
	glUniform3f(col_uni, indicatorColour[0], indicatorColour[1], indicatorColour[2]);
	glDrawArrays(GL_TRIANGLES, 0, 18);

	delete[] indicator;
	for (int i = 0; i < 5; i++) {
		delete[] indicator_vertex[i];
	}
	delete[] indicator_vertex;

  CHECK_GL_ERRORS;

}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A1::guiLogic()
{
	// We already know there's only going to be one window, so for
	// simplicity we'll store button states in static local variables.
	// If there was ever a possibility of having multiple instances of
	// A1 running simultaneously, this would break; you'd want to make
	// this into instance fields of A1.
	static bool showTestWindow(false);
	static bool showDebugWindow(true);

	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Debug Window", &showDebugWindow, ImVec2(100,100), opacity, windowFlags);
		if( ImGui::Button( "Quit Application (Q)" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}
		if ( ImGui::Button( "Reset Current Cell (Shift + R)")) {
			resetCur();
		}
		if ( ImGui::Button( "Reset Grid (R)")) {
			resetAll();
		}

		ImGui::ColorEdit3( "Indicator Colour", indicatorColour );

		// Eventually you'll create multiple colour widgets with
		// radio buttons.  If you use PushID/PopID to give them all
		// unique IDs, then ImGui will be able to keep them separate.
		// This is unnecessary with a single colour selector and
		// radio button, but I'm leaving it in as an example.

		// Prefixing a widget name with "##" keeps it from being
		// displayed.
    for (int i = 0; i < 8; i++) {
			ImGui::PushID( i );
			ImGui::ColorEdit3( "##Colour", colour[i] );
			ImGui::SameLine();
			if( ImGui::RadioButton( "##Col", &current_col, i ) ) {
				curCell->setColour(current_col);
			}
		}
		for (int i = 0; i < 8; i++) {
	  	ImGui::PopID();
		}

/*
		// For convenience, you can uncomment this to show ImGui's massive
		// demonstration window right in your application.  Very handy for
		// browsing around to get the widget you want.  Then look in
		// shared/imgui/imgui_demo.cpp to see how it's done.
		if( ImGui::Button( "Test Window" ) ) {
			showTestWindow = !showTestWindow;
		}
*/

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();

	if( showTestWindow ) {
		ImGui::ShowTestWindow( &showTestWindow );
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A1::draw()
{
	// Create a global transformation for the model (centre it).
	mat4 W;
	W = glm::translate( W, vec3( -float(DIM)/2.0f, 0, -float(DIM)/2.0f ) );

	m_shader.enable();
		glEnable( GL_DEPTH_TEST );

		glUniformMatrix4fv( P_uni, 1, GL_FALSE, value_ptr( proj ) );
		glUniformMatrix4fv( V_uni, 1, GL_FALSE, value_ptr( view ) );
		glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );

		// Just draw the grid for now.
		glBindVertexArray( m_grid_vao );
		glUniform3f( col_uni, 1, 1, 1 );
		glDrawArrays( GL_LINES, 0, (3+DIM)*4 );

		// Draw the cubes
		for (int i = 0; i < DIM; i++) {
			for (int j = 0; j < DIM; j++) {
				cube(i, j);
			}
		}
		// Highlight the active square.
		glDisable(GL_DEPTH_TEST);
		indicators(curX, curZ);
	m_shader.disable();

	// Restore defaults
	glBindVertexArray( 0 );

	CHECK_GL_ERRORS;
}

/*
 * Reset the current active cell
 */
void A1::resetCur() {
	int curColour = Cells[curX][curZ]->getColour();
	Cells[curX][curZ]->setData(0, curColour);
}

/*
 * Reset the whole grid
 */
void A1::resetAll() {
  // Set all cells to 0 heights and default color
	for (int i = 0; i < DIM; i++) {
		for (int j =  0; j < DIM; j++) {
			Cells[i][j]->setData(0, 0);
		}
	}

	// Resets all the colors
	for (int i = 0; i < 8; i++) {
		for (int j =  0; j < 3; j++) {
			colour[i][j] = 0.0f;
		}
	}
	current_col = 0;

        // Reset position to default
	curX = 0;
	curZ = 0;
	curCell  = Cells[curX][curZ];

        // Reset rotation and zoom
	view = glm::lookAt(
		glm::vec3( 0.0f, float(DIM)*2.0*M_SQRT1_2, float(DIM)*2.0*M_SQRT1_2 ),
		glm::vec3( 0.0f, 0.0f, 0.0f ),
		glm::vec3( 0.0f, 1.0f, 0.0f ) );

	proj = glm::perspective(
               glm::radians( 45.0f ),
               float( m_framebufferWidth ) / float( m_framebufferHeight ),
               1.0f, 1000.0f );

}
//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A1::cleanup()
{
	for (int i = 0; i < DIM; i++) {
		for (int j =  0; j < DIM; j++) {
			delete Cells[i][j];
		}
	}
	for (int i = 0; i < 9; i++) {
		delete[] colour[i];
	}
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A1::cursorEnterWindowEvent (
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
bool A1::mouseMoveEvent(double xPos, double yPos)
{
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// Put some code here to handle rotations.  Probably need to
		// check whether we're *dragging*, not just moving the mouse.
		// Probably need some instance variables to track the current
		// rotation amount, and maybe the previous X position (so
		// that you can rotate relative to the *change* in X.
		if (dragging) {
			float realRotate = xPos - rotator;
			rotateX += realRotate * 0.03f;
      rotateZ += realRotate * 0.03f;
			rotator = xPos;
      GLfloat radius = 20.0f;
      GLfloat camX = sin(rotateX) * radius;
      GLfloat camZ = cos(rotateZ) * radius;
      view = glm::lookAt(
             	glm::vec3( camX, float(DIM)*2.0*M_SQRT1_2, camZ),
              glm::vec3( 0.0f, 0.0f, 0.0f ),
              glm::vec3( 0.0f, 1.0f, 0.0f ) );
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A1::mouseButtonInputEvent(int button, int actions, int mods) {
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// The user clicked in the window.  If it's the left
		// mouse button, initiate a rotation.
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			if (actions == GLFW_PRESS) {
				dragging = true;
			}
			else if (actions == GLFW_RELEASE) {
				dragging = false;
			}
			eventHandled = true;
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A1::mouseScrollEvent(double xOffSet, double yOffSet) {
	bool eventHandled(false);

	// Zoom in or out.
	zoomer += yOffSet;
        // Custom set range
	if (zoomer >= 19.0f && zoomer <= 100.0f) {
            proj = glm::perspective(
                       glm::radians( zoomer ),
                       float( m_framebufferWidth ) / float( m_framebufferHeight ),
                       1.0f, 1000.0f );
            eventHandled = true;
        }
        return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A1::windowResizeEvent(int width, int height) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A1::keyInputEvent(int key, int action, int mods) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if( action == GLFW_PRESS ) {
		// Respond to some key events.
		if (key == GLFW_KEY_Q) {
    	glfwSetWindowShouldClose(m_window, GL_TRUE);
    	eventHandled = true;
		}
		else if (key == GLFW_KEY_R) {
			if (shiftCopy) {
				// hold shift to reset only the
				// active cell
				resetCur();
			}
			else {
				resetAll();
			}
			eventHandled = true;
		}
		else if (key == GLFW_KEY_SPACE) {
			//cout << "Stacking..." << endl;
			if ( curCell->getHeight() == 0) {
				// if the bar is empty, it must use
				// the current color to grow
				curCell->setColour(current_col);
			}
			curCell->increaseHeight();
                        eventHandled = true;
		}
		else if (key == GLFW_KEY_BACKSPACE) {
			if (curCell->getHeight() > 0) {
				curCell->shrinkHeight();
			}
			eventHandled = true;
		}
		else if (key == GLFW_KEY_UP) {
    	if (!indicateHelp) {
				if(curZ < 1) {
					curZ = DIM;
				}
				curZ--;
				if (shiftCopy) {
					Cell * pre = curCell;
					Cells[curX][curZ]->copyPrevious(pre);
				}
				curCell  = Cells[curX][curZ];
        eventHandled = true;
  		}
			//cout << "You are at (" << curX << ", " << curZ << ")." << endl;
		}
		else if (key == GLFW_KEY_DOWN) {
    	if (!indicateHelp) {
				if(curZ >= DIM - 1) {
					curZ = -1;
				}
				curZ++;
				if (shiftCopy) {
					Cell * pre = curCell;
					Cells[curX][curZ]->copyPrevious(pre);
				}
				curCell  = Cells[curX][curZ];
        eventHandled = true;
  		}
			//cout << "You are at (" << curX << ", " << curZ << ")." << endl;
		}
		else if (key == GLFW_KEY_LEFT) {
    	if (!indicateHelp) {
				if(curX < 1) {
					curX = DIM;
				}
				curX--;
				if (shiftCopy) {
					Cell * pre = curCell;
					Cells[curX][curZ]->copyPrevious(pre);
				}
				curCell  = Cells[curX][curZ];
      	eventHandled = true;
    	}
			//cout << "You are at (" << curX << ", " << curZ << ")." << endl;
		}
		else if (key == GLFW_KEY_RIGHT) {
  		if (!indicateHelp) {
				if(curX >= DIM - 1) {
					curX = -1;
				}
				curX++;
				if (shiftCopy) {
					Cell * pre = curCell;
					Cells[curX][curZ]->copyPrevious(pre);
				}
				curCell  = Cells[curX][curZ];
        	eventHandled = true;
    	}
			//cout << "You are at (" << curX << ", " << curZ << ")." << endl;
		}
		else if (key == GLFW_KEY_LEFT_SHIFT or key == GLFW_KEY_RIGHT_SHIFT) {
			shiftCopy = true;
      eventHandled = true;
		}
		else if (key == GLFW_KEY_A) {
      indicateHelp = true;
      curCell->setColour(8);
      eventHandled = true;
  	}
	}

	if (action == GLFW_RELEASE) {
			if (key == GLFW_KEY_LEFT_SHIFT or key == GLFW_KEY_RIGHT_SHIFT) {
				shiftCopy = false;
				eventHandled = true;
			}
			else if (key == GLFW_KEY_A) {
        indicateHelp = false;
        curCell->setColour(current_col);
      	eventHandled = true;
  		}
	}

	return eventHandled;
}
