// Must be 1.2 or lower to work easily with SFML.
#version 120

uniform sampler2D sourceTexture;
uniform int imageType;
uniform float smoothing;

void main()
{
	gl_FragColor = texture2D( sourceTexture, gl_TexCoord[0].xy );
		
	if( imageType == 0 )
	{
		float greyColor = 0.2126 * gl_FragColor.r + 0.7152 * gl_FragColor.g + 0.0722 * gl_FragColor.b;
		float greySmooth = smoothstep( 0.5 - smoothing, 0.5 + smoothing, greyColor );

		gl_FragColor.r = greySmooth;
		gl_FragColor.g = greySmooth;
		gl_FragColor.b = greySmooth;
	}

	else if( imageType == 1 || imageType == 3 )
	{
		float alphaSmooth = smoothstep( 0.5 - smoothing, 0.5 + smoothing, gl_FragColor.a );
		gl_FragColor.a = alphaSmooth;
	}

	else if( imageType == 2 )
	{
		float redSmooth = smoothstep( 0.5 - smoothing, 0.5 + smoothing, gl_FragColor.r );
		float greenSmooth = smoothstep( 0.5 - smoothing, 0.5 + smoothing, gl_FragColor.g );
		float blueSmooth = smoothstep( 0.5 - smoothing, 0.5 + smoothing, gl_FragColor.b );

		gl_FragColor.r = redSmooth;
		gl_FragColor.g = greenSmooth;
		gl_FragColor.b = blueSmooth;
	}
}
