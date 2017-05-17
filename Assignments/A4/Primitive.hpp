#pragma once

#include <glm/glm.hpp>
#include "SceneNode.hpp"
#include "polyroots.hpp"
#include <vector>
#include <algorithm>


class Primitive {
public:
  virtual ~Primitive();
  virtual Intersection Intersect(const glm::vec3 & eye, const glm::vec3 & worldC);

};

class BoundingVolume : public Primitive {
public:
  BoundingVolume(std::vector<glm::vec3> vert)
    : vert(vert)
  {
  }
  virtual ~BoundingVolume();
  Intersection Intersect(const glm::vec3 & eye, const glm::vec3 & worldC);
private:
  std::vector<glm::vec3> vert;
};

class NonhierSphere : public Primitive {
public:
  NonhierSphere(const glm::vec3& pos, double radius)
    : m_pos(pos), m_radius(radius)
  {
  }
  virtual ~NonhierSphere();
  Intersection Intersect(const glm::vec3 & eye, const glm::vec3 & worldC);

private:
  glm::vec3 m_pos;
  double m_radius;
};

class NonhierBox : public Primitive {
public:
  BoundingVolume * bBox;
  NonhierBox(const glm::vec3& pos, double size)
    : m_pos(pos), m_size(size)
  {
    std::vector<glm::vec3> vertices;
    for (float i = m_pos.x; i <= m_pos.x + m_size; i += m_size) {
      for (float j = m_pos.y; j <= m_pos.y + m_size; j += m_size) {
        for (float k = m_pos.z; k <= m_pos.z + m_size; k += m_size) {
          vertices.push_back(glm::vec3(i, j, k));
        }
      }
    }
    bBox = new BoundingVolume(vertices);
  }

  virtual ~NonhierBox();
  Intersection Intersect(const glm::vec3 & eye, const glm::vec3 & worldC);

private:
  glm::vec3 m_pos;
  double m_size;
};

class Sphere : public Primitive {
public:
  Sphere();
  NonhierSphere * nhSphere;
  virtual ~Sphere();
  virtual Intersection Intersect(const glm::vec3 & eye, const glm::vec3 & worldC);

};

class Cube : public Primitive {
public:
  Cube();
  NonhierBox * nhBox;
  virtual ~Cube();
  virtual Intersection Intersect(const glm::vec3 & eye, const glm::vec3 & worldC);
};

Intersection intersectTriangle(const glm::vec3 & eye, const glm::vec3 &worldC, glm::vec3 u, glm::vec3 v, glm::vec3 w);
