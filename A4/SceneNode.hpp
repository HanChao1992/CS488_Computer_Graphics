#pragma once

#include "Material.hpp"

#include <glm/glm.hpp>

#include <list>
#include <string>
#include <iostream>



/*
class Intersect {
public:
  bool exist();
private:
  bool exist;
}
*/
class Intersection;

enum class NodeType {
	SceneNode,
	GeometryNode,
	JointNode
};

class SceneNode {
public:
	virtual Intersection Intersect(const glm::vec3 & eye, const glm::vec3 & worldC);
	SceneNode(const std::string & name);
	SceneNode(const SceneNode & other);
	virtual ~SceneNode();
	int totalSceneNodes() const;

  const glm::mat4& get_transform() const;
  const glm::mat4& get_inverse() const;

  void set_transform(const glm::mat4& m);

  void add_child(SceneNode* child);

  void remove_child(SceneNode* child);

	//-- Transformations:
  void rotate(char axis, float angle);
  void scale(const glm::vec3& amount);
  void translate(const glm::vec3& amount);


	friend std::ostream & operator << (std::ostream & os, const SceneNode & node);

    // Transformations
    glm::mat4 trans;
    glm::mat4 invtrans;

    std::list<SceneNode*> children;

	NodeType m_nodeType;
	std::string m_name;
	unsigned int m_nodeId;

private:
	// The number of SceneNode instances.
	static unsigned int nodeInstanceCount;
};

class Intersection {
public:
  double root;
  bool hit;
  glm::vec3 v;
  glm::vec3 n;
  SceneNode * cur;
  Intersection(double root, bool hit, glm::vec3 v, glm::vec3 n) : root(root), hit(hit), v(v), n(n), cur(NULL) {}
};
