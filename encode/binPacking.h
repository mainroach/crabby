#pragma once

class Rect2D
{
public:
	int left;
	unsigned int width;
	int top;
	unsigned int height;
	int id;

	Rect2D():left(0),top(0),width(0),height(0),id(0){};

	int right() const { return left + width;}
	int bottom() const { return top + height;}
	int area() const { return width * height;}
	float ratio() const { return (float)width / (float)height;}

	void set(const int l, const int t, const int w, const int h)
	{
		left = l; width = w; top = t; height = h;
	}
};

//------------------------------------
enum eSortType
{
	cSort_None = 0,
	cSort_Width = 1,
	cSort_Height = 2,
	cSort_MaxSide = 3,
	cSort_Area = 4
};

//------------------------------------
void packRectangles(std::vector<Rect2D>& blocks, const unsigned int sortType) ;