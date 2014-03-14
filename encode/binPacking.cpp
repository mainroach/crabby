#include <vector>
#include <algorithm>
#include "binPacking.h"
/******************************************************************************

ORIGIONAL AUTHOR : https://github.com/jakesgordon/bin-packing/
I (Colt McAnlis) converted it to C++ for use in this project

This is a binary tree based bin packing algorithm that is more complex than
the simple Packer (packer.js). Instead of starting off with a fixed width and
height, it starts with the width and height of the first block passed and then
grows as necessary to accomodate each subsequent block. As it grows it attempts
to maintain a roughly square ratio by making 'smart' choices about whether to
grow right or down.

When growing, the algorithm can only grow to the right OR down. Therefore, if
the new block is BOTH wider and taller than the current target then it will be
rejected. This makes it very important to initialize with a sensible starting
width and height. If you are providing sorted input (largest first) then this
will not be an issue.

A potential way to solve this limitation would be to allow growth in BOTH
directions at once, but this requires maintaining a more complex tree
with 3 children (down, right and center) and that complexity can be avoided
by simply chosing a sensible starting block.

Best results occur when the input blocks are sorted by height, or even better
when sorted by max(width,height).

https://github.com/jakesgordon/bin-packing/



******************************************************************************/



class Node
{
public:
	bool used;
	Rect2D bounds;
	Node* down;
	Node* right;
	Node():down(NULL),right(NULL),used(false){};
	//------------------------------------
	void split(const unsigned int w, const unsigned int h) 
	{
		 used = true;
		 down = new Node();
		 down->bounds.set( bounds.left,  bounds.top + h, bounds.width, bounds.height-h);
		 right = new Node();
		 right->bounds.set(bounds.left + w, bounds.top, bounds.width - w, bounds.height);
	};
};

Node* globalRoot = new Node();
//------------------------------------
Node* findNode(Node* pRoot, const unsigned int w, const unsigned int h) 
{
    if (pRoot->used)
	 {
		Node* outNode = findNode(pRoot->right, w, h);
		if(outNode) return outNode;

		return findNode(pRoot->down, w, h);
	 }

    else if ((w <= pRoot->bounds.width) && (h <= pRoot->bounds.height))
      return pRoot;
    
    return NULL;
};
//------------------------------------
Node* growRight(const unsigned int w, const unsigned int h) 
{
	Node* newNode = new Node();
   newNode->used = true;
	newNode->bounds.set(0,0,globalRoot->bounds.width + w, globalRoot->bounds.height);
	newNode->right=new Node();
	newNode->right->bounds.set(globalRoot->bounds.width,0, w, globalRoot->bounds.height);
	newNode->down=globalRoot;

    globalRoot = newNode;
    Node* node = findNode(globalRoot, w, h);
    if (node)
	 {
		node->split(w,h);
		return node;
	 }
    
    return NULL;
  };
//------------------------------------
Node* growDown(const unsigned int w, const unsigned int h) 
{
	Node* newNode = new Node();
    newNode->used = true;
	 newNode->bounds.set(0,0,globalRoot->bounds.width,globalRoot->bounds.height+h);
	 newNode->down=new Node();
	 newNode->down->bounds.set(0,globalRoot->bounds.height, globalRoot->bounds.width,h);
	 newNode->right=globalRoot;

	 globalRoot = newNode;

    Node* node = findNode(globalRoot, w, h);
    if (node)
	 {
      node->split( w, h);
		return node;
	 }
    
    return NULL;
}

//------------------------------------
Node* growNode(const unsigned int w, const unsigned int h) 
  {
    const bool canGrowDown  = (w <= globalRoot->bounds.width);
    const bool canGrowRight = (h <= globalRoot->bounds.height);

    const bool shouldGrowRight = canGrowRight && (globalRoot->bounds.height >= (globalRoot->bounds.width + w)); // attempt to keep square-ish by growing right when height is much greater than width
    const bool shouldGrowDown  = canGrowDown  && (globalRoot->bounds.width >= (globalRoot->bounds.height + h)); // attempt to keep square-ish by growing down  when width  is much greater than height

	 

    if (shouldGrowRight)
      return growRight(w, h);
    else if (shouldGrowDown)
      return growDown(w, h);
    else if (canGrowRight)
     return growRight(w, h);
    else if (canGrowDown)
      return growDown(w, h);
    
    return NULL; // need to ensure sensible root starting size to avoid this happening
};
//------------------------------------
inline int max(const int a, const int b) { return a<b?b:a;}
inline int min(const int a, const int b) { return a>b?b:a;}
inline int _minVal (Rect2D i,Rect2D j) { return min(j.width,j.height) - min(i.width,i.height); }
inline int _maxVal (Rect2D i,Rect2D j) { return max(j.width,j.height) - max(i.width,i.height); }
inline int _widthDelta (Rect2D i,Rect2D j) { return j.width - i.width; }
inline int _heightDelta (Rect2D i,Rect2D j) { return j.height - i.height; }
inline int _areaDelta (Rect2D i,Rect2D j) { return (j.width*j.height) - (i.width*i.height); }
//------------------------------------
bool _heightWidthSort (Rect2D i,Rect2D j) 
{
	int v[2] = {_heightDelta(i,j), _widthDelta(i,j)};
	for(unsigned char q =0; q < 2; q++)
	{
		if (v[q] ==0) continue;
		return v[q] <0;
	}

	return false;
}
//------------------------------------
bool _widthHeightSort (Rect2D i,Rect2D j) 
{
	int v[2] = {_widthDelta(i,j), _heightDelta(i,j)};
	for(unsigned char q =0; q < 2; q++)
	{
		if (v[q] ==0) continue;
		return v[q] <0;
	}

	return false;
}
//------------------------------------
bool _maxSideSort(Rect2D i, Rect2D j)
{
	int v[4] = {_maxVal(i,j),_minVal(i,j),_widthDelta(i,j), _heightDelta(i,j)};
	for(unsigned char q =0; q < 4; q++)
	{
		if (v[q] ==0) continue;
		return v[q] <0;
	}

	return false;
}
//------------------------------------
bool _areaSort(Rect2D i, Rect2D j)
{
	int v[3] = {_areaDelta(i,j),_widthDelta(i,j), _heightDelta(i,j)};
	for(unsigned char q =0; q < 3; q++)
	{
		if (v[q] ==0) continue;
		return v[q] <0;
	}

	return false;
}
//------------------------------------
void packRectangles(std::vector<Rect2D>& blocks, const unsigned int sortType) 
{
	//sort the blocks first.
	if(sortType == cSort_Width)
		sort (blocks.begin(), blocks.end(), _widthHeightSort);
	else if(sortType == cSort_Height)
		sort (blocks.begin(), blocks.end(), _heightWidthSort);
	else if(sortType == cSort_MaxSide)
		sort (blocks.begin(), blocks.end(), _maxSideSort);
	else if(sortType == cSort_Area)
		sort (blocks.begin(), blocks.end(), _areaSort);

	const unsigned int len = blocks.size();

   const unsigned int w = len > 0 ? blocks[0].width : 0;
   const unsigned int h = len > 0 ? blocks[0].height : 0;
	
	globalRoot->bounds.set(0,0,w,h);

	for (unsigned int n = 0; n < len ; n++) 
	{
      Rect2D* block = &blocks[n];

		Node* node = findNode(globalRoot, block->width, block->height);

      if (node)
		{
        node->split(block->width, block->height);
		  block->left = node->bounds.left;
		  block->top = node->bounds.top;
		}
      else
		{
        node = growNode(block->width, block->height);
		  block->left = node->bounds.left;
		  block->top = node->bounds.top;
		}
    }
};
//------------------------------------

void testPacker()
{
	std::vector<Rect2D> blocks;
	Rect2D bk;
	bk.set(0,0,400,100);blocks.push_back(bk);
	bk.set(0,0,100,400);blocks.push_back(bk);
	bk.set(0,0,400,100);blocks.push_back(bk);
	bk.set(0,0,100,400);blocks.push_back(bk);
	bk.set(0,0,400,100);blocks.push_back(bk);
	bk.set(0,0,100,400);blocks.push_back(bk);
	
	
	packRectangles(blocks,cSort_MaxSide);

	for(unsigned i =0; i < blocks.size(); i++)
	{
		printf("%i,%i\n",blocks[i].left,blocks[i].top);
	}

}
