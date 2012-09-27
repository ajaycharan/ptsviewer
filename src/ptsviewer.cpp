/*******************************************************************************
 *
 *       Filename:  ptsviewer.c
 *
 *    Description:  OpenGL viewer for pts point cloud files
 *
 *        Created:  05/11/2011 20:42:39 PM
 *       Compiler:  gcc
 *
 *         Author:  Lars Kiesow (lkiesow), lkiesow@uos.de
 *        Company:  Universität Osnabrück
 *
 *     Extensions:  Tomasz Malisiewicz (quantombone), tomasz@csail.mit.edu
 *    Affiliation:  MIT
 *
 ******************************************************************************/

#include "ptsviewer.h"
#include <Eigen/Dense>
#include <string>
#include <vector>

#define min(A,B) ((A)<(B) ? (A) : (B)) 
#define max(A,B) ((A)>(B) ? (A) : (B)) 

void write_point_chunk(FILE* f, double* point1, double* point2, int NCUT, uint8_t* color);

typedef struct {
    double r,g,b;
} COLOUR;

COLOUR GetColour(double v,double vmin,double vmax);

int dump_ply(const char* filename, const char* points_file, const char* reconstruction_file);
int dump_ply_camera(const char* filename, const char* points_file, const char* reconstruction_file);

/*******************************************************************************
 *         Name:  mouseMoved
 *  Description:  Handles mouse drag'n'drop for rotation.
 ******************************************************************************/
void mouseMoved( int x, int y ) {
  
  if ( g_last_mousebtn == GLUT_LEFT_BUTTON ) {
    if ( g_mx >= 0 && g_my >= 0 ) {
      g_rot.x += ( y - g_my ) * g_invertroty / 4.0f;
      g_rot.y += ( x - g_mx ) * g_invertrotx / 4.0f;
      glutPostRedisplay();
    }
  } else if ( g_last_mousebtn == GLUT_MIDDLE_BUTTON ) {
    if ( g_mx >= 0 && g_my >= 0 ) {
      g_rot.x += ( y - g_my ) * g_invertroty / 4.0f;
      g_rot.z += ( x - g_mx ) * g_invertrotx / 4.0f;
      glutPostRedisplay();
    }
  } else if ( g_last_mousebtn == GLUT_RIGHT_BUTTON ) {
    g_translate.y -= ( y - g_my ) / 1000.0f * g_maxdim;
    g_translate.x += ( x - g_mx ) / 1000.0f * g_maxdim;
    glutPostRedisplay();
  }
  g_mx = x;
  g_my = y;
  
}


/*******************************************************************************
 *         Name:  mousePress
 *  Description:  Start drag'n'drop and handle zooming per mouse wheel.
 ******************************************************************************/
void mousePress( int button, int state, int x, int y ) {
  
  if ( state == GLUT_DOWN ) {
    switch ( button ) {
    case GLUT_LEFT_BUTTON:
    case GLUT_RIGHT_BUTTON:
    case GLUT_MIDDLE_BUTTON:
      g_last_mousebtn = button;
      g_mx = x;
      g_my = y;
      break;
    case 3: /* Mouse wheel up */
      g_translate.z += g_movespeed * g_maxdim / 100.0f;
      glutPostRedisplay();
      break;
    case 4: /* Mouse wheel down */
      g_translate.z -= g_movespeed * g_maxdim / 100.0f;
      glutPostRedisplay();
      break;
    }
  }
  
}


/*******************************************************************************
 *         Name:  drawScene
 *  Description:  Display point cloud.
 ******************************************************************************/
void drawScene() {

  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  
  glEnableClientState( GL_VERTEX_ARRAY );
  /* Set point size */
  glPointSize( g_pointsize );
    
  int i;
  for ( i = 0; i < (int)g_cloudcount; i++ ) {
    if ( g_clouds[i].enabled ) {
      glLoadIdentity();
      
      /* Enable colorArray. */
      if ( g_clouds[i].colors ) {
        glEnableClientState( GL_COLOR_ARRAY );
      } else {
        /* Set cloudcolor to opposite of background color. */
        float rgb[3];
        glGetFloatv( GL_COLOR_CLEAR_VALUE, rgb );
        if ( *rgb < 0.5 ) {
          glColor3f( 1.0f, 1.0f, 1.0f );
        } else {
          glColor3f( 0.0f, 0.0f, 0.0f );
        }
      }
      
      
         

      glScalef( g_zoom, g_zoom, -1 );
      
      glTranslatef( g_translate.x, g_translate.y, g_translate.z );
      //fprintf(stdout,"x y z is %.4f %.4f %.4f\n",g_translate.x,g_translate.y,g_translate.z);
      
      
      glRotatef( (int) g_rot.x, 1, 0, 0 );
      glRotatef( (int) g_rot.y, 0, 1, 0 );
      glRotatef( (int) g_rot.z, 0, 0, 1 );
      
      
      //TJM: comment this out
      //glTranslatef( -g_trans_center.x, -g_trans_center.y, -g_trans_center.z );
      
      //if (g_clouds[i].mat)
      //glLoadMatrixd(g_clouds[i].mat);
      
      
      
      /* local (this cloud only) */
      /* glTranslatef( g_clouds[i].trans.x, g_clouds[i].trans.y, */
      /* 		g_clouds[i].trans.z ); */
      
      /* glRotatef( (int) g_clouds[i].rot.x, 1, 0, 0 ); */
      /* glRotatef( (int) g_clouds[i].rot.y, 0, 1, 0 ); */
      /* glRotatef( (int) g_clouds[i].rot.z, 0, 0, 1 ); */
      
      /* Set vertex and color pointer. */
      glVertexPointer( 3, GL_FLOAT, 0, g_clouds[i].vertices );
      if ( g_clouds[i].colors ) {
        glColorPointer(  3, GL_UNSIGNED_BYTE, 0, g_clouds[i].colors );
      }
      
      /*int qqq; 
        fprintf(stdout,"\n");
        for (qqq = 0; qqq < 16; ++qqq)
        fprintf(stdout, "[i]=%d element %d is %.3f\n",i,qqq,g_clouds[i].mat[qqq]);
      */
      
      //fprintf(stdout,"cpy is %d\n",current_ply_index);
      
      /* fprintf(stdout,"Vert[%d] is %f %f %f\n",i,g_clouds[i].vertices[0],g_clouds[i].vertices[1],g_clouds[i].vertices[2]); */
      
      /* fprintf(stdout,"PC is %d\n",g_clouds[i].pointcount); */
      /* int q; */
      /* for (q = 0; q < 16; ++q) */
      /*   fprintf(stdout,"Element[%d]=%.5f\n",i,g_clouds[i].mat[q]); */
      
      /* for (q = 0; q < 16; ++q) */
      /*   fprintf(stdout,"Element INV [%d]=%.5f\n",i,g_clouds[i].invmat[q]); */
      
      
      glMultMatrixd(g_clouds[current_ply_index].invmat);
      glMultMatrixd(g_clouds[i].mat);
      
      
      
      /* Draw point cloud */
      glDrawArrays( GL_POINTS, 0, g_clouds[i].pointcount );
      
      /* Disable colorArray. */
      if ( g_clouds[i].colors ) {
        glDisableClientState( GL_COLOR_ARRAY );
      }
    }
  }
  
  /* Reset ClientState */
  glDisableClientState( GL_VERTEX_ARRAY );  

  glFlush();
  glutSwapBuffers();
  
}


/*******************************************************************************
 *         Name:  selectionKey
 *  Description:  
 ******************************************************************************/
void selectionKey( unsigned char key ) {
	
  if ( key == 8 && strlen( g_selection ) ) { /* Enter selection */
    g_selection[ strlen( g_selection ) - 1 ] = 0;
  } else if ( ( key >= '0' && key <= '9' ) || key == ',' ) { /* Enter selection */
    sprintf( g_selection, "%s%c", g_selection, key );
    
  } else if ( key == 13 ) { /* Apply selection, go to normal mode */
    
    g_mode = VIEWER_MODE_NORMAL;
    char * s = g_selection;
    
    int sel;
    while ( strlen( s ) ) {
      /* Jump over comma. */
      if ( *s == ',' ) {
        s++;
        continue;
      }
      sel = strtol( s, &s, 0 );
      /* int sel = atoi( g_selection ); */
      /* Check if cloud exists */
      if ( sel < g_cloudcount ) {
        g_clouds[ sel ].selected = !g_clouds[ sel ].selected;
        printf( "Cloud %d %sselected\n", sel, 
                g_clouds[ sel ].selected ? "" : "un" );
      }
    }
    g_selection[0] = 0;
    
  } else if ( key == 27 ) { /* Just switch back to normal mode. */
    g_mode = VIEWER_MODE_NORMAL;
    g_selection[0] = 0;
  }
  glutPostRedisplay();
  
}


/*******************************************************************************
 *         Name:  moveKeyPressed
 *  Description:  
 ******************************************************************************/
void moveKeyPressed( unsigned char key ) {

#define FORALL  for ( i = 0; i < g_cloudcount; i++ ) {
#define FORSEL  for ( i = 0; i < g_cloudcount; i++ ) { if ( g_clouds[i].selected )
#define FORSELC for ( i = 0; i < g_cloudcount; i++ ) { if ( g_clouds[i].selected ) g_clouds[i]
#define FSEND } break

  int i;
  switch ( key ) {
  case 27:
  case 'm' : g_mode = VIEWER_MODE_NORMAL; break;
    /* movement */
  case 'a': FORSELC.trans.x -= 1 * g_movespeed; FSEND;
  case 'd': FORSELC.trans.x += 1 * g_movespeed; FSEND;
  case 'w': FORSELC.trans.z -= 1 * g_movespeed; FSEND;
  case 's': FORSELC.trans.z += 1 * g_movespeed; FSEND;
  case 'q': FORSELC.trans.y += 1 * g_movespeed; FSEND;
  case 'e': FORSELC.trans.y -= 1 * g_movespeed; FSEND;
    /* Uppercase: fast movement */
  case 'A': FORSELC.trans.x -= 0.1 * g_movespeed; FSEND;
  case 'D': FORSELC.trans.x += 0.1 * g_movespeed; FSEND;
  case 'W': FORSELC.trans.z -= 0.1 * g_movespeed; FSEND;
  case 'S': FORSELC.trans.z += 0.1 * g_movespeed; FSEND;
  case 'Q': FORSELC.trans.y += 0.1 * g_movespeed; FSEND;
  case 'E': FORSELC.trans.y -= 0.1 * g_movespeed; FSEND;
    /* Rotation */
  case 'r': FORSELC.rot.x -= 1; FSEND;
  case 'f': FORSELC.rot.x += 1; FSEND;
  case 't': FORSELC.rot.y -= 1; FSEND;
  case 'g': FORSELC.rot.y += 1; FSEND;
  case 'z': FORSELC.rot.z -= 1; FSEND;
  case 'h': FORSELC.rot.z += 1; FSEND;
    /* Precise rotations */
  case 'R': FORSELC.rot.x -= 0.1; FSEND;
  case 'F': FORSELC.rot.x += 0.1; FSEND;
  case 'T': FORSELC.rot.y -= 0.1; FSEND;
  case 'G': FORSELC.rot.y += 0.1; FSEND;
  case 'Z': FORSELC.rot.z -= 0.1; FSEND;
  case 'H': FORSELC.rot.z += 0.1; FSEND;
    /* Other stuff */
  case '*': g_movespeed  *= 10;  break;
  case '/': g_movespeed  /= 10;  break;
  case ' ': FORSELC.enabled = !g_clouds[i].enabled; FSEND;
  case 'p': FORALL printf( "%s: %f %f %f  %f %f %f\n", g_clouds[i].name,
                           g_clouds[i].trans.x, g_clouds[i].trans.y,
                           -g_clouds[i].trans.z, -g_clouds[i].rot.x,
                           -g_clouds[i].rot.y, g_clouds[i].rot.z); FSEND;
  }
  /* Generate and save pose files */
  glutPostRedisplay();
  

}


/*******************************************************************************
 *         Name:  keyPressed
 *  Description:  Handle keyboard control events.
 ******************************************************************************/
void keyPressed( unsigned char key, int x, int y ) {

  if ( g_mode == VIEWER_MODE_SELECT ) {
    selectionKey( key );
    return;
  } else if ( g_mode == VIEWER_MODE_MOVESEL ) {
    moveKeyPressed( key );
    return;
  }
  
  float rgb[3];
  int i;
  switch ( key ) {
  case 27:
    glutDestroyWindow( g_window );
    exit( EXIT_SUCCESS );
  case 'j': 
    g_translate.x = 0;
    g_translate.y = 0;
    g_translate.z = 0;
    g_rot.x       = 0;
    g_rot.y       = 0;
    g_rot.z       = 0;
    g_zoom        = 1;
    break;
  case '+': g_zoom      *= 1.1; break;
  case '-': g_zoom      /= 1.1; break;
    /* movement */
  case 'a': g_translate.x += 1 * g_movespeed; break;
  case 'd': g_translate.x -= 1 * g_movespeed; break;
  case 'w': g_translate.z += 1 * g_movespeed; break;
  case 's': g_translate.z -= 1 * g_movespeed; break;
  case 'q': g_translate.y += 1 * g_movespeed; break;
  case 'e': g_translate.y -= 1 * g_movespeed; break;
    /* Uppercase: fast movement */
  case 'A': g_translate.x -= 0.1 * g_movespeed; break;
  case 'D': g_translate.x += 0.1 * g_movespeed; break;
  case 'W': g_translate.z += 0.1 * g_movespeed; break;
  case 'S': g_translate.z -= 0.1 * g_movespeed; break;
  case 'Q': g_translate.y += 0.1 * g_movespeed; break;
  case 'E': g_translate.y -= 0.1 * g_movespeed; break;
    /* Mode changes */
  case 13 : g_mode = VIEWER_MODE_SELECT; break;
  case 'm': g_mode = VIEWER_MODE_MOVESEL; break;
    /* Other stuff. */
  case 'i': g_pointsize   = g_pointsize < 2 ? 1 : g_pointsize - 1; break;
  case 'o': g_pointsize   = 1.0; break;
  case 'p': g_pointsize  += 1.0; break;
  case '*': g_movespeed  *= 10;  break;
  case '/': g_movespeed  /= 10;  break;
  case 'x': g_invertrotx *= -1;  break;
  case 'y': g_invertroty *= -1;  break;
  case 'f': g_rot.y      += 180; break;
  case 'C': g_showcoord = !g_showcoord; break;
  case 'c': glGetFloatv( GL_COLOR_CLEAR_VALUE, rgb );
    /* Invert background color */
    if ( *rgb < 0.9 ) {
      glClearColor( 1.0f, 1.0f, 1.0f, 0.0f );
    } else {
      glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    }
    break;
  case 'u':
    for ( i = 0; i < g_cloudcount; i++ ) {
      g_clouds[i].selected = 0;
    }
    break;
  case 't':
    for ( i = 0; i < g_cloudcount; i++ ) {
      g_clouds[i].enabled = !g_clouds[i].enabled;
    }
  }
  /* Control point clouds */
  if ( key >= '0' && key <= '9' ) {
    if ( g_cloudcount > key - 0x30 ) {
      g_clouds[ key - 0x30 ].enabled = !g_clouds[ key - 0x30 ].enabled;
    }
    
  }
  glutPostRedisplay();  
}


/*******************************************************************************
 *         Name:  resizeScene
 *  Description:  Handle resize of window.
 ******************************************************************************/
void resizeScene( int w, int h ) {
  fprintf(stdout,"Resize scene called w=%d d=%d\n",w,h);
  glViewport( 0, 0, w, h );
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
        
  if (1) {
  // lots of good stuff (HZ matrix to openGl matrix): http://strawlab.org/2011/11/05/augmented-reality-with-OpenGL/
  double width = 640;
  double height = 480;
  double zfar = 200;
  double znear = .1;
  double K00 = 533.0692;
  double K11 = 533.0692;
  double K02 = 320.0;
  double K12 = 240.0;
  double K01 = 0.0;
  double x0 = 0;
  double y0 = 0;
  
  float m[16];
  int i; 
  for (i = 0; i < 16; ++i)
    m[i] = 0;
  
  m[0] = 2*K00/width;
  m[4] = -2*K01/width;
  m[5] = -2*K11/height;
  m[8] = (width - 2*K02 + 2*x0)/width;
  m[9] =  (height - 2*K12 + 2*y0)/height;
  m[10] = (-zfar - znear)/(zfar - znear);
  m[11] = -1;
  m[14] = -2*zfar*znear/(zfar - znear);
  
  glLoadMatrixf(m);
  } else {
    gluPerspective( 45, w / (float) h, 0.1, 200 );
  }
  
  //the hud is broken, but who cares
  g_left = (int) ( -tan( 0.39 * w / h ) * 100 ) - 13;
  
  glMatrixMode( GL_MODELVIEW );
  glEnable(     GL_DEPTH_TEST );
  //glDepthFunc(  GL_LEQUAL );
  glDepthFunc(GL_LESS);
}


/*******************************************************************************
 *         Name:  init
 *  Description:  Do some initialization.
 ******************************************************************************/
void init() {

  /**
   * Set mode for GLUT windows:
   * GLUT_RGBA       Red, green, blue, alpha framebuffer.
   * GLUT_DOUBLE     Double-buffered mode.
   * GLUT_DEPTH      Depth buffering.
   * GLUT_LUMINANCE  Greyscale color mode.
   **/
  
  glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
  
  glutInitWindowSize( 640, 480 );
  g_window = glutCreateWindow( "3D Reconstruction Viewer" );
  
  glutDisplayFunc(  &drawScene );
  glutReshapeFunc(  &resizeScene );
  glutKeyboardFunc( &keyPressed );
  glutMotionFunc(   &mouseMoved );
  glutMouseFunc(    &mousePress );
  
  /* Set black as background color */
  glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
  
}


/*******************************************************************************
 *         Name:  cleanup
 *  Description:  Free all allocated memory and do additional stuff.
 ******************************************************************************/
void cleanup() {

  int i;
  for ( i = 0; i < g_cloudcount; i++ ) {
    
    if ( g_clouds[i].vertices ) {
      free( g_clouds[i].vertices );
    }
    if ( g_clouds[i].colors ) {
      free( g_clouds[i].colors );
    }
    if (g_clouds[i].mat) {
      free(g_clouds[i].mat);
    }
  }
  if ( g_clouds ) {
  }
  
}


/*******************************************************************************
 *         Name:  determineFileFormat
 *  Description:  
 ******************************************************************************/
uint8_t determineFileFormat( char * filename ) {
	
  char * ext = strrchr( filename, '.' );
  if ( !ext ) {
    return FILE_FORMAT_NONE;
  }
  if ( !strcmp( ext, ".pts" ) || !strcmp( ext, ".3d" ) ) {
    return FILE_FORMAT_UOS;
  } else if ( !strcmp( ext, ".txt") ) {
    return FILE_FORMAT_TXT;
  } else if ( !strcmp( ext, ".ply" ) ) {
    FILE * f = fopen( filename, "r" );
    if ( f ) {
      char magic_number[5] = { 0 };
      fread( magic_number, 1, 4, f );
      if ( !strcmp( magic_number, "ply\n" ) ) {
        fclose(f);
        return FILE_FORMAT_PLY;
      }
      fclose( f );
    }
  }
  return FILE_FORMAT_NONE;
  
}


/*******************************************************************************
 *         Name:  main
 *  Description:  Main function
 ******************************************************************************/
int main( int argc, char ** argv ) {

  /* Check if we have enough parameters */
  if ( argc < 3 ) {
    printf( "Usage: %s allpoints.bin out.reconstruction MOVIEFLAG \"0 r 1\"\n", argv[0] );
    exit( EXIT_SUCCESS );
  }

  int movieflag = 0;
  char* ply_file = 0;
  if (argc==4 && atoi(argv[3])==1) {
    movieflag = 1;
    fprintf(stdout,"Enabled movie\n");
  } else if (argc==4){
    ply_file = argv[3];
    fprintf(stdout,"Movie diabled, writing cloud to %s\n",ply_file);
  }

  char* points_file = argv[1];
  char* reconstruction_file = argv[2];

  /* First we read all points */
  FILE * f = fopen( points_file, "r" );
  if (!f) {
    fprintf(stderr, "Cannot read %s\n",points_file);
    exit(EXIT_FAILURE);
  }
  int num_points = -1;
  fread(&num_points,sizeof(int),1,f);
  fprintf(stdout, " Num points is  %d\n",num_points);
  
  float* allpoints = (float*) malloc(sizeof(float)*num_points*3);
  uint8_t* allcolors = (uint8_t*) malloc(sizeof(uint8_t)*num_points*3);
  int* allids = (int*) malloc(sizeof(int)*num_points);

  fread(allpoints,sizeof(float),num_points*3,f);
  fread(allcolors,sizeof(uint8_t),num_points*3,f);
  fread(allids,sizeof(int),num_points*1,f);

  fclose(f);
  int i;
  int maxid = -1;
  for (i  = 0; i < num_points; ++i) {
    if (allids[i] > maxid)
      maxid = allids[i];
  }

  fprintf(stdout,"Maxid is %d\n",maxid);

  int* counts = (int*)malloc(sizeof(int)*maxid);
  int* startid = (int*)malloc(sizeof(int)*maxid);
  memset(counts,0,sizeof(int)*maxid);
  memset(startid,0,sizeof(int)*maxid);
  for (i = 0; i < num_points; ++i)
    counts[allids[i]-1]++;

  for (i = num_points; i >=0 ; i--)
    startid[allids[i]-1] = i;

  //for (i = 0; i < maxid; ++i) {
  //  fprintf(stdout,"counts/startid for %d is %d %d\n",i+1,counts[i],startid[i]+1);
  //}
      
  g_cloudcount = maxid;
  /* Prepare array */
  g_clouds = (cloud_t *) malloc( g_cloudcount * sizeof( cloud_t ) );
  if ( !g_clouds ) {
    fprintf( stderr, "Could not allocate memory for point clouds!\n" );
    exit( EXIT_FAILURE );
  }


  for (i = 0; i < g_cloudcount; ++i) {
    //fprintf(stdout,"Processing cloud %d\n",i);
    memset( g_clouds + i, 0, sizeof( cloud_t ) );
    
    g_clouds[i].name = (char*)malloc(100*sizeof(char));
    sprintf(g_clouds[i].name,"%05d",i+1);
    
    g_clouds[i].enabled = 1;

    g_clouds[i].colors = allcolors+(3*startid[i]);
    g_clouds[i].pointcount = counts[i];
    g_clouds[i].vertices = allpoints+(3*startid[i]);
  }

  
  // now open the transformations file
  f = fopen(reconstruction_file, "r");  
  if (!f) {
    fprintf(stderr, "Cannot read %s\n", reconstruction_file);
    exit(EXIT_FAILURE);
  }
  int numimages2 = -1;
  fread(&numimages2, sizeof(int),1,f);
  //fscanf(f, "%d", &numimages2);
  fprintf(stdout, " Num images2 is  %d\n",numimages2);
  if (maxid != numimages2) {
    fprintf(stderr, "maxid != numimages2\n");
    exit( EXIT_FAILURE );
  }
    
  double score1, score2;
  fread(&score1, sizeof(double),1,f);
  fread(&score2, sizeof(double),1,f);
  fprintf(stdout,"Scores are %.8f %.8f\n",score1,score2);
    
  int mode = -1;
  for (i = 0; i < g_cloudcount; ++i) {

    fread(&mode, sizeof(int),1,f);
    double* mat;
    double* invmat;

    mat = (double*)malloc(16*sizeof(double));
    invmat = (double*)malloc(16*sizeof(double));

    fread(mat,sizeof(double),16,f);
    fread(invmat,sizeof(double),16,f);

    /* if (i==49) { */
    /* fprintf(stdout,"--printing out xform %d\n",i); */
    /*  int q;  */
    /* for (q = 0; q < 16; ++q) */
    /*   fprintf(stdout,"Element[%d]=%.5f\n",q,mat[q]); */

    /* for (q = 0; q < 16; ++q) */
    /*   fprintf(stdout,"Element INV [%d]=%.5f\n",q,invmat[q]); */
    
    //fscanf(f,"%i %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
    //       &mode,&mat[0],&mat[1],&mat[2],&mat[3],&mat[4],&mat[5],&mat[6],&mat[7],&mat[8],&mat[9],&mat[10],&mat[11],&mat[12],&mat[13],&mat[14],&mat[15]);    
    g_clouds[i].mat = mat;  
    //fscanf(f,"%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
    //       &invmat[0],&invmat[1],&invmat[2],&invmat[3],&invmat[4],&invmat[5],&invmat[6],&invmat[7],&invmat[8],&invmat[9],&invmat[10],&invmat[11],&invmat[12],&invmat[13],&invmat[14],&invmat[15]);
    
    g_clouds[i].invmat = invmat;
    g_clouds[i].enabled = mode;
    
    if (current_ply_index == -1 && mode == 1)
      current_ply_index = i;
  }

  // note: it needs to be freed sometime?
  //free(mat);
 
  fclose(f);
  if (ply_file != 0) {
    //here we dump the ply file
    //char* plyfile = "/Users/tomasz/Desktop/tmp.ply";
    std::string camfile(ply_file);
    camfile += std::string(".camera.ply");
    dump_ply(ply_file, points_file, reconstruction_file );
    dump_ply_camera(camfile.c_str(), points_file, reconstruction_file);
    exit(1);
  }
  
  /* Print usage information to stdout. */
  printHelp();

  /* Initialize GLUT */
  glutInit( &argc, argv );
  init();

  if (movieflag == 1) {
    int value = -1;
    glutTimerFunc(1,update_movie_index,value);
  }
  
  /* Run program */
  glutMainLoop();
  
  cleanup();
  return EXIT_SUCCESS;
}


/*******************************************************************************
 *         Name:  printHelp
 *  Description:  Prints control information.
 ******************************************************************************/
void printHelp() {

  printf( "\n=== CONTROLS: ======\n"
          "-- Mouse: ---\n"
          " drag left   Rotate point cloud (x/y axis)\n"
          " drag middle Rotate point cloud (x/z axis)\n"
          " drag right  Move up/down, left/right\n"
          " wheel       Move forward, backward (fact)\n"
          "-- Keyboard (normal mode): ---\n"
          " i,o,p       Increase, reset, decrease pointsize\n"
          " a,d         Move left, right\n"
          " w,s         Move forward, backward\n"
          " q,e         Move up, down\n"
          " j           Jump to start position\n"
          " +,-         Zoom in, out\n"
          " *,/         Increase/Decrease movement speed\n"
          " 0...9       Toggle visibility of point clouds 0 to 9\n"
          " t           Toggle visibility of all point clouds\n"
          " u           Unselect all clouds\n"
          " c           Invert background color\n"
          " C           Toggle coordinate axis\n"
          " <return>    Enter selection mode\n"
          " m           Enter move mode\n"
          " <esc>       Quit\n"
          "-- Keyboard (selection mode): ---\n"
          " 0..9        Enter cloud number\n"
          " <return>    Apply selection.\n"
          " <esc>       Cancel selection\n"
          "-- Keyboard (move mode): ---\n"
          " a,d         Move left, right (fast)\n"
          " w,s         Move forward, backward (fast)\n"
          " q,e         Move up, down (fast)\n"
          " r,f         Rotate around x-axis\n"
          " t,g         Rotate around y-axis\n"
          " z,h         Rotate around z-axis\n"
          " p           Print pose\n"
          " P           Generate pose files in current directory\n"
          " l           Load pose files for selected clouds from current directory.\n"
          " L           Load pose files for selected clouds from cloud directory.\n"
          " m,<esc>     Leave move mode\n"
    );
  
}

void update_movie_index(int value) {

  // first find first index
  int i;
  int first_index = -1;
  for (i = 0 ; i < g_cloudcount; ++i) {
    if (first_index == -1 && g_clouds[i].enabled) {
      first_index = i;
      break;
    }
  } 

  if (current_ply_index == g_cloudcount-1) {
    current_ply_index = first_index;
  } else {
    
    int next_index = -1;
    for (i =  current_ply_index+1; i < g_cloudcount; ++i) {
      if (next_index == -1 && g_clouds[i].enabled) {
        next_index = i;
        break;
      }
    }
    if (next_index == -1)
      current_ply_index = first_index;
    else
      current_ply_index = next_index;
  }
  
  fprintf(stdout,"Update movie has ply index = %d\n", current_ply_index);
  drawScene();
  glutTimerFunc(1,update_movie_index,value);
}

int dump_ply(const char* filename, const char* points_file, const char* reconstruction_file) {
  if (filename == 0)
    return -1;
  std::cout<<"Dumping ply file to " << std::string(filename)<<std::endl;
  FILE* f = fopen(filename,"w");
  if (!f) {
    fprintf(stderr, "Cannot read %s\n",filename);
    return -1;
  } 
  
  int count = 0;
  for (int i = 0; i < g_cloudcount; ++i)
    if (g_clouds[i].enabled)
      count+=g_clouds[i].pointcount;

  std::vector<int> valid_inds;

  for (int i = 0; i < g_cloudcount; ++i)
    if (g_clouds[i].enabled) {
      valid_inds.push_back(i);
    }

  
  fprintf (f, "ply\n");
  fprintf (f, "format binary_little_endian 1.0\n");
  fprintf (f, "comment Made by Tomasz Malisiewicz (tomasz@csail.mit.edu)\n");
  fprintf (f, "comment Made with ptsviewer %s %s\n", points_file, reconstruction_file);
  fprintf (f, "comment This is the reconstrution file\n");
  fprintf (f, "element vertex %d\n", count);
  fprintf (f, "property float x\n");
  fprintf (f, "property float y\n");
  fprintf (f, "property float z\n");
  fprintf (f, "property uchar red\n");
  fprintf (f, "property uchar green\n");
  fprintf (f, "property uchar blue\n");
  fprintf (f, "end_header\n");

  for (int iii = 0; iii < valid_inds.size(); ++iii) {
    int i = valid_inds[iii];
    
    //for (int i = 0; i < g_cloudcount; ++i) {
    //if (!g_clouds[i].enabled)
    //  continue;
    
    COLOUR c = GetColour((double)iii,(double)0,(double)(valid_inds.size()-1));
    uint8_t c2[] = {(c.r*255),(c.g*255),(c.b*255)};
    //std::cout<<"color float is " << c.r <<" " << c.g <<" " <<c.b<<std::endl;
    //std::cout<<"color is " << int(c2[0]) <<" " << int(c2[1]) << " " << int(c2[2])<<std::endl;

    Eigen::Matrix4f T;
    for (int q = 0; q < 16; ++q)
      T(q) = (g_clouds[i].mat[q]);
    for (int j = 0; j < g_clouds[i].pointcount; ++j) {
      Eigen::Vector4f x;
      x(3) = 1;
      for (int q = 0; q < 3; ++q)
        x(q) = g_clouds[i].vertices[3*j+q];
      x = T*x;    
      fwrite((void*)(&x),sizeof(float),3,f);
      // write real scene RGB point
      //fwrite((void*)(&g_clouds[i].colors[3*j]),sizeof(uint8_t),3,f);
      // write color based on index of scan
      fwrite((void*)(&c2),sizeof(uint8_t),3,f);
    }
  }
  fclose(f);
  return 1;
}

int dump_ply_camera(const char* filename, const char* points_file, const char* reconstruction_file) {
  if (filename == 0)
    return -1;
  std::cout<<"Dumping ply file to " << std::string(filename)<<std::endl;
  FILE* f = fopen(filename,"w");
  if (!f) {
    fprintf(stderr, "Cannot read %s\n",filename);
    return -1;
  } 

  std::vector<int> valid_inds;

  for (int i = 0; i < g_cloudcount; ++i)
    if (g_clouds[i].enabled) {
      valid_inds.push_back(i);
    }

  // number of points to draw line chunk between camera cetners
  int NCUT = 10;

  // number of points to draw line line chunk between cameras and orientation vectors
  int NCUT_visible = 100;

  int size = valid_inds.size() + (valid_inds.size()-1)*NCUT + valid_inds.size()*NCUT_visible;

  fprintf (f, "ply\n");
  fprintf (f, "format binary_little_endian 1.0\n");
  fprintf (f, "comment Made by Tomasz Malisiewicz (tomasz@csail.mit.edu)\n");
  fprintf (f, "comment Made with ptsviewer %s %s\n", points_file, reconstruction_file);
  fprintf (f, "comment This is the camera centers file\n");
  fprintf (f, "element vertex %d\n", size);
  fprintf (f, "property float x\n");
  fprintf (f, "property float y\n");
  fprintf (f, "property float z\n");
  fprintf (f, "property uchar red\n");
  fprintf (f, "property uchar green\n");
  fprintf (f, "property uchar blue\n");
  fprintf (f, "end_header\n");

  for (int iii = 0; iii < valid_inds.size(); ++iii) {
    int i = valid_inds[iii];

    Eigen::Matrix4f T;
    for (int q = 0; q < 16; ++q)
      T(q) = (g_clouds[i].mat[q]);
    
    for (int j = 0; j < 3; ++j) {
      fwrite((void*)(&T(j,3)),sizeof(float),1,f);
    }
    COLOUR res = GetColour(iii,0,valid_inds.size()-1);
    uint8_t cols[3] = {res.r*255,res.g*255,res.b*255};
    fwrite((void*)(&cols),sizeof(uint8_t),3,f);
  }

  for (int iii = 0; iii < valid_inds.size()-1; ++iii) {
    int i = valid_inds[iii];
    int j = valid_inds[iii+1];
    
    double* point1 = (g_clouds[i].mat+12);
    double* point2 = (g_clouds[j].mat+12);

    uint8_t cols[3] = {0,0,255};

    write_point_chunk(f,point1,point2,NCUT,cols);
  }

  for (int iii = 0; iii < valid_inds.size(); ++iii) {

    int i = valid_inds[iii];    
    COLOUR res = GetColour(iii,0,valid_inds.size()-1);
    uint8_t cols[3] = {res.r*255,res.g*255,res.b*255};
    

    double* point1 = (g_clouds[i].mat+12);
    
    Eigen::Matrix3d R;
    Eigen::Matrix4d T(g_clouds[i].mat);

    for (int a = 0; a < 3; ++a)
      for (int b = 0; b < 3; ++b)
      {
        R(a,b) = T(a,b);
      }

    Eigen::Vector3d normal,point2;
    normal(0) = 0;
    normal(1) = 0;
    normal(2) = .05;
    point2 = R*normal + Eigen::Vector3d(point1);

    Eigen::Vector3d diff = point2-Eigen::Vector3d(point1);

    write_point_chunk(f,point1,point2.data(),NCUT_visible,cols);

  }

  fclose(f);
  return 1;
}

COLOUR GetColour(double v,double vmin,double vmax)
{
   COLOUR c = {1.0,1.0,1.0}; // white
   double dv;

   if (v < vmin)
      v = vmin;
   if (v > vmax)
      v = vmax;
   dv = vmax - vmin;

   if (v < (vmin + 0.25 * dv)) {
      c.r = 0;
      c.g = 4 * (v - vmin) / dv;
   } else if (v < (vmin + 0.5 * dv)) {
      c.r = 0;
      c.b = 1 + 4 * (vmin + 0.25 * dv - v) / dv;
   } else if (v < (vmin + 0.75 * dv)) {
      c.r = 4 * (v - vmin - 0.5 * dv) / dv;
      c.b = 0;
   } else {
      c.g = 1 + 4 * (vmin + 0.75 * dv - v) / dv;
      c.b = 0;
   }

   return(c);
}

void write_point_chunk(FILE* f, double* point1, double* point2, int NCUT, uint8_t* cols) {

  float diff0 = (point2[0]-point1[0]) / (NCUT+1);
  float diff1 = (point2[1]-point1[1]) / (NCUT+1);
  float diff2 = (point2[2]-point1[2]) / (NCUT+1);
  
  for (int i = 1; i <= NCUT; ++i) {
    float x0 = point1[0]+diff0*i;
    float x1 = point1[1]+diff1*i;
    float x2 = point1[2]+diff2*i;
    fwrite((void*)&x0,sizeof(float),1,f);
    fwrite((void*)&x1,sizeof(float),1,f);
    fwrite((void*)&x2,sizeof(float),1,f);

    fwrite((void*)(cols),sizeof(uint8_t),3,f);
  }

}
