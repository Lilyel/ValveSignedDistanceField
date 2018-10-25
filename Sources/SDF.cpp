#include "SDF.h"

#include <iostream>

SDF::SDF() :
    m_sourceSprite( m_sourceTexture ),
    spread( 20.0f ),
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

    m_resizeShader.loadFromFile( dataPath + "Default.vert", dataPath + "Resize.frag" );
    m_resizeRenderStates.shader = &m_resizeShader;

    m_alphaTestedShader.loadFromFile( dataPath + "Default.vert", dataPath + "Smooth.frag" );
    m_alphaTestedStates.shader = &m_alphaTestedShader;
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
    ProcessAlphaTest();
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

    if( !m_sdfFBO.generateMipmap() )
        std::cerr << "Failed to generate SDF mipmap." << std::endl;

    m_sdfSprite.setTexture( m_sdfFBO.getTexture(), true );
}

void SDF::ProcessResize()
{
    m_resizeShader.setUniform( "sourceTexture", sf::Shader::CurrentTexture );
    m_resizeShader.setUniform( "mipmapLevel", resizeFactor );

    m_resizeFBO.clear( sf::Color::Transparent );
    // The source picture will pass through the fragment shader
    // and the result will be stored in the FBO texture.
    m_resizeFBO.draw( m_sdfSprite, m_resizeRenderStates );
    m_resizeFBO.display();

    m_resizeSprite.setTexture( m_resizeFBO.getTexture(), true );
}

void SDF::ProcessAlphaTest()
{
    m_alphaTestedShader.setUniform( "sourceTexture", sf::Shader::CurrentTexture );
    m_alphaTestedShader.setUniform( "imageType", imageType );
    m_alphaTestedShader.setUniform( "smoothing", 1.0f / smoothing );

    m_alphaTestedFBO.clear( sf::Color::Transparent );
    // The source picture will pass through the fragment shader
    // and the result will be stored in the FBO texture.
    m_alphaTestedFBO.draw( m_resizeSprite, m_alphaTestedStates );
    m_alphaTestedFBO.display();

    m_alphaTestedSprite.setTexture( m_alphaTestedFBO.getTexture(), true );
}


void SDF::ResetFBOsToSourceTexture()
{
    const sf::Vector2u& textureSize = m_sourceTexture.getSize();

    m_sdfFBO.create( textureSize.x, textureSize.y );
    m_resizeFBO.create( textureSize.x, textureSize.y );
    m_alphaTestedFBO.create( textureSize.x, textureSize.y );
}
