// Must be 1.2 or lower to work easily with SFML.
#version 120

uniform sampler2D sourceTexture;
uniform int imageType;
uniform float smoothing;

void main()
{
	vec4 pixel = texture2D( sourceTexture, gl_TexCoord[0].xy );
		
	if( imageType == 0 )
	{
		float greyColor = 0.2126 * pixel.r + 0.7152 * pixel.g + 0.0722 * pixel.b;
		float greySmooth = smoothstep( 0.5 - smoothing, 0.5 + smoothing, greyColor );

		gl_FragColor = vec4( greySmooth, greySmooth, greySmooth, pixel.a );
	}

	else if( imageType == 1 || imageType == 3 )
	{
		float alphaSmooth = smoothstep( 0.5 - smoothing, 0.5 + smoothing, pixel.a );
		gl_FragColor = vec4( pixel.xyz, alphaSmooth );
	}
	else if( imageType == 2 )
	{
		float redSmooth = smoothstep( 0.5 - smoothing, 0.5 + smoothing, pixel.r );
		float greenSmooth = smoothstep( 0.5 - smoothing, 0.5 + smoothing, pixel.g );
		float blueSmooth = smoothstep( 0.5 - smoothing, 0.5 + smoothing, pixel.b );

		gl_FragColor = vec4( redSmooth, greenSmooth, blueSmooth, pixel.a );
	}
	else
		gl_FragColor = pixel;
}