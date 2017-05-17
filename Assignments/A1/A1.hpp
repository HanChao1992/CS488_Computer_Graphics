#pragma once

#include <glm/glm.hpp>

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include "grid.hpp"
#include "Cell.hpp"


class A1 : public CS488Window {
public:
	A1();
	virtual ~A1();

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

private:
	void initGrid();
        void cube(int x, int z);
	void indicators(int x, int z);
	void one_vertex_to_cube(GLfloat * ver, GLfloat * cCube);
	void one_triangle_to_cube(GLint a, GLint b, GLint c, GLfloat *cCube, GLfloat** cCell);
	void resetCur();
	void resetAll();


	Cell * curCell; // current active cell
	
	// Fields related to the shader and uniforms.
	ShaderProgram m_shader;
	GLint P_uni; // Uniform location for Projection matrix.
	GLint V_uni; // Uniform location for View matrix.
	GLint M_uni; // Uniform location for Model matrix.
	GLint col_uni;   // Uniform location for cube colour.
        
	
	// Fields related to grid geometry.
	GLuint m_grid_vao; // Vertex Array Object
	GLuint m_grid_vbo; // Vertex Buffer Object

        GLuint m_cube_vao; // Vertex Array Object
	GLuint m_cube_vbo; // Vertex Buffer Object

	GLuint m_indicator_vao; // Vertex Array Object
	GLuint m_indicator_vbo; // Vertex Buffer Object

	GLint curX; // current x position
	GLint curZ; // current y position
	// Matrices controlling the camera and projection.
	glm::mat4 proj;
	glm::mat4 view;

        int cubeIndex; // Index for vertices of cube/cuboid.
	bool shiftCopy = false; // Indicates whether shift is held.
	bool dragging = false; // Indicates whether left mouse key is held.
	bool indicateHelp = false; // Indicates whether A key is pressed.
	float ** colour;
	int current_col;
	int cur_indi_col; // current color for the indicator
};
