#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <math.h>
#include <cstdio>

#define DEFAULT_GLOBAL_ANGLE_STEP 16
#define DEFAULT_GLOBAL_TRANSLATE_STEP 1
#define DEFAULT_GLOBAL_SCALE_STEP 1

/*rotation*/
float globalAngleStep = DEFAULT_GLOBAL_ANGLE_STEP;
float globalAngle = 0;
float globalRotateX = 0;
float globalRotateY = 0;
float globalRotateZ = 0;

/*translation*/
float globalTranslateStep = DEFAULT_GLOBAL_TRANSLATE_STEP;
float globalTranslateX = 0;
float globalTranslateY = 0;
float globalTranslateZ = 0;

/*scaling*/
float globalScaleStep = DEFAULT_GLOBAL_SCALE_STEP;
float globalScaleX = 1;
float globalScaleY = 1;
float globalScaleZ = 1;

/*coloring*/
float globalBackgroundColorStep = 1;
float globalBackgroundColorR = 0;
float globalBackgroundColorG = 0;
float globalBackgroundColorB = 0;

/*camera*/
#define DEFAULT_GLOBAL_EYE_STEP 1.0f
#define DEFAULT_GLOBAL_EYE_X 5.0f
#define DEFAULT_GLOBAL_EYE_Y 5.0f
#define DEFAULT_GLOBAL_EYE_Z 0.0f

#define DEFAULT_GLOBAL_CENTER_STEP 1.0f
#define DEFAULT_GLOBAL_CENTER_X 0.0f
#define DEFAULT_GLOBAL_CENTER_Y 0.0f
#define DEFAULT_GLOBAL_CENTER_Z 0.0f

#define DEFAULT_GLOBAL_UP_STEP 1.0f
#define DEFAULT_GLOBAL_UP_X 0.0f
#define DEFAULT_GLOBAL_UP_Y 1.0f
#define DEFAULT_GLOBAL_UP_Z 0.0f

float globalEyeStep = DEFAULT_GLOBAL_EYE_STEP;
float globalEyeX = DEFAULT_GLOBAL_EYE_X;
float globalEyeY = DEFAULT_GLOBAL_EYE_Y;
float globalEyeZ = DEFAULT_GLOBAL_EYE_Z;

float globalCenterStep = DEFAULT_GLOBAL_CENTER_STEP;
float globalCenterX = DEFAULT_GLOBAL_CENTER_X;
float globalCenterY = DEFAULT_GLOBAL_CENTER_Y;
float globalCenterZ = DEFAULT_GLOBAL_CENTER_Z;

float globalUpStep = DEFAULT_GLOBAL_UP_STEP;
float globalUpX = DEFAULT_GLOBAL_UP_X;
float globalUpY = DEFAULT_GLOBAL_UP_Y;
float globalUpZ = DEFAULT_GLOBAL_UP_Z;

#define DEFAULT_GLOBAL_STACK 10
#define DEFAULT_GLOBAL_SLICE 10

int globalStackSliceStep = 1;
int globalStack = DEFAULT_GLOBAL_STACK;
int globalSlice = DEFAULT_GLOBAL_SLICE;


void changeSize(int w, int h) {

	// Prevent a divide by zero, when window is too short
	// (you can't make a window with zero width).
	if(h == 0)
		h = 1;

	// compute window's aspect ratio 
	float ratio = (float)w * 1.0f / (float)h;

	// Set the projection matrix as current
	glMatrixMode(GL_PROJECTION);
	// Load Identity Matrix
	glLoadIdentity();
	
	// Set the viewport to be the entire window
    glViewport(0, 0, w, h);

	// Set perspective
	gluPerspective(45.0f ,ratio, 1.0f ,1000.0f);

	// return to the model view matrix mode
	glMatrixMode(GL_MODELVIEW);
}


void renderScene(void) {

	// clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set the camera
	glLoadIdentity(); /*helps in avoiding rounding mistakes, discards old matrix, read more*/
	gluLookAt(globalEyeX,globalEyeY,globalEyeZ,
              globalCenterX,globalCenterY,globalCenterZ,
			  globalUpX,globalUpY,globalUpZ);

    /*draw absolute (before any transformation) axes*/
        glBegin(GL_LINES);
        /*X-axis in red*/
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(
                -100.0f, 0.0f, 0.0f);
        glVertex3f( 100.0f, 0.0f, 0.0f);

        /*Y-Axis in Green*/
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f,
                   -100.0f, 0.0f);
        glVertex3f(0.0f, 100.0f, 0.0f);

        /*Z-Axis in Blue*/
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f,
                   -100.0f);
        glVertex3f(0.0f, 0.0f, 100.0f);
        glEnd();
    /*end of draw absolute (before any transformation) axes*/

    // put the geometric transformations here
    glRotatef(globalAngle,globalRotateX,globalRotateY,globalRotateZ);
    glTranslatef(globalTranslateX,globalTranslateY,globalTranslateZ);
    glScalef(globalScaleX,globalScaleY,globalScaleZ);

    // put drawing instructions here

    /*draw sphere using function*/
    GLUquadricObj *qobj = nullptr;
    qobj = gluNewQuadric();
    gluSphere(qobj,2,globalSlice,globalStack);

    /*draw sphere manually*/
/*    float c = M_PI/180.0f;
    for(float phi=-80.0; phi<=80.0; phi+=20.0)
    {
        float phir=c*phi;
        float phir20=c*(phi+20);
        glBegin(GL_QUAD_STRIP);
        for(float theta=-180.0; theta<=180.0;theta+=20.0)
        {
            float thetar=c*theta;
            float x=sin(thetar)*cos(phir);
            float y=cos(thetar)*cos(phir);
            float z=sin(phir);
            glVertex3d(x,y,z);
            x=sin(thetar)*cos(phir20);
            y=cos(thetar)*cos(phir20);
            z=sin(phir20);
            glVertex3d(x,y,z);
        }
        glEnd();
    }

    glBegin(GL_TRIANGLE_FAN);
        glVertex3d(0.0, 0.0 , 1.0);
        c=M_PI/180.0;
        float c80=c*80.0;
        float z=sin(c80);
        for(float theta=-180.0; theta<=180.0;theta+=20.0)
        {
        float thetar=c*theta;
        float x=sin(thetar)*cos(c80);
        float y=cos(thetar)*cos(c80);
        glVertex3d(x,y,z);
        }
        glEnd();
        glBegin(GL_TRIANGLE_FAN);
        glVertex3d(0.0, 0.0, -1.0);
        z=-sin(c80);
        for(float theta=-180.0;theta<=180.0;theta+=20.0)
        {
        float thetar=c*theta;
        float x=sin(thetar)*cos(c80);
        float y=cos(thetar)*cos(c80);
        glVertex3d(x,y,z);
        }
    glEnd();*/

    /*draw pyramid*/
        glBegin(GL_TRIANGLES);
        /*Front red*/
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(1.0f, 0.0f, -1.0f);
        glVertex3f(0.0f, 1.0f, 0.0f);
        glVertex3f(1.0f, 0.0f, 1.0f);

        /*Right green*/
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(1.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-1.0f, 0.0f, 1.0f);

        /*Left blue*/
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(-1.0f, 0.0f, -1.0f);
        glVertex3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-1.0f, 0.0f, 1.0f);

        /*Back magenta*/
        glColor3f(1.0f, 0.0f, 1.0f);
        glVertex3f(1.0f, 0.0f, -1.0f);
        glVertex3f(-1.0f, 0.0f, -1.0f);
        glVertex3f(0.0f, 1.0f, 0.0f);

        /*Bases*/

        /*Cyan*/
        glColor3f(0.0f, 1.0f, 1.0f);
        glVertex3f(-1.0f, 0.0f, 1.0f);
        glVertex3f(-1.0f, 0.0f, -1.0f);
        glVertex3f(1.0f, 0.0f, -1.0f);

        /*Yellow*/
        glColor3f(1.0f, 1.0f, 0.0f);
        glVertex3f(-1.0f, 0.0f, 1.0f);
        glVertex3f(1.0f, 0.0f, -1.0f);
        glVertex3f(1.0f, 0.0f, 1.0f);
        glEnd();
    /*end of draw pyramid*/

	// End of frame
	glutSwapBuffers();
}

// write function to process keyboard events
void keyboardFunc (unsigned char key, int xmouse, int ymouse)
{
/*
    Rotation:
        x y z       x-- y-- z--
        X Y Z       x++ y++ z++
    angle:
        , .     θ-=s    θ+=s
        < >     s--     s++
    translation:
        q w e       y-- z++ y++
        a s d       x-- z-- x++
    scaling:
        u i o       y-- z++ y++
        j k l       x-- z-- x++
    coloring: r g b R G B
    camera:
        1 2 3       ! @ #       EyeX     EyeY     EyeZ
        4 5 6       $ % ^       CenterX  CenterY  CenterZ
        7 8 9       & * (       UpX      UpY      UpZ
 */
    switch (key){
        case 'x':
            globalRotateX -= 1;
            break;

        case 'X':
            globalRotateX += 1;
            break;

        case 'y':
            globalRotateY -= 1;
            break;

        case 'Y':
            globalRotateY += 1;
            break;

        case 'z':
            globalRotateZ -= 1;
            break;

        case 'Z':
            globalRotateZ += 1;
            break;

        case ',':
            globalAngle -= globalAngleStep;
            break;

        case '.':
            globalAngle += globalAngleStep;
            break;

        case '<':
            globalAngleStep /= 2;
            break;

        case '>':
            globalAngleStep *= 2;
            break;

            /*translation*/

            /*z-axis*/
        case 'w':
            globalTranslateZ += globalTranslateStep;
            break;
        case 's':
            globalTranslateZ -= globalTranslateStep;
            break;

            /*x-axis*/
        case 'a':
            globalTranslateX -= globalTranslateStep;
            break;
        case 'd':
            globalTranslateX += globalTranslateStep;
            break;

            /*y-axis*/
        case 'q':
            globalTranslateY -= globalTranslateStep;
            break;
        case 'e':
            globalTranslateY += globalTranslateStep;
            break;
            /*end of translation*/

            /*scaling*/

            /*z-axis*/
        case 'i':
            globalScaleZ += globalScaleStep;
            break;
        case 'k':
            globalScaleZ -= globalScaleStep;
            break;

            /*x-axis*/
        case 'j':
            globalScaleX -= globalScaleStep;
            break;
        case 'l':
            globalScaleX += globalScaleStep;
            break;

            /*y-axis*/
        case 'u':
            globalScaleY -= globalScaleStep;
            break;
        case 'o':
            globalScaleY += globalScaleStep;
            break;
            /*end of scaling*/

        /*coloring*/
        case 'r':
            globalBackgroundColorR -= globalBackgroundColorStep;
            break;
        case 'R':
            globalBackgroundColorR += globalBackgroundColorStep;
            break;
        case 'g':
            globalBackgroundColorG -= globalBackgroundColorStep;
            break;
        case 'G':
            globalBackgroundColorG += globalBackgroundColorStep;
            break;
        case 'b':
            globalBackgroundColorB -= globalBackgroundColorStep;
            break;
        case 'B':
            globalBackgroundColorB += globalBackgroundColorStep;
            break;

        /*camera*/
        case '1':
            globalEyeX -= globalEyeStep;
            break;
        case '!':
            globalEyeX += globalEyeStep;
            break;
        case '2':
            globalEyeY -= globalEyeStep;
            break;
        case '@':
            globalEyeY += globalEyeStep;
            break;
        case '3':
            globalEyeZ -= globalEyeStep;
            break;
        case '#':
            globalEyeZ += globalEyeStep;
            break;

        case '4':
            globalCenterX -= globalCenterStep;
            break;
        case '$':
            globalCenterX += globalCenterStep;
            break;
        case '5':
            globalCenterY -= globalCenterStep;
            break;
        case '%':
            globalCenterY += globalCenterStep;
            break;
        case '6':
            globalCenterZ -= globalCenterStep;
            break;
        case '^':
            globalCenterZ += globalCenterStep;
            break;
        case '7':
            globalUpX -= globalUpStep;
            break;
        case '&':
            globalUpX += globalUpStep;
            break;
        case '8':
            globalUpY -= globalUpStep;
            break;
        case '*':
            globalUpY += globalUpStep;
            break;
        case '9':
            globalUpZ-= globalUpStep;
            break;
        case '(':
            globalUpZ += globalUpStep;
            break;
        case '[':
            globalStack += globalStackSliceStep;
            break;
        case '{':
            globalStack -= globalStackSliceStep;
            break;
        case ']':
            globalSlice += globalStackSliceStep;
            break;
        case '}':
            globalSlice -= globalStackSliceStep;
            break;

        /*reset environment*/
        case '0':
            /*Reset Rotation*/
            globalAngle = 0;
            globalRotateX = 0;
            globalRotateY = 0;
            globalRotateZ = 0;
            globalAngleStep = DEFAULT_GLOBAL_ANGLE_STEP;

            /*Reset Translation*/
            globalTranslateStep = DEFAULT_GLOBAL_TRANSLATE_STEP;
            globalTranslateX = 0;
            globalTranslateY = 0;
            globalTranslateZ = 0;

            /*Reset Scale*/
            globalScaleStep = DEFAULT_GLOBAL_SCALE_STEP;
            globalScaleX = 1;
            globalScaleY = 1;
            globalScaleZ = 1;

            /*Reset Background Color*/
            globalBackgroundColorR = 0;
            globalBackgroundColorG = 0;
            globalBackgroundColorB = 0;

            /*Reset Camera*/
            globalEyeStep = DEFAULT_GLOBAL_EYE_STEP;
            globalEyeX = DEFAULT_GLOBAL_EYE_X;
            globalEyeY = DEFAULT_GLOBAL_EYE_Y;
            globalEyeZ = DEFAULT_GLOBAL_EYE_Z;

            globalCenterStep = DEFAULT_GLOBAL_CENTER_STEP;
            globalCenterX = DEFAULT_GLOBAL_CENTER_X;
            globalCenterY = DEFAULT_GLOBAL_CENTER_Y;
            globalCenterZ = DEFAULT_GLOBAL_CENTER_Z;

            globalUpStep = DEFAULT_GLOBAL_UP_STEP;
            globalUpX = DEFAULT_GLOBAL_UP_X;
            globalUpY = DEFAULT_GLOBAL_UP_Y;
            globalUpZ = DEFAULT_GLOBAL_UP_Z;

            globalStack = DEFAULT_GLOBAL_STACK;
            globalSlice = DEFAULT_GLOBAL_SLICE;
            break;

        default:
            break;
    }
    glClearColor(globalBackgroundColorR,globalBackgroundColorG,globalBackgroundColorB,1.0); //update background color
    glutPostRedisplay(); //request display() call ASAP
}


int main(int argc, char **argv) {

// init GLUT and the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(800,800);
	glutCreateWindow("CG@DI-UM");
		
// Required callback registry 
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);
    glutKeyboardFunc(keyboardFunc);

	
// put here the registration of the keyboard callbacks



//  OpenGL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);

// enter GLUT's main cycle
	glutMainLoop();
	
	return 1;
}
