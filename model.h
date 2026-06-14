#pragma once

#include<vector>
#include<unordered_map>
#include"tgaimage.h"

#include"geometry.h"
class Model
{
	struct Key
	{
		vec3 vertex;
		vec3 normal;
		vec2 texcoord;

		bool operator==(const Key& other) const {
			
			return vertex==other.vertex &&
				normal==other.normal &&
				texcoord==other.texcoord;
		}
	};

	struct KeyHash
	{
		size_t operator()(const Key& k) const {
			auto hashFloat = std::hash<float>{};
			size_t h = hashFloat(k.vertex.x);
			h = h * 31 + hashFloat(k.vertex.y);
			h = h * 31 + hashFloat(k.vertex.z);
			h = h * 31 + hashFloat(k.normal.x);
			h = h * 31 + hashFloat(k.normal.y);
			h = h * 31 + hashFloat(k.normal.z);
			h = h * 31 + hashFloat(k.texcoord.x);
			h = h * 31 + hashFloat(k.texcoord.y);
			return h;
		}
	};

	std::vector<vec3> verts = {};
	std::vector<vec3> verts_Normal = {};
	std::vector<int> vertexIndex_InFaces = {};
	std::vector<int> normalIndex_InFaces = {};
	std::vector<int> texCoordIndex_InFaces = {};
	std::vector<vec2> Texcoord = {};
	std::vector<vec3> Tangents = {};
	std::vector<vec3> Bitangents = {};
	std::vector<float> flipSigns = {};
    TGAImage diffuseMap = {};
	TGAImage normalMap = {};
	TGAImage specularMap = {};
	TGAImage glowMap = {};
	std::unordered_map<Key, std::vector<int>, KeyHash> group;
public:
	
	
	Model(const std::string filename);
	int sumVert()const;
	int sumFace()const;
	vec3 vert(const int i)const;  //0<=i<sumVert
	vec3 vert(const int faceIndex, const int nthVert)const; //-<=iFace<sumFace, 0<= nthVert <3
	vec3 vert_N(const int faceIndex, const int nthVert_N)const;
	vec3 vert_TBN_T(const int faceIndex, const int nthVert)const;
	vec3 vert_TBN_B(const int faceIndex, const int nthVert)const;
	float vert_TBN_flip(const int faceIndex, const int nthVert)const;
	vec2 uv(const int faceIndex, const int nthVert_T)const;
	vec4 normal( const vec2& uv)const;
	const TGAImage& diffuse()const { return diffuseMap; }
	const TGAImage& specular()const { return specularMap; }
	const TGAImage& glow()const { return glowMap; }
};