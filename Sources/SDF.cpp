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
    m_sdfShader.loadFromFile( dataPath + "Default.vert", dataPath + "SDF.frag" );
    m_sdfRenderStates.shader = &m_sdfShader;
    //m_resizeShader.loadFromFile( dataPath + "Default.vert", dataPath + "Resize.frag" );
    //m_resizeRenderStates.shader = &m_resizeShader;

    m_alphaTestedShader.loadFromFile( dataPath + "Default.vert", dataPath + "Smooth.frag" );
    m_alphaTestedStates.shader = &m_alphaTestedShader;
}

void SDF::SetTexture( const std::string& fileName )
{
    m_sourceTexture.loadFromFile( fileName );
    m_sourceTexture.setSmooth( false );
    //m_sourceTexture.generateMipmap();
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

    //if( !m_sdfFBO.generateMipmap() )
    //    std::cerr << "Failed to generate SDF mipmap." << std::endl;

    m_sdfSprite.setTexture( m_sdfFBO.getTexture(), true );
}

void SDF::ProcessResize()
{
    //m_resizeShader.setUniform( "sourceTexture", sf::Shader::CurrentTexture );
    //m_resizeShader.setUniform( "mipmapLevel", resizeFactor );


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
    m_resizeFBO.draw( m_sdfSprite );
    m_resizeFBO.display();

    m_resizeTexture = m_resizeFBO.getTexture();
    m_resizeTexture.setSmooth( true );
    m_resizeSprite.setTexture( m_resizeTexture, true );
    //m_resizeSprite.setTexture( m_resizeFBO.getTexture(), true );

    

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
