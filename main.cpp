 
#include "Shader.h"
#include "UBO.h"
#include "model.h"



constexpr int width = 3840;
constexpr int height = 2160;
int shadowW = 2048;
int shadowH = 2048;

constexpr TGAColor white = { 255,255,255,255 };  //在编译期运行，运作常量
constexpr TGAColor green = { 0, 255,   0, 255 };
constexpr TGAColor red = { 0,   0, 255, 255 };
constexpr TGAColor blue = { 255, 128,  64, 255 };
constexpr TGAColor yellow = { 0, 200, 255, 255 };
 












int main(int argc, char** argv) {
      

	vec3 eye = { 1,1,4 };
	vec3 center = { 0,0,0 };
	vec3 up = { 0,1,0 };
    vec3 lightPos = { 4,6,3 };
	Ubo phongUBO;
	phongUBO.setCameraInfoAndBuild(eye, center, up);
	phongUBO.perspectiveMatBuild(30, static_cast<double>(width) / static_cast<double>(height),0.1, 20);
	phongUBO.setViewPort(0, 0, width, height);
    TGAImage framebuffer(width, height, TGAImage::RGB);
    TGAImage shadow(shadowW, shadowW, TGAImage::GRAYSCALE);
    TGAImage ssao(width, width, TGAImage::GRAYSCALE);
    TGAImage ssaoBuffer(width, width, TGAImage::GRAYSCALE);
    
    Buffer zBuffer(width, height, 1);
    Buffer shadowBuffer(shadowW, shadowW, 1);
    Buffer ssaoZBuffer(width, height, 1);
    Buffer trashBuffer(width, height, 1);


    Ubo shadowUBO;
    shadowUBO.setViewPort(0, 0, shadowW, shadowW);
    shadowUBO.setCameraInfoAndBuild(lightPos, center, up);
    shadowUBO.orthographicMatBuild(4, 4, 0.1,12);

    mat<4, 4> lmat =  shadowUBO.getProjMat() * shadowUBO.getViewMat();

    for (int m = 1; m < argc; m++)
    {
        Model model(argv[m]);
        ShadowShader shader(model, shadowUBO);
        for (int i =0; i < model.sumFace(); i++)
        {
            vec4 clip[3];
            for (int j : {0, 1, 2})
                clip[j] = shader.vertProcess(i, j);
            shader.rasterize(clip, shadow, shadowBuffer);
        }
    }

    shadow.write_tga_file("shadow.tga");

    for (int m = 1; m < argc; m++)
    {
        Model model(argv[m]);
        ShadowShader shader(model, phongUBO);
        for (int i = 0; i < model.sumFace(); i++)
        {
            vec4 clip[3];
            for (int j : {0, 1, 2})
                clip[j] = shader.vertProcess(i, j);
            shader.rasterize(clip, ssao, ssaoZBuffer);
        }
    }
    
    for (int m = 1; m < argc; m++)
    {
        Model model(argv[m]);
        SSAOShader shader(model, phongUBO,ssaoZBuffer);
        for (int i = 0; i < model.sumFace(); i++)
        {
            vec4 clip[3];
            for (int j : {0, 1, 2})
                clip[j] = shader.vertProcess(i, j);
            shader.rasterize(clip, ssaoBuffer, trashBuffer);
        }
    }
    ssaoBuffer.write_tga_file("ssao.tga");


    for (int m = 1; m < argc; m++)
    {
        Model model(argv[m]);
        phongWithShadowWithSSAOShader shader(model, phongUBO, ssaoBuffer,shadowBuffer);
        shader.lightTrans = lmat;
        shader.camPos = eye;
        shader.lightPos = lightPos;
        shader.lightViewPort = shadowUBO.getViewPortMat();


        for (int i = 0; i < model.sumFace(); i++)
        {
            vec4 clip[3];
            for (int j : {0, 1, 2})
                clip[j] = shader.vertProcess(i, j);
            shader.rasterize(clip, framebuffer, zBuffer);
        }
    }

    framebuffer.write_tga_file("out.tga");


    
    return 0;
}





