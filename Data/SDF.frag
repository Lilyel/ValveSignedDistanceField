// Must be 1.2 or lower to work easily with SFML.
#version 120

uniform sampler2D sourceTexture;
uniform int imageType;
uniform int spread;
uniform ivec2 size;

const float ERROR_OK = 0.05;
const float IN_THRESHOLD = 1.0 - ERROR_OK;

// According to user setting, return the channel of the color to use for processing
// the signed distance field.
float GetPixelToProcess( vec4 _pixel )
{	
	if( imageType == 0 )
		return _pixel.r;

	else if( imageType == 1 )
		return _pixel.a;

	else if( imageType == 2 )
		return ( _pixel.r + _pixel.g + _pixel.b ) / 3.0;

	else if( imageType == 3 )
		return _pixel.a;

	else	
		return 0.0;
}

void SetFragColor( vec4 _pixel, float _pixelDistance )
{
	gl_FragColor = _pixel;

	if( imageType == 0 )
	{
		gl_FragColor.r = _pixelDistance;
		gl_FragColor.g = _pixelDistance;
		gl_FragColor.b = _pixelDistance;
	}

	else if( imageType == 1 )
		gl_FragColor.a = _pixelDistance;

	else if( imageType == 2 )
	{
		gl_FragColor.r = _pixel.r / 3.0 * _pixelDistance;
		gl_FragColor.g = _pixel.g / 3.0 * _pixelDistance;
		gl_FragColor.b = _pixel.b / 3.0 * _pixelDistance;
	}
	
	else if( imageType == 3 )
		gl_FragColor.a = _pixelDistance;

}

ivec2 GetTexCoordInPix( vec2 _texCoord, ivec2 _size )
{
	return ivec2( floor( _texCoord.x * float( _size.x ) ), floor( _texCoord.y * float( _size.y ) ) );
}

void GetMinMaxArea( out ivec2 _minArea, out ivec2 _maxArea, ivec2 _texCoordInPix, ivec2 _size )
{
	_minArea = ivec2( _texCoordInPix.x - spread, _texCoordInPix.y - spread );
	_maxArea = ivec2( _texCoordInPix.x + spread, _texCoordInPix.y + spread );

	_minArea.x = int( max( float( _minArea.x ), 0.0 ) );
	_minArea.y = int( max( float( _minArea.y ), 0.0 ) );

	_maxArea.x = int( min( float( _maxArea.x ), float( _size.x ) ) );
	_maxArea.y = int( min( float( _maxArea.y ), float( _size.y ) ) );

}

vec2 GetUVNormalized( int _i, int _j, ivec2 _size )
{
	return vec2( float(_i) / float(_size.x), float(_j) / float(_size.y) );
}

float GetLength( vec2 _vector )
{
	//return abs( _vector.x ) + abs( _vector.y );
	//return ( _vector.x * _vector.x + _vector.y * _vector.y );
	return length( _vector );
}

float GetPixelDistance( int _i, int _j, ivec2 _texCoordInPix )
{
	vec2 vecToPixel = vec2( float( _i - _texCoordInPix.x ), float( _j - _texCoordInPix.y ) );
	
	return GetLength( vecToPixel );
}

// Process the distance between the pixel and the nearest pixel outside the shape.
float GetDistanceToNearestOut( vec2 _texCoord, float _maxDistance )
{
	ivec2 texCoordInPix = GetTexCoordInPix( _texCoord, size );

	// Search area around the pixel.
	// The distance can't exceed the spread parameter
	// so we search in the square of size spread * 2 around the pixel.
	ivec2 minArea;
	ivec2 maxArea;
	GetMinMaxArea( minArea, maxArea, texCoordInPix, size );

	float shortestDistance = _maxDistance;
	float channelToTest = 0.0;

	for( int i = minArea.x; i < maxArea.x; i++ )
	{
		for( int j = minArea.y; j < maxArea.y; j++ )
		{
			// Process distance.
			float currentDistance = GetPixelDistance( i, j, texCoordInPix );
			

			// If the distance is better, check if that the pixel is outside the shape.
			if( currentDistance < shortestDistance )
			{
				vec2 uvToTest = GetUVNormalized( i, j, size );
				vec4 pixelToTest = texture2D( sourceTexture, uvToTest );
				channelToTest = GetPixelToProcess( pixelToTest );

				// By logic, we should test < IN_THRESHOLD, but it doesn't seems to work well...
				if( channelToTest < ERROR_OK )
					shortestDistance = currentDistance;
			}
		}
	}
	// In this case, if we didn't find the nearest pixel, that fine, 
	// we just put a big value as distance (max).
	// That will "simulate" a pixel very far away.
	return shortestDistance;
}

// Process the distance between the pixel and the nearest pixel inside the shape.
float GetDistanceToNearestIn( out vec4 _nearestColor, vec2 _texCoord, float _maxDistance )
{
	ivec2 texCoordInPix = GetTexCoordInPix( _texCoord, size );

	// Search area around the pixel.
	// The distance can't exceed the spread parameter
	// so we search in the square of size spread * 2 around the pixel.
	ivec2 minArea;
	ivec2 maxArea;
	GetMinMaxArea( minArea, maxArea, texCoordInPix, size );

	float shortestDistance = _maxDistance;
	float channelToTest = 0.0;
	bool foundNearest = false;

	for( int i = minArea.x; i < maxArea.x; i++ )
	{
		for( int j = minArea.y; j < maxArea.y; j++ )
		{
			// Process distance.
			float currentDistance = GetPixelDistance( i, j, texCoordInPix );

			// If the distance is better, check if that the pixel is inside the shape.
			if( currentDistance < shortestDistance )
			{
				vec2 uvToTest = GetUVNormalized( i, j, size );
				vec4 pixelToTest = texture2D( sourceTexture, uvToTest );
				channelToTest = GetPixelToProcess( pixelToTest );

				if( channelToTest >= IN_THRESHOLD )
				{				
					shortestDistance = currentDistance;
					// Save also the color to preserve image color and not generating white edges.
					_nearestColor = pixelToTest;
					foundNearest = true;
				}
			}
		}
	}

	return shortestDistance;
}

void main()
{
	vec2 texCoord = gl_TexCoord[0].xy;
    vec4 pixel = texture2D( sourceTexture, texCoord );

	float maxDistance = float( spread );
	float minDistance = -maxDistance;


	float pixelChannelToProcess = GetPixelToProcess( pixel );
	float toNearestDistance = maxDistance;
	vec4 finalColor = pixel;

	bool isIn = pixelChannelToProcess >= IN_THRESHOLD;

	// If the pixel value in bigger than the threshold, we consider the pixel inside the shape.
	if( isIn )
	{
		toNearestDistance = GetDistanceToNearestOut( texCoord, maxDistance );
		// Transform the distance to an euclydean distance
		// and bring it in the [0, 1] interval, where 0.5 is the edge of the shape.
		// The pixel will have a high value if it is in far from the out area
		// (if the pixel is deep in the shape)
		//toNearestDistance = clamp( sqrt( toNearestDistance ) / spread, 0.0, 1.0 );
		//toNearestDistance = toNearestDistance * 0.5 + 0.5;

		//SetPixelToProcess( gl_FragColor, pixel, toNearestDistance );
		//toNearestDistance = sqrt( toNearestDistance );
	}
	// Otherwise the pixel is considered outside the shape.
	else
	{
		toNearestDistance = GetDistanceToNearestIn( finalColor, texCoord, maxDistance );
		// Transform the distance to an euclydean distance
		// and bring it in the [0, 1] interval, where 0.5 is the edge of the shape.
		// (and reverse it, we need the pixel value to decrease if it is far away from the shape.
		//toNearestDistance = clamp( sqrt( toNearestDistance ) / spread, 0.0, 1.0 );
		//toNearestDistance = ( 1.0 - toNearestDistance ) * 0.5;

		//SetPixelToProcess( gl_FragColor, finalColor, toNearestDistance );

		// We are out, so the distance is negative (0 is the surface and positive values are inside).
		//toNearestDistance = sqrt( toNearestDistance );
		toNearestDistance = -toNearestDistance;
	}


	
	toNearestDistance = clamp( toNearestDistance, minDistance, maxDistance );
	toNearestDistance = ( toNearestDistance - minDistance ) / ( maxDistance - minDistance );
	
	SetFragColor( finalColor, toNearestDistance );
	//gl_FragColor = vec4( toNearestDistance , toNearestDistance, toNearestDistance, 1.0 );
}