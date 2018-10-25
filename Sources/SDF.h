#pragma once

#include "SFML/Graphics.hpp"

#include <string>


enum ImageType
{
    Grey, // The grey channel will be used.
    Grey_Alpha, // The alpha channel will be used.
    AverageRGB, // An average of the R, G and B channels will be used.
    RGBA, // Alpha channel will be used.
};

class SDF
{
public:
    SDF();
    void Init( const std::string& dataPath );

    void SetTexture( const std::string& fileName );

    void Process();

private:
    void ProcessDistanceField();
    void ProcessResize();
    void ProcessAlphaTest();

    void ResetFBOsToSourceTexture();

private:
    // Input datas.
    sf::Sprite m_sourceSprite;
    sf::Texture m_sourceTexture;
    sf::Vector2u m_sourceSize;
    
    // Signed distance field.
    sf::RenderTexture m_sdfFBO;
    sf::Shader m_sdfShader;
    sf::RenderStates m_sdfRenderStates;
    sf::Sprite m_sdfSprite;

    // Resize of the signed distance field result.
    sf::RenderTexture m_resizeFBO;
    sf::Shader m_resizeShader;
    sf::RenderStates m_resizeRenderStates;
    sf::Sprite m_resizeSprite;

    // Render final result with alpha testing.
    sf::RenderTexture m_alphaTestedFBO;
    sf::Shader m_alphaTestedShader;
    sf::RenderStates m_alphaTestedStates;
    sf::Sprite m_alphaTestedSprite;

public:
    float spread;
    float smoothing;
    int resizeFactor;
    int imageType;
};