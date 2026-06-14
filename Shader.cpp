#include<cmath>
#include<algorithm>
#include"Shader.h"

vec4 phongShader::vertProcess(const int nthfaces, const int nthverts)
{
	vec3 v = model.vert(nthfaces, nthverts);
	
	uv[nthverts] = model.uv(nthfaces, nthverts);
	normal[nthverts] = model.vert_N(nthfaces, nthverts);
	TBN_T[nthverts] = model.vert_TBN_T(nthfaces, nthverts);
	TBN_B[nthverts] = cross(normal[nthverts], TBN_T[nthverts]) * model.vert_TBN_flip(nthfaces, nthverts);
	vec4 gl_Position = ubo.getViewMat() * ubo.getModelMat() * vec4 { v.x, v.y, v.z, 1.f };
	
	return ubo.getProjMat() * gl_Position;
}	

fragmentOutput phongShader::fragment(const vec3& baryCoord, float z)const
{
	fragmentOutput output;
	vec2 bary_uv = { uv[0] * baryCoord.x + uv[1] * baryCoord.y + uv[2] * baryCoord.z };
	vec3 bary_N = normalize(normal[0] * baryCoord.x + normal[1] * baryCoord.y + normal[2] * baryCoord.z);
	vec3 bary_T = normalize(TBN_T[0] * baryCoord.x + TBN_T[1] * baryCoord.y + TBN_T[2] * baryCoord.z);
	vec3 bary_B = normalize(TBN_B[0] * baryCoord.x + TBN_B[1] * baryCoord.y + TBN_B[2] * baryCoord.z);
	
	vec3 T = normalize(bary_T - dot(bary_N, bary_T) * bary_N);
	vec3 B = cross(bary_N, bary_T);
	B = B * (dot(B, bary_B) >= 0.0 ? 1. : -1.);

	vec3 bary_worldPos = worldPos[0] * baryCoord.x + worldPos[1] * baryCoord.y + worldPos[2] * baryCoord.z;	

	vec3 lightDir = normalize(lightPos - bary_worldPos);
	vec3 viewDir = normalize(camPos - bary_worldPos);

	mat<4,4> TBN = { vec4{bary_T.x,bary_T.y,bary_T.z,0},
		vec4{bary_B.x,bary_B.y,bary_B.z,0},
		vec4{bary_N.x,bary_N.y,bary_N.z,0},
		vec4{0,0,0,1} };
	
	vec4 normalT = normalize(TBN.transpose() * model.normal(bary_uv));
	vec4 diffuseCol = sample2D(model.diffuse(), bary_uv);

	vec3 dColor = diffuseCol.xyz()*std::max(0.0, dot(normalT.xyz(), lightDir));
	vec3 specularMask = sample2D(model.specular(), bary_uv).xyz();
	vec3 glowCol = sample2D(model.glow(), bary_uv).xyz();
	vec3 specularCol = specularMask.x*normalize(vec3{ 1,1,1 })* std::pow(std::max(0., dot(normalT.xyz(), normalize(lightDir + viewDir))), 64);
	vec3 ambient = (diffuseCol*0.3).xyz();
	vec3 finalColor =glowCol+ dColor +specularCol +ambient ;//头顶宝石是法线原因
	finalColor = vec3{
		std::clamp(finalColor.x,0.,1.),
		std::clamp(finalColor.y,0.,1.),
		std::clamp(finalColor.z,0.,1.)
	};
	output.color = TGAColor{ static_cast<unsigned char>(finalColor.x * 255),
	                         static_cast<unsigned char>(finalColor.y * 255),
	                         static_cast<unsigned char>(finalColor.z * 255),
	                         255 };
	return output;
}

vec4 ShadowShader::vertProcess(const int nthfaces, const int nthverts)
{
	vec3 v = model.vert(nthfaces, nthverts);
	
	vec4 gl_Position = ubo.getViewMat() * ubo.getModelMat() * vec4 { v.x, v.y, v.z, 1.f };

	return ubo.getProjMat() * gl_Position;
}

fragmentOutput ShadowShader::fragment(const vec3& baryCoord, float z)const
{
	fragmentOutput output;
	output.color = TGAColor{ static_cast<unsigned char>(z * 255) ,
							static_cast<unsigned char>(z * 255),
							static_cast<unsigned char>(z * 255),1 };
	return output;
}

vec4 phongWithShadowShader::vertProcess(const int nthfaces, const int nthverts)
{
	vec3 v = model.vert(nthfaces, nthverts);
	worldPos[nthverts] = v;
	uv[nthverts] = model.uv(nthfaces, nthverts);
	normal[nthverts] = model.vert_N(nthfaces, nthverts);
	TBN_T[nthverts] = model.vert_TBN_T(nthfaces, nthverts);
	TBN_B[nthverts] = cross(normal[nthverts], TBN_T[nthverts]) * model.vert_TBN_flip(nthfaces, nthverts);
	vec4 gl_Position = ubo.getViewMat() * ubo.getModelMat() * vec4 { v.x, v.y, v.z, 1.f };

	return ubo.getProjMat() * gl_Position;
}

fragmentOutput phongWithShadowShader::fragment(const vec3& baryCoord, float z)const
{
	fragmentOutput output;
	vec2 bary_uv = { uv[0] * baryCoord.x + uv[1] * baryCoord.y + uv[2] * baryCoord.z };
	vec3 bary_N = normalize(normal[0] * baryCoord.x + normal[1] * baryCoord.y + normal[2] * baryCoord.z);
	vec3 bary_T = normalize(TBN_T[0] * baryCoord.x + TBN_T[1] * baryCoord.y + TBN_T[2] * baryCoord.z);
	vec3 bary_B = normalize(TBN_B[0] * baryCoord.x + TBN_B[1] * baryCoord.y + TBN_B[2] * baryCoord.z);

	vec3 T = normalize(bary_T - dot(bary_N, bary_T) * bary_N);
	vec3 B = cross(bary_N, bary_T);
	B = B * (dot(B, bary_B) >= 0.0 ? 1. : -1.);

	vec3 bary_worldPos = worldPos[0] * baryCoord.x + worldPos[1] * baryCoord.y + worldPos[2] * baryCoord.z;

	vec4 lightSpacePos = lightTrans * vec4{ bary_worldPos.x,bary_worldPos.y, bary_worldPos.z,1 };

	vec4 lightSpacePosNormalized = lightSpacePos / lightSpacePos.w;
	vec4 viewPort = lightViewPort * lightSpacePosNormalized;

	float lightCurrentZ = (lightSpacePosNormalized.z + 1) * 0.5;
	//std::cout << "当前z: " << lightCurrentZ << std::endl;
	float visable = PCSS(shadowBuffer, viewPort.x, viewPort.y, lightCurrentZ);
	//std::cout << "PCF的z: " << visable << std::endl;
	float f_v = std::clamp(visable+0.3f, 0.f, 1.f);
	vec3 lightDir = normalize(lightPos - bary_worldPos);
	vec3 viewDir = normalize(camPos - bary_worldPos);

	mat<4, 4> TBN = { vec4{bary_T.x,bary_T.y,bary_T.z,0},
		vec4{bary_B.x,bary_B.y,bary_B.z,0},
		vec4{bary_N.x,bary_N.y,bary_N.z,0},
		vec4{0,0,0,1} };

	vec4 normalT = normalize(TBN.transpose() * model.normal(bary_uv));
	vec4 diffuseCol = sample2D(model.diffuse(), bary_uv);

	vec3 dColor = diffuseCol.xyz() * std::max(0.0, dot(normalT.xyz(), lightDir));
	vec3 specularMask = sample2D(model.specular(), bary_uv).xyz();
	vec3 glowCol = sample2D(model.glow(), bary_uv).xyz();
	vec3 specularCol = specularMask.x * normalize(vec3{ 1,1,1 }) * std::pow(std::max(0., dot(normalT.xyz(), normalize(lightDir + viewDir))), 64);
	vec3 ambient = (diffuseCol * 0.3).xyz();
	vec3 finalColor = f_v *(glowCol + dColor + specularCol + ambient);//头顶宝石是法线原因
	//vec3 finalColor=vec3{(double)visable,(double)visable, (double)visable};
	finalColor = vec3{
		std::clamp(finalColor.x,0.,1.),
		std::clamp(finalColor.y,0.,1.),
		std::clamp(finalColor.z,0.,1.)
	};
	output.color = TGAColor{ static_cast<unsigned char>(finalColor.x * 255),
							 static_cast<unsigned char>(finalColor.y * 255),
							 static_cast<unsigned char>(finalColor.z * 255),
							 255 };
	return output;
}

vec4 SSAOShader::vertProcess(const int nthfaces, const int nthverts)
{
	vec3 v = model.vert(nthfaces, nthverts);
	worldPos[nthverts] = v;
	uv[nthverts] = model.uv(nthfaces, nthverts);
	normal[nthverts] = model.vert_N(nthfaces, nthverts);
	TBN_T[nthverts] = model.vert_TBN_T(nthfaces, nthverts);
	TBN_B[nthverts] = cross(normal[nthverts], TBN_T[nthverts]) * model.vert_TBN_flip(nthfaces, nthverts);
	vec4 gl_Position = ubo.getViewMat() * ubo.getModelMat() * vec4 { v.x, v.y, v.z, 1.f };

	return ubo.getProjMat() * gl_Position;
}


fragmentOutput SSAOShader::fragment(const vec3& baryCoord, float z)const
{
	fragmentOutput output;
	vec2 bary_uv = { uv[0] * baryCoord.x + uv[1] * baryCoord.y + uv[2] * baryCoord.z };
	vec3 bary_N = normalize(normal[0] * baryCoord.x + normal[1] * baryCoord.y + normal[2] * baryCoord.z);
	vec3 bary_T = normalize(TBN_T[0] * baryCoord.x + TBN_T[1] * baryCoord.y + TBN_T[2] * baryCoord.z);
	vec3 bary_B = normalize(TBN_B[0] * baryCoord.x + TBN_B[1] * baryCoord.y + TBN_B[2] * baryCoord.z);

	vec3 T = normalize(bary_T - dot(bary_N, bary_T) * bary_N);
	vec3 B = cross(bary_N, bary_T);
	B = B * (dot(B, bary_B) >= 0.0 ? 1. : -1.);

	vec3 bary_worldPos = worldPos[0] * baryCoord.x + worldPos[1] * baryCoord.y + worldPos[2] * baryCoord.z;

	

	

	mat<4, 4> TBN = { vec4{bary_T.x,bary_T.y,bary_T.z,0},
		vec4{bary_B.x,bary_B.y,bary_B.z,0},
		vec4{bary_N.x,bary_N.y,bary_N.z,0},
		vec4{0,0,0,1} };

	vec4 normalT = normalize(TBN.transpose() * model.normal(bary_uv));

	float ao = SSAO(ssaoBuffer, &ubo, bary_worldPos, z, normalT.xyz());

	
	vec3 finalColor = vec3{
		std::clamp(ao,0.f,1.f),
		std::clamp(ao,0.f,1.f),
		std::clamp(ao,0.f,1.f)
	};
	output.color = TGAColor{ static_cast<unsigned char>(finalColor.x * 255),
							 static_cast<unsigned char>(finalColor.y * 255),
							 static_cast<unsigned char>(finalColor.z * 255),
							 255 };
	return output;
}


vec4 phongWithShadowWithSSAOShader::vertProcess(const int nthfaces, const int nthverts)
{
	vec3 v = model.vert(nthfaces, nthverts);
	worldPos[nthverts] = v;
	uv[nthverts] = model.uv(nthfaces, nthverts);
	normal[nthverts] = model.vert_N(nthfaces, nthverts);
	TBN_T[nthverts] = model.vert_TBN_T(nthfaces, nthverts);
	TBN_B[nthverts] = cross(normal[nthverts], TBN_T[nthverts]) * model.vert_TBN_flip(nthfaces, nthverts);
	vec4 gl_Position = ubo.getViewMat() * ubo.getModelMat() * vec4 { v.x, v.y, v.z, 1.f };
	
	return ubo.getProjMat() * gl_Position;
}


fragmentOutput phongWithShadowWithSSAOShader::fragment(const vec3& baryCoord, float z)const
{
	fragmentOutput output;
	vec2 bary_uv = { uv[0] * baryCoord.x + uv[1] * baryCoord.y + uv[2] * baryCoord.z };
	vec3 bary_N = normalize(normal[0] * baryCoord.x + normal[1] * baryCoord.y + normal[2] * baryCoord.z);
	vec3 bary_T = normalize(TBN_T[0] * baryCoord.x + TBN_T[1] * baryCoord.y + TBN_T[2] * baryCoord.z);
	vec3 bary_B = normalize(TBN_B[0] * baryCoord.x + TBN_B[1] * baryCoord.y + TBN_B[2] * baryCoord.z);

	vec3 T = normalize(bary_T - dot(bary_N, bary_T) * bary_N);
	vec3 B = cross(bary_N, bary_T);
	B = B * (dot(B, bary_B) >= 0.0 ? 1. : -1.);

	vec3 bary_worldPos = worldPos[0] * baryCoord.x + worldPos[1] * baryCoord.y + worldPos[2] * baryCoord.z;
	
	vec4 projPos=ubo.getProjMat()*ubo.getViewMat()* vec4 { bary_worldPos.x, bary_worldPos.y, bary_worldPos.z, 1 };

	vec4 ndcPos = projPos / projPos.w;
	vec4 viewPortPos = ubo.getViewPortMat() * ndcPos;

	vec4 lightSpacePos = lightTrans * vec4{ bary_worldPos.x,bary_worldPos.y, bary_worldPos.z,1 };

	vec4 lightSpacePosNormalized = lightSpacePos / lightSpacePos.w;
	vec4 viewPort = lightViewPort * lightSpacePosNormalized;

	float lightCurrentZ = (lightSpacePosNormalized.z + 1) * 0.5;
	//std::cout << "当前z: " << lightCurrentZ << std::endl;
	float visable = PCSS(shadowBuffer, viewPort.x, viewPort.y, lightCurrentZ);
	//std::cout << "PCF的z: " << visable << std::endl;
	float f_v = std::clamp(visable + 0.3f, 0.f, 1.f);
	vec3 lightDir = normalize(lightPos - bary_worldPos);
	vec3 viewDir = normalize(camPos - bary_worldPos);

	mat<4, 4> TBN = { vec4{bary_T.x,bary_T.y,bary_T.z,0},
		vec4{bary_B.x,bary_B.y,bary_B.z,0},
		vec4{bary_N.x,bary_N.y,bary_N.z,0},
		vec4{0,0,0,1} };

	vec4 normalT = normalize(TBN.transpose() * model.normal(bary_uv));
	
	float ao = getSSAO(ssaoBuffer, viewPortPos.x,viewPortPos.y);
	
	vec4 diffuseCol = sample2D(model.diffuse(), bary_uv);

	vec3 dColor = diffuseCol.xyz() * std::max(0.0, dot(normalT.xyz(), lightDir));
	vec3 specularMask = sample2D(model.specular(), bary_uv).xyz();
	vec3 glowCol = sample2D(model.glow(), bary_uv).xyz();
	vec3 specularCol = specularMask.x * normalize(vec3{ 1,1,1 }) * std::pow(std::max(0., dot(normalT.xyz(), normalize(lightDir + viewDir))), 64);
	vec3 ambient = (ao * diffuseCol ).xyz();
	vec3 finalColor =  f_v*(glowCol + dColor + specularCol+ ambient);//头顶宝石是法线原因
	//vec3 finalColor=vec3{ ao,ao, ao };
	finalColor = vec3{
		std::clamp(finalColor.x,0.,1.),
		std::clamp(finalColor.y,0.,1.),
		std::clamp(finalColor.z,0.,1.)
	};
	output.color = TGAColor{ static_cast<unsigned char>(finalColor.x * 255),
							 static_cast<unsigned char>(finalColor.y * 255),
							 static_cast<unsigned char>(finalColor.z * 255),
							 255 };
	return output;
}

void baseShader::rasterize(vec4 clip[3], TGAImage& frameBuffer, Buffer& zBuffer)
{
	vec4 ndc[3];
	vec3 screen[3];
	float invert_w[3];
#pragma omp parallel for
	for (int i = 0; i < 3; i++)
	{
		ndc[i] = clip[i] / clip[i].w;
		screen[i] = (ubo.getViewPortMat() * ndc[i]).xyz();
		invert_w[i] = 1. / clip[i].w;
		//std::cout << "顶点位置" << ndc[i].x << " " << ndc[i].y << " " << ndc[i].z << " " << clip[i].w << std::endl;
	}
	//并行结束

	mat<3,3> abc =
	{
		{{ screen[0].x, screen[1].x, screen[2].x },
		 { screen[0].y, screen[1].y, screen[2].y },
		 { 1 ,1 , 1}}
	};

	//std::cout << screen[0].x << " " << screen[0].y << " " << screen[1].x << " " << screen[1].y << " " << screen[2].x << " " << screen[2].y << std::endl;

	if (std::fabs(abc.det()) < 1e-6)return;//退化三角形

	auto [minx, maxx] = std::minmax({ screen[0].x, screen[1].x, screen[2].x });
	auto [miny, maxy] = std::minmax({ screen[0].y, screen[1].y, screen[2].y });

#pragma omp parallel for
	for(int x = std::max(0, static_cast<int>(minx)); x <= std::min(static_cast<int>(maxx), frameBuffer.width() - 1); x++)//屏幕裁剪
	{
		for (int y = std::max(0, static_cast<int>(miny)); y <= std::min(static_cast<int>(maxy), frameBuffer.height() - 1); y++)
		{
			vec3 baryCoord = abc.invert() * vec3 { static_cast<double>(x), static_cast<double>(y), 1. };
			
			vec3 baryCoord_New = vec3{baryCoord.x * invert_w[0], baryCoord.y * invert_w[1], baryCoord.z * invert_w[2]};//透视校正
			vec3 baryCoord_Normalized = baryCoord_New / (baryCoord_New.x + baryCoord_New.y + baryCoord_New.z);//重心坐标归一化
			
			if (baryCoord_Normalized.x < 0 || baryCoord_Normalized.y < 0 || baryCoord_Normalized.z < 0)continue;//三角形外


			float z = dot(baryCoord_Normalized, vec3{ ndc[0].z,ndc[1].z,ndc[2].z });
			
			z = (z + 1.) * 0.5;



			//std::cout << z << std::endl;
			if (z < 0 || z > 1.)continue;//深度裁剪

			if(z<zBuffer.get(x,y))//深度测试
			{
				zBuffer.update(x, y, z);
				//std::cout << z << std::endl;
			}
			else continue;

			fragmentOutput output = fragment(baryCoord_Normalized,z);
			//std::cout << "颜色" << (int)output.color[0] << " " << (int)output.color[1] << " " << (int)output.color[2] << std::endl;
			if (output.discard)continue;
			frameBuffer.set(x, y, output.color);
		}
	}
}

vec4 sample2D(const TGAImage& image, const vec2& uv)
{
    // Bilinear sampling. Map uv in [0,1] to texel centers in [0, w-1]/[0, h-1]
	int w = image.width();
	int h = image.height();
	if (w <= 0 || h <= 0) return vec4{0,0,0,0};

	double u = uv.x * (w - 1);
	double v = uv.y * (h - 1);

	int x0 = std::clamp(static_cast<int>(std::floor(u)), 0, w - 1);
	int y0 = std::clamp(static_cast<int>(std::floor(v)), 0, h - 1);
	int x1 = std::clamp(x0 + 1, 0, w - 1);
	int y1 = std::clamp(y0 + 1, 0, h - 1);

	double sx = u - x0;
	double sy = v - y0;

	TGAColor c00 = image.get(x0, y0);
	TGAColor c10 = image.get(x1, y0);
	TGAColor c01 = image.get(x0, y1);
	TGAColor c11 = image.get(x1, y1);

	// Interpolate each channel. Keep the same channel indices as before: c[i]
	double ch[4] = {0,0,0,0};
	for (int i = 0; i < 4; ++i) {
		double v00 = c00[i];
		double v10 = c10[i];
		double v01 = c01[i];
		double v11 = c11[i];

		double i0 = v00 * (1.0 - sx) + v10 * sx;
		double i1 = v01 * (1.0 - sx) + v11 * sx;
		ch[i] = (i0 * (1.0 - sy) + i1 * sy) / 255.0;
	}

	return vec4{ ch[0], ch[1], ch[2], ch[3] };
}
double generate_random_double(double min, double max)
{
	// 1. 定义随机数引擎（推荐使用 mt19937，梅森旋转算法，随机性好）
	// 注意：引擎只需初始化一次，避免每次调用函数都重新初始化（否则随机性差）
	static thread_local std::mt19937 engine(std::random_device{}());
	static thread_local std::uniform_real_distribution<double> dist(min, max);
	return dist(engine);
}
double lerp(double a, double b, double f)
{
	return a + f * (b - a);
}

int  shadowGet(Buffer* shadowBuffer, const float x, const float y,  const float currentZ)
{
	//std::cout << "标准化x: " << x << " 标准化y " << y << std::endl;
	// x, y 范围是 [-1, 1]，转换到 [0, 1]
	//std::cout << "进入 shadowGet 函数" << std::endl;
	
	int BufferHeight = shadowBuffer->getHeight();
	int BufferWidth = shadowBuffer->getWidth();

	// 转换到像素坐标（使用浮点数，然后转为整数）
	//float fx = x * (BufferWidth - 1);
	//float fy = y * (BufferHeight - 1);

	//std::cout << "转换后x: " << fx << " 转换后y " << fy << std::endl;

	int newX = static_cast<int>(x);
	int newY = static_cast<int>(y);
	
	// 边界检查
	if (newX < 0 || newX >= BufferWidth || newY < 0 || newY >= BufferHeight)
		return 1;  // 边界外返回可见
	float z = shadowBuffer->get(newX, newY);
	

	
	//std::cout << "当前的z: " << currentZ << std::endl;
	if (z == 1)
		return 0;
    //std::cout << "阴影缓冲的z: " << z << std::endl;
	if (z + 0.01 < currentZ)
		return 0;
	else
		return 1;
}



float PCF(Buffer* shadowBuffer, const float x, const float y, const float currentZ)
{
	int BufferHeight = shadowBuffer->getHeight();
	int BufferWidth = shadowBuffer->getWidth();

	// 转换到像素坐标（使用浮点数，然后转为整数）
	//float fx = x * (BufferWidth - 1);
	//float fy = y * (BufferHeight - 1);

	//std::cout << "转换后x: " << fx << " 转换后y " << fy << std::endl;

	int newX = static_cast<int>(x);
	int newY = static_cast<int>(y);

	int total = 1;

	if (newX < 0 || newX >= BufferWidth || newY < 0 || newY >= BufferHeight)
		return 0;
	
	float rz = shadowBuffer->get(newX, newY);
	
	if (rz + 0.01 > currentZ)
		return 1;
	
	
	else
	{
		total = std::ceil((currentZ - rz) * 100.f / rz);
		//std::cout << "范围：" << total<<std::endl;
		if (total==0)
			total = 1;

	}

	float z = 0;
	
	for (int i = -total; i <=total; i++)
	{
		int x = newX + i;
		// 边界检查
		if (x < 0 || x >= BufferWidth || newY < 0 || newY >= BufferHeight)
			z += 0;

		float tz = shadowBuffer->get(x, newY);


		//std::cout << "当前的z: " << currentZ << std::endl;
		if (tz == 1)
			z+= 1;
		//std::cout << "阴影缓冲的z: " << z << std::endl;
		if (tz + 0.01 < currentZ)
			z += 0;
		else
			z += 1;
	}

	z /= total*2;
	for (int i = -total; i <= total; i++)
	{
		int y = newY + i;
		// 边界检查
		if (newX < 0 || newX >= BufferWidth || y < 0 || y >= BufferHeight)
			z += 0;

		float tz = shadowBuffer->get(newX, y);


		//std::cout << "当前的z: " << currentZ << std::endl;
		if (tz == 1)
			z += 1;
		//std::cout << "阴影缓冲的z: " << z << std::endl;
		if (tz + 0.01 < currentZ)
			z += 0;
		else
			z += 1;
	}
	z /= total * 2;
	
	

	return z;
}


float PCSS(Buffer* shadowBuffer, const float x, const float y, const float currentZ)
{
	int BufferHeight = shadowBuffer->getHeight();
	int BufferWidth = shadowBuffer->getWidth();

	// 转换到像素坐标（使用浮点数，然后转为整数）
	//float fx = x * (BufferWidth - 1);
	//float fy = y * (BufferHeight - 1);

	//std::cout << "转换后x: " << fx << " 转换后y " << fy << std::endl;

    // Convert to integer texel coords (shadow map is in texel space)
	int cx = static_cast<int>(std::round(x));
	int cy = static_cast<int>(std::round(y));

	if (cx < 0 || cx >= BufferWidth || cy < 0 || cy >= BufferHeight)
		return 1.f; // treat outside as lit

	const float bias = 0.005f; // depth bias to reduce acne
	float centerDepth = shadowBuffer->get(cx, cy);
	if (centerDepth == 1.f)
		return 1.f; // no blocker recorded -> lit

	// If center is lit according to shadow map, return fully lit
	if (centerDepth + bias > currentZ)
		return 1.f;



	
	float blockSum = 0;
	int radius = std::round((currentZ-centerDepth) * 50);
	if (radius == 0)
		return 0;
	for (int i = -radius; i <= radius; i++)
	{
		int newX = cx + i;
		if (newX<0 || newX>BufferWidth)
			continue;

		for (int j = -radius; j <= radius; j++)
		{
			int newY = cy + j;
			if (newY<0 || newY>BufferHeight)
			{
				continue;
			}

			float cz = shadowBuffer->get(newX, newY);
			
			if (cz == 1)
			{
				continue;
			}

			if (cz + bias < currentZ)
				blockSum += cz;
			else
				continue;
	
		}
	}

	

	

	float avgBlocker = blockSum / std::pow(radius*2,2);
	
	const float lightScale = 30.f; // tunable scale factor (in texels)
	int filterRadius = static_cast<int>(std::ceil(lightScale * (currentZ - avgBlocker) / std::max(1e-6f, avgBlocker)));
	filterRadius = std::clamp(filterRadius, 1, 40);

	int samples = 0;
	int lit = 0;
	for (int oy = -filterRadius; oy <= filterRadius; ++oy) {
		int py = cy + oy;
		if (py < 0 || py >= BufferHeight) continue;
		float d = shadowBuffer->get(cx, py);
		if (d == 1.f) { // no depth === treat as lit
			++lit; ++samples; continue;
		}
		if (d + bias > currentZ) ++lit;
		++samples;
	}

	for (int ox = -filterRadius; ox <= filterRadius; ++ox) {
		int px = cx + ox;
		if (px < 0 || px >= BufferWidth) continue;
		float d = shadowBuffer->get(px, cy);
		if (d == 1.f) { // no depth === treat as lit
			++lit; ++samples; continue;
		}
		if (d + bias > currentZ) ++lit;
		++samples;
	}
	if (samples == 0) return 0.f;
	return float(lit) / float(samples);
}

float getSSAO(TGAImage* ssaoBuffer, float x, float y)
{
    int width = ssaoBuffer->width();
	int height = ssaoBuffer->height();

	
	if (width <= 0 || height <= 0) return 1.f;

	
	int cx = static_cast<int>(std::round(x));
	int cy = static_cast<int>(std::round(y));

	
	cx = std::clamp(cx, 0, width - 1);
	cy = std::clamp(cy, 0, height - 1);

	// 3x3  (sum = 16)
	constexpr int kernel[3][3] = {
		{1, 2, 1},
		{2, 4, 2},
		{1, 2, 1}
	};

	int ksum = 16;
	float accum = 0.f;
	for (int ky = -1; ky <= 1; ++ky) {
		int py = std::clamp(cy + ky, 0, height - 1);
		for (int kx = -1; kx <= 1; ++kx) {
			int px = std::clamp(cx + kx, 0, width - 1);
			float sample = ssaoBuffer->get(px, py)[0] / 255.f;
			accum += sample * static_cast<float>(kernel[ky + 1][kx + 1]);
		}
	}

	float ao = accum / static_cast<float>(ksum);
	return std::clamp(ao, 0.f, 1.f);
}


float SSAO(Buffer* ssaoBuffer, Ubo* ubo, vec3 worldPos, const float currentZ,vec3 normal)
{
	int bufferWidth = ssaoBuffer->getWidth();
	int bufferHeight = ssaoBuffer->getHeight();

	float ao = 0;

	for (int i = 0; i < 6; i++)
	{
		vec3 newNormal = normalize(vec3{
			normal.x + generate_random_double(0.,1.),
			normal.y + generate_random_double(0.,1.),
			normal.z + generate_random_double(0.,1.),
		});

		vec3 newPos = worldPos + newNormal;
		vec4 projPos = ubo->getProjMat() * ubo->getViewMat() * vec4 { newPos.x, newPos.y, newPos.z, 1 };
		vec4 ndcPos = projPos / projPos.w;
		vec4 viewPortPos = ubo->getViewPortMat() * ndcPos;
		int cx = std::round(viewPortPos.x);
		int cy= std::round(viewPortPos.y);

		if (cx<0 || cx > bufferWidth || cy<0 || cy > bufferHeight)
		        continue;
		float currentZ = (ndcPos.z + 1) * 0.5;
		float bufferZ = ssaoBuffer->get(cx, cy);

		if (currentZ <= bufferZ)
			ao += bufferZ;
		else
			continue;

	}

	ao /= 6;
	//std::cout << "ao值： " << ao << std::endl;
	ao=std::clamp(ao, 0.f, 1.f);
	return  ao;
}