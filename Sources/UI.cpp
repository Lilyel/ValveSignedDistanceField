#include "UI.h"

#include "SDF.h"

#include "imgui.h"
#include "imgui-SFML.h"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Image.hpp>

namespace ui
{
    static constexpr char paperRef[] = "Chris Green. 2007. Improved alpha-tested magnification for vector textures and special effects."
        " In ACM SIGGRAPH 2007 courses (SIGGRAPH '07). ACM, New York, NY, USA, 9-18."
        " DOI: https://doi.org/10.1145/1281500.1281665";
    static constexpr char paperRefShort[] = "Chris Green Improved alpha-tested magnification for vector textures and special effects";


    void LoadImage( char _fileName[100], SDF& _sdf )
    {
        ImGui::Text( "File name : " );
        ImGui::SameLine();
        ImGui::InputText( "", _fileName, 99 );
        ImGui::SameLine();
        if( ImGui::Button( "Load" ) )
            _sdf.SetTexture( _fileName );
    }

    void ImageType( int& _imageType )
    {
        ImGui::Text( "Image Type" );
        ImGui::RadioButton( "Grey (grey channel will be used)", &_imageType, ImageType::Grey );
        ImGui::RadioButton( "Grey + Alpha (Alpha channel will be used)", &_imageType, ImageType::Grey_Alpha );
        ImGui::RadioButton( "RGB (Average of the R, G and B channel wil be used)", &_imageType, ImageType::AverageRGB );
        ImGui::RadioButton( "RGBA (Alpha channel will be used)", &_imageType, ImageType::RGBA );
    }

    void SpreadSmoothResize( int& _spread, float& _smooth, int& _resize )
    {
        ImGui::SliderInt( "Spread", &_spread, 1, 50 );
        ImGui::SliderFloat( "Smoothing", &_smooth, 2.0f, 128.0f );
        ImGui::SliderInt( "Resize Factor", &_resize, 1, 30 );
    }

    void ViewMode( int& _viewMode )
    {
        ImGui::Text( "View Mode" );
        ImGui::RadioButton( "Distance field without resize.", &_viewMode, ViewMode::NoResize );
        ImGui::RadioButton( "Distance field with resize.", &_viewMode, ViewMode::Resized );
        ImGui::RadioButton( "Distance field alpha tested.", &_viewMode, ViewMode::AlphaTested );
    }

    void PaperRef()
    {

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
    }

    void Apply( bool& _apply, bool& _auto )
    {
        ImGui::Checkbox( "Auto Apply", &_auto );
        ImGui::SameLine();
        if( ImGui::Button( "Apply" ) || _auto )
            _apply = true;
    }


    void Image( const sf::Sprite& _sprite )
    {
        ImGui::BeginChild( "", ImVec2( 0, 0 ), false, ImGuiWindowFlags_HorizontalScrollbar );
        ImGui::Image( _sprite );
        ImGui::EndChild();
    }


    float Zoom( int& _zoom, int _min, int _max )
    {
        ImGui::SliderInt( "Zoom", &_zoom, _min, _max, "%d%%" );
        return static_cast<float>( _zoom ) / 100.0f;
    }


    void SaveImage( const std::string& _dataPath, char _prefix[100], SDF& _sdf )
    {
        ImGui::Begin( "Save to file" );

        ImGui::Text( "File prefix : " );
        ImGui::SameLine();
        ImGui::InputText( "", _prefix, 99 );
        if( ImGui::Button( "Save" ) )
        {
            sf::Image imageToSave = _sdf.GetSDFSprite().getTexture()->copyToImage();
            imageToSave.saveToFile( _dataPath + "Saved/" + _prefix + "_SDF.png" );

            imageToSave = _sdf.GetResizeSprite().getTexture()->copyToImage();
            imageToSave.saveToFile( _dataPath + "Saved/" + _prefix + "_Resized.png" );

            imageToSave = _sdf.GetAlphaSprite().getTexture()->copyToImage();
            imageToSave.saveToFile( _dataPath + "Saved/" + _prefix + "_AlphaTested.png" );
        }

        ImGui::End();
    }

}
