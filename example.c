static YImage* s_img = NULL ;

static char s_img_name[100] ;
static unsigned int s_img_count = 0 ;

static int glut_width = 512 ;
static int glut_height = 512 ;

// somewhere
{
	s_img = new YImage() ;
	s_img -> resize( glut_width, glut_height ) ;
	
	time_t theTime = time(NULL) ;
	sprintf( s_img_name, "%s%10d.png", ctime(&theTime), 0 ) ;
	s_img_name[24] = ' ' ;
}

void savePNG()
{
	glReadPixels( 0, 0, glut_width, glut_height, GL_BGRA, GL_UNSIGNED_BYTE, s_img->data() ) ;
	
	sprintf( s_img_name + 25, "%10u.png", s_img_count++ ) ;
	s_img -> save( s_img_name ) ;
}
