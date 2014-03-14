
# CRABBY #
![alt text](./icon.png "Logo Title Text 1")
A texture compression format for spritesheets

## Why does the encoder only take TGA files? ##
Simply to simplify distribution.


## Building the encoder ##
Currently, a visual studio project is provided. For ideal output, you should build the crabby.exe file to the ../../bin folder, so it can interact with the source textures

## Running the encoder ##
Crabby will take, as an input a text file which lists what images to consume and process, and output a set of files which start with 'outtextureprefix'
* crabby.exe <texturelilst> <outtextureprefix>
* e.g. crabby.exe in-textures/crab/imglist.txt out-textures/crab
* e.g. crabby.exe in-textures/jelly/imglist.txt out-textures/jellyfish


