#version 120

uniform sampler2D sourceTexture;
//uniform int mipmapLevel;

void main()
{
    gl_FragColor = texture2D( sourceTexture, gl_TexCoord[0].xy );
}
