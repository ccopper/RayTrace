
#include "Scene.h"

Scene::Scene()
{

	primIndex = 0;

}
Scene::~Scene()
{
	//delete primList;
}

void Scene::allocPrimitive(int size)
{
	primList.resize(size);

}

void Scene::addPrimitive(Primitive *p)
{
	primList[primIndex] = p;

	primIndex++;

}

void Scene::render(Image &i)
{
	vec3 ulCorner = viewPort.llCorner + viewPort.screenHeight;


	for(int y = 0; y < viewPort.pxHeight; y++)
	{
		for(int x = 0; x < viewPort.pxWidth; x++)
		{

			float xFac = (x + 0.5) / viewPort.pxWidth;
			float yFac = (y + 0.5) / viewPort.pxHeight;

			vec3 cop = viewPort.screenWidth * xFac;
			cop = cop - (viewPort.screenHeight * yFac);
			cop = ulCorner + cop;

			vec3 ray = cop - viewPort.eyeLoc;

			float minT = -1,temp = 0;
			vec3 small, point;
			int sIndex = 0;

			for(int pi = 0; pi < primIndex; pi++)
			{

				temp = primList[pi]->Intersection(viewPort.eyeLoc, ray, point);


				if((minT == -1 && temp > 0) || (minT > temp && temp > 0))
				{
					minT = temp;
					small = point;
					sIndex = pi;
				}
			}

			if(minT > 0)
			{

				RGB color;

				bool inShad = inShadow(sIndex, small, ray);
				//bool inShad = false;

				color = primList[sIndex]->getLight(small, viewPort.eyeLoc, viewPort.lightSource, inShad);

				i.getPixel(x,y).r = color.r;
				i.getPixel(x,y).g = color.g;
				i.getPixel(x,y).b = color.b;

			} else
			{

				i.getPixel(x,y).r = 0;
				i.getPixel(x,y).g = 0;
				i.getPixel(x,y).b = 0;
			}
		}
	}

}
bool Scene::inShadow(int pi, vec3 point, vec3 ray)
{
	vec3 pb  = viewPort.lightSource.loc - point;

	vec3 tempPoint;

	if(dot(primList[pi]->Normal(point, viewPort.eyeLoc), pb) < 0)
	{
		return true;
	}

	float temp = -1;
	for(int x = 0; x < primIndex; x++)
	{
		if(pi == x)
			continue;

		temp = primList[x]->Intersection(point, pb, tempPoint);

		if(temp >= 0 && temp <= 1)
		{
			return true;
		}
	}
	return false;
}

void read_input_file(Scene &scene, const char * fileName)
{
	ifstream ifs(fileName);
	assert(ifs);

	int number_of_primitives;

	ifs >> scene.viewPort.pxWidth >> scene.viewPort.pxHeight;
	ifs >> scene.viewPort.eyeLoc.x >> scene.viewPort.eyeLoc.y >> scene.viewPort.eyeLoc.z;
	ifs >> scene.viewPort.llCorner.x >> scene.viewPort.llCorner.y >> scene.viewPort.llCorner.z;
	ifs >> scene.viewPort.screenWidth.x>> scene.viewPort.screenWidth.y >> scene.viewPort.screenWidth.z;
	ifs >> scene.viewPort.screenHeight.x >> scene.viewPort.screenHeight.y >> scene.viewPort.screenHeight.z;
	ifs >> scene.viewPort.lightSource.loc.x >> scene.viewPort.lightSource.loc.y >> scene.viewPort.lightSource.loc.z;
	ifs >> scene.viewPort.lightSource.I;
	ifs >> scene.viewPort.lightSource.ambientI;
	ifs >> number_of_primitives;


	scene.allocPrimitive(number_of_primitives);

	for ( int i=0; i<number_of_primitives; i++ )
    {
		char primitive_type;
		ifs >> primitive_type;
		switch(primitive_type)
		{
			case 's':
			case 'S':
			{
				Sphere *s = new Sphere();

				ifs >> s->center.x >> s->center.y >> s->center.z;
				ifs >> s->radius;
				ifs >> s->Kdr >> s->Kdg >> s->Kdb;
				ifs >> s->Kar >> s->Kag >> s->Kab;
				ifs >> s->Ks >> s->nSpec;


				/*cout << "Sphere" << endl;
				cout << s->center.x << s->center.y << s->center.z;
				cout << s->radius << endl;
				cout << s->Kdr << s->Kdg << s->Kdb;
				cout << s->Kar << s->Kag << s->Kab;
				cout << s->Ks << s->nSpec << endl;*/

				scene.primList[scene.primIndex] = s;
				scene.primIndex++;

				// add the sphere to your datastructures (primitive list, sphere list or such) here
				break;
			}
			case 'T':
			case 't':
			{
				Triangle *t = new Triangle();

				ifs >> t->a1.x >> t->a1.y >> t->a1.z;
				ifs >> t->a2.x >> t->a2.y >> t->a2.z;
				ifs >> t->a3.x >> t->a3.y >> t->a3.z;
				ifs >> t->Kdr >> t->Kdg >> t->Kdb;
				ifs >> t->Kar >> t->Kag >> t->Kab;
				ifs >> t->Ks >> t->nSpec;

				scene.primList[scene.primIndex] = t;
				scene.primIndex++;

				// add the triangle to your datastructure (primitive list, sphere list or such) here
				break;
			}
			default:
				assert(0);
		}
    }
}
