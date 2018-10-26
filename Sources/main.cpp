#include "SDF.h"

#include "imgui.h"
#include "imgui-SFML.h"


#include <SFML/Graphics.hpp>

#include <string>
#include <functional>


#define HIGH_DPI

#ifdef HIGH_DPI
static constexpr unsigned int WINDOW_SIZE_X = 2800;
static constexpr unsigned int WINDOW_SIZE_Y = 1900;
static constexpr float GLOBAL_SCALE = 2.0f;
#else
static constexpr unsigned int WINDOW_SIZE_X = 1280;
static constexpr unsigned int WINDOW_SIZE_Y = 720;
static constexpr float GLOBAL_SCALE = 1.0f;
#endif


enum ViewMode
{
    NoResize,
    Resized,
    AlphaTested
};




int main()
{
    std::string DATA_PATH;

#ifdef _MSC_VER 
    DATA_PATH = "../../Data/";
#else
    DATA_PATH = "../Data/";
#endif

    const char paperRef[] = "Chris Green. 2007. Improved alpha-tested magnification for vector textures and special effects."
        " In ACM SIGGRAPH 2007 courses (SIGGRAPH '07). ACM, New York, NY, USA, 9-18."
        " DOI: https://doi.org/10.1145/1281500.1281665";
    const char paperRefShort[] = "Chris Green Improved alpha-tested magnification for vector textures and special effects";


    sf::RenderWindow window( sf::VideoMode( WINDOW_SIZE_X, WINDOW_SIZE_Y ), "Improved Alpha-Tested Magnfication for Vector Textures and Special Effects" );
    window.setFramerateLimit( 60 );

    ImGui::SFML::Init( window );

    ImGui::GetIO().FontGlobalScale = GLOBAL_SCALE;
    ImGui::GetStyle().ScaleAllSizes( GLOBAL_SCALE );


    sf::Clock deltaClock;

    SDF sdf;
    sdf.Init( DATA_PATH );
    sdf.SetTexture( DATA_PATH + "Circle1024.png" );

    char fileName[100] = "\0";

    int viewMode = ViewMode::AlphaTested;
    int zoomOriginal = 100;
    int zoomProcessed = 100;
    bool autoApply = false;
    bool firstLoop = true;
    bool apply = false;
    ImVec4 backgroundProcessedColor( 0.0f, 0.0f, 0.0f, 0.0f );

    sf::RenderTexture processedFBO;

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


        /********************************************************/
        /***             Retrieve user settings               ***/
        /********************************************************/
        ImGui::Begin( "Settings" );

        ImGui::Text( "File name : " );
        ImGui::SameLine();
        ImGui::InputText( "", fileName, 99 );
        ImGui::SameLine();
        if( ImGui::Button( "Load" ) )
            sdf.SetTexture( fileName );

        ImGui::Separator();

        ImGui::Text( "Image Type" );
        ImGui::RadioButton( "Grey (grey channel will be used)", &sdf.imageType, ImageType::Grey );
        ImGui::RadioButton( "Grey + Alpha (Alpha channel will be used)", &sdf.imageType, ImageType::Grey_Alpha );
        ImGui::RadioButton( "RGB (Average of the R, G and B channel wil be used)", &sdf.imageType, ImageType::AverageRGB );
        ImGui::RadioButton( "RGBA (Alpha channel will be used)", &sdf.imageType, ImageType::RGBA );

        ImGui::Separator();

        ImGui::SliderInt( "Spread", &sdf.spread, 1, 50 );
        ImGui::SliderFloat( "Smoothing", &sdf.smoothing, 2.0f, 128.0f );
        ImGui::SliderInt( "Resize Factor", &sdf.resizeFactor, 1, 30 );

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
                ImGui::SetClipboardText( paperRef );

            ImGui::SameLine();

            if( ImGui::Button( "Copy author and title" ) )
                ImGui::SetClipboardText( paperRefShort );

            ImGui::SameLine();

            if( ImGui::Button( "Close" ) )
                ImGui::CloseCurrentPopup();

            ImGui::EndChild();
            ImGui::EndPopup();
        }

        ImGui::Separator();

        ImGui::Checkbox( "Auto Apply", &autoApply );
        ImGui::SameLine();
        if( ImGui::Button( "Apply" ) || autoApply || firstLoop )
        {
            apply = true;
            firstLoop = false;
        }

        ImGui::End();

        /********************************************************/
        /***                   Show Results                   ***/
        /********************************************************/

        // Source

        sf::Sprite& sourceSprite = sdf.GetSourceSprite();
        sourceSprite.setScale( zoomOriginal / 100.0f, zoomOriginal / 100.0f );

        ImGui::Begin( "Original Picture" );
        ImGui::SliderInt( "Zoom", &zoomOriginal, 10, 2000, "%d%%" );
        ImGui::BeginChild( "SourceImage", ImVec2( 0, 0 ), false, ImGuiWindowFlags_HorizontalScrollbar );
        ImGui::Image( sourceSprite );
        ImGui::EndChild();
        ImGui::End();

        sourceSprite.setScale( 1.0f, 1.0f );

        // Processed

        ImGui::Begin( "Processed Picture" );
        ImGui::SliderInt( "Zoom", &zoomProcessed, 10, 2000, "%d%%" );
        ImGui::SameLine();
        ImGui::ColorEdit4( "Background", &backgroundProcessedColor.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreview );

        // Process the signed distance field, resize it then render the final result.
        if( apply )
        {
            sdf.Process();
            apply = false;
        }

        const float scaleProcessed = static_cast<float>( zoomProcessed ) / 100.0f;
        const sf::Color backgroundColor(
            ( sf::Uint8 )( backgroundProcessedColor.x * 255.0f ),
            ( sf::Uint8 )( backgroundProcessedColor.y * 255.0f ),
            ( sf::Uint8 )( backgroundProcessedColor.z * 255.0f ),
            ( sf::Uint8 )( backgroundProcessedColor.w * 255.0f ) );

        // Process the final render with the last processed signed distance field resized.
        sdf.ProcessAlphaTest( scaleProcessed, sf::Color( backgroundProcessedColor ) );

        // Retrieve the sprite that the user want.
        // If it is not the final result, we have to apply the scale.
        std::reference_wrapper<sf::Sprite> processedSprite = std::ref( sdf.GetAlphaSprite() );
        if( viewMode == ViewMode::NoResize )
        {
            processedSprite = std::ref( sdf.GetSDFSprite() );
            processedSprite.get().setScale( scaleProcessed, scaleProcessed );
        }

        else if( viewMode == ViewMode::Resized )
        {
            processedSprite = std::ref( sdf.GetResizeSprite() );
            processedSprite.get().setScale( scaleProcessed, scaleProcessed );
        }

        ImGui::BeginChild( "ProcessedImage", ImVec2( 0, 0 ), false, ImGuiWindowFlags_HorizontalScrollbar );
        ImGui::Image( processedSprite.get() );
        ImGui::EndChild();
        ImGui::End();

        processedSprite.get().setScale( 1.0f, 1.0f );


        window.clear();
        ImGui::SFML::Render( window );
        window.display();
    }

    ImGui::SFML::Shutdown();

    return 0;
}