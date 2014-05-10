/*
 * Primitives.h
 *
 * Includes:
 * 	vec3 ( 3 float vector)
 * 	Primitive ( base class for primitives)
 * 		Triangle
 * 		Sphere
 * 	Light
 * 	RGB
 */

#ifndef PRIMITIVES_H_
#define PRIMITIVES_H_

#include <math.h>
#include <fstream>
#include <iostream>
#include <assert.h>

#include <glm/glm.hpp>

using namespace std;
using namespace glm;

//RGB Struct from provided code
typedef struct
{
  float r,g,b;
} RGB;

typedef struct
{
	int type;			//0 sphere 1 triangle
	float Kdr, Kdg, Kdb;
	float Kar, Kag, Kab;
	float Ks, nSpec;

	float radius;

	float v1x, v1y, v1z;
	float v2x, v2y, v2z;
	float v3x, v3y, v3z;

} p_struct;



//Image class from provided code
class Image
{
	private:
		int height, width; // resolution
		RGB *rgb;        // pixel intensities
	public:
		Image ( int m, int n );       // allocates image of specified size
		RGB &getPixel ( int i, int j );  // access to a specific pixel
		void saveToPPM ( char *filename );
};
/* ----------- image class: methods ---------- */

typedef struct
{
	vec3 loc;
	float I;
	float ambientI;

} Light;

//Class for a basice primitve
class Primitive
{
	public:
		//Diffuse constants
		float Kdr,Kdg,Kdb;
		//Ambient constants
		float Kar,Kag,Kab;
		//Specular constants
		float Ks,nSpec;

		Primitive();
		Primitive(float kdr,float kdg,float kdb,
				float kar, float kag,float kab,
				float ks, float nspec);
		virtual ~Primitive();

		RGB getLight(vec3 point, vec3 cop, Light l, bool inShadow);
		virtual vec3 Normal(vec3 point, vec3 eye);
		virtual float Intersection(vec3 origin, vec3 ray, vec3 &intersect);

		virtual p_struct getStruct();
};

class Sphere : public Primitive
{
	public:
		vec3 center;
		float radius;

		Sphere();
		Sphere(float kdr,float kdg,float kdb,
				float kar, float kag,float kab,
				float ks, float nspec, vec3 c, float r);
		~Sphere();

		vec3 Normal(vec3, vec3);
		float Intersection(vec3 origin, vec3 ray, vec3 &intersect);

		p_struct getStruct();
};

class Triangle : public Primitive
{
	public:
		vec3 a1, a2, a3;
		vec3 norVec;
		bool hasNormal;

		Triangle();
		Triangle(float kdr,float kdg,float kdb,
				float kar, float kag,float kab,
				float ks, float nspec,
				vec3,vec3,vec3);
		~Triangle();

		vec3 Normal(vec3, vec3);
		float Intersection(vec3 origin, vec3 ray, vec3 &intersect);

		p_struct getStruct();

};

#endif /* PRIMITIVES_H_ */
