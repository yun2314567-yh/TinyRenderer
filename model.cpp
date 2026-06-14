
#include<sstream>
#include<fstream>
#include<algorithm>
#include"model.h"
Model::Model(const std::string filename)
{
	std::ifstream in;

	in.open(filename, std::ifstream::in);

	if (in.fail())return;  

	std::string line;

	while (std::getline(in, line))//读取一行
	{
      	std::istringstream iss(line.c_str());

		char trash;
		
		if (!line.compare(0, 2, "v "))
		{
			iss >> trash;
			vec3 v;
			for (int i : {0, 1, 2}) iss >> v[i];
			verts.push_back(v);
		}

		else if (!line.compare(0, 3, "vt "))
		{
			iss >> trash>>trash;
			vec2 v;
			for (int i : {0, 1}) iss >> v[i];
			Texcoord.push_back({ v.x,(1. - v.y) });
			
		}
		
		else if (!line.compare(0, 3, "vn "))
		{
			iss >> trash >> trash;
			vec3 v;
			for (int i : {0, 1, 2}) iss >> v[i];
			verts_Normal.push_back(v);

		}

		else if (!line.compare(0, 2, "f "))
		{
			iss >> trash;
			int f, t, n, cnt = 0;
			while (iss >> f >> trash >> t >> trash >> n) //顶点/该点UV/该点法线
			{
				vertexIndex_InFaces.push_back(--f);
				
				texCoordIndex_InFaces.push_back(--t);
				normalIndex_InFaces.push_back(--n);
				cnt++;
			}
			if (3 != cnt)
			{
				std::cerr<< "Error: the obj file is supposed to be triangulated" << std::endl;
				return;
			}
		}
	}
		
	std::vector<vec3> tangents (sumFace());
	std::vector<vec3> bitangents (sumFace());

	for (int i = 0; i < sumFace(); i++)
	{
		

		vec3 p[3];
		vec2 t[3];
		for (int j : {0, 1, 2})
		{
						p[j] = vert(i, j);
						t[j] = uv(i, j);
						vec3 n = vert_N(i, j);
						Key key{ p[j],n,t[j] };
						group[key].push_back(i);
		}

		vec3 dE1 = p[1] - p[0];
		vec3 dE2 = p[2] - p[0];

		vec2 dT1 = t[1] - t[0];
		vec2 dT2 = t[2] - t[0];

		float dU1 = dT1.x, dV1 = dT1.y;
		float dU2 = dT2.x, dV2 = dT2.y;

		float f = dU1 * dV2 - dU2 * dV1;

		if(fabs(f) < 1e-6)
		{
			tangents[i] = vec3{ 1,0,0 };
			bitangents[i] = vec3{ 0,1,0 };
			continue;
		}

		vec3 T, B;

		T = (dV2 * dE1 - dV1 * dE2) / f;
		B = (dU1 * dE2 - dU2 * dE1) / f;

		tangents[i] = T;
		bitangents[i] = B;
		
	}

    Tangents.resize(sumVert());
	Bitangents.resize(sumVert());
	flipSigns.resize(sumVert());
	// Robustly compute per-vertex tangents by averaging face tangents that reference each vertex.
	// This avoids assuming texcoords/normals are indexed 1:1 with vertices (some OBJ files store
	// normals/texcoords differently) and prevents degenerate TBN bases that produce zero normals.
	
	for (int fi = 0; fi < sumFace(); ++fi)
	{
		for (int j : {0, 1, 2})
		{
			vec3 pos = vert(fi, j);
			vec3 normal = vert_N(fi, j);
			vec2 texCoord = uv(fi, j);

			Key key{ pos,normal,texCoord };

			auto it = group.find(key);
			if (it == group.end())
			{
				Tangents[vertexIndex_InFaces[fi * 3 + j]] = vec3{ 1,0,0 };
				Bitangents[vertexIndex_InFaces[fi * 3 + j]] = vec3{ 0,1,0 };
				flipSigns[vertexIndex_InFaces[fi * 3 + j]] = 1;
				continue;
			}
			vec3 avg_T, avg_B;

			for (int k : it->second)
			{
				avg_T = avg_T + tangents[k];
				avg_B = avg_B + bitangents[k];
			}

			vec3 N = normalize(normal);
			vec3 T = normalize(avg_T - dot(N, avg_T) * N);
			vec3 B = normalize(cross(N, T));

			float flip = dot(B, avg_B) >= 0.0 ? 1. : -1.;

			Tangents[vertexIndex_InFaces[fi * 3 + j]] = T;
			Bitangents[vertexIndex_InFaces[fi * 3 + j]] = B;
			flipSigns[vertexIndex_InFaces[fi * 3 + j]] = flip;
		}
	}

	std::cerr << "# v# " << sumVert() << " f# " << sumFace() << std::endl;

	auto load_texture = [&filename](const std::string suffix, TGAImage& img) {
		size_t dot = filename.find_last_of(".");
		if (dot == std::string::npos) return;
		std::string texfile = filename.substr(0, dot) + suffix;
		std::cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
		};
	
	load_texture("_diffuse.tga", diffuseMap);
	load_texture("_spec.tga", specularMap);
	load_texture("_glow.tga", glowMap);
	load_texture("_nm_tangent.tga", normalMap);
}

int Model::sumVert()const
{
	return verts.size();
}

int Model::sumFace()const
{
	return vertexIndex_InFaces.size()/3;

}

vec3 Model::vert(const int i)const
{
	return verts[i];
}

vec3 Model::vert(const int faceIndex, const int nthVert)const
{
	return verts[vertexIndex_InFaces[faceIndex*3 + nthVert]];//1个面对应三个顶点，从面访问顶点的步长为3
}

vec3 Model::vert_N(const int faceIndex, const int nthVert)const
{
	return verts_Normal[normalIndex_InFaces[faceIndex * 3 + nthVert]];
}

vec3 Model::vert_TBN_T(const int faceIndex, const int nthVert)const
{
	return Tangents[vertexIndex_InFaces[faceIndex * 3 + nthVert]];
}
vec3 Model::vert_TBN_B(const int faceIndex, const int nthVert)const
{
	return Bitangents[vertexIndex_InFaces[faceIndex * 3 + nthVert]];
}

float Model::vert_TBN_flip(const int faceIndex, const int nthVert)const
{
	return flipSigns[vertexIndex_InFaces[faceIndex * 3 + nthVert]];
}

vec2 Model::uv(const int faceIndex, const int nthVert_T)const
{
	 
	return Texcoord[texCoordIndex_InFaces[faceIndex * 3 + nthVert_T]];;
}

vec4 Model::normal(const vec2& uv) const
{
    // Bilinear sample the normal map. Keep the same per-channel ordering as the rest of the code.
	int w = normalMap.width();
	int h = normalMap.height();
	if (w <= 0 || h <= 0) return vec4{0,0,1,0};

	double u = uv[0] * (w - 1);
	double v = uv[1] * (h - 1);

	int x0 = std::clamp(static_cast<int>(std::floor(u)), 0, w - 1);
	int y0 = std::clamp(static_cast<int>(std::floor(v)), 0, h - 1);
	int x1 = std::clamp(x0 + 1, 0, w - 1);
	int y1 = std::clamp(y0 + 1, 0, h - 1);

	double sx = u - x0;
	double sy = v - y0;

	TGAColor c00 = normalMap.get(x0, y0);
	TGAColor c10 = normalMap.get(x1, y0);
	TGAColor c01 = normalMap.get(x0, y1);
	TGAColor c11 = normalMap.get(x1, y1);

	double ch[4] = {0,0,0,0};
	for (int i = 0; i < 4; ++i)
	{
		double v00 = c00[i];
		double v10 = c10[i];
		double v01 = c01[i];
		double v11 = c11[i];

		double i0 = v00 * (1.0 - sx) + v10 * sx;
		double i1 = v01 * (1.0 - sx) + v11 * sx;
		ch[i] = (i0 * (1.0 - sy) + i1 * sy);
	}

	// Current code expects the sampled components in the ordering used below.
	vec4 sampled = vec4{ ch[2], ch[1], ch[0], 0.0 };
	vec4 n = sampled * (2.0 / 255.0) - vec4{ 1,1,1,0 };
	// Flip Y to match tangent-space convention (kept as before)
	
	return normalize(n);
}
