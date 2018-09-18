#include "OpenGLWnd.h"
#include <QtOpenGL\qglfunctions.h>

GLint OpenGLWnd::MaximumUniformBindingPoints = 0;
GLuint OpenGLWnd::NextUniformBindingPoint = 0;


OpenGLWnd::OpenGLWnd(QWidget *widget, Qt::WindowFlags f) : QOpenGLWidget(widget, f)
{
}
void OpenGLWnd::resetRotationAndZoom(){

	glfMoveZ = 9;
	glfRotateX = 0.0;
	glfRotateY = 0.0;
	CameraPosition = QVector4D(0.0f, 0.0f, glfMoveZ, 1.0);

}

void OpenGLWnd::initializeGL(){
	//Qt already prepared for us a window to render data, so we got Display Context, now we need to start openGL
	
	initializeOpenGLFunctions();							//We call this method to let qt creates and connects for us Render Context with window Display Context
	if (!context()) {
		QMessageBox::information(
			this,
			"INITIALIZATION ERROR",
			"context is not initialized");
		exit(1);
	}
	loadShaders();											//Next we need to load shaders programs and declare uniform variables 
	
	ViewMatrix.fill(0);										//reseting and initializing other variables
	ModelMatrix.fill(0);
	PerspectiveMatrix.fill(0);
	MVPMatrix.fill(0);
	CameraPosition = QVector4D(0,0,0,0);
	memset(&Lights, 0, sizeof(QVector4D)*MAX_LIGHTS*4);
	resetRotationAndZoom();

	setModelMatrix(QVector3D(0.0f,1.0f,0.0f), 0.0f);
	setViewMatrix();
	initLights();

	//Next point of intrest is in resizeGL function
}

void OpenGLWnd::loadShaders(){
	// I could link uniform variables with Qt shader program interface, 
	// but qt does not support passing arrays of custom values and I would have to pass each lights parameters separately,
	// furthermore I would lose flexibility of the code, so i will stay here with gl methods.
	static const int nShaderFiles = 5;
	static const int nShaderNumber = 5;
	static const QString strShaderFiles[nShaderFiles] = { "CustomColorInterpolation.glsl.vert", "CustomColorInterpolation.glsl.geom", "CustomColorInterpolation.glsl.frag", "Base.glsl.vert", "Base.glsl.frag" };

	CShader* Shaders[nShaderNumber];

	Shaders[0] = new CShader(this, GL_VERTEX_SHADER, 1, &strShaderFiles[0]);
	Shaders[1] = new CShader(this, GL_GEOMETRY_SHADER, 1, &strShaderFiles[1]);
	Shaders[2] = new CShader(this, GL_FRAGMENT_SHADER, 1, &strShaderFiles[2]);

	ShaderPrograms[0] = new CShaderProgram(this, 3, &Shaders[0]);

	Shaders[3] = new CShader(this, GL_VERTEX_SHADER, 1, &strShaderFiles[3]);
	Shaders[4] = new CShader(this, GL_FRAGMENT_SHADER, 1, &strShaderFiles[4]);

	ShaderPrograms[1] = new CShaderProgram(this, 2, &Shaders[3]);

	static const int TransBlockVariablesCounter = 5;
	static const int LightsBlockVariablesCounter = 7;
	static const char *UniTransBlocksNames[TransBlockVariablesCounter + 1] = { "TransBlock", "TransBlock.mm", "TransBlock.vm", "TransBlock.pm", "TransBlock.mvpm", "TransBlock.eyepos" };
	static const char *UniLightsBlockNames[LightsBlockVariablesCounter + 1] = { "LSBlock", "LSBlock.ls[0].ambient", "LSBlock.ls[0].direct", "LSBlock.ls[0].position", "LSBlock.ls[0].attenuation", "LSBlock.ls[1].ambient", "LSBlock.nls", "LSBlock.mask" };

	GLuint nTransBlockBindingPoint, nLightBlockBindingPoint;
	GLint nTransUniBlockSize, nLightsUniBlockSize;

	TransformationsUBlock = ShaderPrograms[0]->BindNewUniformBlock(TransBlockVariablesCounter, &UniTransBlocksNames[0]);
	LightsUBlock = ShaderPrograms[0]->BindNewUniformBlock(LightsBlockVariablesCounter, &UniLightsBlockNames[0]);
	ShaderPrograms[1]->BindToUniformBlock(TransformationsUBlock);
	
	checkGLErrors("LoadMyShaders", this);
	
}

void OpenGLWnd::setModelMatrix(QVector3D axis, float angle){
	ModelMatrix.setToIdentity();
	axis.normalize();
	ModelMatrix.rotate(angle, axis);
	glBindBuffer(GL_UNIFORM_BUFFER, TransBufferHandle);
	glBufferSubData(GL_UNIFORM_BUFFER, TransformUniformLocations[Model], 16 * sizeof(GLfloat), ModelMatrix.data());
	checkGLErrors("setModelMatrix", this);
	setMVPMatrix();
}
void OpenGLWnd::setMVPMatrix(){
	QMatrix4x4 mvp;
	mvp = PerspectiveMatrix*(ViewMatrix * ModelMatrix);
	glBindBuffer(GL_UNIFORM_BUFFER, TransBufferHandle);
	glBufferSubData(GL_UNIFORM_BUFFER, TransformUniformLocations[MVP], 16 * sizeof(GLfloat), mvp.data());
	checkGLErrors("setNVPMatrix", this);
}
void OpenGLWnd::setViewMatrix(){
	ViewMatrix.setToIdentity();
	ViewMatrix.translate(-1*QVector3D(CameraPosition));

	//stTransformations.vm.Translatef(-viewer_pos0.data[0], -viewer_pos0.data[1], -viewer_pos0.data[2]);
	glBindBuffer(GL_UNIFORM_BUFFER, TransBufferHandle);
	glBufferSubData(GL_UNIFORM_BUFFER, TransformUniformLocations[View], 16 * sizeof(GLfloat), ViewMatrix.data());
	glBufferSubData(GL_UNIFORM_BUFFER, TransformUniformLocations[Camera], 4 * sizeof(GLfloat), &CameraPosition[0]);
	checkGLErrors("InitViewMatrix", this);
	setMVPMatrix();
}
void OpenGLWnd::initLights(){
	
	QVector4D LightAmbient = QVector4D(0.1,0.1,0.1,1.0);
	QVector4D LightGlobal = QVector4D(0.5, 0.5, 0.5, 1.0);
	QVector4D LightPosition = QVector4D(0.0, 1.0, 5.0, 0.0);
	QVector4D LightAttenaution = QVector4D(1.0, 0.0, 0.0, 0.0);

	setLightParam(0, LightAmbient, Ambient );
	setLightParam(0, LightGlobal, Direct);
	setLightParam(0, LightPosition, Position);
	setLightParam(0, LightAttenaution, Attenaution);
	toggleLight(0, true);

	LightPosition.setY(-1.0);

	setLightParam(1, LightAmbient, Ambient);
	setLightParam(1, LightGlobal, Direct);
	setLightParam(1, LightPosition, Position);
	setLightParam(1, LightAttenaution, Attenaution);
	toggleLight(1, true);
}

void OpenGLWnd::setLightParam(GLubyte lightID, QVector4D LightParam, LightsParams paramType){
	GLint offset;

	if (lightID < 0 || lightID >= MAX_LIGHTS)
		return;
	Lights[lightID][paramType] = LightParam;
	offset = lightID * UniformLightsBindingDistance + LightsUniformLocations[paramType];
	glBindBuffer(GL_UNIFORM_BUFFER, LightsBufferHandle);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, 4 * sizeof(GLfloat), &Lights[lightID][paramType]);
	checkGLErrors("SetLightParam", this);
}
void OpenGLWnd::toggleLight(GLubyte lightID, bool lightOn){
	GLuint mask;
	if (lightID < 0 || lightID >= MAX_LIGHTS)
		return;

	mask = 0x01 << lightID;
	if (lightOn) {
		LightsMask |= mask;
		if (lightID >= LightsCounter)
			LightsCounter = lightID + 1;
	}
	else {
		LightsMask &= ~mask;
		for (mask = 0x01 << (LightsCounter - 1); mask; mask >>= 1) {
			if (LightsMask & mask)
				break;
			else
				LightsCounter--;
		}
	}
	glBindBuffer(GL_UNIFORM_BUFFER, LightsBufferHandle);
	glBufferSubData(GL_UNIFORM_BUFFER, LightsUniformLocations[Counter], sizeof(GLuint), &LightsCounter);
	glBufferSubData(GL_UNIFORM_BUFFER, LightsUniformLocations[Mask], sizeof(GLuint), &LightsMask);
	checkGLErrors("ToggleLight", this);
}
////////////////////////////////////////////////////////

// Paint objects on screen, QT calls this every time when the app sends repaint message
void OpenGLWnd::paintGL(void){

	glClearColor(1.0, 1.0, 1.0, 1.0);   // Clear color and depth buffers before each frame
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//here are actual drawing functions.
	//paintView();				
	glBegin(GL_QUADS);
	glColor4f(1.0, 1.0, 0.0, 1.0);
	glNormal3f(0.5, 0.5, 1.0);
	glVertex3f(-0.5, 0.5, 0.0);
	glVertex3f(0.5, 0.5, 0.0);
	glVertex3f(0.5, -0.5, 0.0);
	glVertex3f(-0.5, -0.5, 0.0);
	glEnd();
	
	glFlush();
	checkGLErrors("Paint", this);;
}										

// Resize window, Qt calls it when the window is resized, also on first initialization resize
void OpenGLWnd::resizeGL(int width, int height){
	glViewport(0, 0, width, height);		//I want draw with glide in entire window.
	//PrepareVBO();						
	GLfloat fFrustrumScale = 0.5533f * (float)width / (float)height;		//0.5533	
	PerspectiveMatrix.setToIdentity();
	PerspectiveMatrix.frustum(-fFrustrumScale, fFrustrumScale, -0.5533, 0.5533, 5.0, 20.0);
	checkGLErrors("ResizeGL", this);
	setStage();
}					
void OpenGLWnd::setStage()
{
	glBindBuffer(GL_UNIFORM_BUFFER, TransBufferHandle);
	glBufferSubData(GL_UNIFORM_BUFFER, TransformUniformLocations[Perspective], 16 * sizeof(float), PerspectiveMatrix.data());
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	checkGLErrors("SetStage", this);
	setMVPMatrix();
	//glUseProgram(CustomColorInterpolationProgram.programId());
	//glUseProgram(BasicProgram.programId());
}
/*INPUT DEVICES EVENTS*/
void OpenGLWnd::mousePressEvent(QMouseEvent *event){}				// Mouse button press event
void OpenGLWnd::mouseReleaseEvent(QMouseEvent *event){}				// Mouse button release event
void OpenGLWnd::mouseMoveEvent(QMouseEvent *event){}				// Mouse movement event
void OpenGLWnd::keyPressEvent(QKeyEvent *event){}					// Keyboard key press event
void OpenGLWnd::keyReleaseEvent(QKeyEvent *event){}					// Keyboard key release event