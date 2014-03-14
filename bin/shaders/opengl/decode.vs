uniform mat4 a_MVP;
void main( void ){
    gl_Position = a_MVP * gl_Vertex;
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_FrontColor = gl_Color;
};