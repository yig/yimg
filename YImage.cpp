/*
Author: Yotam Gingold <yotam (strudel) yotamgingold.com>
License: Public Domain [CC0](http://creativecommons.org/publicdomain/zero/1.0/)
GitHub: https://github.com/yig/yimg
*/

#include "YImage.hpp"

#include <cassert>
#include <cstring>

#include <string>
#include <algorithm> // tolower


// For load()
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// For save()
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// For rescale()
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

// for: #define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
// we use this to determine the pixel format on the fly
#include <cstddef>

namespace
{
    // stbir*() functions require input to be RGBA.
    // If we have a different pixel layout, we can swizzle in-place.
    
    const bool kYPixelIsRGBA = (
	    offsetof( YImage::YPixel, r ) == 0
	    &&
	    offsetof( YImage::YPixel, g ) == 1
	    &&
	    offsetof( YImage::YPixel, b ) == 2
	    &&
	    offsetof( YImage::YPixel, a ) == 3
	    );
	
	// Given YPixel data, swizzles it in-place so that the unsigned char layout
	// becomes RGBA. No-op if YPixel layout is already RGBA.
	void SwizzleToRGBA( YImage::YPixel* data, int num_pixels )
	{
	    // data must be non-null.
	    assert( data );
	    assert( num_pixels > 0 );
	    
	    if( kYPixelIsRGBA ) return;
	    
	    YImage::YPixel pix_saved;
	    unsigned char* pix_RGBA;
	    for( int i = 0; i < num_pixels; ++i ) {
	        pix_saved = data[i];
	        pix_RGBA = reinterpret_cast< unsigned char* >( &data[i] );
	        
	        pix_RGBA[0] = pix_saved.r;
	        pix_RGBA[1] = pix_saved.g;
	        pix_RGBA[2] = pix_saved.b;
	        pix_RGBA[3] = pix_saved.a;
	    }
	}
	// Given RGBA data, swizzles it in-place so that it becomes YPixel layout.
	// No-op if YPixel layout is already RGBA.
	void SwizzleFromRGBA( YImage::YPixel* data, int num_pixels )
	{
	    // data must be non-null.
	    assert( data );
	    assert( num_pixels > 0 );
	    
	    if( kYPixelIsRGBA ) return;
	    
	    YImage::YPixel pix_saved;
	    unsigned char* pix_RGBA;
	    for( int i = 0; i < num_pixels; ++i ) {
	        pix_saved = data[i];
	        pix_RGBA = reinterpret_cast< unsigned char* >( &data[i] );
	        
	        pix_saved.r = pix_RGBA[0];
	        pix_saved.g = pix_RGBA[1];
	        pix_saved.b = pix_RGBA[2];
	        pix_saved.a = pix_RGBA[3];
	    }
	}
}

namespace
{
/// From my: stl.h

// Behaves like the python os.path.splitext() function.
inline std::pair< std::string, std::string > os_path_splitext( const std::string& path )
{
    const std::string::size_type split_dot = path.find_last_of( "." );
    const std::string::size_type split_slash = path.find_last_of( "/" );
    if( split_dot != std::string::npos && (split_slash == std::string::npos || split_slash < split_dot) )
        return std::make_pair( path.substr( 0, split_dot ), path.substr( split_dot ) );
    else
        return std::make_pair( path, std::string() );
}
}

YImage::YImage()
	: m_width( 0 ), m_height( 0 ), m_data( NULL )
{
	assert( sizeof( YPixel ) == 4 && "YPixel struct shouldn't be padded" ) ;
}

YImage::YImage( const YImage& rhs )
	: m_width( 0 ), m_height( 0 ), m_data( NULL )
{
	*this = rhs ;
}

YImage&
YImage::operator=( const YImage& rhs )
{
    if( !rhs.m_data )
    {
        assert( 0 == rhs.m_width ) ;
        assert( 0 == rhs.m_height ) ;
        
        clear();
        
        return *this ;
    }
    
	if( m_width != rhs.m_width || m_height != rhs.m_height )
	{
		m_width = rhs.m_width ;
		m_height = rhs.m_height ;
		m_data = (YPixel*) realloc( m_data, m_width * m_height * sizeof(YPixel) ) ;
	}
	
	assert( m_data ) ;
	memcpy( m_data, rhs.m_data, m_width * m_height * sizeof(YPixel) ) ;
	
	return *this ;
}

YImage::~YImage()
{
	clear();
}

YImage::YPixel*
YImage::data()
{
	return m_data ;
}
const YImage::YPixel*
YImage::data()
const
{
	return m_data ;
}
YImage::YPixel&
YImage::at( int i, int j )
{
	assert( i >= 0 && i < m_width ) ;
	assert( j >= 0 && j < m_height ) ;
	
	return m_data[ i + j * m_width ] ;
}
const YImage::YPixel&
YImage::at( int i, int j )
const
{
	assert( i >= 0 && i < m_width ) ;
	assert( j >= 0 && j < m_height ) ;
	
	return m_data[ i + j * m_width ] ;
}

int YImage::width() const
{
	return m_width ;
}
int YImage::height() const
{
	return m_height ;
}

void YImage::clear()
{
    if( m_data ) free( m_data );
    
    m_data = NULL;
    m_width = m_height = 0;
    return;
}

void YImage::resize( int widthh, int heightt )
{
    // New dimensions must be non-negative.
    assert( widthh >= 0 && heightt >= 0 );
    // New dimensions must be both 0 or both non-zero.
    assert( widthh > 0 == heightt > 0 );
    
    // If they are both zero, clear the image and return.
    if( 0 == widthh || 0 == heightt )
    {
        assert( widthh > 0 == heightt > 0 );
        
        clear();
        return;
    }
    
    // If they are the same, do nothing.
	if( m_width == widthh && m_height == heightt )
	{
		assert( m_data ) ;
		return ;
	}
	
	YPixel* new_data = (YPixel*) malloc( widthh * heightt * sizeof(YPixel) ) ;
	memset( new_data, 0, widthh * heightt * sizeof(YPixel) ) ;
	
	if( m_data )
	{
		int min_width = std::min( m_width, widthh ) ;
		int min_height = std::min( m_height, heightt ) ;
		
		for( int j = 0 ; j < min_height ; ++j )
			memcpy( new_data + j*widthh, m_data + j*m_width, min_width * sizeof(YPixel) ) ;
		
		free( m_data ) ;
	}
	
	m_width = widthh ;
	m_height = heightt ;
	m_data = new_data ;
}


void YImage::rescale( int widthh, int heightt )
{
    // New dimensions must be non-negative.
    assert( widthh >= 0 && heightt >= 0 );
    // New dimensions must be both 0 or both non-zero.
    assert( widthh > 0 == heightt > 0 );
    
    // If they are both zero, clear the image and return.
    if( 0 == widthh || 0 == heightt )
    {
        assert( widthh > 0 == heightt > 0 );
        
        clear();
        return;
    }
    
    // If they are the same, do nothing.
	if( m_width == widthh && m_height == heightt )
	{
		assert( m_data ) ;
		return ;
	}
	
	YPixel* new_data = (YPixel*) malloc( widthh * heightt * sizeof(YPixel) ) ;
	
	// If there is currently no data, set the output to transparent black and return.
	if( !m_data )
	{
	    assert( 0 == m_width && 0 == m_height );
	    
	    memset( new_data, 0, widthh * heightt * sizeof(YPixel) ) ;
	    m_width = widthh ;
        m_height = heightt ;
        m_data = new_data ;
        return;
	}
	
	// If we are here, we have data.
	assert( m_data );
	assert( m_width > 0 && m_height > 0 );
	
	// stbir_resize_uint8() requires input to be RGBA.
	// If we have a different order, swizzle.
	SwizzleToRGBA( m_data, m_width*m_height );
	
	// Call stbir_resize_uint8()
	const bool success = stbir_resize_uint8(
	    reinterpret_cast< const unsigned char* >( m_data ), m_width, m_height, 0,
	    reinterpret_cast< unsigned char* >( new_data ), widthh, heightt, 0,
	    4 // number of channels
	    );
	assert( success );
	
	// Set the output.
	free( m_data );
	m_data = new_data;
	m_width = widthh;
	m_height = heightt;
	
	// Swizzle again if needed.
	SwizzleFromRGBA( m_data, m_width*m_height );
}


void YImage::greyscale()
{
	for( int i = 0 ; i < m_width * m_height ; ++i )
	{
		YPixel* pix = m_data + i ;
		int greyval = (int) pix->r + (int) pix->g + (int) pix->b ;
		greyval /= 3 ;
		pix->r = pix->g = pix->b = greyval ;
	}
}

void YImage::flip()
{
	for( int j = 0 ; j < m_height / 2 ; ++j )
	for( int i = 0 ; i < m_width ; ++i )
	{
		YPixel* lhs = &at( i,j ) ;
		YPixel* rhs = &at( i, m_height - j - 1 ) ;
		
		YPixel temp = *lhs ;
		
		*lhs = *rhs ;
		*rhs = temp ;
	}
}

void YImage::mirror()
{
	for( int j = 0 ; j < m_height ; ++j )
	for( int i = 0 ; i < m_width / 2 ; ++i )
	{
		YPixel* lhs = &at( i,j ) ;
		YPixel* rhs = &at( m_width - i - 1, j ) ;
		
		YPixel temp = *lhs ;
		
		*lhs = *rhs ;
		*rhs = temp ;
	}
}

bool YImage::same( const YImage& rhs ) const
{
    if( m_height != rhs.m_height || m_width != rhs.m_width ) return false;
    
    // Check if this->m_data and rhs.m_data point to the same data.
    // This also checks for if both are NULL.
    if( m_data == rhs.m_data ) return true;
    
    assert( m_data && rhs.m_data );
    return 0 == memcmp( m_data, rhs.m_data, m_width * m_height * sizeof(YPixel) );
}
bool YImage::same_rgb( const YImage& rhs ) const
{
    if( m_height != rhs.m_height || m_width != rhs.m_width ) return false;
    
    // Check if this->m_data and rhs.m_data point to the same data.
    // This also checks for if both are NULL.
    if( m_data == rhs.m_data ) return true;
    
    assert( m_data && rhs.m_data );
    
    for( int i = 0; i < m_height*m_width; ++i )
    {
        if(
            m_data[i].r != rhs.m_data[i].r ||
            m_data[i].g != rhs.m_data[i].g ||
            m_data[i].b != rhs.m_data[i].b )
            return false;
    }
    
    return true;
}

bool YImage::save( const char* fname, const bool fast )
const
{
    // NOTE: We switched from libpng to stb image writing.
    //       This reduced dependencies and allows us to save many more formats,
    //       but the `fast` parameter is ignored.
    
    
    // Don't call this with an empty image.
    assert( m_data );
    assert( m_width > 0 && m_height > 0 );
    
    std::string extension = os_path_splitext( fname ).second;
    std::transform( extension.begin(), extension.end(), extension.begin(), std::tolower );
    
    // These functions need swizzled data.
    SwizzleToRGBA( m_data, m_width*m_height );
    
    bool success = false;
    if( extension == ".png" ) {
        success = stbi_write_png( fname, m_width, m_height, 4, m_data, 0 );
    } else if( extension == ".bmp" ) {
        success = stbi_write_bmp( fname, m_width, m_height, 4, m_data );
    } else if( extension == ".tga" ) {
        success = stbi_write_tga( fname, m_width, m_height, 4, m_data );
    }
    
    // Unswizzle data.
    SwizzleFromRGBA( m_data, m_width*m_height );
    
    return success;
}

bool YImage::load(const char* fname)
{
    // NOTE: We switched from libpng to stb image writing.
    //       This reduced dependencies and allows us to load many more formats,
    //       but it doesn't support 16-bit-per-channel PNG's (which anyways we only
    //       loaded by truncating the extra precision).
    
    int widthh, heightt, num_raw_channels;
    unsigned char *data = stbi_load( fname, &widthh, &heightt, &num_raw_channels, 4 );
    // An error is indicated by stbi_load() returning NULL.
    if( 0 == data ) {
        stbi_image_free( data );
        return false;
    }
    
    // Reallocate memory as needed.
    if( m_width != widthh || m_height != heightt ) {
        m_width = widthh;
        m_height = heightt;
        m_data = (YPixel*) realloc( m_data, m_width * m_height * sizeof(YPixel) ) ;
    }
    assert( m_data );
    
    // Copy the loaded data.
    memcpy( m_data, data, m_width * m_height * sizeof(YPixel) ) ;
    
    stbi_image_free( data );
    
    // Unswizzle data.
    SwizzleFromRGBA( m_data, m_width*m_height );
    
    return true;
}
