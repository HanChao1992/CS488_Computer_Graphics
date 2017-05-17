#include <iostream>
#include <fstream>

#include <glm/ext.hpp>

// #include "cs488-framework/ObjFileDecoder.hpp"
#include "Mesh.hpp"

// To turn off acceleration, set BV and BVS to 0.
// Set BV = 1 for acceleration only.
// Set BVS = 1 and BV = 0 to see the Bounding Box.
int BV = 1;
int BVS = 0;

Mesh::Mesh( const std::string& fname )
	: m_vertices()
	, m_faces()
{
	std::string code;
	double vx, vy, vz;
	size_t s1, s2, s3;

	std::ifstream ifs( fname.c_str() );
	while( ifs >> code ) {
		if( code == "v" ) {
			ifs >> vx >> vy >> vz;
			m_vertices.push_back( glm::vec3( vx, vy, vz ) );
		} else if( code == "f" ) {
			ifs >> s1 >> s2 >> s3;
			m_faces.push_back( Triangle( s1 - 1, s2 - 1, s3 - 1 ) );
		}
	}
	bBox = new BoundingVolume(m_vertices);
}

std::ostream& operator<<(std::ostream& out, const Mesh& mesh)
{
  out << "mesh {";
  /*

  for( size_t idx = 0; idx < mesh.m_verts.size(); ++idx ) {
  	const MeshVertex& v = mesh.m_verts[idx];
  	out << glm::to_string( v.m_position );
	if( mesh.m_have_norm ) {
  	  out << " / " << glm::to_string( v.m_normal );
	}
	if( mesh.m_have_uv ) {
  	  out << " / " << glm::to_string( v.m_uv );
	}
  }

*/
  out << "}";
  return out;
}

Intersection Mesh::Intersect(const glm::vec3 & eye, const glm::vec3 & worldC) {
	if(BV) {
		Intersection inB = bBox->Intersect(eye, worldC);
		if (!inB.hit) {
			return Intersection(0, false, glm::vec3(), glm::vec3());
		}
	}
	else if(BVS) {
		return bBox->Intersect(eye, worldC);
	}
	double minRoot = INFINITY;
	Intersection in = Intersection(0, false, glm::vec3(), glm::vec3());
	for (int i = 0; i < m_faces.size(); i++) {
		Intersection inter = intersectTriangle(eye, worldC,
			m_vertices[m_faces[i].v1],
			m_vertices[m_faces[i].v2],
			m_vertices[m_faces[i].v3]);
		if (inter.hit && minRoot > inter.root) {
			minRoot = inter.root;
			in = inter;
		}
	}
	return in;
}
