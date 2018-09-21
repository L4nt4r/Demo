#include "OpenGLWnd.h"
#include <QtOpenGL\qglfunctions.h>



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
	// furthermore I would lose flexibility of the code, so i will stay here with gl methods. Sometimes is nice dto do something by yourself :)
	static const int nShaderFiles = 5;
	static const int nShaderNumber = 5;
	static const char *strShaderFiles[] = { "CustomColorInterpolation.glsl.vert", "CustomColorInterpolation.glsl.geom", "CustomColorInterpolation.glsl.frag", "Base.glsl.vert", "Base.glsl.frag" };

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
	static const char *UniLightsBlockNames[LightsBlockVariablesCounter + 1] = { "LSBlock", "LSBlock.nls", "LSBlock.mask", "LSBlock.ls[0].ambient", "LSBlock.ls[0].direct", "LSBlock.ls[0].position", "LSBlock.ls[0].attenuation", "LSBlock.ls[1].ambient" };


	TransformationsUBlock = ShaderPrograms[0]->BindNewUniformBlock(TransBlockVariablesCounter, &UniTransBlocksNames[0]);
	LightsUBlock = ShaderPrograms[0]->BindNewUniformBlock(LightsBlockVariablesCounter, &UniLightsBlockNames[0]);
	LightsUBlock->SetStructOffsetParams(2, 4);
	ShaderPrograms[1]->BindToUniformBlock(TransformationsUBlock);
	
	checkGLErrors("LoadMyShaders", this);
}



void OpenGLWnd::setModelMatrix(QVector3D axis, float angle){
	ModelMatrix.setToIdentity();
	axis.normalize();
	ModelMatrix.rotate(angle, axis);
	TransformationsUBlock->SetUniformData(ModelMatrix.data(), 16 * sizeof(GLfloat), Model);
	checkGLErrors("setModelMatrix", this);
	setMVPMatrix();
}
void OpenGLWnd::setMVPMatrix(){
	QMatrix4x4 mvp;
	mvp = PerspectiveMatrix*(ViewMatrix * ModelMatrix);
	TransformationsUBlock->SetUniformData(mvp.data(), 16 * sizeof(GLfloat), 3);
	checkGLErrors("setNVPMatrix", this);
}
void OpenGLWnd::setViewMatrix(){
	ViewMatrix.setToIdentity();
	ViewMatrix.translate(-1*QVector3D(CameraPosition));
	TransformationsUBlock->SetUniformData(ViewMatrix.data(), 16 * sizeof(GLfloat), View);
	TransformationsUBlock->SetUniformData(&CameraPosition[0], 4 * sizeof(GLfloat), Camera);
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
	LightsUBlock->SetUniformData(&Lights[lightID][paramType], 4 * sizeof(GLfloat), paramType, lightID);
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
	LightsUBlock->SetUniformData( &LightsCounter, sizeof(GLuint), Counter);
	LightsUBlock->SetUniformData( &LightsMask, sizeof(GLuint), Mask);
	checkGLErrors("ToggleLight", this);
}
////////////////////////////////////////////////////////

// Paint objects on screen, QT calls this every time when the app sends repaint message
void OpenGLWnd::paintGL(void){

	glClearColor(0.0, 0.0, 1.0, 1.0);   // Clear color and depth buffers before each frame
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);

	ShaderPrograms[1]->Bind();
	//glUseProgram(nProgramHandle[0]);
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
	float *d;
	d = PerspectiveMatrix.data();
	glBindBuffer(GL_UNIFORM_BUFFER, TransformationsUBlock->Buffer/*nTransBufferHandle*/);
	glBufferSubData(GL_UNIFORM_BUFFER, TransformationsUBlock->VariablesLocation[2]/*nTransUniformBlock[2]*/, 16 * sizeof(float), PerspectiveMatrix.data());
	//TransformationsUBlock->SetUniformData(PerspectiveMatrix.data(), 16 * sizeof(GLfloat), 2);
	checkGLErrors("SetStage", this);
	setMVPMatrix();
	//glUseProgram(CustomColorInterpolationProgram.programId());
	//glUseProgram(BasicProgram.programId());
}
/*INPUT DEVICES EVENTS*/
void OpenGLWnd::mousePressEvent(QMouseEvent *event){

	qpLastMousePos = event->pos();
}				// Mouse button press event
void OpenGLWnd::mouseReleaseEvent(QMouseEvent *event){}				// Mouse button release event
void OpenGLWnd::mouseMoveEvent(QMouseEvent *event){
	// Mouse movement factor
	// Mouse sensitivity index
	const float MouseSensitivity = 5.0f;
	// New mouse coordinates
	// Right click only rotation
	if (event->buttons() & Qt::RightButton)
	{
		int nMouseX = event->x();
		int nMouseY = event->y();
		if (nMouseX != qpLastMousePos.x() || nMouseY != qpLastMousePos.y()) {
			SetRotation((nMouseX - qpLastMousePos.x()) / MouseSensitivity, (nMouseY - qpLastMousePos.y()) / MouseSensitivity);
			qpLastMousePos = event->pos();
			RefreshDisplay();
		//	emit RotationOccured();
		}
	}

}				// Mouse movement event
void OpenGLWnd::keyPressEvent(QKeyEvent *event){}					// Keyboard key press event
void OpenGLWnd::keyReleaseEvent(QKeyEvent *event){}					// Keyboard key release event

void OpenGLWnd::RefreshDisplay()
{
	if (isVisible()){
		update();
	}
}
void OpenGLWnd::SetRotation(double delta_xi, double delta_eta){
	//glfRotateX = RotX;
	//glfRotateY = RotY;
	float lgt, angi, angk;
	QVector3D vi, vk;
	QMatrix4x4 rY, rX, pom;

	if (delta_xi == 0 && delta_eta == 0)
		ModelMatrix.setToIdentity(); /* natychmiast uciekamy - nie chcemy dzieliæ przez zero */
	else{
		if (glfRotateY + (delta_xi) >= -90 && glfRotateY + (delta_xi) <= 90)
			glfRotateY += (delta_xi);
		if (glfRotateX + (delta_eta) >= -90 && glfRotateX + (delta_eta) <= 90)
			glfRotateX += (delta_eta);
		rX.setToIdentity();
		rX.rotate(glfRotateX,1,0,0);
		rY.setToIdentity();
		rY.rotate(glfRotateY, 0, 1, 0);

		ModelMatrix = rX*rY;
	}
	checkGLErrors("PreRotateViewer",this);
	TransformationsUBlock->SetUniformData(ModelMatrix.data(), 16 * sizeof(GLfloat), Model);
	checkGLErrors("RotateViewer",this);
	setMVPMatrix();
}

