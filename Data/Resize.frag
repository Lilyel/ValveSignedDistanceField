#version 130

uniform sampler2D sourceTexture;
uniform int mipmapLevel;

void main()
{
    gl_FragColor = textureLod( sourceTexture, gl_TexCoord[0].xy, mipmapLevel );
}