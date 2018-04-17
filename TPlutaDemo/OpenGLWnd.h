#pragma once
#include <QtWidgets>
//#include <qopenglwidget.h>
//#include <QOpenGLExtraFunctions>
//1#include <QMatrix4x4>
//#include <qvector4d.h>
//#include <qvector3d.h>
#define MAX_LIGHTS 8

enum Uniform1{
	View = 0,
	Model = 1,
	Perspective = 2,
	MVP = 3,
	Camera = 4
};

enum Uniform2{
	Mask = 0,
	Light1 = 1,
	Light2 = 2,
	Light3 = 3,
	Light4 = 4,
	Light5 = 5,
	Light6 = 6,
	Light7 = 7,
	Light8 = 8
};

class OpenGLWnd : public QOpenGLWidget, public QOpenGLExtraFunctions {
	Q_OBJECT
public:
	OpenGLWnd(QWidget *widget = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags()){};		//Default QOpenGLWidget constructor, will call InitializeGL
	~OpenGLWnd(){};																			//Destructor contains cleaning functions 

protected:
	/*DEFAULT QOPENGLWIDGETS METHODS*/
	void initializeGL();									// Initialize OpenGL, all starts here
	void paintGL();											// Paint objects on screen, QT calls this every time when the app sends repaint message
	void resizeGL(int width, int height);					// Resize window, Qt calls it when the window is resized

	/*INPUT DEVICES EVENTS*/
	void mousePressEvent(QMouseEvent *event);				// Mouse button press event
	void mouseReleaseEvent(QMouseEvent *event);				// Mouse button release event
	void mouseMoveEvent(QMouseEvent *event);				// Mouse movement event
	void keyPressEvent(QKeyEvent *event);					// Keyboard key press event
	void keyReleaseEvent(QKeyEvent *event);					// Keyboard key release event

	/*SHADERS LOADING METHODS*/
	void loadShaders();

	/*LIGHTS CONFIGURATION METHODS*/
	void initLights();
	void setLightAmbient(GLubyte lightID, QVector4D ambient);
	void setLightGlobal(GLubyte lightID, QVector4D global);
	void setLightPosition(GLubyte lightID, QVector4D position);
	void setLightAttenuation(GLubyte lightID, QVector3D attenaution);
	void toggleLight(GLubyte lightID, bool on);

	/*CAMERA CONFIGURATION METHODS*/
	void setModelMatrix();
	void setViewMatrix();
	void setPerspectiveMatrix();
	void setMVPMatrix();

	/*ERROR CONTROL*/
	void checkGLErrors(QString msg);
	void checkCudaErrors(QString msg);

protected:
	/*LIGHTS CONFIGURATION VARIABLES*/
	struct Light{
		QVector4D Ambient;
		QVector4D Global;
		QVector4D Position;
		QVector4D Attenaution;
	}Lights[MAX_LIGHTS];
	GLubyte LightsMask;

	/*CAMERA CONFIGURATION VARIABLES*/
	QMatrix4x4 ViewMatrix;
	QMatrix4x4 ModelMatrix;
	QMatrix4x4 PerspectiveMatrix;
	QMatrix4x4 MVPMatrix;
	QVector4D CameraPosition;
	
	/*SHADERS VARIABLES*/
	int LightsUniformLocations[9];
	int TransformUniformLocations[5];
	QOpenGLShaderProgram CustomColorInterpolationProgram;

};
