#include "GeometryNode.hpp"
#include <iostream>
#include <glm/gtx/io.hpp>

using namespace std;
//---------------------------------------------------------------------------------------
GeometryNode::GeometryNode(
	const std::string & name, Primitive *prim, Material *mat )
	: SceneNode( name )
	, m_material( mat )
	, m_primitive( prim )
{
	m_nodeType = NodeType::GeometryNode;
}


void GeometryNode::setMaterial( Material *mat )
{
	// Obviously, there's a potential memory leak here.  A good solution
	// would be to use some kind of reference counting, as in the
	// C++ shared_ptr.  But I'm going to punt on that problem here.
	// Why?  Two reasons:
	// (a) In practice we expect the scene to be constructed exactly
	//     once.  There's no reason to believe that materials will be
	//     repeatedly overwritten in a GeometryNode.
	// (b) A ray tracer is a program in which you compute once, and
	//     throw away all your data.  A memory leak won't build up and
	//     crash the program.

	m_material = mat;
}


Intersection GeometryNode::Intersect(const glm::vec3 & eye, const glm::vec3 & worldC) {
	glm::vec4 eyeH = get_inverse() * glm::vec4(eye.x, eye.y, eye.z, 1);
  glm::vec4 worldCH = get_inverse() * glm::vec4(worldC.x, worldC.y, worldC.z, 1);
	Intersection h1 = m_primitive->Intersect(glm::vec3(eyeH.x, eyeH.y, eyeH.z),
										          							glm::vec3(worldCH.x, worldCH.y, worldCH.z));
	if (h1.hit) {
		glm::vec4 vrhelp = get_transform() * glm::vec4(h1.v.x, h1.v.y, h1.v.z, 1);
		h1.v = glm::vec3(vrhelp.x, vrhelp.y, vrhelp.z);
		h1.n = glm::transpose(glm::mat3(get_inverse())) * h1.n;
		h1.cur = this;
	}
	return h1;

}
