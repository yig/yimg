#include <iostream>

#include <GLUT/glut.h>

#include "YImage.hpp"

void motionFunc( int x, int y ) ;
void mouseFunc( int button, int state, int x, int y ) ;
void keyboardFunc( unsigned char key, int x, int y ) ;
void idleFunc( void ) ;

void displayFunc( void ) ;
void reshapeFunc( int width, int height ) ;

static YImage* gImg ;
extern char **environ;

int main( int argc, char* argv[] )
{
	// glutInit( &argc, argv ) ;
	
	int bar = 1 ;
	char* foo[] = { "a.out" } ;
	glutInit( &bar, foo ) ;
	// for( int i = 0 ; environ[i] ; ++i )
	//{
	//	std::cerr << environ[i] << std::endl ;
	//}
	
	std::string toLoad ;
	
	if( argc != 2 )
	{
		toLoad = "sakura.png" ;
		// std::cerr << "Usage: " << argv[0] << " image.png" << std::endl ;
		// exit(1) ;
	}
	else
	{
		toLoad = argv[1] ;
	}
	gImg = new YImage() ;
	if( !gImg->load( toLoad.c_str() ) )
	{
		std::cerr << "Error loading PNG file: " << toLoad << std::endl ;
		exit(1) ;
	}
	gImg->flip() ;
	
	glutInitDisplayMode( GLUT_RGBA|GLUT_DEPTH|GLUT_DOUBLE ) ;
	glutInitWindowSize( gImg->width(), gImg->height() ) ;
	glutCreateWindow( toLoad.c_str() ) ;
	
	
	/*
	glutMouseFunc( mouseFunc ) ;
	glutMotionFunc( motionFunc ) ;
	glutPassiveMotionFunc( motionFunc ) ;
	*/
	glutKeyboardFunc( keyboardFunc ) ;
	
	glutDisplayFunc( displayFunc ) ;
	glutReshapeFunc( reshapeFunc ) ;
	
	// call reshapeFunc once to set up my projection matrix
	reshapeFunc( glutGet( GLUT_WINDOW_WIDTH ), glutGet( GLUT_WINDOW_HEIGHT ) ) ;
	
	glutMainLoop() ;
	
	exit(0) ;
}

void motionFunc( int x, int y )
{
	// glutPostRedisplay() ;
}

void mouseFunc( int button, int state, int x, int y )
{
	/*
	if( state == GLUT_UP )
		return ;
	
	glutPostRedisplay() ;
	*/
}

void keyboardFunc( unsigned char key, int x, int y )
{
	if( key == 'q' || key == 'Q' )
		exit(0) ;
	
	if( key == 'g' )
		gImg->greyscale() ;
	
	if( key == 'f' )
		gImg->flip() ;
	
	if( key == 'm' )
		gImg->mirror() ;
	
	if( key == 's' )
		gImg->save( "output.png" ) ;
	
	glutPostRedisplay() ;
}

void displayFunc( void )
{
	glClearColor( 0,1,0,1 ) ;
	glClear( GL_COLOR_BUFFER_BIT ) ;
	
	glEnable( GL_BLEND ) ;
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) ;
	
	glRasterPos2f( 0,0 ) ;
	glDrawPixels( gImg->width(), gImg->height(), GL_BGRA, GL_UNSIGNED_BYTE, gImg->data() ) ;
	
	glutSwapBuffers() ;
}

void idleFunc( void )
{
	// glutPostRedisplay() ;
}

void reshapeFunc( int width, int height )
{
	// in my world, each pixel is a unit
	
	glMatrixMode( GL_PROJECTION ) ;
	glLoadIdentity() ;
	// glTranslatef( -1, -1, 0 ) ;
	// glScalef( 2. * POINT_SIZE / (Real)width, 2. * POINT_SIZE / (Real)height, 1 ) ;
	glOrtho( 0, width, 0, height, -10000, 10000 ) ;
	
	glViewport(0,0,width,height) ;
}
