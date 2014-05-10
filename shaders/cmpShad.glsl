#version 430
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable

//Structs for the buffer
struct p_struct
{
	int type;			//0 sphere 1 triangle
	
	float Kdr,Kdg,Kdb;
	float Kar,Kag,Kab;
	
	float Ks;
	float nSpec;	
	
	float radius;
	
	float v1x, v1y, v1z;
	float v2x, v2y, v2z;
	float v3x, v3y, v3z;// corners fro a triangle or v1 is center for a sphere
};
//This struct exits because structs with vec3 do not work nicely with buffers
struct Primitive
{
	int type;			//0 sphere 1 triangle
	vec3 Kd;
	vec3 Ka;
	float Ks;
	float nSpec;	
	float radius;
	vec3 v1, v2, v3;	// corners fro a triangle or v1 is center for a sphere
};

//The final storage buffer to use
layout(RGBA32F) uniform image2D destTex;

//The input buffer of primitives
layout(shared, binding=5) buffer p_buf
{
	p_struct primList[];

};

//Input size for the workgroups
layout (local_size_x = 16, local_size_y = 16) in;

//Uniform for subdividing dispatch
uniform ivec2 subStart;

//Uniforms for the viewport
uniform int pxWidth, pxHeight;
uniform int primCount;
uniform vec3 eyeLoc;
uniform vec3 llCorner;
uniform vec3 sWidth, sHeight;

//Uniforms for Lighting
uniform vec3 lLoc;
uniform float I;
uniform float aI;

//Returns a normal for the primitive at the point provided
vec3 getNormal(Primitive p, vec3 point)
{
	vec3 norm;
	
	if(p.type == 0)
	{
		norm = point - p.v1;
		norm = normalize(norm);
	} else
	{
		vec3 v12 = p.v1 - p.v2;
		vec3 v13 = p.v3 - p.v1;

		norm = cross(v12 , v13);

		norm = normalize(norm);

		vec3 eyea1 =  eyeLoc - p.v1;

		if(dot(norm, eyea1) < 0)
		{
			norm = norm * -1.0f;
		}
	}
	return norm;
}

//Calcuates the intersection of the ray from origin
// Returns t value of the paraterize eqn origin + ray*t
// Point is also the point of intersection
float triIntersection(vec3 origin, vec3 ray, Primitive p, out vec3 point)
{
	vec3 a1origin = p.v1 - origin;

	vec3 norm = cross((p.v2 - p.v1), (p.v3 - p.v1));

	float x = dot(a1origin, norm);
	float y = dot(ray, norm);

	float t = x / y;

	if( t < 0 )
	{
		return -1;
	} else
	{

		vec3 res = t * ray;
		res = res + origin;
		
		
		vec3 v1 = cross((p.v1 - res), (p.v2 - res));
		vec3 v2 = cross((p.v2 - res), (p.v3 - res));
		vec3 v3 = cross((p.v3 - res), (p.v1 - res));

		if(dot(v1, v2) > 0 && dot(v2, v3) > 0 && dot(v3,v1) > 0)
		{
			point = res;

			return t;
		}
		return -1;
	}

}
// Same as triIntersection but with a sphere
// See above
float sphIntersection(vec3 origin, vec3 ray, Primitive p, out vec3 point)
{
	vec3 co = origin - p.v1; //center;

	float c = pow(length(co), 2) - pow(p.radius, 2);
	float b = 2 * dot(co, ray);
	float a = pow(length(ray), 2);

	float delta = pow(b, 2) - (4 * a * c);

	if(delta < 0)
	{
		return -1.0;
	}else if (delta == 0)
	{
		float t = (-1 * b) / (2 * a);
		vec3 res = eyeLoc + ( ray * t);

		point.x = res.x;
		point.y = res.y;
		point.z = res.z;
		return t;
	} else
	{
		float t1 = ((-1 * b) - sqrt(delta)) / (2 * a);
		float t2 = ((-1 * b) + sqrt(delta)) / (2 * a);

		if(t1 < 0 && t2 < 0)
			return -1;
		if((t1 > 0 && t2 < 0) || (t1 > 0 && t1 < t2))
		{
			vec3 res = eyeLoc + ( ray * t1);

			point.x = res.x;
			point.y = res.y;
			point.z = res.z;

			return t1;
		} else if((t1 < 0 && t2 > 0) || (t2 > 0 && t1 > t2))
		{
			vec3 res = eyeLoc + ( ray * t2);

			point.x = res.x;
			point.y = res.y;
			point.z = res.z;

			return t2;
		}
	}
	return -1.0;

}
//Converts a p_struct(raw buffer data) to a primitive
Primitive makePrimitive(p_struct ps)
{
	Primitive p;
	p.type = ps.type;
	
	p.nSpec = ps.nSpec;
	p.radius = ps.radius;
	
	p.Ks = ps.Ks;
	
	p.Kd = vec3(ps.Kdr, ps.Kdg, ps.Kdb);
	p.Ka = vec3(ps.Kar, ps.Kag, ps.Kab);
	
	p.v1 = vec3(ps.v1x, ps.v1y, ps.v1z);
	p.v2 = vec3(ps.v2x, ps.v2y, ps.v2z);
	p.v3 = vec3(ps.v3x, ps.v3y, ps.v3z);
	

	return p;
}
//Checks to see if the primitive is in shadow
// si is the index of p in the buffer
// Point is the point to check if it is in shadow
bool inShadow(Primitive p, int si, vec3 point)
{

	vec3 pb = lLoc - point;
	
	vec3 tempPoint;
	
	if(dot(getNormal(p,point), pb) < 0)
	{
		return true;
	}
	
	float temp = -1;
	
	for(int x = 0; x < primCount; x++)
	{
		if(si == x)
			continue;
		
		Primitive op = makePrimitive(primList[x]);
		
		if(op.type == 0)
		{
			temp = sphIntersection(point,pb, op, tempPoint);
		}
		else
		{
			temp = triIntersection(point, pb, op, tempPoint);
		}
		
		if(temp > 0.0001 && temp <= 1.00)
		{
			return true;
		}
		
	}
	return false;
}

void main() 
{
	
	vec4 color;
	//Use the id coords for the storage location
	ivec2 imgPos = ivec2(gl_GlobalInvocationID.xy) + subStart;	
	
	//Calculate Center of pixel
	vec3 cop = sWidth * ((imgPos.x + 0.5) / pxWidth);
	cop = cop + (sHeight * ((imgPos.y + 0.5) / pxHeight));
	cop = llCorner + cop;
	//Calculate the ray 
	vec3 ray = cop - eyeLoc;
	
	//Vars for tracking least intersection
	float minT = -1, temp = 0;
	int sIndex = 0;
	vec3 small = vec3(0.0, 0.0, 0.0), point;
	
	Primitive p;
	
	//Check all primitives for the least intersection
	for(int x = 0; x < primCount; x++)
	{
		p = makePrimitive(primList[x]);
		if(p.type == 0)
		{
			temp = sphIntersection(eyeLoc,ray, p, point);
		}
		else
		{
			temp = triIntersection(eyeLoc, ray, p, point);
		}
		
		if((minT == -1 && temp > 0) || (minT > temp && temp > 0))
		{
			minT = temp;
			small = point;
			sIndex = x;
		}
		
	}
	//Check for any intersection and calculate color
	if(minT >= 0)
	{
		p = makePrimitive(primList[sIndex]);
		
		vec3 normal = getNormal(p, small);
		
		vec3 L = lLoc - small;
		L = normalize(L);
		vec3 V = eyeLoc - small;
		V = normalize(V);
		vec3 H = V + L;
		H = normalize(H);
		
		color = vec4(aI * p.Ka, 1.0);
		
		
		if(!inShadow(p, sIndex, small))
		{
			vec3 cKd = p.Kd * dot(normal,L);
			vec3 cKs = vec3(p.Ks, p.Ks, p.Ks) * pow(dot(normal,H), p.nSpec);
			
			color += vec4(I * (cKd + cKs), 1.0);
		}
		
		//color = vec4(1.0,1.0,1.0,1.0);
	} else
	{
		color = vec4(0.0, 0.0, 0.0, 1.0);
	}
	
	//Store the color
	imageStore(destTex, imgPos, color);
}
