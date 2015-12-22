/*
Author: Yotam Gingold <yotam (strudel) yotamgingold.com>
License: Public Domain [CC0](http://creativecommons.org/publicdomain/zero/1.0/)
GitHub: https://github.com/yig/yimg
*/

#ifndef __YImage_hpp__
#define __YImage_hpp__

class YImage
{
public:
    // file loading/saving automatically picks up changes to this struct.
    // The possibilities are: ARGB, ABGR, RGBA, BGRA.
    struct YPixel {
        unsigned char r ;
        unsigned char g ;
        unsigned char b ;
        unsigned char a ;
    };
    
    // An empty constructor creates an empty image with data set to NULL
    // and width and height set to 0.
    YImage() ;
	YImage( const YImage& ) ;
	~YImage() ;
	
	YImage& operator=( const YImage& ) ;
	
	// Returns true if the images are the same size and have the same data (for r,g,b,a).
	bool same( const YImage& rhs ) const ;
	// Returns true if the images are the same size and have the same data for r,g,b.
	bool same_rgb( const YImage& rhs ) const ;
	
	// Saves a PNG image.  If 'fast' is true, sacrifices compression ratio for speed.
	bool save( const char* fname, const bool fast = false ) const ;
	bool load( const char* fname ) ;
	
	// Sets the width and height to 0 and data to NULL.
	void clear();
	
	YPixel* data() ;
	const YPixel* data() const ;
	
	YPixel& at( int i, int j ) ;
	const YPixel& at( int i, int j ) const ;
	
	int width() const ;
	int height() const ;
	// Resizes the image as in a window or canvas resize.
	// Preserves as much as possible the old image (in the upper left).
    // Newly visible pixels are set to transparent black.
	void resize( int width, int height ) ;
	// Resizes the image by scaling it to fit in the new dimensions.
	void rescale( int width, int height ) ;
	
	// flip vertically
	void flip() ;
	// flip horizontally
	void mirror() ;
	// average rgb
	void greyscale() ;
	
protected:
	int m_width ;
	int m_height ;
	YPixel* m_data ; // raw image data
};

#endif /* __YImage_hpp__ */
