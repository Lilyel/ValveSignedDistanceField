// Must be 1.2 or lower to work easily with SFML.
#version 120

uniform sampler2D sourceTexture;
uniform int imageType;
uniform float spread;
uniform ivec2 size;

const float ERROR_OK = 0.01;
const float IN_THRESHOLD = 1.0 - ERROR_OK;

// According to user setting, return the channel of the color to use for processing
// the signed distance field.
float GetPixelToProcess( vec4 _pixel )
{	
	if( imageType < 0 || imageType > 3 )
		return 0.0;

	vec4 channels = vec4( _pixel.x, _pixel.w, ( _pixel.x + _pixel.y + _pixel.z ) / 3.0, _pixel.w );

	return channels[imageType];
}

void SetPixelToProcess( out vec4 newPixel, vec4 _pixel, float pixelDistance )
{
	if( imageType < 0 || imageType > 3 )
	{
		newPixel = _pixel;	
		return;
	}

	if( imageType == 0 )
		newPixel = vec4( pixelDistance, pixelDistance, pixelDistance, _pixel.w );

	else if( imageType == 1 )
		newPixel = vec4( _pixel.xyz, pixelDistance );

	else if( imageType == 2 )
	{
		newPixel.x = _pixel.x / 3.0 * pixelDistance;
		newPixel.y = _pixel.y / 3.0 * pixelDistance;
		newPixel.z = _pixel.z / 3.0 * pixelDistance;
		newPixel.w = _pixel.w;
	}
	
	else if( imageType == 2 )
		newPixel = vec4( _pixel.xyz, pixelDistance );

}

void GetMinMaxArea( out ivec2 minArea, out ivec2 maxArea, out ivec2 texCoordInPix, vec2 texCoord, ivec2 size )
{	
	texCoordInPix = ivec2( texCoord.x * size.x, texCoord.y * size.y );

	minArea = ivec2( max( texCoordInPix.x - spread, 0 ), max( texCoordInPix.y - spread, 0 ) );
	maxArea = ivec2( min( texCoordInPix.x + spread, size.x - 1 ), min( texCoordInPix.y + spread, size.y - 1 ) );
}

vec2 GetUVNormalized( int i, int j, ivec2 size )
{
	return vec2( float(i) / float(size.x), float(j) / float(size.y) );
}

// Process the distance between the pixel and the nearest pixel outside the shape.
float GetDistanceToNearestOut( vec2 texCoord )
{
	ivec2 texCoordInPix;

	// Search area around the pixel.
	// The distance can't exceed the spread parameter
	// so we search in the square of size spread * 2 around the pixel.
	ivec2 minArea;
	ivec2 maxArea;
	GetMinMaxArea( minArea, maxArea, texCoordInPix, texCoord, size );

	float shortestDistance = spread * spread;
	float channelToTest = 0.0;

	for( int i = minArea.x; i < maxArea.x; i++ )
	{
		for( int j = minArea.y; j < maxArea.y; j++ )
		{
			vec2 uvToTest = GetUVNormalized( i, j, size );
			channelToTest = GetPixelToProcess( texture2D( sourceTexture, uvToTest ) );

			// Test only if it is outside the shape.
			// By logic, we should test <= IN_THRESHOLD, but it doesn't seems to work well...
			if( channelToTest <= 0.0 )
			{
				// Process manhatan distance.
				float currentDistance = pow( float(i) - float(texCoordInPix.x), 2.0 ) + pow( float(j) - float(texCoordInPix.y), 2.0 );

				if( currentDistance < shortestDistance )
					shortestDistance = currentDistance;
			}
		}
	}
	// In this case, if we didn't find the nearest pixel, that fine, 
	// we just put a big value as distance (spread * spread).
	// That will "simulate" a pixel very far away.
	return shortestDistance;
}

// Process the distance between the pixel and the nearest pixel inside the shape.
bool GetDistanceToNearestIn( out vec4 nearestColor, out float toNearestDistance, vec2 texCoord )
{
	ivec2 texCoordInPix;

	// Search area around the pixel.
	// The distance can't exceed the spread parameter
	// so we search in the square of size spread * 2 around the pixel.
	ivec2 minArea;
	ivec2 maxArea;
	GetMinMaxArea( minArea, maxArea, texCoordInPix, texCoord, size );

	toNearestDistance = spread * spread;
	float channelToTest = 0.0;
	bool foundNearest = false;

	for( int i = minArea.x; i < maxArea.x; i++ )
	{
		for( int j = minArea.y; j < maxArea.y; j++ )
		{
			vec2 uvToTest = GetUVNormalized( i, j, size );
			vec4 pixelToTest = texture2D( sourceTexture, uvToTest );
			channelToTest = GetPixelToProcess( pixelToTest );

			// Test only if it is inside the shape.
			if( channelToTest >= IN_THRESHOLD )
			{
				// Process manhatan distance.
				float currentDistance = pow( float(i) - float(texCoordInPix.x), 2.0 ) + pow( float(j) - float(texCoordInPix.y), 2.0 );

				if( currentDistance < toNearestDistance )
				{
					toNearestDistance = currentDistance;
					// Save also the color to preserve image color and not generating white edges.
					nearestColor = pixelToTest;
				}
			}
		}
	}
	// Signal that we didn't find an in pixel in the area.
	return foundNearest;
}

void main()
{
	vec2 texCoord = gl_TexCoord[0].xy;
    vec4 pixel = texture2D( sourceTexture, texCoord );

	float pixelChannelToProcess = GetPixelToProcess( pixel );

	// If the pixel value in bigger than the threshold, we consider the pixel inside the shape.
	if( pixelChannelToProcess >= IN_THRESHOLD )
	{
		float toNearestDistance = GetDistanceToNearestOut( texCoord );
		// Transform the distance to an euclydean distance
		// and bring it in the [0, 1] interval, where 0.5 is the edge of the shape.
		// The pixel will have a high value if it is in far from the out area
		// (if the pixel is deep in the shape)
		toNearestDistance = clamp( sqrt( toNearestDistance ) / spread, 0.0, 1.0 );
		toNearestDistance = toNearestDistance * 0.5 + 0.5;

		SetPixelToProcess( gl_FragColor, pixel, toNearestDistance );
	}
	// Otherwise the pixel is considered outside the shape.
	else
	{
		vec4 nearestColor;
		float toNearestDistance = 0.0;
		bool nearestFound = GetDistanceToNearestIn( nearestColor, toNearestDistance, texCoord );
		// Transform the distance to an euclydean distance
		// and bring it in the [0, 1] interval, where 0.5 is the edge of the shape.
		// (and reverse it, we need the pixel value to decrease if it is far away from the shape.
		toNearestDistance = clamp( sqrt( toNearestDistance ) / spread, 0.0, 1.0 );
		toNearestDistance = ( 1.0 - toNearestDistance ) * 0.5;

		// If we found the nearest pixel inside the shape, take its color
		// to prevent white edges.
		vec4 finalPixelColor = nearestFound ? nearestColor : pixel;

		SetPixelToProcess( gl_FragColor, finalPixelColor, toNearestDistance );
	}
}