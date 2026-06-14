#pragma once
#include"geometry.h"
class Ubo
{
public:
	void setModelMat(const mat<4, 4>& m) { modelM = m; }
	void setCameraInfoAndBuild(const vec3 eye, const vec3 center, const vec3 up, const double degreeYaxis = 0., const double degreeXaxis = 0.);
	void perspectiveMatBuild(const double f, const double aspect, const double near, const double far);
	void orthographicMatBuild(const double width, const double height, const double near, const double far);
	void setViewPort(const double x, const double y, const double w, const double h);
	
	mat<4, 4> getModelMat()const { return modelM; }
	mat<4, 4> getViewMat()const { return viewM; }
	mat<4, 4> getProjMat()const { return projM; }
	mat<4, 4> getViewPortMat()const { return viewPortMat; }
private:
	struct  quat3
	{
		float w;
		vec3 v;
	};

	vec3 front;
	vec3 right;
	vec3 cameraUp;
	vec3 eye;

	float degreeXaxis;
	float degreeYaxis;

	vec3 quarterRotate(double degree, vec3 axisUnit, vec3 target);

	void cameraBuild();

	mat<4, 4> modelM{
		vec4{1,0,0,0},
		vec4{0,1,0,0},
		vec4{0,0,1,0},
		vec4{0,0,0,1}
	};

	mat<4,4> viewM{
		vec4{1,0,0,0},
		vec4{0,1,0,0},
		vec4{0,0,1,0},
		vec4{0,0,0,1}
	};

	mat<4, 4> projM{
		vec4{1,0,0,0},
		vec4{0,1,0,0},
		vec4{0,0,1,0},
		vec4{0,0,0,1}
	};

	mat<4, 4> viewPortMat;
};