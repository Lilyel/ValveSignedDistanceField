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
float GetChannelToProcess( vec4 _pixel )
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

	_maxArea.x = int( min( float( _maxArea.x ), float( _size.x - 1 ) ) );
	_maxArea.y = int( min( float( _maxArea.y ), float( _size.y - 1 ) ) );

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

vec4 GetPixel( int _i, int _j )
{
	vec2 uvToTest = GetUVNormalized( _i, _j, size );
	return texture2D( sourceTexture, uvToTest );
}



// Process the distance between the pixel and the nearest pixel outside the shape.
float GetDistanceToNearestOut( ivec2 _texCoordInPix, float _maxDistance, ivec2 _minArea, ivec2 _maxArea )
{
	float shortestDistance = _maxDistance;
	float channelToTest = 0.0;

	for( int i = _minArea.x; i < _maxArea.x; i++ )
	{
		for( int j = _minArea.y; j < _maxArea.y; j++ )
		{
			// Process distance.
			float currentDistance = GetPixelDistance( i, j, _texCoordInPix );			

			// If the distance is better, check if that the pixel is outside the shape.
			if( currentDistance < shortestDistance )
			{
				vec4 pixelToTest = GetPixel( i, j );
				channelToTest = GetChannelToProcess( pixelToTest );

				if( channelToTest < IN_THRESHOLD )
					shortestDistance = currentDistance;
			}
		}
	}

	return shortestDistance;
}

// Process the distance between the pixel and the nearest pixel inside the shape.
float GetDistanceToNearestIn( out vec4 _nearestColor, ivec2 _texCoordInPix, float _maxDistance, ivec2 _minArea, ivec2 _maxArea )
{
	float shortestDistance = _maxDistance;
	float channelToTest = 0.0;

	for( int i = _minArea.x; i < _maxArea.x; i++ )
	{
		for( int j = _minArea.y; j < _maxArea.y; j++ )
		{
			// Process distance.
			float currentDistance = GetPixelDistance( i, j, _texCoordInPix );

			// If the distance is better, check if that the pixel is inside the shape.
			if( currentDistance < shortestDistance )
			{
				vec4 pixelToTest = GetPixel( i, j );
				channelToTest = GetChannelToProcess( pixelToTest );

				if( channelToTest >= IN_THRESHOLD )
				{				
					shortestDistance = currentDistance;
					// Save also the color to preserve image color and not generating white edges.
					_nearestColor = pixelToTest;
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
	
	// The UV are in [0,1] interval, bring it back to pixel coordinates.
	ivec2 texCoordInPix = GetTexCoordInPix( texCoord, size );

	// Search area around the pixel.
	// The distance can't exceed the spread parameter
	// so we search in the square of size spread * 2 around the pixel.
	ivec2 minArea;
	ivec2 maxArea;
	GetMinMaxArea( minArea, maxArea, texCoordInPix, size );

	// The max distance is the spread radius given by the user.
	// Positive distance will be inside the shape, and negative distance will be outside.
	// 0 Is the frontier.
	float maxDistance = float( spread );
	float minDistance = -maxDistance;

	float pixelChannelToProcess = GetChannelToProcess( pixel );
	float toNearestDistance = maxDistance;
	vec4 finalColor = pixel;

	bool isIn = pixelChannelToProcess >= IN_THRESHOLD;

	// If the pixel value in bigger than the threshold, we consider the pixel inside the shape.
	if( isIn )
		toNearestDistance = GetDistanceToNearestOut( texCoordInPix, maxDistance, minArea, maxArea );
	
	// Otherwise the pixel is considered outside the shape.
	// We negate the distance, so the further we are from the shape, the smaller the value will be.
	else
	{
		toNearestDistance = GetDistanceToNearestIn( finalColor, texCoordInPix, maxDistance, minArea, maxArea );
		toNearestDistance = -toNearestDistance;
	}


	// Bring back the distance between 0 and 1 with the shape frontier at 0.5.
	toNearestDistance = clamp( toNearestDistance, minDistance, maxDistance );
	toNearestDistance = ( toNearestDistance - minDistance ) / ( maxDistance - minDistance );
	
	SetFragColor( finalColor, toNearestDistance );
}
