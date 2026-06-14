#pragma once
#include"geometry.h"
#include"tgaimage.h"
#include <random>
#include"UBO.h"
#include"Buffer.h"
#include"model.h"
struct fragmentOutput
{
	bool discard = false;
	TGAColor color;
	double ao=1;
};

double generate_random_double(double min, double max);
double lerp(double a, double b, double f);




vec4 sample2D(const TGAImage& image, const vec2& uv);

// x,y are light-space normalized device coordinates in [-1,1].
// currentZ is the light-space depth in [0,1].
int shadowGet(Buffer* shadowBuffer, const float x, const float y, const float currentZ);
float PCF(Buffer* shadowBuffer, const float x, const float y, const float currentZ);
float PCSS(Buffer* shadowBuffer, const float x, const float y, const float currentZ);
float SSAO(Buffer* ssaoBuffer,Ubo*ubo, vec3 worldPos, const float currentZ, vec3 normal);
float getSSAO(TGAImage* ssaoBuffer, float x, float y);


class baseShader
{
public:
	baseShader(Ubo& u) :ubo(u) {}
	Ubo& ubo;
	void rasterize(vec4 clip[3], TGAImage& frameBuffer, Buffer& zBuffer);
	virtual vec4 vertProcess(const int nthfaces, const int nthverts) = 0;
private:

	virtual fragmentOutput fragment(const vec3& baryCoord, float z=0)const = 0;
};


class phongShader :public baseShader
{
public:
	vec3 lightPos = {};
	vec3 camPos = {};

	phongShader(const Model& m, Ubo& u) :baseShader(u), model(m) {}
	
	virtual vec4 vertProcess(const int nthfaces, const int nthverts)override;
	virtual fragmentOutput fragment(const vec3& baryCoord, float z = 0)const override;
private:
	const Model& model;
	vec2 uv[3]{};
	vec3 normal[3]{};
	vec3 TBN_T[3]{};
	vec3 TBN_B[3]{};
    double TBN_flip[3]{};
	vec3 worldPos[3]{};
	
};

class ShadowShader :public baseShader
{
public:
	ShadowShader(const Model& m, Ubo& u):baseShader(u),model(m){}
	virtual vec4 vertProcess(const int nthfaces, const int nthverts)override;
	virtual fragmentOutput fragment(const vec3& baryCoord, float z = 0)const override;
private:
	const Model& model;
	
};

class phongWithShadowShader :public baseShader
{
public:
	mat<4, 4> lightTrans = {};
	mat<4, 4> lightViewPort = {};
	vec3 camPos = {};
	vec3 lightPos = {};
	Buffer* shadowBuffer=nullptr;
	phongWithShadowShader(const Model&m,Ubo &u,Buffer& targetBuffer) :baseShader(u), model(m), shadowBuffer(&targetBuffer) {}

	virtual vec4 vertProcess(const int nthfaces, const int nthverts)override;
	virtual fragmentOutput fragment(const vec3& baryCoord, float z = 0)const override;
private:
	const Model& model;
	vec2 uv[3]{};
	vec3 normal[3]{};
	vec3 TBN_T[3]{};
	vec3 TBN_B[3]{};
	double TBN_flip[3]{};
	vec3 worldPos[3]{};

	
};

class SSAOShader :public baseShader
{
public:
	
	Buffer* ssaoBuffer = nullptr;
	SSAOShader(const Model& m, Ubo& u, Buffer& targetBuffer) :baseShader(u), model(m), ssaoBuffer(&targetBuffer) {}

	virtual vec4 vertProcess(const int nthfaces, const int nthverts)override;
	virtual fragmentOutput fragment(const vec3& baryCoord, float z = 0)const override;
private:
	const Model& model;
	vec2 uv[3]{};
	vec3 normal[3]{};
	vec3 TBN_T[3]{};
	vec3 TBN_B[3]{};
	double TBN_flip[3]{};
	vec3 worldPos[3]{};

};
class phongWithShadowWithSSAOShader :public baseShader
{
public:
	mat<4, 4> lightTrans = {};
	mat<4, 4> lightViewPort = {};
	vec3 camPos = {};
	vec3 lightPos = {};
	TGAImage* ssaoBuffer = nullptr;
	Buffer* shadowBuffer = nullptr;
	phongWithShadowWithSSAOShader(const Model& m, Ubo& u, TGAImage& targetBuffer,Buffer& shadowBuffer) :baseShader(u), model(m), ssaoBuffer(&targetBuffer),shadowBuffer(&shadowBuffer){}

	virtual vec4 vertProcess(const int nthfaces, const int nthverts)override;
	virtual fragmentOutput fragment(const vec3& baryCoord, float z = 0)const override;
private:
	const Model& model;
	vec2 uv[3]{};
	vec3 normal[3]{};
	vec3 TBN_T[3]{};
	vec3 TBN_B[3]{};
	double TBN_flip[3]{};
	vec3 worldPos[3]{};
	
};


