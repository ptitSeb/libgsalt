#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include<GL/glut.h>
#include <gsalt/gsalt.h>
#include "viewer/arcball.h"                           /*  Arc Ball  Interface         */
#include "Mesh/TriangleSoup.h"
#include "bmp/RgbImage.h"

//compiling command
//g++ -lGL -lglut -lGLU -IMeshLib view_mesh.C -o view_mesh

using namespace MeshLib::TriangleSoup;
using namespace MeshLib;


CTriangleMesh mesh;

/* window width and height */
int win_width, win_height;
int gButton;
int startx,starty;
int shadeFlag   = 0 ;

/* rotation quaternion and translation vector for the object */
MeshLib::CQrot       ObjRot(0,0,1,0);
MeshLib::CPoint      ObjTrans(0,0,0);
/* arcball object */
MeshLib::CArcball arcball;

/* geometry flag */
int geometryFlag = 0;

/* texture flag */
int textureFlag =  2; 
/* texture id and image */
GLuint texName;
RgbImage image;


/*! setup the object, transform from the world to the object coordinate system */
void setupObject(void)
{
    double rot[16];

    glTranslated( ObjTrans[0], ObjTrans[1], ObjTrans[2]);
    ObjRot.convert( rot );
    glMultMatrixd(( GLdouble *)  rot );
}

/*! the eye is always fixed at world z = +5 */

void setupEye(void){
  glLoadIdentity();
  gluLookAt( 0,0, 5,0,0,0,0,1,0);
}

/*! setup light */
void setupLight()
{
	//MeshLib::CPoint position(0,0,-1);
	MeshLib::CPoint position(0,0,1);
	GLfloat lightOnePosition[4]={position[0], position[1], position[2], 0};
	glLightfv(GL_LIGHT1, GL_POSITION, lightOnePosition);
	MeshLib::CPoint position2(0,0,-1);
	GLfloat lightTwoPosition[4]={position2[0], position2[1], position2[2], 0};
	glLightfv(GL_LIGHT2, GL_POSITION, lightTwoPosition);
}

/*! draw mesh */
void draw_mesh()
{
	  glBindTexture(GL_TEXTURE_2D, texName);

	//glColor3f( 1.0, 1.0, 1.0 );
	//glDisable( GL_LIGHTING );
	for( size_t i = 0; i < mesh.faces().size(); i ++ )
	{
		CFace * pF = mesh.faces()[i];
  		glBegin(GL_TRIANGLES);
		if( shadeFlag == 0 )
		{
			glNormal3f( pF->m_normal[0], pF->m_normal[1], pF->m_normal[2] );
			//glNormal3f( -pF->m_normal[0], -pF->m_normal[1], -pF->m_normal[2] );
			//std::cout << pF->m_normal[0] << " " << pF->m_normal[1] << " " << pF->m_normal[2] << std::endl;
		}
		for( int j = 0; j < 3; j ++ )
		{
			CVertex * pV = pF->m_v[j];
			if( shadeFlag == 1 )
				glNormal3f( pV->m_normal[0], pV->m_normal[1], pV->m_normal[2] );

	
			if( mesh.m_has_uv )
			{
				MeshLib::CPoint2 uv = pV->m_uv;
				glTexCoord2d( uv[0], uv[1] );
			}

			MeshLib::CPoint skin_rgb( 235/255.0,180/255.0,173/255.0);

			if( mesh.m_has_rgb )
				glColor3f( pV->m_rgb[0], pV->m_rgb[1], pV->m_rgb[2] );
			else
				//glColor3f( 1,1,1 );
				glColor3f( skin_rgb[0], skin_rgb[1],skin_rgb[2] );

			if( geometryFlag && mesh.m_has_uv )
			{
				glVertex3d( pV->m_uv[0], pV->m_uv[1] , 0);
			}
			else
			{
				glVertex3d( pV->m_point[0], pV->m_point[1], pV->m_point[2] );
			}
		}
		glEnd();
	}

}


/*! display call back function
*/
void display()
{
	/* clear frame buffer */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	setupLight();
	/* transform from the eye coordinate system to the world system */
	setupEye();
	glPushMatrix();
	/* transform from the world to the ojbect coordinate system */
	setupObject();
  	/* draw the mesh */
	draw_mesh();

	glPopMatrix();
	glutSwapBuffers();
}

/*! Called when a "resize" event is received by the window. */

void reshape(int w, int h)
{
  float ar;

  win_width=w;
  win_height=h;

  ar = (float)(w)/h;
  glViewport(0, 0, w, h);               /* Set Viewport */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  // magic imageing commands
  gluPerspective( 40.0, /* field of view in degrees */
		  ar, /* aspect ratio */
		  1.0, /* Z near */
		  100.0 /* Z far */);

  glMatrixMode(GL_MODELVIEW);

  glutPostRedisplay();
}

/*! helper function to remind the user about commands, hot keys */
void help()
{
  printf("w - Wireframe Display\n");
  
  printf("f  -  Flat Shading \n");
  printf("s  -  Smooth Shading\n");
  printf("t  -  Texture Mapping\n");
  printf("r  -  Reset the view\n" );
  printf("g  -  Toggle Geometry and UV view\n");

  printf("o  -  Save frame buffer to snap_n.bmp\n");
  printf("? -  Help Information\n");
  printf("esc - quit\n");
}

/*! Keyboard call back function */

void keyBoard(unsigned char key, int x, int y)
{

	switch( key )
	{
	case 'f':
		//Flat Shading
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		shadeFlag = 0;
		break;
	case 's':
    		//Smooth Shading
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		shadeFlag = 1;
		break;
	case 'w':
	  	//Wireframe mode
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;

  case 't':
    textureFlag = (textureFlag +1 )%3;
    switch( textureFlag )
    {
    case 0:
        glDisable(GL_TEXTURE_2D);
        break;
    case 1:
        glEnable(GL_TEXTURE_2D);
		    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        break;
    case 2:
       glEnable(GL_TEXTURE_2D);
		    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
       break;
    }
    break;

  case 'g':
	  geometryFlag = (geometryFlag + 1)%2;
	  break;

  case 'r':
	  	ObjRot = MeshLib::CQrot(0,0,1,0);
		ObjTrans = MeshLib::CPoint(0,0,0);
	  break;

  case '?':
    help();
    break;



  case 27:
		exit(0);
		break;
	}
  glutPostRedisplay();
}

/*! setup GL states */
void setupGLstate(){

  GLfloat lightOneColor[] = {1, 1, 1, 1};
  GLfloat globalAmb[] = {.1, .1, .1, 1};
  GLfloat lightOnePosition[] = {.0,  .0, 1, 0.0};

  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  //glFrontFace(GL_CW);
  glEnable(GL_DEPTH_TEST);
  //glClearColor(0,0,0,0);
  glClearColor(1.0,1.0,1.0,1.0);
  glShadeModel(GL_SMOOTH);


  glEnable(GL_LIGHT1);
  glEnable(GL_LIGHT2);
  glEnable(GL_LIGHTING);
  glEnable(GL_NORMALIZE);
  glEnable(GL_COLOR_MATERIAL);

  glLightfv(GL_LIGHT1, GL_DIFFUSE, lightOneColor);
  glLightfv(GL_LIGHT2, GL_DIFFUSE, lightOneColor);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmb);
  glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

  //glColorMaterial(GL_BACK, GL_AMBIENT_AND_DIFFUSE);

  glLightfv(GL_LIGHT1, GL_POSITION, lightOnePosition);
}

/*! mouse click call back function */
void  mouseClick(int button , int state, int x, int y){


  /* set up an arcball around the Eye's center
	  switch y coordinates to right handed system  */

  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
  {
      gButton = GLUT_LEFT_BUTTON;
	  arcball = MeshLib::CArcball( win_width, win_height,  x-win_width/2,  win_height-y-win_height/2);
  }

  if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN) {
      startx = x;
      starty = y;
      gButton = GLUT_MIDDLE_BUTTON;
   }

  if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
      startx = x;
      starty = y;
      gButton = GLUT_RIGHT_BUTTON;
   }
  return ;
}

/*! mouse motion call back function */

void mouseMove(int x, int y)
{
	MeshLib::CPoint trans;
	MeshLib::CQrot       rot;

  /* rotation, call arcball */
  if (gButton == GLUT_LEFT_BUTTON )
  {
      rot = arcball.update( x-win_width/2, win_height-y-win_height/2);
      ObjRot =  rot * ObjRot;
      glutPostRedisplay();
  }

  /*xy translation */
  if (gButton == GLUT_MIDDLE_BUTTON)
  {
	  double scale = 10./win_height;
      trans =  MeshLib::CPoint(scale*(x-startx), scale*(starty-y), 0  );
	    startx = x;
	    starty = y;
      ObjTrans = ObjTrans + trans;
      glutPostRedisplay();
  }

  /* zoom in and out */
  if (gButton == GLUT_RIGHT_BUTTON ) {
      double scale = 10./win_height;
      trans =  MeshLib::CPoint(0,0, scale*(starty-y)   );
	    startx = x;
	    starty = y;
      ObjTrans = ObjTrans+trans;
      glutPostRedisplay();
  }

}


void normalize( CTriangleMesh & mesh )
{
	MeshLib::CPoint center(0,0,0);
	for( size_t i = 0; i < mesh.vertices().size(); i ++ )
	{
		CVertex * pV = mesh.vertices()[i];
		center += pV->m_point;
	}
	center /= (double)(mesh.vertices().size());

	for( size_t i = 0; i < mesh.vertices().size(); i ++ )
	{
		CVertex * pV = mesh.vertices()[i];
		pV->m_point -= center;
	}

	double len = -1;
	for( size_t i = 0; i < mesh.vertices().size(); i ++ )
	{
		CVertex * pV = mesh.vertices()[i];
		for( int j = 0; j < 3; j ++ )
		{
			len = (len > fabs(pV->m_point[j]))? len: fabs(pV->m_point[j]);
		}
	}

	for( size_t i = 0; i < mesh.vertices().size(); i ++ )
	{
		CVertex * pV = mesh.vertices()[i];
		pV->m_point /= len;
	}

}

void compute_normal( CTriangleMesh & mesh )
{
	for( size_t i = 0; i < mesh.faces().size(); i ++ )
	{
		CFace * pF = mesh.faces()[i];
		MeshLib::CPoint p[3];
		for( int j = 0; j < 3; j ++ )
		{
			p[j] = pF->m_v[j]->m_point;
		}
		MeshLib::CPoint nor = (p[1]-p[0])^(p[2]-p[0]);
		pF->m_area = nor.norm();

		nor /= nor.norm();
		pF->m_normal = nor;
	}

	for( size_t i = 0; i < mesh.faces().size(); i ++ )
	{
		CFace * pF = mesh.faces()[i];
		MeshLib::CPoint p[3];
		for( int j = 0; j < 3; j ++ )
		{
			p[j] = pF->m_v[j]->m_point;
		}
		MeshLib::CPoint nor = (p[1]-p[0])^(p[2]-p[0]);
		nor /= nor.norm();
		pF->m_normal = nor;

		for( int j = 0; j < 3; j ++ )
		{
			pF->m_v[j]->m_normal += pF->m_normal * pF->m_area;
		}
	}

	for( size_t i = 0; i < mesh.vertices().size(); i ++ )
	{
		CVertex * pV = mesh.vertices()[i];
		pV->m_normal /= pV->m_normal.norm();
	}

    for( size_t i = 0; i < mesh.vertices().size(); i ++ )
	{
		CVertex * pV = mesh.vertices()[i];
		pV->m_area = 0;
	}

    //compute the vertex area
	for( size_t i = 0; i < mesh.faces().size(); i ++ )
	{
		CFace * pF = mesh.faces()[i];
		for( int j = 0; j < 3; j ++ )
		{
			pF->m_v[j]->m_area += pF->m_area/3.0;
		}
	}
}

/*! initialize bitmap image texture */

void initialize_bmp_texture()
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &texName);
	glBindTexture(GL_TEXTURE_2D, texName);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,   GL_LINEAR);

	int ImageWidth  = image.GetNumCols();
	int ImageHeight = image.GetNumRows();
	GLubyte * ptr   = (GLubyte * )image.ImageData();

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 
				        ImageWidth,
						ImageHeight, 
				        0, 
				        GL_RGB, 
				        GL_UNSIGNED_BYTE,
						ptr);

    if(textureFlag == 1)
		  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    else if(textureFlag == 2)
		  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_TEXTURE_2D);
}


/*! main function for viewer
*/
int main( int argc, char * argv[] )
{
	int reduce = 0;

	if(argc==1) {
		printf("Usage %s SampleFile [Texture] [-0..-100]  (to have -x%% of the triangles)\n", argv[0]);
		exit(0);
	}

	std::string name( argv[1] );

	unsigned pos = name.find_last_of(".");
	std::string type  = name.substr( pos +1 );

	if( type == "obj" )
	{

		mesh.read_obj( argv[1] );
		normalize( mesh );
		compute_normal( mesh );

	}
	else if( type == "m" )
	{
		mesh.read_m( argv[1] );
		normalize( mesh );
		compute_normal( mesh );
	}

	textureFlag = 0;

	if( argc > 2 )
	{
		if( argv[2][0]!='-' ) 
		{
			image.LoadBmpFile( argv[2] );
			textureFlag = 2;
		}
		else
		{
			reduce = atoi(argv[2]);
		}
		if( argc > 3 )
		{
			if( argv[3][0]!='-' )
			{
				image.LoadBmpFile( argv[3] );
				textureFlag = 2;
			}
			else
			{
				reduce = atoi(argv[3]);
			}
		}
	}
	if (reduce>0) reduce = 0;
	if(reduce<-100) reduce = -100;
	if(reduce<0) {
		int num_vertex=0, num_triangles=0;
		bool with_uv, with_normal, with_color;
		num_vertex = mesh.vertices().size();
		num_triangles = mesh.faces().size();
		with_uv = mesh.m_has_uv;
		with_normal = mesh.m_has_normal;
		with_color = mesh.m_has_rgb;

		printf("Reducing geometry (%s uv, %s normals and %s colors) from %d triangles (%d vertices) to %d\n", (with_uv)?"with":"w/o",(with_normal)?"with":"w/o",(with_color)?"with":"w/o",num_triangles, num_vertex, (100+reduce)*num_triangles/100);
		unsigned int flags = GSALT_FACE | GSALT_VERTEX;
		if(with_uv) flags |= GSALT_TEXCOORD;
		if(with_normal) flags |= GSALT_NORMAL;
		if(with_color) flags |= GSALT_COLOR;
		GSalt gsalt = gsalt_new(num_vertex, num_triangles, flags);

		for (int i=0; i<num_vertex; i++) {
			const CVertex *v = mesh.vertices()[i];
			gsalt_add_vertex(gsalt, v->m_point[0], v->m_point[1], v->m_point[2]);
			if(with_uv) gsalt_add_texcoord(gsalt, v->m_uv[0], v->m_uv[1]);
			if(with_normal) gsalt_add_normal(gsalt, v->m_normal[0], v->m_normal[1], v->m_normal[2]);
			if(with_color) gsalt_add_color(gsalt, v->m_rgb[0], v->m_rgb[1], v->m_rgb[2]);
		}
		for (int i=0; i<num_triangles; i++) {
			const CFace *f = mesh.faces()[i];
			gsalt_add_triangle(gsalt, f->m_v[0]->m_cnt, f->m_v[1]->m_cnt, f->m_v[2]->m_cnt);
		}

		int ret = gsalt_simplify(gsalt, (100+reduce)*num_triangles/100);

		//put the new geometry
		int new_vertex = gsalt_query_numvertex(gsalt);
		int new_triangles = gsalt_query_numtriangles(gsalt);
		if((new_vertex<num_vertex) && (new_triangles<num_triangles))
		{
			mesh.vertices().clear();
			mesh.faces().clear();
			for (int i=0; i<new_vertex; i++) {
				float x, y, z;
				CVertex *v = new CVertex;
				v->m_area = 0.0;
				v->m_id = i;
				v->m_cnt = i;
				gsalt_query_vertex(gsalt, i, &x, &y, &z);
				v->m_point = CPoint(x,y,z);
				if(with_uv) {
					gsalt_query_texcoord(gsalt, i, &x, &y);
					v->m_uv = CPoint2(x,y);
				}
				if(with_color) {
					gsalt_query_color(gsalt, i, &x, &y, &z);
				} else {
					x=1.0f; y=1.0f; z=1.0f;
				}
				v->m_rgb = CPoint(x,y,z);
				if(with_normal) {
					gsalt_query_normal(gsalt, i, &x, &y, &z);
					v->m_normal = CPoint(x,y,z);
				}
				mesh.vertices().push_back(v);
			}
			for (int i=0; i<new_triangles; i++) {
				uint32_t a, b, c;
				CFace *f = new CFace;
				gsalt_query_triangle_uint32(gsalt, i, &a, &b, &c);
				f->m_id = i;
				f->m_area = 0.0;
				f->m_v[0] = mesh.vertices()[a];
				f->m_v[1] = mesh.vertices()[b];
				f->m_v[2] = mesh.vertices()[c];
				mesh.faces().push_back(f);
			}
			compute_normal( mesh );
		}

		gsalt_delete(gsalt);
	} else
		printf("Geometry (%s uv, %s normals and %s colors) is %d triangles (%d vertices) to %d\n", (mesh.m_has_uv)?"with":"w/o",(mesh.m_has_normal)?"with":"w/o",(mesh.m_has_rgb)?"with":"w/o",mesh.faces().size(), mesh.vertices().size());

	/* glut stuff */
	glutInit(&argc, argv);                /* Initialize GLUT */
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowSize(600, 600);
	glutCreateWindow("Mesh Viewer");        /* Create window with given title */
	glViewport(0,0,800,800 );

	glutDisplayFunc(display);             /* Set-up callback functions */
	glutReshapeFunc(reshape);
	glutMouseFunc(mouseClick);
	glutMotionFunc(mouseMove);
	glutKeyboardFunc(keyBoard);
	setupGLstate();


	if( textureFlag )
	{
		printf("Image loaded, size=%lux%lu\n", image.GetNumRows(), image.GetNumCols());
		initialize_bmp_texture();

	}

	glutMainLoop();                       /* Start GLUT event-processing loop */

	return 0;
}


