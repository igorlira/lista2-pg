#include "Main.h"
#include "util.h"
#include <math.h>
#include <Windows.h>
#include <math.h>

// OpenCV includes
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/calib3d/calib3d.hpp"

// GLM
#include "glm/glm.hpp"

#include <iostream>
#include <ctype.h>

using namespace cv;

#define PI 3.1415926535897932384626433832795

//inicializadores
GLfloat mouse_x, mouse_y;

bool buffer[250];

double g_Width, g_Height;

double mousex, mousey;
double deltax, deltay;

int captureSource = 0;
int newCaptureSource = captureSource;

int leftLowH = 172;
int leftHighH = 179;
int leftLowS = 126; 
int leftHighS = 255;
int leftLowV = 0;
int leftHighV = 255;
glm::vec2 leftNormal(1, 0);

int rightLowH = 21;
int rightHighH = 31;
int rightLowS = 126; 
int rightHighS = 255;
int rightLowV = 00;
int rightHighV = 255;
glm::vec2 rightNormal(-1, 0);

float barSizeX = 0.02;
float barSizeY = 0.3;

float leftX = 0.1;
float leftY = 0.5;

float rightX = 0.9;
float rightY = 0.5;

float ballRadius = 0.02;
float ballX = 0.5;
float ballY = 0.5;
glm::vec2 ballVector(1, 0.7);

bool gameStarted = false;

void FimDoPrograma()
{
	exit(1);
}

cv::VideoCapture* cap = 0;// cap(0);
Mat frame;

void changeCapture()
{
	if(cap)
	{
		cap->release();
	}

	if(captureSource == 0) // camera
	{
		cap = new VideoCapture(0);
	}
	else // video
	{
		cap = new VideoCapture();
		cap->open("Resources\\InputData\\video2.mp4");
	}

	if (!cap->isOpened())
	{
		cout << "error, could not open the capture" << endl;
		system("pause");
		exit(1);
	}
}
void initCV()
{
	changeCapture();

	namedWindow("video", WINDOW_AUTOSIZE);
	namedWindow("Control", WINDOW_AUTOSIZE);
	namedWindow("src", WINDOW_AUTOSIZE);

	createTrackbar("Fonte da captura", "Control", &newCaptureSource, 1);

	/*createTrackbar("LowH", "Control", &rightLowH, 179); //Hue (0 - 179)
	createTrackbar("HighH", "Control", &rightHighH, 179);

	createTrackbar("LowS", "Control", &rightLowS, 255); //Saturation (0 - 255)
	createTrackbar("HighS", "Control", &rightHighS, 255);

	createTrackbar("LowV", "Control", &rightLowV, 255);//Value (0 - 255)
	createTrackbar("HighV", "Control", &rightHighV, 255);*/
}

void initialize()
{
	initCV();

	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);							// Enables Depth Testing
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
}

void myreshape(GLsizei w, GLsizei h)
{
	g_Width = w;
	g_Height = h;

	glViewport(0, 0, g_Width, g_Height);
}

double getPosition(int barIndex)
{
	int iLowH, iLowS, iLowV;
	int iHighH, iHighS, iHighV;

	if(barIndex == 0)
	{
		iLowH = leftLowH;
		iLowS = leftLowS;
		iLowV = leftLowV;

		iHighH = leftHighH;
		iHighS = leftHighS;
		iHighV = leftHighV;
	}
	else
	{
		iLowH = rightLowH;
		iLowS = rightLowS;
		iLowV = rightLowV;

		iHighH = rightHighH;
		iHighS = rightHighS;
		iHighV = rightHighV;
	}

	Mat imgHSV;
	cvtColor(frame, imgHSV, CV_BGR2HSV);

	Mat imgThreshold;
	inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThreshold);

	//morphological opening (removes small objects from the foreground)
	erode(imgThreshold, imgThreshold, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
	dilate(imgThreshold, imgThreshold, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

	//morphological closing (removes small holes from the foreground)
	dilate(imgThreshold, imgThreshold, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 
	erode(imgThreshold, imgThreshold, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

	if(barIndex == 1)
	{
		imshow("src", imgThreshold);
	}

	Moments oMoments = moments(imgThreshold);
	
	double dM01 = oMoments.m01;
	double dM10 = oMoments.m10;
	double dArea = oMoments.m00;

	if (dArea > 10000)
	{
		//calculate the position of the ball
		int posX = dM10 / dArea;
		int posY = dM01 / dArea;

		rectangle(frame, Point(posX - 5, posY - 5), Point(posX + 5, posY + 5), Scalar(128, 118, 0));

		return (float)posY / (float)frame.rows;
	}
	else
	{
		return 0.5;
	}
}

void updateCV()
{
	*cap >> frame;

	leftY = getPosition(0);
	rightY = getPosition(1);

	if(!gameStarted && leftY != 0.5 && rightY != 0.5)
	{
		gameStarted = true;
	}

	// loopzinho
	if (cap->get(CV_CAP_PROP_POS_FRAMES) == 500)
	{
		cap->set(CV_CAP_PROP_POS_FRAMES, 10);
	}

	imshow("video", frame);
	//imshow("src", imgThreshold);
}

// aqui o sistema de coordenadas da tela está variando de -1 a 1 no eixo x e y
void mydisplay()
{
	// OpenCV Processing

	updateCV();

	// End of OpenCV Processing

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();
	glViewport(0, 0, g_Width, g_Height);
	glOrtho(0, 1, 1, 0, 0, 0.1);

	glMatrixMode(GL_MODELVIEW);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glBegin(GL_QUADS);

	glColor3d(1, 1, 1);
	glVertex2d(leftX - (barSizeX / 2), leftY - (barSizeY / 2));
	glVertex2d(leftX + (barSizeX / 2), leftY - (barSizeY / 2));
	glVertex2d(leftX + (barSizeX / 2), leftY + (barSizeY / 2));
	glVertex2d(leftX - (barSizeX / 2), leftY + (barSizeY / 2));

	glVertex2d(rightX - (barSizeX / 2), rightY - (barSizeY / 2));
	glVertex2d(rightX + (barSizeX / 2), rightY - (barSizeY / 2));
	glVertex2d(rightX + (barSizeX / 2), rightY + (barSizeY / 2));
	glVertex2d(rightX - (barSizeX / 2), rightY + (barSizeY / 2));

	glEnd();

	double radius = ballRadius; 
	// Draw circle
	glTranslated(ballX, ballY, 0);
	glBegin(GL_POLYGON);
	for(double i = 0; i < 2 * PI; i += PI / 100) //<-- Change this Value
 		glVertex3f(cos(i) * radius, sin(i) * radius, 0.0);
	glEnd();

	glColor3d(1, 0, 1);
	glBegin(GL_LINES);
	glVertex2f(0, 0);
	glVertex2f(ballVector[0] * 0.1, ballVector[1] * 0.1);
	glEnd();

	glFlush();
	glutPostRedisplay();
	glutSwapBuffers();
}

void handleKeyboardPressed(unsigned char key, int x, int y){
	buffer[(int) key] = true;
}

void handleKeyboardUp(unsigned char key, int x, int y){
	buffer[(int) key] = false;
}

bool isBallCollidingWithRight()
{
	return (ballX + ballRadius) <= (rightX + (barSizeX / 2.0))
		&& (ballX + ballRadius) >= (rightX - (barSizeX / 2.0))
		&& (ballY + ballRadius) >= (rightY - (barSizeY / 2.0))
		&& (ballY - ballRadius) <= (rightY + (barSizeY / 2.0));
}

bool isBallCollidingWithLeft()
{
	return (ballX + ballRadius) >= (leftX - (barSizeX / 2.0))
		&& (ballX - ballRadius) <= (leftX + (barSizeX / 2.0))
		&& (ballY + ballRadius) >= (leftY - (barSizeY / 2.0))
		&& (ballY - ballRadius) <= (leftY + (barSizeY / 2.0));
}

bool isBallCollidingWithTop()
{
	return (ballY - ballRadius) <= 0;
}

bool isBallCollidingWithBottom()
{
	return (ballY + ballRadius) >= 1;
}

bool isBallMoreThanHalfRighBar()
{
	return ballY > rightY;
}

bool isBallMoreThanHalfLeftBar()
{
	return ballY > leftY;
}

void idleFunction()
{
	// Atualizar fonte da captura
	if(newCaptureSource != captureSource)
	{
		captureSource = newCaptureSource;
		changeCapture();
	}

	// Atualizar estado do jogo
	if(gameStarted)
	{
		if(isBallCollidingWithRight())
		{
			ballVector = glm::reflect(ballVector, rightNormal);
		}
		else if(isBallCollidingWithLeft())
		{
			ballVector = glm::reflect(ballVector, leftNormal);
		}
		else if(isBallCollidingWithTop())
		{
			ballVector = glm::reflect(ballVector, glm::vec2(0, -1));
		}
		else if(isBallCollidingWithBottom())
		{
			ballVector = glm::reflect(ballVector, glm::vec2(0, 1));
		}

		glm::vec3 v1(ballVector[0], ballVector[1], 0);
		glm::vec3 v2(1, 0, 0);

		double cos = glm::dot(v1, v2) / (glm::length(v1) * glm::length(v2));
		double sin = sqrt(1 - pow(cos, 2));
		if(ballVector[1] < 0)
			sin = -sin;

		ballX += 0.02 * cos;
		ballY += 0.02 * sin;

		if(ballX - ballRadius > 1 || ballX + ballRadius < 0)
		{
			ballX = 0.5;
			ballY = 0.5;
			gameStarted = false;
		}
	}

	if (buffer[27] == true)//ESC
		FimDoPrograma();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(600, 600);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("OpenGL");
	glutDisplayFunc(mydisplay);
	glutReshapeFunc(myreshape);
	glutKeyboardUpFunc(handleKeyboardUp);
	glutKeyboardFunc(handleKeyboardPressed);
	glutIdleFunc(idleFunction);
	initialize();
	glutMainLoop();

	cap->release();
	return 0;
}




