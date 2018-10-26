#include "SDF.h"

#include <iostream>

SDF::SDF() :
    m_sourceSprite( m_sourceTexture ),
    spread( 20 ),
    smoothing( 64.0f ),
    resizeFactor( 3 ),
    imageType( ImageType::RGBA )
{
    m_sourceTexture.setSmooth( true );
}

void SDF::Init( const std::string& dataPath )
{
    // Note for grey + alpha and RGBA image : 
    // That is very important to remove the blend mode (set to sf::BlendNone).
    // By default SFML set the blend mode to sf::BlendAlpha
    // but it will apply alpha on rgb channels too and we don't want that,
    // because that implies gradient on color and not only on alpha channel.


    m_sdfShader.loadFromFile( dataPath + "Default.vert", dataPath + "SDF.frag" );
    m_sdfRenderStates.shader = &m_sdfShader;
    m_sdfRenderStates.blendMode = sf::BlendNone;

    // We have a resize shader to ensure that the color channel won't be modify by the alpha on grey + alpha and RGBA images.
    m_resizeShader.loadFromFile( dataPath + "Default.vert", dataPath + "Resize.frag" );
    m_resizeRenderStates.shader = &m_resizeShader;
    m_resizeRenderStates.blendMode = sf::BlendNone;

    m_alphaTestedShader.loadFromFile( dataPath + "Default.vert", dataPath + "Smooth.frag" );
    m_alphaTestedStates.shader = &m_alphaTestedShader;
    m_alphaTestedStates.blendMode = sf::BlendNone;
}

void SDF::SetTexture( const std::string& fileName )
{
    m_sourceTexture.loadFromFile( fileName );
    m_sourceSize = m_sourceTexture.getSize();
    m_sourceSprite.setTexture( m_sourceTexture, true );

    ResetFBOsToSourceTexture();
}

sf::Sprite& SDF::GetSourceSprite()
{
    return m_sourceSprite;
}

sf::Sprite& SDF::GetSDFSprite()
{
    return m_sdfSprite;
}
sf::Sprite& SDF::GetResizeSprite()
{
    return m_resizeSprite;
}
sf::Sprite& SDF::GetAlphaSprite()
{
    return m_alphaTestedSprite;
}

void SDF::Process()
{
    ProcessDistanceField();
    ProcessResize();
}

void SDF::ProcessDistanceField()
{
    m_sdfShader.setUniform( "sourceTexture", sf::Shader::CurrentTexture );
    m_sdfShader.setUniform( "imageType", imageType );
    m_sdfShader.setUniform( "spread", spread );
    m_sdfShader.setUniform( "size", ( sf::Vector2i )m_sourceSize );

    m_sdfFBO.clear( sf::Color::Transparent );
    // The source picture will pass through the fragment shader
    // and the result will be stored in the FBO texture.
    m_sdfFBO.draw( m_sourceSprite, m_sdfRenderStates );
    m_sdfFBO.display();

    m_sdfTexture = m_sdfFBO.getTexture();
    // Important : Set smooth to true to use bi-linear filter.
    m_sdfTexture.setSmooth( true );
    m_sdfSprite.setTexture( m_sdfTexture, true );   
}

void SDF::ProcessResize()
{
    m_resizeShader.setUniform( "sourceTexture", sf::Shader::CurrentTexture );


    sf::Vector2u newSize( m_sourceSize.x / static_cast<unsigned int>( resizeFactor ), 
                          m_sourceSize.y / static_cast<unsigned int>( resizeFactor ) );
    newSize.x = std::max( newSize.x, 1u );
    newSize.y = std::max( newSize.y, 1u );

    const float scale = 1.0f / static_cast<float>( resizeFactor );
    m_sdfSprite.setScale( scale, scale );

    m_resizeFBO.create( newSize.x, newSize.y );
    m_resizeFBO.clear( sf::Color::Transparent );
    // The source picture will pass through the fragment shader
    // and the result will be stored in the FBO texture.
    m_resizeFBO.draw( m_sdfSprite, m_resizeRenderStates );
    m_resizeFBO.display();

    m_resizeTexture = m_resizeFBO.getTexture();
    // Set smooth to true to use bi-linear filter.
    m_resizeTexture.setSmooth( true );
    m_resizeSprite.setTexture( m_resizeTexture, true );    

    m_sdfSprite.setScale( 1.0f, 1.0f );
}

void SDF::ProcessAlphaTest( float finalScale, const sf::Color& clearColor )
{
    m_alphaTestedShader.setUniform( "sourceTexture", sf::Shader::CurrentTexture );
    m_alphaTestedShader.setUniform( "imageType", imageType );
    m_alphaTestedShader.setUniform( "smoothing", 1.0f / smoothing );

    // Find the final size of the scaled sprite.
    const sf::FloatRect& resizedSpriteRect = m_resizeSprite.getLocalBounds();
    sf::Vector2u newSize( static_cast<unsigned int>( resizedSpriteRect.width * finalScale ),
                          static_cast<unsigned int>( resizedSpriteRect.width * finalScale ) );
    newSize.x = std::max( newSize.x, 1u );
    newSize.y = std::max( newSize.y, 1u );

    // Apply scale to sprite.
    m_resizeSprite.setScale( finalScale, finalScale );

    m_alphaTestedFBO.create( newSize.x, newSize.y );
    m_alphaTestedFBO.clear( clearColor );
    // The source picture will pass through the fragment shader
    // and the result will be stored in the FBO texture.
    m_alphaTestedFBO.draw( m_resizeSprite, m_alphaTestedStates );
    m_alphaTestedFBO.display();

    m_alphaTestedSprite.setTexture( m_alphaTestedFBO.getTexture(), true );

    m_resizeSprite.setScale( 1.0f, 1.0f );
}


void SDF::ResetFBOsToSourceTexture()
{
    const sf::Vector2u& textureSize = m_sourceTexture.getSize();

    m_sdfFBO.create( textureSize.x, textureSize.y );
}
