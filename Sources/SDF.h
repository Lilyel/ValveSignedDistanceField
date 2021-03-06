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
    void ProcessAlphaTest( float finalScale, const sf::Color& clearColor = sf::Color::Transparent );

    sf::Sprite& GetSourceSprite();

    sf::Sprite& GetSDFSprite();
    sf::Sprite& GetResizeSprite();
    sf::Sprite& GetAlphaSprite();

private:
    void ProcessDistanceField();
    void ProcessResize();

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
    sf::Texture m_sdfTexture;

    // Resize of the signed distance field result.
    sf::RenderTexture m_resizeFBO;
    sf::Shader m_resizeShader;
    sf::RenderStates m_resizeRenderStates;
    sf::Sprite m_resizeSprite;
    sf::Texture m_resizeTexture;

    // Render final result with alpha testing.
    sf::RenderTexture m_alphaTestedFBO;
    sf::Shader m_alphaTestedShader;
    sf::RenderStates m_alphaTestedStates;
    sf::Sprite m_alphaTestedSprite;
    sf::Texture m_alphaTestedTexture;

public:
    int spread;
    int resizeFactor;
    int imageType;


    float smoothing;
    bool outline;
    sf::Color outlineColor;
    sf::Vector2f outlineDepth; // x : outside, y : inside.


    bool glow;
    sf::Color glowColor;
    sf::Vector2f glowOffset; // For shadow.
    float glowStrength;
};
