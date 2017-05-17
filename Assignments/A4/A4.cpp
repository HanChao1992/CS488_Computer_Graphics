#include <glm/ext.hpp>
#include <iostream>
#include <math.h>
#include <time.h>
#include "A4.hpp"
#include "GeometryNode.hpp"
#include "PhongMaterial.hpp"
#include <algorithm>
#include <glm/gtx/io.hpp>

using namespace std;

float PI = 3.1415926535;
#define sample 3

glm::vec3 worldMapping (glm::vec3 pixel, const glm::vec3 eye, const glm::vec3 view, double fovy, size_t w, size_t h) {
	glm::vec3 worldCoor;
	double wOffset = w/2;
	double hOffset = h/2;
	pixel.x -= wOffset;
	if (pixel.y <= hOffset)	{
		pixel.y = hOffset - pixel.y;
	}
	else {
		pixel.y = -(pixel.y - hOffset);
	}
	double help = abs(view.z) * tan(fovy / 2 * PI / 180);
	double coefficientW = 2 * help / w;
	double coefficientH = 2 * help / h;
	worldCoor.x = pixel.x * coefficientW;
	worldCoor.y = pixel.y * coefficientH + eye.y;
	worldCoor.z = pixel.z;

	return worldCoor;
}

void A4_Render(
		// What to render
		SceneNode * root,

		// Image to write to, set to a given width and height
		Image & image,

		// Viewing parameters
		const glm::vec3 & eye,
		const glm::vec3 & view,
		const glm::vec3 & up,
		double fovy,

		// Lighting parameters
		const glm::vec3 & ambient,
		const std::list<Light *> & lights
) {

  // Fill in worldCtracing code here...

  std::cout << "Calling A4_Render(\n" <<
		  "\t" << *root <<
      "\t" << "Image(width:" << image.width() << ", height:" << image.height() << ")\n"
      "\t" << "eye:  " << glm::to_string(eye) << std::endl <<
		  "\t" << "view: " << glm::to_string(view) << std::endl <<
		  "\t" << "up:   " << glm::to_string(up) << std::endl <<
		  "\t" << "fovy: " << fovy << std::endl <<
      "\t" << "ambient: " << glm::to_string(ambient) << std::endl <<
		  "\t" << "lights{" << std::endl;

	for(const Light * light : lights) {
		std::cout << "\t\t" <<  *light << std::endl;
	}
	std::cout << "\t}" << std::endl;
	std::cout <<")" << std::endl;

	size_t h = image.height();
	size_t w = image.width();

	int count =  0;
	cout << endl << "Progress:" << endl << endl;
        time_t rawtime;
  	struct tm * timeinfo;
	time ( &rawtime );
  	timeinfo = localtime ( &rawtime );
	cout << "Starting at " << asctime (timeinfo)  << endl;
    
    	float pixcount = 0.0;
	for (uint y = 0; y < h; ++y) {
		
		for (uint x = 0; x < w; ++x) {
			if (pixcount < w * h - 1) {
			int per = (int) ((pixcount / (w * h)) * 100);
			cout << "\r" << "rendering... "<< per << "%" << flush;
			}
			else {
			cout << "\r" << "rendered... "<< 100 << "%" << flush;
			}
			pixcount = pixcount + 1.0;
			


			image(x, y, 0) = (double)2.5 * y / h;
			image(x, y, 1) = (double)1.6 * x / w;
			//image(x, y ,2) = 2.5 * (x + y) / (h + w);

  			if((y < h/2 && x < w/2) || (y >= h/2 && x >= w/2)) {
                          image(x, y ,2) = 1.0 + x/w;
			}
                        else {
                          image(x, y ,2) = 0.0 + y/h;
			}
			float xs, ys;
			float help = 1;
			glm::vec3 color = glm::vec3(0);
			float factor = 1.0f / (sample -1);
			Intersection sn = Intersection(0, false, glm::vec3(), glm::vec3());
			glm::vec3 kd, ks;
			double sh;
			for (xs = x; xs <= x + 1; xs+= factor) {
				for (ys = y; ys <= y + 1; ys+= factor) {
					glm::vec3 worldC = worldMapping(glm::vec3(xs, ys, (eye.z + view.z)), eye, view, fovy, w, h);
					glm::vec3 vr;
					glm::vec3 norm;
					sn = root->Intersect(eye, worldC);
					if (!sn.hit) {
						continue;
					}

					if (sn.hit) {
						vr = sn.v;
						SceneNode * fin = sn.cur;
						norm = glm::normalize(sn.n);
						if (fin && fin->m_nodeType == NodeType::GeometryNode) {
							GeometryNode *gchild = static_cast<GeometryNode *>(fin);
							PhongMaterial *pchild = static_cast<PhongMaterial *>(gchild->m_material);
							kd = pchild->getkd();
							ks = pchild->getks();
							sh = pchild->getShininess();
						}

						for (Light * light : lights) {
							glm::vec3 lSource = light->position;
							bool isShadow = false;
							Intersection sns = Intersection(0, false, glm::vec3(), glm::vec3());
							sns = root->Intersect(vr, lSource);
							if (sns.hit && sns.root > 0.000001 && sns.root < 0.999999) {
								isShadow = true;
							}

							if (!isShadow) {
								glm::vec3 lRay = glm::normalize(lSource - vr);
								double costheta = glm::dot(lRay, norm);
								glm::vec3 eyeP = glm::normalize(eye - vr);
								double dis = glm::length(lSource - vr);
								glm::vec3 refl = glm::normalize(-lRay + 2.0f * costheta * norm);
								glm::vec3 inten = light->colour / (float) (light->falloff[0] + light->falloff[1] * dis + light->falloff[2] * dis * dis);
								glm::vec3 diffuse = kd * fmax(costheta, 0.0) * inten;
								glm::vec3 specular =  ks * pow(fmax(glm::dot(refl,eyeP), 0.0), sh) * inten;
            		color += diffuse + specular;
							}
						}
					}
				}
			}
			color += kd * ambient;
			if (sn.hit) {
				image(x,y,0) = color.x/(sample * sample);
				image(x,y,1) = color.y/(sample * sample);
				image(x,y,2) = color.z/(sample * sample);
			}
		}
	}
	time_t nrawtime;
	cout << endl << endl;
	time ( &nrawtime );
  	timeinfo = localtime ( &nrawtime );
	cout << "Finishing at " << asctime (timeinfo)  << endl;
	cout << "Used " << nrawtime - rawtime << " seconds" << endl;
      
}
