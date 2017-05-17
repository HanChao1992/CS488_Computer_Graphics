#include <glm/ext.hpp>
#include <iostream>
#include <math.h>
#include <time.h>
#include "A5.hpp"
#include "GeometryNode.hpp"
#include "Perlin.hpp"
#include "PhongMaterial.hpp"
#include <algorithm>
#include <glm/gtx/io.hpp>


using namespace std;

float PI = 3.1415926535;
#define sample 3


// initialize 3 texture images
Image texture1, texture2, texture3, texture4;

// determine whether a color is too distinct
// used in adaptive sampling
int colorClose(vector<glm::vec3> colorVec) {
	glm::vec3 avgColor = (colorVec[0] + colorVec[1] + colorVec[2] + colorVec[3]) / 4.0;
	float maxDiff = 0;
	int result = -1;
	for (int i = 0; i < 4; i++) {
		glm::vec3 diffColor = colorVec[i] - avgColor;
		float sumDiff = abs(diffColor.x) + abs(diffColor.y) + abs(diffColor.z);
		if (sumDiff >= 0.35 && sumDiff > maxDiff) {
			maxDiff = sumDiff;
			result = i;
		}
	}
	return result;
}

glm::vec3 worldMapping (
	glm::vec3 pixel, 
	const glm::vec3 eye, 
	const glm::vec3 view, 
	double fovy, 
	size_t w, 
	size_t h, 
	double focal
) {
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
	double help = tan(fovy / 2 * PI / 180);
	double coefficientW = 2 * help * focal / w;
	double coefficientH = 2 * help * focal / h;
	worldCoor.x = pixel.x * coefficientW + eye.x + view.x;
	worldCoor.y = pixel.y * coefficientH + eye.y + view.y;
	worldCoor.z = pixel.z;

	return worldCoor;
}

glm::vec3 lightRayColor(
	Intersection inter,
	SceneNode * root,
	const std::list<Light *> & lights,
	const glm::vec3 & hP,
	const glm::vec3 & ambient,
	float x,
	float y,
	size_t w,
	size_t h
) {
	glm::vec3 color = glm::vec3(0);
	glm::vec3 hitPoint = inter.v;
	SceneNode * fin = inter.cur;
	glm::vec3 norm = glm::normalize(inter.n);
	glm::vec3 kd, ks;
	int texture;
	double sh, bump;
	if (fin && fin->m_nodeType == NodeType::GeometryNode) {
		GeometryNode *gchild = static_cast<GeometryNode *>(fin);
		PhongMaterial *pchild = static_cast<PhongMaterial *>(gchild->m_material);
		// bump mapping
		bump = pchild->getBump();
		if (bump != 0) {
			double noCoX = noise(
				0.1 * double(hitPoint.x),
				0.1 * double(hitPoint.y),
				0.1 * double(hitPoint.z));
			double noCoY = noise(
				0.1 * double(hitPoint.y),
				0.1 * double(hitPoint.z),
				0.1 * double(hitPoint.x));
			double noCoZ = noise(
				0.1 * double(hitPoint.z),
				0.1 * double(hitPoint.x),
				0.1 * double(hitPoint.y));
			norm = (1.0f - bump) * norm + bump * glm::vec3(noCoX, noCoY, noCoZ);
			double temp = glm::dot(norm, norm);
			if (temp != 0.0) {
				temp = 1.0 / sqrt(temp);
				norm = temp * norm;
			}
		}

		// texture mapping
		texture = pchild->getTexture();
		if (texture != 0) {
			double mx, my;
			switch (texture) {
				case 1:
					mx = (norm[0] + 1) * w;
					if (mx > texture1.width()) {
						mx = texture1.width();
					}
					my = (norm[1] + 1) * h;
					if (my > texture1.height()) {
						my = texture1.height();
					}
					pchild->setkd(glm::vec3(
									texture1(mx, my, 0),
									texture1(mx, my, 1),
									texture1(mx, my, 2)));
					break;
				case 2:
					mx = (norm[0] + 1) * w;
					if (mx > texture2.width()) {
						mx = texture2.width();
					}
					my = (norm[1] + 1) * h;
					if (my > texture2.height()) {
						my = texture2.height();
					}
					pchild->setkd(glm::vec3(
									texture2(mx, my, 0),
									texture2(mx, my, 1),
									texture2(mx, my, 2)));
					break;
				case 3:
					mx = (norm[0] + 1) * w;
					if (mx > texture3.width()) {
						mx = texture3.width();
					}
					my = (norm[1] + 1) * h;
					if (my > texture3.height()) {
						my = texture3.height();
					}
					pchild->setkd(glm::vec3(
									texture3(mx, my, 0),
									texture3(mx, my, 1),
									texture3(mx, my, 2)));
					break;
				case 4: // for planar mapping
					mx = x / w * texture4.width();
					if (mx > texture4.width()) {
						mx = texture4.width();
					}
					my = y / h * texture4.height();
					if (my > texture4.height()) {
						my = texture4.height();
					}
					pchild->setkd(glm::vec3(
						texture4(mx, my, 0),
						texture4(mx, my, 1),
						texture4(mx, my, 2)));
					break;
			}
			pchild->setks(glm::vec3(0, 0, 0));
		}
		kd = pchild->getkd();
		ks = pchild->getks();
		sh = pchild->getShininess();
	}
	for (Light * light : lights) {

		// area light
		if (light->type == 1) {
			glm::vec3 lColor = light->colour;
			for (int i = 0; i < 6; i++) {
				glm::vec3 lSource = light->position;
				switch (i) {
					case 0:
						lSource.x = light->position.x + 6;
						break;
					case 1:
						lSource.x = light->position.x - 6;
						break;
					case 2:
						lSource.y = light->position.y + 6;
						break;
					case 3:
						lSource.y = light->position.y - 6;
						break;
					case 4:
						lSource.z = light->position.z + 6;
						break;
					case 5:
						lSource.z = light->position.z - 6;
						break;

				}
				lColor.x = light->colour.x / 6.0;
				lColor.y = light->colour.y / 6.0;
				lColor.z = light->colour.z / 6.0;

				bool isShadow = false;
				Intersection sns = Intersection(0, false, glm::vec3(), glm::vec3());
				glm::vec3 dir = glm::normalize(lSource - hitPoint);
				sns = root->Intersect(hitPoint + 0.0001 * dir, lSource);
				if (sns.hit && sns.root > 0.00001){
					isShadow = true;
				}

				if (!isShadow) {
					glm::vec3 lRay = glm::normalize(lSource - hitPoint);
					double costheta = glm::dot(lRay, norm);
					glm::vec3 eyeP = glm::normalize(hP - hitPoint);
					double dis = glm::length(lSource - hitPoint);
					glm::vec3 refl = glm::normalize(-lRay + 2.0f * costheta * norm);
					glm::vec3 inten = lColor / (float) (light->falloff[0] + light->falloff[1] * dis + light->falloff[2] * dis * dis);
					glm::vec3 diffuse = kd * fmax(abs(costheta), 0.0) * inten;
					glm::vec3 specular =  ks * pow(fmax(abs(glm::dot(refl,eyeP)), 0.0), sh) * inten;
					color += diffuse + specular;
				}

			}

		}
		// point light
		else {
			glm::vec3 lSource = light->position;
			bool isShadow = false;
			Intersection sns = Intersection(0, false, glm::vec3(), glm::vec3());
			glm::vec3 dir = glm::normalize(lSource - hitPoint);
			sns = root->Intersect(hitPoint + 0.0001 * dir, lSource);
			if (sns.hit && sns.root > 0.00001){
				isShadow = true;
			}

			if (!isShadow) {
				glm::vec3 lRay = glm::normalize(lSource - hitPoint);
				double costheta = glm::dot(lRay, norm);
				glm::vec3 eyeP = glm::normalize(hP - hitPoint);
				double dis = glm::length(lSource - hitPoint);
				glm::vec3 refl = glm::normalize(-lRay + 2.0f * costheta * norm);
				glm::vec3 inten = light->colour / (float) (light->falloff[0] + light->falloff[1] * dis + light->falloff[2] * dis * dis);
				glm::vec3 diffuse = kd * fmax(abs(costheta), 0.0) * inten;
				glm::vec3 specular =  ks * pow(fmax(abs(glm::dot(refl,eyeP)), 0.0), sh) * inten;
				color += diffuse + specular;
			}
		}
	}
  	color += kd * ambient;
	return color;
}




glm::vec3 rayTrace(
	const glm::vec3 & eye,
	const glm::vec3 & worldC,
	SceneNode * root,
	const std::list<Light *> & lights,
	uint x,
	uint y,
	size_t w,
	size_t h,
	const glm::vec3 & ambient
) {
	glm::vec3 color = glm::vec3(0);
	Intersection sn = Intersection(0, false, glm::vec3(), glm::vec3());
	double reflection, refraction;
	glm::vec3 hitPoint;
	glm::vec3 norm;
	sn = root->Intersect(eye, worldC);

	// background color
	if (!sn.hit){
		color.x = (double)2.5 * y / h;
		color.y = (double)1.6 * x / w;
		if((y < h/2 && x < w/2) || (y >= h/2 && x >= w/2)) {
			color.z = 1.0 + x/w;
		}
		else {
			color.z = y/h;
		}
	}

	if (sn.hit){
		SceneNode * fin = sn.cur;
		hitPoint = sn.v;
		norm = glm::normalize(sn.n);
		if (fin && fin->m_nodeType == NodeType::GeometryNode) {
			GeometryNode *gchild = static_cast<GeometryNode *>(fin);
			PhongMaterial *pchild = static_cast<PhongMaterial *>(gchild->m_material);
			reflection = pchild->getReflection();
			refraction = pchild->getRefraction();
		}

		// reflection
		if (reflection != 0) {
			glm::vec3 eyeRay = hitPoint - eye;
			glm::vec3 mrefl = glm::normalize(eyeRay - 2.0f * glm::dot(norm, eyeRay) * norm);
			glm::vec3 mreflP = hitPoint + mrefl;
			Intersection mreflI = root->Intersect(hitPoint + 0.0001 * mrefl, mreflP);
			if (mreflI.hit && mreflI.root > 0.001) {
				color += reflection * lightRayColor(mreflI, root, lights, hitPoint, ambient, x, y, w, h);
			}
		}

		// refraction
		if (refraction != 1.0) {
			double n = 1.0f / refraction;
			glm::vec3 eyeRay = hitPoint - eye;
			double costhetaf = glm::dot(norm, eyeRay);
			double sinT = n * n * (1.0 - costhetaf * costhetaf);

			if (sinT <= 1.0) {
				glm::vec3 refrRay = glm::normalize(n * eyeRay + (n * costhetaf - sqrt(1.0 - sinT)) * norm);
				Intersection refrI = root->Intersect(
					hitPoint
					+ 0.01 * refrRay
					,
					hitPoint + refrRay);
				if (refrI.hit && refrI.root > 0.00001) {
					n = refraction / 1.0f;
					glm::vec3 refrnorm = glm::normalize(refrI.n);
					glm::vec3 refrRay = refrI.v - hitPoint;
					costhetaf = glm::dot(refrnorm, refrRay);
					sinT = n * n * (1.0 - costhetaf * costhetaf);
					color += 0.7 * lightRayColor(refrI, root, lights, hitPoint, ambient, x, y, w, h);
					if (sinT <= 1.0) {
						glm::vec3 refrRay2 = glm::normalize(n * refrRay + (n* costhetaf - sqrt(1.0 - sinT)) * refrnorm);
						Intersection refrIR = root->Intersect(refrI.v + 0.01 * refrRay2, refrI.v + refrRay2);
						if (refrIR.hit && refrIR.root > 0.00001) {
							color += 0.7 * lightRayColor(refrIR, root, lights, refrI.v, ambient, x, y, w, h);
						}
					}
				}
			}
		}
		color += lightRayColor(sn, root, lights, eye, ambient, x, y, w, h);
	}
	return color;
}

glm::vec3 adaptiveSampling(
	float fac,
	int counter,
	const glm::vec3 & eye,
	const glm::vec3 & view,
	double fovy,
	SceneNode * root,
	const std::list<Light *> & lights,
	float x,
	float y,
	size_t w,
	size_t h,
	const glm::vec3 & ambient
) {
	glm::vec3 tempColor;
	glm::vec3 color;
	vector<glm::vec3> colorVec;
	float xa, ya;
	for (xa = x; xa <= x + fac; xa+= fac) {
		for (ya = y; ya <= y + fac; ya+= fac) {
			glm::vec3 worldC = worldMapping(glm::vec3(xa, ya, (eye.z + view.z)), eye, view, fovy, w, h, 1);
			tempColor = rayTrace(eye, worldC, root, lights, x, y, w, h, ambient);
			colorVec.push_back(tempColor);
		}
	}
	int index = colorClose(colorVec);
	float xaa, yaa;
	if (counter < 5) {
		switch(index) {
			case 0:
				xaa = x;
				yaa = y;
				color = (adaptiveSampling(fac / 2.0, counter + 1, eye, view, fovy, root, lights, xaa, yaa, w, h, ambient) +
				colorVec[1] + colorVec[2] + colorVec[3]) / 4.0;
				break;
		 	case 1:
				xaa = x;
				yaa = y + fac / 2.0;
				color = (adaptiveSampling(fac / 2.0, counter + 1, eye, view, fovy, root, lights, xaa, yaa, w, h, ambient) +
				colorVec[0] + colorVec[2] + colorVec[3]) / 4.0;
				break;
		 	case 2:
				xaa = x + fac / 2.0;
				yaa = y;
				color = (adaptiveSampling(fac / 2.0, counter + 1, eye, view, fovy, root, lights, xaa, yaa, w, h, ambient) +
				colorVec[1] + colorVec[0] + colorVec[3]) / 4.0;
				break;
		 	case 3:
				xaa = x + fac / 2.0;
				yaa = y + fac / 2.0;
				color = (adaptiveSampling(fac / 2.0, counter + 1, eye, view, fovy, root, lights, xaa, yaa, w, h, ambient) +
				colorVec[1] + colorVec[2] + colorVec[0]) / 4.0;
				break;
		 	case -1:
				color = (colorVec[1] + colorVec[2] + colorVec[0] + colorVec[3]) / 4.0;
				break;
		}
	}
	else {
		color = (colorVec[1] + colorVec[2] + colorVec[0] + colorVec[3]) / 4.0;
	}
	return color;
}


void A5_Render(
		// What to render
		SceneNode * root,

		// Image to write to, set to a given width and height
		Image & image,

		// Viewing parameters
		const glm::vec3 & eye,
		const glm::vec3 & view,
		const glm::vec3 & up,
		double fovy,
		double focal,
		int samp,
		// Lighting parameters
		const glm::vec3 & ambient,
		const std::list<Light *> & lights,
		// Texture parameter
    	const std::vector<std::string> & textureLib
) {

  // Fill in worldCtracing code here...

  std::cout << "Calling A5_Render(\n" <<
		  "\t" << *root <<
      "\t" << "Image(width:" << image.width() << ", height:" << image.height() << ")\n"
      "\t" << "eye:  " << glm::to_string(eye) << std::endl <<
		  "\t" << "view: " << glm::to_string(view) << std::endl <<
		  "\t" << "up:   " << glm::to_string(up) << std::endl <<
		  "\t" << "fovy: " << fovy << std::endl <<
			"\t" << "focal: " << focal << std::endl <<
			"\t" << "samp: " << samp << std::endl <<
      "\t" << "ambient: " << glm::to_string(ambient) << std::endl <<
		  "\t" << "lights{" << std::endl;

	for(const Light * light : lights) {
		std::cout << "\t\t" <<  *light << std::endl;
	}
	std::cout << "\t}" << std::endl;
	std::cout <<")" << std::endl;

	size_t h = image.height();
	size_t w = image.width();

	cout << endl;
	// load textures
	for (int i = 0; i < textureLib.size(); i++) {
		if (i == 4) { // limit to 3 texture files
			break;
		}
		switch(i) {
			case 0:
				if(texture1.loadPng(textureLib[i])) {
					cout << textureLib[i] << " loaded" << endl;
				}
				break;
			case 1:
				if(texture2.loadPng(textureLib[i])) {
					cout << textureLib[i] << " loaded" << endl;
				}
				break;
			case 2:
				if(texture3.loadPng(textureLib[i])) {
					cout << textureLib[i] << " loaded" << endl;
				}
				break;
			case 3:
				if(texture4.loadPng(textureLib[i])) {
					cout << textureLib[i] << " loaded" << endl;
				}
				break;
		}
	}

  	// information needed to count the time of rendering
	int count =  0;
	cout << endl << "Progress:" << endl << endl;
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
  	timeinfo = localtime ( &rawtime );
	cout << "Starting at " << asctime (timeinfo)  << endl;
	float pixcount = 0.0;

	// output message for sampling method or depth of field
	if (focal != 0.0) {
		cout << "The image will be renderd with Depth of Filed effect" << endl;
	}
	else {
		switch(samp) {
			case 0:
				cout << "The image will be renderd with no sampling method." << endl;
				break;
			case 1:
				cout << "The image will be renderd with super-sampling." << endl;
				break;
			case 2:
				cout << "The image will be renderd with adaptive-sampling." << endl;
				break;
			default:
				cout << "The image will be renderd with no sampling method." << endl;
		}
	}
	cout << endl;

	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {
			glm::vec3 color = glm::vec3(0);

			// rendering message
			if (pixcount < w * h - 1) {
			int per = (int) ((pixcount / (w * h)) * 100);
			cout << "\r" << "rendering... "<< per << "%" << flush;
			}
			else {
			cout << "\r" << "rendered... "<< 100 << "%" << flush;
			}
			pixcount = pixcount + 1.0;

			glm::vec3 worldC;
			float factor = 1.0f / (sample -1);

			// Depth of Field
			if (focal > 0) {
				worldC = worldMapping(glm::vec3(x, y, (eye.z + view.z)), eye, view, fovy, w, h, 1);
				worldC = eye + focal * (worldC - eye);
				for (int yf = -3; yf < 4; yf++) {
					for (int xf = -3; xf < 4; xf++) {
						glm::vec3 eyef = glm::vec3(eye.x + xf*3, eye.y + yf*3, eye.z);
						color += rayTrace(eyef, worldC, root, lights, x, y, w, h, ambient);
					}
				}
				image(x,y,0) = color.x/36;
				image(x,y,1) = color.y/36;
				image(x,y,2) = color.z/36;
			}

			else {
				
			// sampling options
				switch (samp) {
					case 0: // no sampling method
						worldC = worldMapping(glm::vec3(x, y, (eye.z + view.z)), eye, view, fovy, w, h, 1);
						color = rayTrace(eye, worldC, root, lights, x, y, w, h, ambient);
						break;
					case 1: // super sampling
						float xs, ys;
						for (xs = x; xs <= x + 1; xs+= factor) {
							for (ys = y; ys <= y + 1; ys+= factor) {
								worldC = worldMapping(glm::vec3(xs, ys, (eye.z + view.z)), eye, view, fovy, w, h, 1);
								color += rayTrace(eye, worldC, root, lights, x, y, w, h, ambient);
							}
						}
						break;
					case 2: // adaptive sampling
						color = adaptiveSampling(1.0, 0, eye, view, fovy, root, lights, x, y, w, h, ambient);
						break;
					default:
						worldC = worldMapping(glm::vec3(x, y, (eye.z + view.z)), eye, view, fovy, w, h, 1);
						color = rayTrace(eye, worldC, root, lights, x, y, w, h, ambient);
				}

				// coloring
				switch(samp) {
					case 0:
						image(x,y,0) = color.x;
						image(x,y,1) = color.y;
						image(x,y,2) = color.z;
						break;
					case 1:
						image(x,y,0) = color.x/(sample * sample);
						image(x,y,1) = color.y/(sample * sample);
						image(x,y,2) = color.z/(sample * sample);
						break;
					case 2:
						image(x,y,0) = color.x;
						image(x,y,1) = color.y;
						image(x,y,2) = color.z;
						break;
					default:
						image(x,y,0) = color.x;
						image(x,y,1) = color.y;
						image(x,y,2) = color.z;
				}
			}
		}
	}

	// out put time information
	time_t nrawtime;
	cout << endl << endl;
	time ( &nrawtime );
  	timeinfo = localtime ( &nrawtime );
	cout << "Finishing at " << asctime (timeinfo)  << endl;
	cout << "Used " << nrawtime - rawtime << " seconds" << endl;

}
