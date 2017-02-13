#include "Cell.hpp"
//#include "cs488-framework/GlErrorCheck.hpp"

//#include <iostream>

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

/*
 * A cube/cuboid can be formed with
 * 8 vertices
 */
Cell::Cell(int x, int z):x(x),z(z) {
  cellVertex = new GLfloat * [8];
  for (int i = 0; i < 8; i++) {
    cellVertex[i] = new GLfloat[3];
  }
  this->setData(0, 0);
}

Cell::~Cell() {
  for (int i = 0; i < 8; i++) {
    delete[] cellVertex[i];
  }
  delete[] cellVertex;
}

GLfloat** Cell::getCellVertex() {
  return this->cellVertex;
}

GLint Cell::getHeight() {
  return height;
}
GLint Cell::getColour() {
  return cellColour;
}

void Cell::setColour(int newColour) {
  cellColour = newColour;
}
/*
 * Increase height by one and
 * change the vertex data accordingly
 */
void Cell::increaseHeight() {
  height++;

  cellVertex[0][1] = height;
  cellVertex[1][1] = height;
  cellVertex[2][1] = height;
  cellVertex[3][1] = height;
}

/*
 * Decrease height by one and
 * change the vertex data accordingly
 */
void Cell::shrinkHeight() {
  height--;

  cellVertex[0][1] = height;
  cellVertex[1][1] = height;
  cellVertex[2][1] = height;
  cellVertex[3][1] = height;
}

/*
 * Set height with a given number, set
 * colour with the given colour, and
 * change the vertex data accordingly
 */
void Cell::setData(int h, int c) {
  height = h;
  cellColour = c;

  cellVertex[0][0] = x;
  cellVertex[0][1] = h;
  cellVertex[0][2] = z;
  cellVertex[1][0] = x;
  cellVertex[1][1] = h;
  cellVertex[1][2] = z + 1;
  cellVertex[2][0] = x + 1;
  cellVertex[2][1] = h;
  cellVertex[2][2] = z + 1;
  cellVertex[3][0] = x + 1;
  cellVertex[3][1] = h;
  cellVertex[3][2] = z;
  cellVertex[4][0] = x;
  cellVertex[4][1] = 0;
  cellVertex[4][2] = z;
  cellVertex[5][0] = x;
  cellVertex[5][1] = 0;
  cellVertex[5][2] = z + 1;
  cellVertex[6][0] = x + 1;
  cellVertex[6][1] = 0;
  cellVertex[6][2] = z + 1;
  cellVertex[7][0] = x + 1;
  cellVertex[7][1] = 0;
  cellVertex[7][2] = z;
}

/*
 * Obtain data from previous cell to copy
 * over
 */
void Cell::copyPrevious(Cell * pre) {
  height = pre->getHeight();
  cellColour = pre->getColour();
  setData(height, cellColour);
}
