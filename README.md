
<img src="https://raw.github.com/mainroach/crabby/master/icon.jpg" width="256" align="right" hspace="20">

# CRABBY #
A texture compression format for spritesheets

## What is Crabby? ##
Crabby is an encoder / texture format for creating a compressed sprite sheet with direct-access decompression on the GPU w/o the need for an intermediate texture.

## Why is Crabby? ##
Flipbook texturs often create lots of bloat and are not ideal for Gif and h264 both have to decomp to intermediate RGBA frame before being used in sampling. 
You could, technically use h264 and the disk format, and expand it to an atlas texture later, but that doesn’t decrease GPU footprint.
While these formats create great compression savings for distribution, they often create bloat on the GPU, which can create problems for devices you're running code on.

## What is Crabby's development status? ##
Crabby represents a work in progress to find alternate ways to compress sprite sheets for 2D games.


## How does Crabby work? ##
**Crabby works by finding removing duplicate 4x4 blocks of pixels from an atlas image.** Spritesheets tend to have lots of blank space, in addition to lots of duplicated color structure. Crabby search all the input textures provided, will list all the unique blocks in a single 'pallete texture' image. For each image provided, Crabby will output a 'frame' texture that references, in raster scan order, what 4x4 pixel block should go there. 

