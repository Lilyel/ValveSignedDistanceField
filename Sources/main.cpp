#include "SDF.h"

#include "imgui.h"
#include "imgui-SFML.h"


#include <SFML/Graphics.hpp>

#include <string>
#include <functional>

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


    sf::RenderWindow window( sf::VideoMode( 2800, 1900 ), "Improved Alpha-Tested Magnfication for Vector Textures and Special Effects" );
    window.setFramerateLimit( 60 );

    ImGui::SFML::Init( window );

#ifdef HIGH_DPI
    ImGui::GetIO().FontGlobalScale = 2.0f;
    ImGui::GetStyle().ScaleAllSizes( 2.0f );
#endif

    sf::Clock deltaClock;

    SDF sdf;
    sdf.Init( DATA_PATH );
    sdf.SetTexture( DATA_PATH + "WhySoSerious1024.png" );

    char fileName[100];

    int viewMode = ViewMode::AlphaTested;
    int zoomOriginal = 100;
    int zoomProcessed = 100;
    bool autoApply = false;

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

        ImGui::SliderFloat( "Spread", &sdf.spread, 0.0f, 100.0f );
        ImGui::SliderFloat( "Smoothing", &sdf.smoothing, 2.0f, 128.0f );
        ImGui::SliderInt( "Resize Factor", &sdf.resizeFactor, 1, 10 );

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
            {
                ImGui::SetClipboardText( paperRef ); 
                ImGui::SameLine();
            }
            if( ImGui::Button( "Copy author and title" ) )
            {
                ImGui::SetClipboardText( paperRefShort ); 
                ImGui::SameLine();
            }
            if( ImGui::Button( "Close" ) )
                ImGui::CloseCurrentPopup();

            ImGui::EndChild();
            ImGui::EndPopup();
        }

        ImGui::Separator();

        ImGui::Checkbox( "Auto Apply", &autoApply );
        ImGui::SameLine();
        if( ImGui::Button( "Apply" ) || autoApply )
            sdf.Process();

        ImGui::End();

        /********************************************************/
        /***                   Show Results                   ***/
        /********************************************************/

        // Source

        sf::Sprite& sourceSprite = sdf.GetSourceSprite();
        sourceSprite.setScale( zoomOriginal / 100.0f, zoomOriginal / 100.0f );

        ImGui::Begin( "Original Picture" );
        ImGui::SliderInt( "Zoom", &zoomOriginal, 10, 1000, "%d%%" );
        ImGui::BeginChild( "SourceImage", ImVec2( 0, 0 ), false, ImGuiWindowFlags_HorizontalScrollbar );
        ImGui::Image( sourceSprite );
        ImGui::EndChild();
        ImGui::End();

        sourceSprite.setScale( 1.0f, 1.0f );

        // Processed

        std::reference_wrapper<sf::Sprite> processedSprite = std::ref( sdf.GetAlphaSprite() );
        if( viewMode == ViewMode::NoResize )
            processedSprite = std::ref( sdf.GetSDFSprite() );

        else if( viewMode == ViewMode::Resized )
            processedSprite = std::ref( sdf.GetResizeSprite() );

        processedSprite.get().setScale( zoomProcessed / 100.0f, zoomProcessed / 100.0f );
        
        ImGui::Begin( "Processed Picture" );
        ImGui::SliderInt( "Zoom", &zoomProcessed, 10, 1000, "%d%%" );
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