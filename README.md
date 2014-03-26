
<img src="https://raw.github.com/mainroach/crabby/master/icon.jpg" width="256" align="right" hspace="20">

# CRABBY #
A texture compression format for spritesheets

## Crabby TL;DR ##
Crabby is a compressed texture format for spritesheets and flipbook animations.

## What is Crabby? ##
Crabby is an encoder / texture format for creating a compressed sprite sheet with direct-access decompression on the GPU w/o the need for an intermediate texture.

## Why is Crabby? ##
Crabby is a compressed texture format for spritesheets, which allows decompression on the GPU without the need of intermediate decode stages. Other formats like JPG, GIF, and H264 require decodes to intermediate CPU side textures, followed by a required upload to the GPU before they can be used. Crabby works by creating a unique palette of pixel blocks (typically 4x4), and transcodes the original images as a combination of the block palette, and pointers to what block indexes. The savings from crabby come from the fact that most sprites in a spritesheet contain large amounts of of duplicate pixels that can be removed in this process. Crabby itself is designed to work hand-in-hand with other GPU compressed texture formats like ETC, DXT, and PVR to achieve maximum savings.

## Who is Crabby For? ##
Crabby is designed for 2D games which are heavy with flipbook based sprite animations. In it's current form, Crabby is targeted to reuce the size of GPU and distribution residency. As such, it makes certain assumptions about your data set, and how you're going to use textures in your game. Crabby is not built for 3D textures, and currently doesn't support mip-mapping.

## What is Crabby's development status? ##
Crabby represents an ALPHA state of development. If it does not meet your needs, please file an issue for a specific feature, and we will address it.


## How does Crabby work? ##
**Crabby works by finding removing duplicate 4x4 blocks of pixels from an atlas image.** Spritesheets tend to have lots of blank space, in addition to lots of duplicated color structure. Crabby search all the input textures provided, will list all the unique blocks in a single 'pallete texture' image. For each image provided, Crabby will output a 'frame' texture that references, in raster scan order, what 4x4 pixel block should go there. For more information, you can see a [document covering the algorithms in more detail](https://docs.google.com/document/d/1cnBuCWvflUUAM70mGelCp7upVXwwWNw7zh_s0B5-6F4/edit?usp=sharing).

## What type of compression savings does Crabby provide? ##

The compression performance of Crabby is based entirely on how much duplicate 4x4 pixel blocks can be removed from a sprite sheet. For some examples, you can view the running [results document](https://docs.google.com/document/d/1Zvn98TA5QRZIf6lZ5HqrHrBWJi7zkeo9UWIruJuUqdc/edit) for a rough idea of what types of images do well with Crabby.

## What type of performance hit does Crabby cause? ##
Pretty well, actually. The largest (potential) bottleneck comes from the dependent texture read.  For desktop hardware, this fetch is negligable (webGL, modern desktop ,console GPUs easily eat this up). For mobile hardware, this entirely depends on your GPU. We've seen some performance concerns on older tiling architectures, and low-cost GPUs.
[results document](https://docs.google.com/document/d/1Zvn98TA5QRZIf6lZ5HqrHrBWJi7zkeo9UWIruJuUqdc/edit) will contain more benchmark tests as they are run and provided. 
