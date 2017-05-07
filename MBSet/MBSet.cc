// Calculate and display the Mandelbrot set
//Saketh Poda ECE 4122
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "complex.h"

using namespace std;

// Min and max complex plane values
Complex  minC(-2.0, -1.2);
Complex  maxC( 1.0, 1.8);
int      maxIt = 2000;     // Max iterations for the set computations
#define WS 512
pthread_mutex_t endMutex;
pthread_mutex_t startMutex;
pthread_cond_t cond;
pthread_barrier_t barrier;
Complex* cArray = new Complex[WS*WS];
int nThreads = 16;
int cPixel[WS * WS];
bool mouseRect;
int mot;
//Struct for Mouse Function
struct Click 
{
float x, y;
Click()
{
x = 0.0;
y = 0.0;
}

};

Click L, R;


//create class and functions for colors 
class RGB
{
public:
RGB() {
r = 0;
g = 0;
b = 0;
}
RGB(double red, double green, double blue) {
r = red;
g = green;
b = blue;
}
public:
double r,g,b;
};


RGB* ColorArray = 0;


void CreateColors()
{
ColorArray = new RGB[maxIt + 1];
for (int i = 0; i < maxIt; ++i) {
if (i < 5) {
   ColorArray[i] = RGB(1.0,1.0,1.0);
}
else {
ColorArray[i] = RGB(drand48(), drand48(), drand48());
}


}
ColorArray[maxIt]= RGB();
}




Complex CreateComplex(int x, int y) {
    double realC = maxC.real-minC.real;
    double imagC = maxC.imag-minC.imag;
    double  numReal = (double) x/WS;
    double numImag = (double) y/WS;
    return minC + Complex(numReal*realC, numImag*imagC); 
}


void CreateComplexArray() {
   for (int i = 0; i < WS; i++) {
       for (int j = 0; j < WS; j++) {
           cArray[i*WS +j] = CreateComplex(i,j);         
       }
   } 
}

//Thread Related Functions

void BarrierInit(pthread_barrier_t* b, unsigned numThreads) {
pthread_barrier_init(b, NULL, numThreads);
}

void BarrierFunc(pthread_barrier_t* b) {
pthread_barrier_wait(b);
}


void* MBThread(void* num) {
unsigned long threadNum = (unsigned long) num;
unsigned long count = 0;
int rowsPerThread = WS/nThreads;
int startingRow = threadNum * rowsPerThread;
for (int i = 0; i < rowsPerThread; i++) {
    int row = startingRow + i;
    for (int j = 0; j < WS; j++) {
        Complex temp = cArray[row*WS + j];
        cPixel[row*WS + j] = 0;
        count++;
        while (cPixel[row*WS+j] < maxIt && temp.Mag2() < 4.0) {
            cPixel[row*WS +j]++;
            temp = (temp*temp) + cArray[row*WS + j];
        }       
    }
}

}


void CreateThread() {
   // pthread_mutex_init(&endMutex, 0);
   // pthread_mutex_init(&startMutex, 0);
   // pthread_cond_init(&cond, 0);
   // pthread_mutex_init(&endMutex,0);
    BarrierInit(&barrier, nThreads);
    for (int i = 0; i < nThreads; ++i) {
        pthread_t thread;
        pthread_create(&thread, 0, MBThread, (void*) i);
    }
   //pthread_cond_wait(&cond, &endMutex);
}

void DrawMBSet() {

CreateComplexArray(); 
CreateThread();
}

void ShowMB() {

glBegin(GL_POINTS);
for(int i = 0; i < WS; i++) {
   for (int j = 0; j < WS; j++) {
       glColor3f(ColorArray[cPixel[i*WS+j]].r, ColorArray[cPixel[i*WS+j]].g, ColorArray[cPixel[i*WS+j]].b);
glVertex2d(i,j); 
   }


}
glEnd();
}


void DrawSquare() {
glColor3f(1,0,0);
glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
glBegin(GL_POLYGON);
glVertex2f(L.x, L.y);
glVertex2f(R.x, L.y);
glVertex2f(R.x, R.y);
glVertex2f(L.x, R.y);
glEnd();
}

void display(void)
{ // Your OpenGL display code here
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
glClearColor(1.0,1.0,1.0, 1.0);
glLoadIdentity();
ShowMB();
if (mouseRect) {
DrawSquare();
}
glutSwapBuffers();
}

void init()
{ // Your OpenGL initialization code here
CreateColors();
glViewport(0,0,WS,WS);
glMatrixMode(GL_PROJECTION);
glLoadIdentity();
gluOrtho2D(0, WS, WS, 0);
glMatrixMode(GL_MODELVIEW);
glLoadIdentity();
}
/*
void reshape(int w, int h)
{ // Your OpenGL window reshape code here
glViewport(0,0,(GLsizei)w, (GLsizei)h);
}
*/
void mouse(int button, int state, int x, int y)
{ // Your mouse click processing here
  // state == 0 means pressed, state != 0 means released
  // Note that the x and y coordinates passed in are in
  // PIXELS, with y = 0 at the top.
   /*
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
      L.x = x;
      L.y = y;
      mouseRect = true;
   }
   if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
       double xvar = abs(x - L.x);
       double yvar = abs(y - L.y);
       double diff = xvar > yvar ? yvar: xvar;
       
       R.x = x > L.x ? L.x + diff : L.x - diff;
       R.x = y > L.y ? L.y + diff : L.y - diff;
       
       int temp = min(L.x, R.x);
       int temp2 = min(L.y, R.y);
       minC.real = cArray[temp*WS+temp2].real;
       minC.imag = cArray[temp*WS+temp2].imag;
       
       int temp3 = max(L.x, R.x);
       int temp4 = max(L.y, R.y);
       maxC.real = cArray[temp3*WS + temp4].real;
       maxC.imag = cArray[temp4*WS + temp4].imag;
       mouseRect = false;
       DrawMBSet();
       glutPostRedisplay();
   } 
 */
 
   if (button == GLUT_LEFT_BUTTON && state == 0) {
       L.x = x;
       R.x = x;
       L.y = y;
       R.y = y;
       mouseRect = true;
   }
   if (button == GLUT_LEFT_BUTTON && state != 0) {
       if (x > L.x && y > L.y) {
           R.x = L.x + mot; 
           R.y = L.y + mot;
       
          for (int i = 0; i < WS; i++) {
            for (int j = 0; j < WS; j++) {
                if ( i == L.x && j == L.y) {
                   minC.real = cArray[i*WS+j].real;
                   minC.imag = cArray[i*WS+j].imag;
               }
               if (i == R.x && j == R.y) {
                   maxC.real = cArray[i*WS+j].real;
                   maxC.imag = cArray[i*WS+j].imag;
               } 
            }
         }
       }

if (x < L.x && y < L.y) {
           R.x = L.x - mot; 
           R.y = L.y - mot;
       
          for (int i = 0; i < WS; i++) {
            for (int j = 0; j < WS; j++) {
                if ( i == R.x && j == R.y) {
                   minC.real = cArray[i*WS+j].real;
                   minC.imag = cArray[i*WS+j].imag;
               }
               if (i == L.x && j == L.y) {
                   maxC.real = cArray[i*WS+j].real;
                   maxC.imag = cArray[i*WS+j].imag;
               } 
            }
         }
       }

if (x < L.x && y > L.y) {
           R.x = L.x - mot; 
           R.y = L.y + mot;
       
          for (int i = 0; i < WS; i++) {
            for (int j = 0; j < WS; j++) {
                if ( i == R.x && j == L.y) {
                   minC.real = cArray[i*WS+j].real;
                   minC.imag = cArray[i*WS+j].imag;
               }
               if (i == L.x && j == R.y) {
                   maxC.real = cArray[i*WS+j].real;
                   maxC.imag = cArray[i*WS+j].imag;
               } 
            }
         }
       }

if (x > L.x && y < L.y) {
           R.x = L.x + mot; 
           R.y = L.y - mot;
       
          for (int i = 0; i < WS; i++) {
            for (int j = 0; j < WS; j++) {
                if ( i == L.x && j == R.y) {
                   minC.real = cArray[i*WS+j].real;
                   minC.imag = cArray[i*WS+j].imag;
               }
               if (i == R.x && j == L.y) {
                   maxC.real = cArray[i*WS+j].real;
                   maxC.imag = cArray[i*WS+j].imag;
               } 
            }
         }
       }
mouseRect = false;
DrawMBSet();
glutPostRedisplay();
    } 
}

void motion(int x, int y)
{ // Your mouse motion here, x and y coordinates are as above
int xv = abs(x-L.x);
int yv = abs(y-L.y);
//int mot; 
if (x > L.x && y > L.y) {
    if (xv > yv) {
        mot = yv;       
     }
     if (xv < yv) {
       mot = xv;
      }
R.x = L.x + mot;
R.y = L.y + mot;
}

if (x < L.x && y < L.y) {
    if (xv > yv) {
        mot = yv;       
     }
     if (xv < yv) {
       mot = xv;
      }
R.x = L.x - mot;
R.y = L.y - mot;
}

if (x > L.x && y < L.y) {
    if (xv > yv) {
        mot = yv;       
     }
     if (xv < yv) {
       mot = xv;
      }
R.x = L.x + mot;
R.y = L.y - mot;
}

if (x < L.x && y > L.y) {
    if (xv > yv) {
        mot = yv;       
     }
     if (xv < yv) {
       mot = xv;
      }
R.x = L.x - mot;
R.y = L.y + mot;
}
glutPostRedisplay();
}

void keyboard(unsigned char c, int x, int y)
{ // Your keyboard processing here
}

int main(int argc, char** argv)
{
  // Initialize OpenGL, but only on the "master" thread or process.
  // See the assignment writeup to determine which is "master" 
  // and which is slave.
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(WS, WS);
  glutInitWindowPosition(100,100);
  glutCreateWindow("MBSet");
  DrawMBSet(); 
  init();
  glutDisplayFunc(display);
  glutIdleFunc(display);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutMainLoop();
  return 0;
}

