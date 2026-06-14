#include"UBO.h"

void Ubo::setCameraInfoAndBuild(const vec3 eye, const vec3 center, const vec3 up, const double degreeYaxis , const double degreeXaxis )
{
	this->eye = eye;
	 front = normalize(center - eye);
	 right = normalize(cross(front, up));
	 cameraUp = cross(right, front);
	 this->degreeXaxis = degreeXaxis;
	 this->degreeYaxis = degreeYaxis;
	 cameraBuild();
}

void Ubo::setViewPort(const double x, const double y, const double w, const double h)
{
	viewPortMat = { {{w / 2., 0, 0, x + w / 2.}, { 0, h / 2., 0, y + h / 2. }, { 0,0,0.5,0.5 }, { 0,0,0,1 }} };
}

void Ubo::perspectiveMatBuild(const double f, const double aspect, const double near, const double far)
{
	double radian = f * PI / 180. / 2.;
	projM = { {} };
	projM[0][0] = 1. / (aspect * std::tan(radian));
	projM[1][1] = 1. / std::tan(radian);
	projM[2][2] = (far + near) / (near - far);
	projM[2][3] = 2 * far * near / (near - far);
	projM[3][2] = -1.;
	projM[3][3] = 0.0;
}

void Ubo::orthographicMatBuild(const double width, const double height, const double near, const double far)
{
	projM = { {} };
	projM[0][0] = 2. / width;
	projM[1][1] = 2. / height;
	projM[2][2] = 2. / (near - far);
	projM[2][3] = (far + near) / (near - far);
	projM[3][3] = 1.0;
}

void Ubo::cameraBuild()
{
	if(degreeYaxis>0)
	front=quarterRotate(degreeYaxis, vec3{0,1,0}, front);

	if(degreeXaxis>0)
	{
		vec3 worldR = normalize(cross(vec3{0,1,0}, front));
		front = quarterRotate(degreeXaxis, worldR, front);
	}

	right = normalize(cross(front, cameraUp));
	 cameraUp = cross(right, front);
	 viewM = {
		 vec4{right.x,right.y,right.z,-dot(right,eye)},
		 vec4{cameraUp.x,cameraUp.y,cameraUp.z,-dot(cameraUp,eye)},
		 vec4{-front.x,-front.y,-front.z,dot(front,eye)},
		 vec4{0.,0.,0.,1.}
	 };
}

vec3 Ubo::quarterRotate(double degree, vec3 axisUnit, vec3 target)
{
	float radian = degree * PI / 180. / 4.;

	float half_cosTheta = std::cos(radian);
	float half_sinTheta = std::sin(radian);

	quat3 q{ half_cosTheta, axisUnit * half_sinTheta };

	

	vec3 final = target + cross(2 * q.v, cross(q.v, target) + q.v * q.w);

	return normalize(final);
}