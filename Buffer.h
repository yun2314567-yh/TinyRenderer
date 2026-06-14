#pragma once
#include<vector>
#include<stdexcept>
#include<cassert>
class Buffer
{
public:
	Buffer(const int& width, const int& height,double initialValue)
	{
		this->width = width;
		this->height = height;
		buffer = std::vector<double>(width*height,initialValue);
	}
	
	int getHeight() { return height; }
	int getWidth() { return width; }

	void update(const int& x, const int& y, const double& value)
	{
		if (buffer.empty()) throw std::runtime_error("«Î…Ë÷√¥Û–°");
		buffer[y + x * height] = value;
	}

	float get(const int& x, const int& y)const
	{
		return buffer[y + x * height];
	}
private:
	int width;
	int height;
	std::vector<double> buffer;
	
};