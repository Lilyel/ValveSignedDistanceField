// Must be 1.2 or lower to work easily with SFML.
#version 120

uniform sampler2D sourceTexture;
uniform int imageType;
uniform float smoothing;

uniform bool outline;
uniform vec4 outlineColor;
uniform vec2 outlineDepth; // x : outisde, y : inside.


uniform bool glow;
uniform vec4 glowColor;
uniform vec2 glowOffset; // For shadow.
uniform float glowStrength;



vec4 GetGlow( vec4 _color, vec4 _colorAlphaTested, int _channel, bool _isGrey, vec2 _uv )
{
	// If we are inside the shape, don't do a glow on this pixel.
	if( _colorAlphaTested[_channel] > 0.0 )
		return _colorAlphaTested;

	// Apply offset for shadow.
	vec4 offsetPixel = texture2D( sourceTexture, _uv + glowOffset );
	
	float offsetChannelValue = offsetPixel[_channel];
	// Handle grey scale.
	if( _isGrey )
		offsetChannelValue = 0.2126 * offsetPixel.r + 0.7152 * offsetPixel.g + 0.0722 * offsetPixel.b;

	// If the offset pixel is outside, apply the glow with it.
	// Otherwise, apply full glow color on the pixel.
	float channelMax = 0.5 - smoothing;
	float t = 1.0;
	if( offsetChannelValue <= channelMax )
	{
		t = offsetChannelValue / channelMax;
		t = t * glowStrength;
		t = clamp( t, 0.0, 1.0 );
	}

	return t * glowColor;	
}

vec4 GetOutline( vec4 _color, vec4 _colorAlphaTested, int _channel, bool _isGrey )
{
	float channelValue = _color[_channel];
	// Handle grey scale.
	if( _isGrey )
		channelValue = 0.2126 * _color.r + 0.7152 * _color.g + 0.0722 * _color.b;

	// Put the outline color in the pixel in the user range.
	if( channelValue >= 0.5 - outlineDepth.x && channelValue <= 0.5 + outlineDepth.y )
		return outlineColor;
	else
		return _colorAlphaTested;
}


void main()
{
	vec2 uv = gl_TexCoord[0].xy;
	vec4 pixel = texture2D( sourceTexture, uv );
		
	gl_FragColor = pixel;

	// Grey scale.
	if( imageType == 0 )
	{
		float greyColor = 0.2126 * pixel.r + 0.7152 * pixel.g + 0.0722 * pixel.b;
		float greySmooth = smoothstep( 0.5 - smoothing, 0.5 + smoothing, greyColor );

		gl_FragColor.r = greySmooth;
		gl_FragColor.g = greySmooth;
		gl_FragColor.b = greySmooth;

		if( outline )
			gl_FragColor = GetOutline( pixel, gl_FragColor, 0, true );
		if( glow )
			gl_FragColor = GetGlow( pixel, gl_FragColor, 0, true, uv );
	}

	// Grey scale + alpha and RGBA.
	else if( imageType == 1 || imageType == 3 )
	{
		float alphaSmooth = smoothstep( 0.5 - smoothing, 0.5 + smoothing, pixel.a );
		gl_FragColor.a = alphaSmooth;

		if( outline )
			gl_FragColor = GetOutline( pixel, gl_FragColor, 3, false );
		if( glow )
			gl_FragColor = GetGlow( pixel, gl_FragColor, 3, false, uv );
	}

	// RGB.
	else if( imageType == 2 )
	{
		float redSmooth = smoothstep( 0.5 - smoothing, 0.5 + smoothing, pixel.r );
		float greenSmooth = smoothstep( 0.5 - smoothing, 0.5 + smoothing, pixel.g );
		float blueSmooth = smoothstep( 0.5 - smoothing, 0.5 + smoothing, pixel.b );

		gl_FragColor.r = redSmooth;
		gl_FragColor.g = greenSmooth;
		gl_FragColor.b = blueSmooth;

		if( outline )
		{
			gl_FragColor.r = GetOutline( pixel, gl_FragColor, 0, false ).r;
			gl_FragColor.g = GetOutline( pixel, gl_FragColor, 1, false ).g;
			gl_FragColor.b = GetOutline( pixel, gl_FragColor, 2, false ).b;
		}
		if( glow )
		{
			gl_FragColor.r = GetGlow( pixel, gl_FragColor, 0, false, uv ).r;
			gl_FragColor.g = GetGlow( pixel, gl_FragColor, 1, false, uv ).g;
			gl_FragColor.b = GetGlow( pixel, gl_FragColor, 2, false, uv ).b;
		}
	}
}
