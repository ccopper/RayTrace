/*
 * Scene.h
 *
 */

#ifndef SCENE_H_
#define SCENE_H_

#include "Primitives.h"
#include <vector>

using namespace glm;


typedef struct
{
	int pxWidth, pxHeight;
	int primCount;
	vec3 eyeLoc;
	vec3 llCorner;
	vec3 screenWidth, screenHeight;
	Light lightSource;

} ViewPort;

class Scene
{
	public:
		vector<Primitive *> primList;
		ViewPort viewPort;
		int primIndex;


		Scene();
		~Scene();
		void allocPrimitive(int size);
		void addPrimitive(Primitive *p);

		void render(Image &I);
		bool inShadow(int pi, vec3 point, vec3 ray);

};

void read_input_file(Scene &scene, const char * fileName);


#endif /* SCENE_H_ */
