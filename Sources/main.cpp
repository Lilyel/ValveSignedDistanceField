#include "imgui.h"
#include "imgui-SFML.h"


#include <SFML/Graphics.hpp>

#include <string>

enum ViewMode
{
    NoResize,
    Resized,
    AlphaTested
};


enum ImageType
{
    Grey, // The grey channel will be used.
    Grey_Alpha, // The alpha channel will be used.
    AverageRGB, // An average of the R, G and B channels will be used.
    RGBA, // Alpha channel will be used.
};


int main()
{
    const std::string DATA_PATH = "../../Data/";
    const char paperRef[] = "Chris Green. 2007. Improved alpha-tested magnification for vector textures and special effects."
                            " In ACM SIGGRAPH 2007 courses (SIGGRAPH '07). ACM, New York, NY, USA, 9-18."
                            " DOI: https://doi.org/10.1145/1281500.1281665";
    const char paperRefShort[] = "Chris Green Improved alpha-tested magnification for vector textures and special effects";


    sf::RenderWindow window( sf::VideoMode( 2800, 1900 ), "Improved Alpha-Tested Magnfication for Vector Textures and Special Effects" );
    window.setFramerateLimit( 60 );

    ImGui::SFML::Init( window );

    ImGuiStyle& imGuiStyle = ImGui::GetStyle();


    ImGui::GetIO().FontGlobalScale = 2.0f;
    imGuiStyle.ScaleAllSizes( 2.0f );


    sf::Clock deltaClock;

    sf::Texture sourceTexture;
    sourceTexture.loadFromFile( DATA_PATH + "ExampleSDF.jpg" );
    sf::Sprite sourceSprite( sourceTexture );

    sf::Shader sdfShader;
    sdfShader.loadFromFile( DATA_PATH + "Default.vert", DATA_PATH + "SDF.frag" );

    sf::RenderStates sdfRenderStates;
    sdfRenderStates.shader = &sdfShader;

    sf::Shader smoothShader;
    smoothShader.loadFromFile( DATA_PATH + "Default.vert", DATA_PATH + "Smooth.frag" );

    sf::RenderStates smoothRenderStates;
    smoothRenderStates.shader = &smoothShader;

    sf::RenderTexture fboDistanceField;
    sf::RenderTexture fboResized;
    sf::RenderTexture fboAlphaTested;

    int zoomOriginal = 100;
    int zoomProcessed = 100;
    float spread = 20.0f;
    float smoothing = 64.0f;
    int resizeFactor = 3;
    int viewMode = ViewMode::AlphaTested;
    int imageType = ImageType::RGBA;


    while( window.isOpen() )
    {
        sf::Event event;
        while( window.pollEvent( event ) )
        {
            ImGui::SFML::ProcessEvent( event );

            if( event.type == sf::Event::Closed )
                window.close();
        }

        ImGui::SFML::Update( window, deltaClock.restart() );

        const sf::Vector2u& windowSize = window.getSize();

        /********************************************************/
        /***             Retrieve user settings               ***/
        /********************************************************/
        ImGui::Begin( "Settings" );

        ImGui::Text( "Image Type" );
        ImGui::RadioButton( "Grey (grey channel will be used)", &imageType, ImageType::Grey );
        ImGui::RadioButton( "Grey + Alpha (Alpha channel will be used)", &imageType, ImageType::Grey_Alpha );
        ImGui::RadioButton( "RGB (Average of the R, G and B channel wil be used)", &imageType, ImageType::AverageRGB );
        ImGui::RadioButton( "RGBA (Alpha channel will be used)", &imageType, ImageType::RGBA );

        ImGui::Separator();

        ImGui::SliderFloat( "Spread", &spread, 0.0f, 100.0f );
        ImGui::SliderFloat( "Smoothing", &smoothing, 2.0f, 128.0f );
        ImGui::SliderInt( "Resize Factor", &resizeFactor, 1, 10 );

        ImGui::Separator();

        ImGui::Text( "View Mode" );
        ImGui::RadioButton( "Distance field without resize.", &viewMode, ViewMode::NoResize );
        ImGui::RadioButton( "Distance field with resize.", &viewMode, ViewMode::Resized );
        ImGui::RadioButton( "Distance field alpha tested.", &viewMode, ViewMode::AlphaTested );

        ImGui::Separator();

        if( ImGui::Button( "Paper Reference" ) )
            ImGui::OpenPopup( "Reference" );

        if( ImGui::BeginPopupModal( "Reference" ) )
        {
            ImGui::BeginChild( "", ImVec2( 600, 300 ) );
            ImGui::TextWrapped( paperRef );

            if( ImGui::Button( "Copy" ) )
                ImGui::SetClipboardText( paperRef ); ImGui::SameLine();
            if( ImGui::Button( "Copy author and title" ) )
                ImGui::SetClipboardText( paperRefShort ); ImGui::SameLine();
            if( ImGui::Button( "Close" ) )
                ImGui::CloseCurrentPopup();

            ImGui::EndChild();
            ImGui::EndPopup();
        }

        ImGui::End();

        /********************************************************/
        /***          Process Signed Distance Field           ***/
        /********************************************************/
        const sf::Vector2u& sourceSize = sourceTexture.getSize();

        sdfShader.setUniform( "sourceTexture", sf::Shader::CurrentTexture );
        sdfShader.setUniform( "imageType", imageType );
        sdfShader.setUniform( "spread", spread );
        sdfShader.setUniform( "size", ( sf::Vector2i )sourceSize );

        fboDistanceField.create( sourceSize.x, sourceSize.y );
        fboDistanceField.clear( sf::Color::Transparent );
        sourceSprite.setScale( 1.0f, 1.0f );
        // The source picture will pass through the fragment shader
        // and the result will be stored in the FBO texture.
        fboDistanceField.draw( sourceSprite, sdfRenderStates );
        fboDistanceField.display();
        
        sf::Sprite distanceFieldSprite( fboDistanceField.getTexture() );

        /********************************************************/
        /***           Resize Signed Distance Field           ***/
        /********************************************************/

        const float resizeFactorInv = 1.0f / static_cast<float>( resizeFactor );
        distanceFieldSprite.setScale( resizeFactorInv, resizeFactorInv );
        
        sf::Vector2f resizedSpriteSize( static_cast<sf::Vector2f>( sourceSize ) * resizeFactorInv );
        // Can't create a texture with size 0 x 0.
        resizedSpriteSize.x = std::max( resizedSpriteSize.x, 1.0f );
        resizedSpriteSize.y = std::max( resizedSpriteSize.y, 1.0f );
        
        fboResized.create( static_cast<unsigned int>( resizedSpriteSize.x), static_cast<unsigned int>( resizedSpriteSize.y ) );
        fboResized.clear( sf::Color::Transparent );
        fboResized.draw( distanceFieldSprite );
        fboResized.display();

        sf::Sprite spriteResize( fboResized.getTexture() );

        /********************************************************/
        /***              Final Result (Smoothed)             ***/
        /********************************************************/

        smoothShader.setUniform( "sourceTexture", sf::Shader::CurrentTexture );
        smoothShader.setUniform( "imageType", imageType );
        smoothShader.setUniform( "smoothing", 1.0f / smoothing );

        fboAlphaTested.create( static_cast<unsigned int>( resizedSpriteSize.x ), static_cast<unsigned int>( resizedSpriteSize.y ) );
        fboAlphaTested.clear( sf::Color::Transparent );
        fboAlphaTested.draw( spriteResize, smoothRenderStates );
        fboAlphaTested.display();


        /********************************************************/
        /***                   Show Results                   ***/
        /********************************************************/

        sourceSprite.setScale( zoomOriginal / 100.0f, zoomOriginal / 100.0f );

        ImGui::Begin( "Original Picture" );
        ImGui::SliderInt( "Zoom", &zoomOriginal, 10, 1000, "%d%%" );
        ImGui::BeginChild( "SourceImage", ImVec2( 0, 0 ), false, ImGuiWindowFlags_HorizontalScrollbar );
        ImGui::Image( sourceSprite );
        ImGui::EndChild();
        ImGui::End();


        sf::Sprite processedSprite( fboAlphaTested.getTexture() );
        processedSprite.setScale( zoomProcessed / 100.0f, zoomProcessed / 100.0f );
        
        ImGui::Begin( "Processed Picture" );
        ImGui::SliderInt( "Zoom", &zoomProcessed, 10, 1000, "%d%%" );
        ImGui::BeginChild( "ProcessedImage", ImVec2( 0, 0 ), false, ImGuiWindowFlags_HorizontalScrollbar );
        ImGui::Image( processedSprite );
        ImGui::EndChild();
        ImGui::End();






        window.clear();
        ImGui::SFML::Render( window );
        window.display();
    }

    ImGui::SFML::Shutdown();

    return 0;
}