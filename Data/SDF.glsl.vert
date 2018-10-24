// Must be 1.2 or lower to work easily with SFML.
#version 120

void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;


    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

    // forward the vertex color
    gl_FrontColor = gl_Color;
}