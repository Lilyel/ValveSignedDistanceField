#pragma once

#include <iostream>

class SDF;
namespace sf
{
    class Sprite;
}

namespace ui
{
    enum ViewMode
    {
        NoResize,
        Resized,
        AlphaTested
    };


    void LoadImage( char _fileName[100], SDF& _sdf );

    void ImageType( int& _imageType );

    void SpreadResize( int& _spread, int& _resize );

    void SmoothOutlineGlow( SDF& _sdf );

    void ViewMode( int& _viewMode );

    void PaperRef();

    void Apply( bool& _apply, bool& _auto );

    void Image( const sf::Sprite& _sprite );

    float Zoom( int& _zoom, int min, int max );

    void SaveImage( const std::string& _dataPath, char _prefix[100], SDF& _sdf );
}
