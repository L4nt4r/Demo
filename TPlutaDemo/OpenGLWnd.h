#pragma once
#include "Shader+DataBridge.h"

#define PI		3.14159265358979323846f
#define MAX_LIGHTS 8

enum TransformationTypes{
	Model = 0,
	View = 1,
	Perspective = 2,
	MVP = 3,
	Camera = 4
};

enum LightsParams{
	Counter = 0,
	Mask = 1,
	Ambient = 2,
	Direct = 3,
	Position = 4,
	Attenaution = 5
	
};



class OpenGLWnd : public QOpenGLWidget, public QOpenGLExtraFunctions {
	Q_OBJECT
public:
	OpenGLWnd(QWidget *widget = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());		//Default QOpenGLWidget constructor, will call InitializeGL
	~OpenGLWnd();																			//Destructor contains cleaning functions 

protected:
	/*DEFAULT QOPENGLWIDGETS METHODS*/
	void initializeGL(void);									// Initialize OpenGL, all starts here
	void paintGL(void);											// Paint objects on screen, QT calls this every time when the app sends repaint message
	void resizeGL(int width, int height);					// Resize window, Qt calls it when the window is resized

	/*INPUT DEVICES EVENTS*/
	void mousePressEvent(QMouseEvent *event);				// Mouse button press event
	void mouseReleaseEvent(QMouseEvent *event);				// Mouse button release event
	void mouseMoveEvent(QMouseEvent *event);				// Mouse movement event
	void keyPressEvent(QKeyEvent *event);					// Keyboard key press event
	void keyReleaseEvent(QKeyEvent *event);					// Keyboard key release event

	/*SHADERS LOADING METHODS*/
	void loadShaders(void);
	

	/*LIGHTS CONFIGURATION METHODS*/
	void initLights(void);
	void setLightParam(GLubyte lightID, QVector4D LightParam, LightsParams paramType);
	void setLightGlobal(GLubyte lightID, QVector4D global);
	void setLightPosition(GLubyte lightID, QVector4D position);
	void setLightAttenuation(GLubyte lightID, QVector3D attenaution);
	void toggleLight(GLubyte lightID, bool on);

	/*CAMERA CONFIGURATION METHODS*/
	void setModelMatrix(QVector3D axis, float angle);
	void setViewMatrix(void);
	void setPerspectiveMatrix(void);
	void setMVPMatrix(void);
	void resetRotationAndZoom();
	void setStage();
	void RefreshDisplay();
	void OpenGLWnd::SetRotation(double delta_xi, double delta_eta);
	/*ERROR CONTROL*/
	//void checkGLErrors(QString msg);
	//void checkCudaErrors(QString msg);

protected:
	/*LIGHTS CONFIGURATION VARIABLES*/
	QVector4D Lights[MAX_LIGHTS][4];
	
	GLuint LightsCounter;
	GLuint LightsMask;

	/*CAMERA CONFIGURATION VARIABLES*/
	QMatrix4x4 ViewMatrix;
	QMatrix4x4 ModelMatrix;
	QMatrix4x4 PerspectiveMatrix;
	QMatrix4x4 MVPMatrix;
	QVector4D CameraPosition;

	GLfloat glfMoveZ;
	GLfloat glfRotateX;
	GLfloat glfRotateY;
	QPoint	qpLastMousePos;						// Last mouse cursor coordinates
	/*SHADERS VARIABLES*/
	CUniformBlock* TransformationsUBlock;
	CUniformBlock* LightsUBlock;
	CShaderProgram* ShaderPrograms[2]; 
	CDataBridge DataBridge;
public slots:
	void SetStatusText(QString str);
signals:
	void showStatusText(QString str);
};
