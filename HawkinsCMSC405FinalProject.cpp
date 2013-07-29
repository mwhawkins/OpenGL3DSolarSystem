/**
* Michael Hawkins
* UMUC CMSC 405 Final Project
* A 3-D lighted, animated Solar System
* with a menu to control aspets of the simulation
* July 20, 2013
**/
#include <stdlib.h> 
#include <GL/glut.h>
#include <math.h>
#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>

void OpenGLInit(void);
static void Animate(void );
static void SpeedUp(void );
static void SlowDown(void );
static void ResizeWindow(int w, int h);
static GLenum spinMode = GL_TRUE;
static GLenum singleStep = GL_FALSE;
void MakeAllImages();
void MakeImage(const char bitmapFilename[], GLuint &textureName, bool hasAlpha);

//Set the lighting position to create a sun effect
const GLfloat LIGHT_ORIGIN_POSITION[]    = { 0.0, 0.0, -8.0, 1.0};

//Structs to represent the actual star positions
struct StarFieldObject {
public:
	GLfloat X, Y, Z;
};
//Object to hold all star objects
std::vector<StarFieldObject> so;

int instructionMenu;
int font=(int)GLUT_BITMAP_8_BY_13;

//Settings
bool DrawStars = true;
bool LookFromTop = false;
bool LookFromBottom = false;
bool LookOrigin = false;
bool UseTeapots = false;
float AttenuationFactor = 0.02f;

//Controls for the animation speed
static float HourOfDay = 1.0;
static float DayOfYear = 1.0;
static float DayOfVenusYear = 1.0;
static float DayOfMercuryYear = 1.0;
static float DayOfCometYear = 1.0;
static float AnimateIncrement = 24.0;  // Time step for animation (hours)

static int window;
static int menu_id;
static int submenu_id;
static int value = 0; 

GLfloat cx, cz, lx, lz = 0.0;
GLfloat angle = 15.0;
GLfloat scale = 0.0;

//Comet fog
GLfloat density = 0.1;
GLfloat fogColor[4] = {0.75, 0.75, 0.75, 1.0};

//Lighting settings
GLfloat mat_emission[] = {1.0, 1.0, 0.0, 1.0};
GLfloat no_mat[] = {0, 0, 0, 0};
GLfloat light_pos[4] = {0.0, 0.0, 0.0, 1.0};
GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat mat_mercury_specular[] = { 190.0, 190.0, 190.0, 190.0 };
GLfloat mat_venus_specular[] = { 0.60, 0.60, 0.60, 1.00 };
GLfloat mat_earth_specular[] = { 0.60, 0.60, 0.60, 1.0 };
GLfloat mat_shininess[] = { 35.0 };
GLfloat mat_star_shininess[] = { 5.0 };
GLfloat white_star_light[] = { 1.0, 1.0, 1.0, 0.15 };
GLfloat white_light[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat lmodel_ambient[] = { 0.1, 0.1, 0.1, 1.0 };

// Convenience function for displaying strings
// Excerpt from instructor provided example code
void renderBitmapString(float x, float y, void *font,char *string)
{  
  char *c;

  glRasterPos3f(x, y,0.0);
  
  for (c=string; *c != '\0'; c++) {
    glutBitmapCharacter(font, *c);
  }
}

//Initialization method for instruction window
void initInstructions(void) {
 glClearColor (1.0, 1.0, 1.0, 0.0);
}

//Code to create the instruction window
void instructionWindow(void){
	glClear (GL_COLOR_BUFFER_BIT);

	glColor3f (0.0, 0.0, 1.0);     
	// Directions 
	renderBitmapString(20,350,(void *)font,"3D Solar System Animation With");
	renderBitmapString(20,330,(void *)font,"Lighting, Transformation and Menu.");
	renderBitmapString(20,310,(void *)font,"Includes Sun, Mercury, Venus and Earth + Moon.");

	glColor3f (1.0, 0.0, 0.0); 
	renderBitmapString(20,280,(void *)font,"Instructions:");
	renderBitmapString(20,265,(void *)font,"For best viewing, expand the main screen.");
	renderBitmapString(20,250,(void *)font,"Click your Left (or Right) Mouse Button in the");
	renderBitmapString(20,235,(void *)font,"Solar System Window to bring up the menu.");
	renderBitmapString(20,205,(void *)font,"Select a menu option to change the viewpoint,");
	renderBitmapString(20,190,(void *)font,"speed, or shapes used in the scene.");
	renderBitmapString(20, 155,(void *)font,"Select 'Reset Scene' to reset to normal.");

	glColor3f (0.0, 0.0, 0.0); 
	renderBitmapString(175,100,(void *)font,"Enjoy!");
    
}

void winReshapeFcn (GLint newWidth, GLint newHeight)
{
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ( );
    gluOrtho2D (0.0, (GLdouble) newWidth, 0.0, (GLdouble) newHeight);
    glClear (GL_COLOR_BUFFER_BIT);  
}

//Speed up animation
static void SpeedUp(void)
{
    AnimateIncrement *= 16.0;			// Double the animation timing
}

//Slow down animation
static void SlowDown(void)
{
    AnimateIncrement /= 16.0;			// Halve the animation timing
	
}

//Increase rotation angle
static void IncreaseAngle(void){
	angle += 15;
}

//Decrease rotation angle
static void DecreaseAngle(void){
	angle -= 15;
}

//Zoom in
static void ZoomIn(void){
	scale += 2.0;
}

//Zoom out
static void ZoomOut(void){
	scale -= 2.0;
}

//Generate the star field coordinates randomly
void GenerateStars(int numStars){
	int v1, v2, v3; 	
	float comb;

	for ( int i = 0; i <= numStars*5; i++ )
    {
		v1 = (rand() % 30);  
		v2 = (rand() % 30); 
		v3 = (rand() % 20)/2;
			StarFieldObject sobj;
			sobj.X = v1;
			sobj.Y = v2;
			sobj.Z = v3;
			so.push_back(sobj);		
    }
}

//Draw the stars based on randomly generated coordinates
void RenderStars(){

	glPointSize(2.0);
	int v1; 
	int v2; 
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_star_shininess);
	glMaterialfv(GL_FRONT, GL_EMISSION, white_star_light);
	glTranslatef ( 0.0, 0.0, -8.0 );
	glBegin( GL_POINTS );
    glColor4f(1, 1, 1, 1);
	for ( int i = 0; i < so.size(); i++ )
    {	
		if (i > so.size()/2){
			glVertex3f( -so[i].X + i, so[i].Y + i, so[i].Z );
		}else{
			glVertex3f( so[i].X - i, so[i].Y - i, -so[i].Z );
		}
    }
    glEnd();

}

//The main animation method
void Animate()
{    
    if (spinMode) {
		//Update animation
        HourOfDay += AnimateIncrement;
        DayOfYear += AnimateIncrement/24.0;

        HourOfDay = HourOfDay - ((int)(HourOfDay/24))*24;
        DayOfYear = DayOfYear - ((int)(DayOfYear/365))*365;

		DayOfVenusYear += AnimateIncrement/24.0;
		DayOfVenusYear = DayOfVenusYear - ((int)(DayOfVenusYear/243))*243;

		DayOfMercuryYear += AnimateIncrement/24.0;
		DayOfMercuryYear = DayOfMercuryYear - ((int)(DayOfMercuryYear/88))*88;

		DayOfCometYear += AnimateIncrement/24.0;
		DayOfCometYear = DayOfCometYear - ((int)(DayOfCometYear/500))*500;

		}
	
    glLoadIdentity();

	//Rotation angles
	if (LookFromBottom){
		glTranslatef(1.0, 1.0, -8.0);
		glRotatef( angle + 75, 1.0, 0.0, 0.0 );
	}else if(LookFromTop){
		glTranslatef(1.0, 1.0, -8.0);		
		glRotatef( angle - 105, 1.0, 0.0, 0.0 );		
	}else if(LookOrigin){
		glTranslatef(0.0, 0.0, 0.0);
	}else{
		//Translate the scene
		glTranslatef ( 0.0, 0.0, -8.0 );
		//Rotate the plane
		glRotatef( angle, 1.0, 0.0, 0.0 );
	}

	//Scaling transformation
	if (scale > 0.0){
		glScalef(scale, scale, scale);
	}
	

    //The Sun
	glPushMatrix();	 
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
		glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
		glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
		glColor4f(1, 1, 0, 1);	
		if (UseTeapots){
			glutSolidTeapot(0.5);					
		}else{
			glutSolidSphere(0.5, 20, 16); /* draw sun */
		}
	glPopMatrix();

	//Mercury
	glPushMatrix();	
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_mercury_specular);
		glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
		glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);
		glRotatef( 360.0*DayOfMercuryYear/88.0, 0.0, 1.0, 0.0 );
		glTranslatef( 2.0, 0.0, 0.0 );    					
		glRotatef( 360.0*HourOfDay/1450.0, 0.0, 1.0, 0.0 );
		glColor3f( 1.0, 0.0, 0.0 );
		if (UseTeapots){
			glutSolidTeapot(0.13);				
		}else{
			glutSolidSphere( 0.13, 15, 15);
		}
    glPopMatrix();

	//Venus	
	glPushMatrix();	
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_venus_specular);
		glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
		glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);
		glRotatef( 360.0*DayOfVenusYear/243.0, 0.0, 1.0, 0.0 );
		glTranslatef( 4.5, 0.0, 0.0 );    					
		glRotatef( 360.0*HourOfDay/24.0, 0.0, 1.0, 0.0 );
		glColor3f( 0.0, 1.0, 0.0 );
		if (UseTeapots){
			glutSolidTeapot(0.17);				
		}else{
			glutSolidSphere( 0.17, 15, 15);
		}
    glPopMatrix();

	//The Earth and Moon
	glPushMatrix();
		//The Earth
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_earth_specular);
		glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
		glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);
		glRotatef( 360.0*DayOfYear/365.0, 0.0, 1.0, 0.0 );
		glTranslatef( 6.0, 0.0, 0.0 );    						
		glRotatef( 360.0*HourOfDay/24.0, 0.0, 1.0, 0.0 );
		glColor3f( 0.2, 0.2, 1.0 );
		if (UseTeapots){
			glutSolidTeapot(0.2);				
		}else{
			glutSolidSphere( 0.2, 15, 15);
		}
    						
		//The Moon
   		glRotatef( 360.0*12.0*DayOfYear/365.0, 0.0, 1.0, 0.0 );
		glTranslatef( 0.4, 0.0, 0.0 );
		glColor3f( 0.6, 0.6, 0.6 );
		if (UseTeapots){
			glutSolidTeapot(0.05);		
		}else{
			glutSolidSphere( .05, 10, 5 );
		}
	glPopMatrix();

	//A comet
	glRotatef(-30.0, 2.0, 0.0, 0.0);
	glPushMatrix();			 		
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
		glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
		glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);
		glRotatef( 360.0*DayOfCometYear/500.0, 0.0, 1.0, 0.0 );
		glTranslatef( 6.0, 3.0, -9.0 );
		glColor3f( 0.55, 0.47, 0.40 );		
		//glutSolidSphere( 0.13, 15, 15);
		glScalef(0.1, 0.1, 0.1);
		glutSolidDodecahedron();
		glScalef(10.0, 10.0, 10.0);		
		//Comet tail
		glEnable (GL_FOG); //enable the fog
		glFogi (GL_FOG_MODE, GL_EXP2); 
		glFogfv (GL_FOG_COLOR, fogColor); 
		glFogf (GL_FOG_DENSITY, density);
		glTranslatef( 0.15, 0.0, 0.1 );
		//glutSolidCone(0.11, 1.0, 5, 5);
		glRotatef(-35.0, 0.0, 0.0, 0.0);
		glBegin(GL_POINTS);
			GLfloat v1, v2, v3;
			for (int i=0; i<=150; i++){
				v1 = (float)(((rand() % 10)*i)/pow(i, 2));
				v2 = (float)(((rand() % 10)*i)/pow(i, 2));
				v3 = -0.01;	
				if (v1 < 0.5){
					v2 /= 3;
					if (i%2 == 0){
					glVertex3f(v1, v2, v3);
				}else{
					glVertex3f(v1, -v2, v3);
				}
				}else if(v1 >= 0.5 && v1 < 1.0){
					v2 /= 3;
					if (i%2 == 0){
						glVertex3f(v1, v2, v3);
					}else{
						glVertex3f(v1, -v2, v3);
					}
			}else if(v1 >= 1.0 && v1 < 1.5){
				v2 /= 2;
					if (i%2 == 0){
						glVertex3f(v1, v2, v3);
					}else{
						glVertex3f(v1, -v2, v3);
					}
				}else if(v1 >= 1.5 && v1 < 2.0){
				v2 /= 1;
					if (i%2 == 0){
						glVertex3f(v1, v2, v3);
					}else{
						glVertex3f(v1, -v2, v3);
					}
				}				
		}
		glEnd();
		glDisable(GL_FOG);
    glPopMatrix();

    glFlush();
    glutSwapBuffers();
	if ( singleStep ) {
		spinMode = GL_FALSE;
	}
	glutPostRedisplay(); //Re-display the scene
}

void OpenGLInit(void)
{
    glShadeModel( GL_FLAT );
    glClearColor( 0.0, 0.0, 0.0, 0.0 );
    glClearDepth( 1.0 );
    glEnable( GL_DEPTH_TEST );
}

//Resets the global scene variables
void ResetScene(){
		UseTeapots = false;
		LookFromBottom = false;
		LookFromTop = false;
		LookOrigin = false;
		angle = 15.0f;
		scale = 0.0;
		AnimateIncrement = 24.0f;
}

//Sub-menu options
void subMenu(int value){
	switch(value) {
	case 7:
		//Increase rotation angle
		IncreaseAngle();
		fprintf(stdout, "Increased angle by 90 degrees\n");
		break;

	case 8:
		//Decrease rotation angle
		DecreaseAngle();
		fprintf(stdout, "Decreased angle by 90 degrees\n");
		break;

	case 9:
		ZoomIn();
		fprintf(stdout, "Zoomed in by 2\n");
		break;

	case 10: 
		ZoomOut();
		fprintf(stdout, "Zoomed out by 2\n");
		break;
	}
}

//User menu actions
void userMenu(int value) {

    switch(value) {

	case 1:
		//View of the top
		LookOrigin = false;
		LookFromBottom = false;
		LookFromTop = true;
		fprintf(stdout, "View of the top\n");
		break;

	case 2:
		//View of the bottom
		LookFromTop = false;
		LookOrigin = false;
		LookFromBottom = true;		
		fprintf(stdout, "View of the bottom\n");
		break;

	case 3:
		//View from the origin
		LookFromBottom = false;
		LookFromTop = false;
		LookOrigin = true;
		fprintf(stdout, "View from the origin\n");
		break;
    
	case 4:
		//Speed up animation
		SpeedUp();
		fprintf(stdout, "Sped up by a factor of 16\n");
		break;

	case 5:
		//Slow down animation
		SlowDown();
		fprintf(stdout, "Slowed down by a factor of 16\n");
		break;

	case 6:
		//Use Teapots instead of spheres
		UseTeapots = true;
		fprintf(stdout, "Using Teapots instead of spheres\n");
		break;

	case 11:
		//Reset scene variables
		ResetScene();
		fprintf(stdout, "Scene has been reset\n");
		break;

    default:
        fprintf(stderr, "Unknown Menu Option %d\n", value);
        break;
    }
}

//The main display method
void displaySolarSystem(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	if (DrawStars){
		RenderStars();
	}
	Animate();
}

static void ResizeWindow(int w, int h)
{
    float aspectRatio;
	h = (h == 0) ? 1 : h;
	w = (w == 0) ? 1 : w;
	glViewport( 0, 0, w, h );	
	aspectRatio = (float)w/(float)h;

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 60.0, aspectRatio, 1.0, 30.0 );

    glMatrixMode( GL_MODELVIEW );
}

// Main method - program entry point
int main( int argc, char** argv )
{
	glutInit(&argc,argv);
	 glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	 glutInitWindowSize(400, 400);
	 glutInitWindowPosition (750, 300);
	 instructionMenu = glutCreateWindow ("Instructions");
	 glutSetWindow(instructionMenu);
    glutSetCursor(GLUT_CURSOR_HELP);
    // Initialize Instructions
    initInstructions ( );
    glutDisplayFunc (instructionWindow);
	glutReshapeFunc (winReshapeFcn);

	//The main (solar system) window
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );	
    glutInitWindowPosition( 0, 0 );
    glutInitWindowSize( 800, 400 );
    glutCreateWindow( "CMSC 405 Final Project - Solar System by Michael Hawkins" );
    OpenGLInit();
    glutReshapeFunc( ResizeWindow );	
	glutDisplayFunc( displaySolarSystem );

	// Set up lighting, shading, etc.
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);								
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	
	glEnable(GL_NORMALIZE);
	glCullFace(GL_BACK);	
	glEnable(GL_CULL_FACE);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);			  	
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	//Position based light source
	glLightfv(GL_LIGHT0,GL_POSITION, LIGHT_ORIGIN_POSITION);	
	//The light's quadratic attenuation factor
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, AttenuationFactor);

	GenerateStars(500); //Generate the star field
	
	submenu_id = glutCreateMenu(subMenu);
    glutAddMenuEntry("Zoom In", 9);
	glutAddMenuEntry("Zoom Out", 10);
	glutAddMenuEntry("Increase Angle", 7);
	glutAddMenuEntry("Decrease Angle", 8);

	//Set up menu - accessible on either right or left mouse click
    glutCreateMenu(userMenu);	
    glutAddMenuEntry("Show Top", 1);
    glutAddMenuEntry("Show Bottom", 2);    
	glutAddMenuEntry("Show From Origin", 3);
	glutAddSubMenu("Transformations", submenu_id);
    glutAddMenuEntry("Speed Up", 4);
    glutAddMenuEntry("Slow Down", 5);
	glutAddMenuEntry("Use Teapots", 6);
	glutAddMenuEntry("Reset Scene", 11);
	glutAttachMenu(GLUT_LEFT_BUTTON);	
	glutAttachMenu(GLUT_RIGHT_BUTTON);	

	glutMainLoop(  );
	return EXIT_SUCCESS;
}



