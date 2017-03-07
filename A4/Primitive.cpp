#include "Primitive.hpp"
#include <iostream>

#include <glm/gtx/io.hpp>

using namespace std;
Primitive::~Primitive()
{
}

Intersection Primitive::Intersect(const glm::vec3 & eye, const glm::vec3 & worldC) {
  return Intersection(0, false, glm::vec3(), glm::vec3());
}

NonhierSphere::~NonhierSphere()
{
}

Intersection NonhierSphere::Intersect(const glm::vec3 & eye, const glm::vec3  & worldC) {
  double roots[2];
  double root;
  double a = glm::dot(worldC - eye, worldC - eye);
  double b = glm::dot(worldC - eye, eye - m_pos);
  b *= 2;
  double c = glm::dot(eye - m_pos, eye - m_pos) - m_radius * m_radius;

	size_t rn = quadraticRoots(a, b ,c, roots);

  glm::vec3 pHit;
  glm::vec3 normal;
  if (rn <= 0) {

    return Intersection(0, false, worldC, glm::vec3());
  }
  else {
    root = roots[0];
    if (rn == 2) {
      if ((roots[1] < 0) && (roots[0] < 0)) {
        return Intersection(0, false, worldC, glm::vec3());
      }
      if (roots[1] < roots[0]) {
        root = roots[1];
      }
    }
    if (root < 0) {
      return Intersection(0, false, worldC, glm::vec3());
    }
  }
  pHit = (eye + (float)root * (worldC - eye));
  normal = pHit - m_pos;

  return Intersection(root, true, pHit, normal);
}

NonhierBox::~NonhierBox()
{
}
Intersection NonhierBox::Intersect(const glm::vec3 & eye, const glm::vec3 & worldC) {
  return bBox->Intersect(eye,worldC);
}

BoundingVolume::~BoundingVolume()
{
}

Intersection BoundingVolume::Intersect(const glm::vec3 & eye, const glm::vec3 & worldC) {
  float tmin, tmax, tymin, tymax, tzmin, tzmax;

  glm::vec3 bMin = vert[0];
  glm::vec3 bMax = vert[0];
  for (vector<glm::vec3>::iterator it = vert.begin(); it != vert.end(); it++) {
    if (it->x < bMin.x) {
      bMin.x = it->x;
    }
    if (it->y < bMin.y) {
      bMin.y = it->y;
    }
    if (it->z < bMin.z) {
      bMin.z = it->z;
    }
    if (it->x > bMax.x) {
      bMax.x = it->x;
    }
    if (it->y > bMax.y) {
      bMax.y = it->y;
    }
    if (it->z > bMax.z) {
      bMax.z = it->z;
    }
  }
  glm::vec3 rDiv = glm::vec3(1.0 / (worldC - eye).x,
                             1.0 / (worldC - eye).y,
                             1.0 / (worldC - eye).z);
  if (rDiv.x >= 0) {
    tmin = (bMin.x - eye.x) * rDiv.x;
    tmax = (bMax.x - eye.x) * rDiv.x;
  }
  else {
    tmin = (bMax.x - eye.x) * rDiv.x;
    tmax = (bMin.x - eye.x) * rDiv.x;
  }
  if (rDiv.y >= 0) {
    tymin = (bMin.y - eye.y) * rDiv.y;
    tymax = (bMax.y - eye.y) * rDiv.y;
  }
  else {
    tymin = (bMax.y - eye.y) * rDiv.y;
    tymax = (bMin.y - eye.y) * rDiv.y;
  }


  if ((tmin > tymax) || (tymin > tmax)) {
    return Intersection(0, false, glm::vec3(), glm::vec3());
  }

  glm::vec3 n;
  if(tymin > tmin) {
    n = glm::vec3(0, 1, 0);
    tmin = tymin;
  }
  if(tymax < tmax) {
    tmax = tymax;
  }

  if (rDiv.z >= 0) {
    tzmin = (bMin.z - eye.z) * rDiv.z ;
    tzmax = (bMax.z - eye.z) * rDiv.z ;
  }
  else {
    tzmin = (bMax.z - eye.z) * rDiv.z ;
    tzmax = (bMin.z - eye.z) * rDiv.z ;
  }

  if ((tmin > tzmax) || (tzmin > tmax)) {
    return Intersection(0, false, glm::vec3(), glm::vec3());
  }
  if(tzmin > tmin) {
    n = glm::vec3(0, 0, 1);
    tmin = tzmin;
  }
  if(tzmax < tmax) {
    tmax = tzmax;
  }

  if (tmin > tmax) {
    n = glm::vec3(1, 0, 0);
  }
  if (tmin < tmax) {
  }
  return Intersection(tmin, true, (eye + (float)tmin * (worldC - eye)), n);

}

Sphere::Sphere() {
  nhSphere = new NonhierSphere(glm::vec3(0.0, 0.0, 0.0), 1.0);
}

Sphere::~Sphere()
{
}

Intersection Sphere::Intersect(const glm::vec3 & eye, const glm::vec3 & worldC) {
  return nhSphere->Intersect(eye, worldC);
}

Cube::Cube() {
  nhBox = new NonhierBox(glm::vec3(0.0, 0.0, 0.0), 1.0);
}

Cube::~Cube()
{
}

Intersection Cube::Intersect(const glm::vec3 & eye, const glm::vec3 & worldC) {
  return nhBox->Intersect(eye, worldC);
}

Intersection intersectTriangle(const glm::vec3 & eye, const glm::vec3 & worldC, glm::vec3 u, glm::vec3 v, glm::vec3 w) {
    glm::vec3 a = u - v;
    glm::vec3 b = u - w;
    glm::vec3 c = worldC - eye;
    glm::vec3 d = u - eye;

    double e = a.x * (b.y * c.z - c.y * b.z) + a.y * (c.x * b.z - b.x * c.z) + a.z * (b.x * c.y - b.y * c.x);
    double f = (d.x * (b.y * c.z - c.y * b.z) + d.y * (c.x * b.z - b.x * c.z) + d.z * (b.x * c.y - b.y * c.x)) / e;
    double g = (c.z * (a.x * d.y - d.x * a.y) + c.y * (d.x * a.z - a.x * d.z) + c.x * (a.y * d.z - d.y * a.z)) / e;
    double h = -(b.z * (a.x * d.y - d.x * a.y) + b.y * (d.x * a.z - a.x * d.z) + b.x * (a.y * d.z - d.y * a.z)) / e;
    glm::vec3 n = glm::cross(v - u, w - u);

    if ((h >= 0.00001 && f >= 0.0 && g >= 0.0 && (f + g) <= 1.0)) {
      return Intersection(h, true,
        
        (eye + (float)h * (worldC - eye)),
        n);
    }
    return Intersection(0, false, glm::vec3(), glm::vec3());
}
