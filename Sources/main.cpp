#include "SDF.h"

#include "UI.h"

#include "imgui.h"
#include "imgui-SFML.h"

#include <SFML/Graphics.hpp>

#include <string>
#include <functional>


#ifdef _MSC_VER
static const std::string DATA_PATH = "../../Data/";
#else
static const std::string DATA_PATH = "../Data/";
#endif

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



void ShowSourceImage( int& _zoom, SDF& _sdf )
{
    ImGui::Begin( "Original Picture" );

    const float scale = ui::Zoom( _zoom, 10, 4000 );

    sf::Sprite& sourceSprite = _sdf.GetSourceSprite();
    sourceSprite.setScale( scale, scale );

    ui::Image( sourceSprite );

    // Restore default scale.
    sourceSprite.setScale( 1.0f, 1.0f );

    ImGui::End();
}

void ShowProcessedImage( int& _zoom, bool& _apply, int _viewMode, SDF& _sdf )
{
    ImGui::Begin( "Processed Picture" );

    const float scale = ui::Zoom( _zoom, 10, 4000 );

    // Process the signed distance field, resize it then render the final result.
    if( _apply )
    {
        _sdf.Process();
        _apply = false;
    }

    // Process the final render with the last processed signed distance field resized.
    // The scale will be applied while rendering to apply smooth.
    _sdf.ProcessAlphaTest( scale );

    // Retrieve the sprite that the user want to see.
    // If it is not the alpha tested result, we have to apply the scale.
    std::reference_wrapper<sf::Sprite> processedSprite = std::ref( _sdf.GetAlphaSprite() );
    if( _viewMode == ui::ViewMode::NoResize )
    {
        processedSprite = std::ref( _sdf.GetSDFSprite() );
        processedSprite.get().setScale( scale, scale );
    }

    else if( _viewMode == ui::ViewMode::Resized )
    {
        processedSprite = std::ref( _sdf.GetResizeSprite() );
        processedSprite.get().setScale( scale, scale );
    }

    ui::Image( processedSprite.get() );

    // Resture default scale.
    processedSprite.get().setScale( 1.0f, 1.0f );

    ImGui::End();
}


int main()
{
    sf::RenderWindow window( sf::VideoMode( WINDOW_SIZE_X, WINDOW_SIZE_Y ),
                             "Improved Alpha-Tested Magnfication for Vector Textures and Special Effects" );
    window.setFramerateLimit( 60 );

    ImGui::SFML::Init( window );

    ImGui::GetIO().FontGlobalScale = GLOBAL_SCALE;
    ImGui::GetStyle().ScaleAllSizes( GLOBAL_SCALE );

    sf::Clock deltaClock;

    SDF sdf;
    sdf.Init( DATA_PATH );
    sdf.SetTexture( DATA_PATH + "Circle1024.png" );

    char fileNameToLoad[100] = "\0";
    char fileNameToSave[100] = "\0";

    int viewMode = ui::ViewMode::AlphaTested;
    int zoomOriginal = 100;
    int zoomProcessed = 100;
    bool autoApply = false;
    bool apply = true; // True to process when oppening the app.


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

        ui::LoadImage( fileNameToLoad, sdf ); ImGui::Separator();
        ui::ImageType( sdf.imageType ); ImGui::Separator();
        ui::SpreadSmoothResize( sdf.spread, sdf.smoothing, sdf.resizeFactor ); ImGui::Separator();
        ui::ViewMode( viewMode ); ImGui::Separator();
        ui::PaperRef(); ImGui::Separator();
        ui::Apply( apply, autoApply );

        ImGui::End();

        /********************************************************/
        /***                   Show Results                   ***/
        /********************************************************/

        ShowSourceImage( zoomOriginal, sdf );

        ShowProcessedImage( zoomProcessed, apply, viewMode, sdf );

        /********************************************************/
        /***          Save to PNG functionnality              ***/
        /********************************************************/

        ui::SaveImage( DATA_PATH, fileNameToSave, sdf );


        window.clear();
        ImGui::SFML::Render( window );
        window.display();
    }

    ImGui::SFML::Shutdown();

    return 0;
}