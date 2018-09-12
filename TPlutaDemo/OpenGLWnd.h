#pragma once
#include <QtWidgets>
#include <qopenglwidget.h>
#include <cuda_gl_interop.h>

//#include <qopenglwidget.h>
//#include <QOpenGLExtraFunctions>
//1#include <QMatrix4x4>
//#include <qvector4d.h>
//#include <qvector3d.h>
#define MAX_LIGHTS 8

enum TransformationTypes{
	Model = 0,
	View = 1,
	Perspective = 2,
	MVP = 3,
	Camera = 4
};

#define UniformLightsBindingDistance (LightsUniformLocations[4] - LightsUniformLocations[0])
enum LightsParams{
	Ambient = 0,
	Direct = 1,
	Position = 2,
	Attenaution = 3,
	Counter = 5,
	Mask = 6
};


class OpenGLWnd : public QOpenGLWidget, public QOpenGLExtraFunctions {
	Q_OBJECT
public:
	OpenGLWnd(QWidget *widget = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());		//Default QOpenGLWidget constructor, will call InitializeGL
	~OpenGLWnd(){};																			//Destructor contains cleaning functions 

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
	void getAccessToUniformBlock(GLuint nProgramID, int nTypesCount, const GLchar **typesNames, GLint *nUniBlockSize, GLint *UniformBlock, GLuint *uniBlockBindingPoint);
	GLuint OpenGLWnd::newUniformBindingPoint(void);
	GLuint OpenGLWnd::newUniformBlockObject(GLint nSize, GLuint nBlockBindingPoint);
	void OpenGLWnd::attachUniformBlockToBP(GLuint programHandle, const GLchar *name, GLuint nBlockBindingPoint);

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

	/*ERROR CONTROL*/
	void checkGLErrors(QString msg);
	void checkCudaErrors(QString msg);

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
	
	/*SHADERS VARIABLES*/
	GLint LightsUniformLocations[7];
	GLint TransformUniformLocations[5];
	GLuint TransBufferHandle;
	GLuint LightsBufferHandle;
	QOpenGLShaderProgram CustomColorInterpolationProgram;
	static GLint MaximumUniformBindingPoints;
	static GLuint NextUniformBindingPoint;
};
