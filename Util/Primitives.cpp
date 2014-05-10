
#include "Primitives.h"

Image::Image ( int m, int n ) : height(m), width(n)
{
  rgb = new RGB[m*n];
}
/* ----------------------- */

RGB &Image::getPixel ( int i, int j )
{
  return rgb[i+width*j];
}

/* ----------------------- */

static unsigned char clampnround ( float x )
{
  if (x>255)
    x = 255;
  if (x<0)
    x = 0;
  return (unsigned char)floor(x+.5);
}

/* ----------------------- */

void Image::saveToPPM ( char *filename )
{
  ofstream ofs(filename,ios::binary);
  assert(ofs);
  ofs << "P6" << endl;
  ofs << width << " " << height << endl << 255 << endl;
  for ( int i=0; i<width*height; i++ )
    {
      unsigned char r = clampnround(256*rgb[i].r);
      unsigned char g = clampnround(256*rgb[i].g);
      unsigned char b = clampnround(256*rgb[i].b);
      ofs.write((char*)&r,sizeof(char));
      ofs.write((char*)&g,sizeof(char));
      ofs.write((char*)&b,sizeof(char));
    }
}


Triangle::Triangle(float kdr,float kdg,float kdb,
				float kar, float kag,float kab,
				float ks, float nspec,
				vec3 p1, vec3 p2, vec3 p3) :
			Primitive(kdr, kdg, kdb,
				kar, kag, kab,
				ks, nspec),
			a1(p1), a2(p2), a3(p3)

{ hasNormal = false; }
Triangle::~Triangle()
{ }
Triangle::Triangle()
{
	hasNormal = false;
}

vec3 Triangle::Normal(vec3 point, vec3 eye)
{
	if(hasNormal)
	{
		return norVec;
	} else
	{
		vec3 a12 = a1 - a2;
		vec3 a13 = a3 - a1;

		norVec = cross(a12, a13);

		norVec = normalize(norVec);

		vec3 eyea1 =  eye - a1;

		if(dot(norVec, eyea1) < 0)
		{
			norVec = norVec * -1.0f;
		}
		hasNormal = true;

		return norVec;

	}

}
float Triangle::Intersection(vec3 origin, vec3 ray, vec3 &intersect)
{
	vec3 a1origin = a1 - origin;

	vec3 norm = cross((a2 - a1), (a3 - a1));

	float x = dot(a1origin, norm);
	float y = dot(ray, norm);

	float t = x / y;

	if( t < 0 )
	{
		return -1;
	} else
	{

		vec3 res = origin + ( ray * t);

		vec3 v1 = cross((this->a1 - res), (this->a2 - res));
		vec3 v2 = cross((this->a2 - res), (this->a3 - res));
		vec3 v3 = cross((this->a3 - res), (this->a1 - res));

		if(dot(v1, v2) > 0 && dot(v2, v3) > 0 && dot(v3,v1) > 0)
		{

			intersect.x = res.x;
			intersect.y = res.y;
			intersect.z = res.z;

			return t;
		}
		return -1;
	}

}

p_struct Triangle::getStruct()
{
	p_struct temp;
	temp.type = 1;

	temp.nSpec = this->nSpec;
	temp.Ks = this->Ks;

	temp.radius = 0.0;

	temp.v1x = this->a1.x;
	temp.v1y = this->a1.y;
	temp.v1z = this->a1.z;

	temp.v2x = this->a2.x;
	temp.v2y = this->a2.y;
	temp.v2z = this->a2.z;

	temp.v3x = this->a3.x;
	temp.v3y = this->a3.y;
	temp.v3z = this->a3.z;


	temp.Kdr = this->Kdr;
	temp.Kdg = this->Kdg;
	temp.Kdb = this->Kdb;

	temp.Kar = this->Kar;
	temp.Kag = this->Kag;
	temp.Kab = this->Kab;

	return temp;
}

p_struct Sphere::getStruct()
{
	p_struct temp;
	temp.type = 0;

	temp.nSpec = this->nSpec;
	temp.Ks = this->Ks;

	temp.radius = this->radius;

	temp.v1x = this->center.x;
	temp.v1y = this->center.y;
	temp.v1z = this->center.z;

	temp.v2x = 0;
	temp.v2y = 0;
	temp.v2z = 0;

	temp.v3x = 0;
	temp.v3y = 0;
	temp.v3z = 0;

	temp.Kdr = this->Kdr;
	temp.Kdg = this->Kdg;
	temp.Kdb = this->Kdb;

	temp.Kar = this->Kar;
	temp.Kag = this->Kag;
	temp.Kab = this->Kab;

	return temp;


}


Sphere::Sphere(float kdr,float kdg,float kdb,
				float kar, float kag,float kab,
				float ks, float nspec, vec3 c, float r) :
			Primitive(kdr, kdg, kdb,
				kar, kag, kab,
				ks, nspec),
			center(c), radius(r)

{

}

Sphere::~Sphere()
{ }
Sphere::Sphere()
{
	radius = 0;
}

vec3 Sphere::Normal(vec3 point, vec3 eye)
{
	vec3 norm = point - center;
	norm = normalize(norm);

	return norm;
}

float Sphere::Intersection(vec3 origin, vec3 ray, vec3 &intersect)
{

	vec3 co = origin - this->center;

	float c = pow(length(co), 2) - pow(this->radius, 2);
	float b = 2 * dot(co, ray);
	float a = pow(length(ray), 2);

	float delta = pow(b, 2) - (4 * a * c);

	if(delta < 0)
	{
		return -1;
	}else if (delta == 0)
	{
		float t = (-1 * b) / (2 * a);
		vec3 res = origin + ( ray * t);

		intersect.x = res.x;
		intersect.y = res.y;
		intersect.z = res.z;
		return t;
	} else
	{
		float t1 = ((-1 * b) - sqrt(delta)) / (2 * a);
		float t2 = ((-1 * b) + sqrt(delta)) / (2 * a);

		if(t1 < 0 && t2 < 0)
			return -1;
		if((t1 > 0 && t2 < 0) || (t1 > 0 && t1 < t2))
		{
			vec3 res = origin + ( ray * t1);

			intersect.x = res.x;
			intersect.y = res.y;
			intersect.z = res.z;

			return t1;
		} else if((t1 < 0 && t2 > 0) || (t2 > 0 && t1 > t2))
		{
			vec3 res = origin + ( ray * t2);

			intersect.x = res.x;
			intersect.y = res.y;
			intersect.z = res.z;

			return t2;
		}
	}
	return -1;
}

Primitive::Primitive(float kdr,float kdg,float kdb,
				float kar, float kag,float kab,
				float ks, float nspec) :
	Kdr(kdr), Kdg(kdg), Kdb(kdb),
	Kar(kar), Kag(kag), Kab(kab),
	Ks(ks), nSpec(nspec)
{ }
Primitive::~Primitive()
{ }
Primitive::Primitive()
{
	Kdr = 0; Kdg = 0; Kdb = 0;
	Kar = 0; Kag = 0; Kab = 0;
	Ks = 0; nSpec = 0;


}
p_struct Primitive::getStruct()
{
	p_struct temp;

	return temp;
}

//Normal function overridden by sub class
vec3 Primitive::Normal(vec3 point, vec3 eye)
{
	return eye;
}
//Intersection function overridden by subclass
float Primitive::Intersection(vec3 origin, vec3 ray, vec3 &intersect)
{
	return -1;
}
//Calculates the light color for a given point
RGB Primitive::getLight(vec3 point,vec3 eye, Light light, bool inShadow)
{
	RGB color;

	color.r = light.ambientI * Kar;
	color.g = light.ambientI * Kag;
	color.b = light.ambientI * Kab;

	if(inShadow)
	{
		return color;
	}

	vec3 normal = Normal(point,eye);

	vec3 L = light.loc - point;
	L = normalize(L);
	vec3 V = eye - point;
	V = normalize(V);
	vec3 H = V + L;
	H = normalize(H);

	color.r += light.I * ( Kdr * dot(normal,L) + Ks * pow(dot(normal,H), nSpec));
	color.g += light.I * ( Kdg * dot(normal,L) + Ks * pow(dot(normal,H), nSpec));
	color.b += light.I * ( Kdb * dot(normal,L) + Ks * pow(dot(normal,H), nSpec));

	return color;
}








