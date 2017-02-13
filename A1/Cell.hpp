#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include <glm/glm.hpp>

/*
 * A Cell at any moment Represents
 * the shape we constructed on a square.
 */
class Cell {
  private:
    GLfloat ** cellVertex;
    GLint x;
    GLint z;
    GLint height;
    GLint cellColour;

  public:
	   Cell(int x, int z);
	   ~Cell();

	   GLint getHeight();
	   GLint getColour();
     GLfloat** getCellVertex();
	   void increaseHeight();
     void shrinkHeight();
	   void setColour(int newColour);
     void setData(int h, int c);
     void copyPrevious(Cell * pre);
  };
