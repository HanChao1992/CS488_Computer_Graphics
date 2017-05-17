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
      double eps = 0.01;
      if ((roots[1] < eps) && (roots[0] < eps)) {
        return Intersection(0, false, worldC, glm::vec3());
      }
      else if (roots[0] < eps && roots[1] > eps) {
        root = roots[1];
      }
      else if (roots[1] < eps && roots[0] > eps) {
        root = roots[0];
      }
      else if (roots[0] > eps && roots[1] > eps && roots[1] < roots[0]) {
        root = roots[1];
      }
      else if (roots[0] > eps && roots[1] > eps && roots[0] < roots[1]) {
        root = roots[0];
      }
    }
  }
  pHit = eye + (float)root * (worldC - eye);
  normal = pHit - m_pos;

  return Intersection(root, true, pHit, normal);
}



NonhierBox::~NonhierBox()
{
}

Intersection NonhierBox::Intersect(const glm::vec3 & eye, const glm::vec3 & worldC) {
  return bBox->Intersect(eye,worldC);
}

Cylinder::~Cylinder()
{
}

Intersection Cylinder::Intersect(const glm::vec3 & eye, const glm::vec3 & worldC) {
  glm::vec3 dir = worldC - eye;
  Intersection circle0 = Intersection(INFINITY, false, glm::vec3(), glm::vec3());
  double root0;
  if (dir.y != 0) {
    root0 = (0 - eye.y) / dir.y;
  }
  if (root0 >= 0.0001) {
    glm::vec3 help = eye + (float)root0 * dir;
    if ((help.x * help.x + help.z * help.z) <= 1) {
      circle0.hit = true;
      circle0.root = root0;
      circle0.v = help;
      circle0.n = glm::vec3(0.0, -1.0, 0.0);
    }
  }
  Intersection circle1 = Intersection(INFINITY, false, glm::vec3(), glm::vec3());
  double root1;
  if (dir.y != 0) {
    root1 = (1 - eye.y) / dir.y;
  }
  if (root1 >= 0.0001) {
    glm::vec3 help1 = eye + (float)root1 * dir;
    if ((help1.x * help1.x + help1.z * help1.z) <= 1) {
      circle1.hit = true;
      circle1.root = root1;
      circle1.v = help1;
      circle1.n = glm::vec3(0.0, 1.0, 0.0);
    }
  }
  Intersection cylinder = Intersection(INFINITY, false, glm::vec3(), glm::vec3());
  double roots[2];

  double a = pow(dir.x, 2) + pow(dir.z, 2);
  double b = 2 * (dir.x * eye.x + dir.z * eye.z);
  double c = pow(eye.x, 2) + pow(eye.z, 2) -1;
  int rn = quadraticRoots(a, b, c, roots);
  if (rn > 0) {
    double rootc = roots[0];
    if (rn == 2) {
      rootc = fmin(roots[1], roots[0]);
    }
    if (rootc >= 0.0001) {
      glm::vec3 helpc = eye + (float)rootc * dir;
      if (helpc.y <= 1 && helpc.y >= 0.0001) {
        cylinder.hit =  true;
        cylinder.root = rootc;
        cylinder.v = helpc;
        cylinder.n = glm::vec3(helpc.x, 0.0, helpc.z);
      }
    }
  }
  if (cylinder.root <= circle0.root && cylinder.root <= circle1.root) {
    return cylinder;
  }
  if (cylinder.root >= circle0.root && circle0.root <= circle1.root) {
    return circle0;
  }
  if (circle0.root >= circle1.root && cylinder.root >= circle1.root ) {
    return circle1;
  }
  return cylinder;
}

Cone::~Cone()
{
}

Intersection Cone::Intersect(const glm::vec3 & eye, const glm::vec3 & worldC) {
  Intersection cone = Intersection(0, false, glm::vec3(), glm::vec3());
  glm::vec3 dir = worldC - eye;
  double roots[2];
  double a = pow(dir.x, 2) - pow(dir.y, 2) + pow(dir.z, 2);
  double b = 2 * (dir.x * eye.x - dir.y * eye.y + dir.z * eye.z);
  double c = pow(eye.x, 2) - pow(eye.y, 2) + pow(eye.z, 2);
  int rn = quadraticRoots(a, b, c, roots);

  if (rn > 0) {
    double rootc = roots[0];
    if (rn == 2) {
      if (roots[0] >= 0.0001 || roots[1] >= 0.0001) {
        rootc = fmin(roots[1], roots[0]);
      }
    }
    if (rootc >= 0) {
      glm::vec3 helpc = eye + (float)rootc * dir;
      if (helpc.y <= 1 && helpc.y >= 0.0001) {
        glm::vec3 norm = glm::vec3((helpc.x/2, -0.5, sqrt(1 - helpc.x * helpc.x) /2));
        norm = glm::normalize(norm);
        cone.hit =  true;
        cone.root = rootc;
        cone.v = helpc;
        cone.n = norm;
      }
    }
  }

  Intersection circle1 = Intersection(INFINITY, false, glm::vec3(), glm::vec3());
  double root1;
  if (dir.y != 0) {
    root1 = (1 - eye.y) / dir.y;
  }
  if (root1 >= 0) {
    glm::vec3 help1 = eye + (float)root1 * dir;
    if ((help1.x * help1.x + help1.z * help1.z) <= 1) {
      circle1.hit = true;
      circle1.root = root1;
      circle1.v = help1;
      circle1.n = glm::vec3(0.0, 1.0, 0.0);
    }
  }
  if (!cone.hit) {
    return circle1;
  }
  if (!circle1.hit) {
    return cone;
  }
  if (circle1.root <= cone.root) {
    return circle1;
  }
  return cone;
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

  if (rDiv.z >= 0) {
    tzmin = (bMin.z - eye.z) * rDiv.z ;
    tzmax = (bMax.z - eye.z) * rDiv.z ;
  }
  else {
    tzmin = (bMax.z - eye.z) * rDiv.z ;
    tzmax = (bMin.z - eye.z) * rDiv.z ;
  }


  double roott = fmin(fmin(tmax, tymax), fmin(tymax, tzmax));
  double root = fmax(fmax(tmin, tymin), fmax(tymin, tzmin));

   glm::vec3 n;
   double EPS = 0.00001;



   if (root > tmin - EPS && root < tmin + EPS) {
     n = glm::vec3(1, 0, 0);
   }
   if (root > tmax - EPS && root < tmax + EPS) {
     n = glm::vec3(-1, 0, 0);
   }
   if (root > tymin - EPS && root < tymin + EPS) {
     n = glm::vec3(0, 1, 0);
   }
   if (root > tymax - EPS && root < tymax + EPS) {
     n = glm::vec3(0, -1, 0);
   }
   if (root > tzmin - EPS && root < tzmin + EPS) {
     n = glm::vec3(0, 0, 1);
   }
   if (root > tzmax - EPS && root < tzmax + EPS) {
     n = glm::vec3(0, 0, -1);
   }
   if (root > roott || root < 0) {
     return Intersection(0, false, glm::vec3(), glm::vec3());
   }
   return Intersection(root, true, (eye + (float)root * (worldC - eye)), n);

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
