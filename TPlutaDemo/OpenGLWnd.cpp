#include "OpenGLWnd.h"
#include <QtOpenGL\qglfunctions.h>

GLint OpenGLWnd::MaximumUniformBindingPoints = 0;
GLuint OpenGLWnd::NextUniformBindingPoint = 0;

void OpenGLWnd::checkGLErrors(QString msg){
	GLenum error;
	if ((error = glGetError()) != GL_NO_ERROR) {
		QMessageBox::information(
			this,
			"GL ERROR",
			msg + " " + QString::number(error));
		exit(1);
	}
}
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
	
	initializeOpenGLFunctions();							//We can call this method to let qt creates and connects for us Render Context with window Display Context
	if (!context()) {
		QMessageBox::information(
			this,
			"INITIALIZATION ERROR",
			"context is not initialized");
		exit(1);
	}
	loadShaders();											//Next we need to load shaders programs and declare uniform variables 
	
	ViewMatrix.fill(0);										//For safety sake reset all data
	ModelMatrix.fill(0);
	PerspectiveMatrix.fill(0);
	MVPMatrix.fill(0);
	CameraPosition = QVector4D(0,0,0,0);
	memset(&Lights, 0, sizeof(QVector4D)*MAX_LIGHTS*4);


	resetRotationAndZoom();
	setModelMatrix(QVector3D(0.0f,1.0f,0.0f), 0.0f);
	setViewMatrix();
	initLights();
}

void OpenGLWnd::loadShaders(){
	static const int numberOfShaders = 3;
	
	static const QString ShaderFiles[numberOfShaders] = { "CustomColorInterpolation.glsl.vert", "CustomColorInterpolation.glsl.geom", "CustomColorInterpolation.glsl.frag" };
	static const QOpenGLShader::ShaderType ShaderTypes[numberOfShaders] = { QOpenGLShader::Vertex, QOpenGLShader::Geometry, QOpenGLShader::Fragment };

	for (int i = 0; i < numberOfShaders; i++)
	if (!(CustomColorInterpolationProgram.addShaderFromSourceFile(ShaderTypes[i], ShaderFiles[i]))){
		QMessageBox::information(
			this,
			"SHADERS LOADING ERROR",
			ShaderFiles[i] + " " + "could not be found or compiled");
		exit(1);
	}
	CustomColorInterpolationProgram.link();
	CustomColorInterpolationProgram.bind();

	// I could link uniform variables with Qt shader program interface, 
	// but qt does not support passing arrays of custom values and I would have to pass each lights parameters separately,
	// furthermore I would lose flexibility of the code, so i will stay here with gl methods.

	static const GLchar *UniTransBlocksNames[] = { "TransBlock", "TransBlock.mm", "TransBlock.vm", "TransBlock.pm", "TransBlock.mvpm", "TransBlock.eyepos" };
	static const GLchar *UniLightsBlockNames[] = { "LSBlock", "LSBlock.ls[0].ambient", "LSBlock.ls[0].direct", "LSBlock.ls[0].position", "LSBlock.ls[0].attenuation", "LSBlock.ls[1].ambient", "LSBlock.nls", "LSBlock.mask" };

	GLuint nTransBlockBindingPoint, nLightBlockBindingPoint;
	GLint nTransUniBlockSize, nLightsUniBlockSize;

	getAccessToUniformBlock(CustomColorInterpolationProgram.programId(), 5, &UniTransBlocksNames[0], &nTransUniBlockSize, TransformUniformLocations, &nTransBlockBindingPoint);
	getAccessToUniformBlock(CustomColorInterpolationProgram.programId(), 7, &UniLightsBlockNames[0], &nLightsUniBlockSize, LightsUniformLocations, &nLightBlockBindingPoint);
	//AttachUniformBlockToBP(nProgramHandle[0], UTBNames[0], nTransBlockBindingPoint);
	TransBufferHandle = newUniformBlockObject(nTransUniBlockSize, nTransBlockBindingPoint);
	LightsBufferHandle = newUniformBlockObject(nLightsUniBlockSize, nLightBlockBindingPoint);

	checkGLErrors("LoadMyShaders");
	
}
void OpenGLWnd::attachUniformBlockToBP(GLuint programHandle, const GLchar *name, GLuint nBlockBindingPoint)
{
	GLuint index;

	index = glGetUniformBlockIndex(programHandle, name);
	glUniformBlockBinding(programHandle, index, nBlockBindingPoint);
	checkGLErrors("AttachUniformBlockToBP");
}

GLuint OpenGLWnd::newUniformBlockObject(GLint nSize, GLuint nBlockBindingPoint)
{
	GLuint nBuffer;
	glGenBuffers(1, &nBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, nBlockBindingPoint, nBuffer);
	glBufferData(GL_UNIFORM_BUFFER, nSize, NULL, GL_DYNAMIC_DRAW);
	checkGLErrors("NewUniformBlockObject");
	return nBuffer;

}
GLuint OpenGLWnd::newUniformBindingPoint(void)
{
	if (!MaximumUniformBindingPoints)
		glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &MaximumUniformBindingPoints);
	if (NextUniformBindingPoint < MaximumUniformBindingPoints)
		return NextUniformBindingPoint++;
	else {
		QMessageBox::information(
			this,
			"GL ERROR",
			"Uniform binding points limit exhausted\n");
		exit(1);
	}
}

void OpenGLWnd::getAccessToUniformBlock(GLuint nProgramID, int nTypesCount, const GLchar **typesNames, GLint *nUniBlockSize, GLint *UniformBlock, GLuint *uniBlockBindingPoint)
{
	GLuint ufi[32];

	GLuint nUniBlockIndex = glGetUniformBlockIndex(nProgramID, typesNames[0]);
	glGetActiveUniformBlockiv(nProgramID, nUniBlockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, nUniBlockSize);
	if (nTypesCount > 0) {
		glGetUniformIndices(nProgramID, nTypesCount, &typesNames[1], ufi);
		glGetActiveUniformsiv(nProgramID, nTypesCount, ufi, GL_UNIFORM_OFFSET, UniformBlock);
	}

	*uniBlockBindingPoint = newUniformBindingPoint();
	glUniformBlockBinding(nProgramID, nUniBlockIndex, *uniBlockBindingPoint);
	checkGLErrors("GetAccessToUniformBlock");

}
void OpenGLWnd::setModelMatrix(QVector3D axis, float angle){
	ModelMatrix.rotate(angle, axis);
	glBindBuffer(GL_UNIFORM_BUFFER, TransBufferHandle);
	glBufferSubData(GL_UNIFORM_BUFFER, TransformUniformLocations[Model], 16 * sizeof(GLfloat), ModelMatrix.data());
	checkGLErrors("setModelMatrix");
	setMVPMatrix();
}
void OpenGLWnd::setMVPMatrix(){
	QMatrix4x4 mvp;
	mvp = PerspectiveMatrix*(ViewMatrix * ModelMatrix);
	glBindBuffer(GL_UNIFORM_BUFFER, TransBufferHandle);
	glBufferSubData(GL_UNIFORM_BUFFER, TransformUniformLocations[MVP], 16 * sizeof(GLfloat), mvp.data());
	checkGLErrors("setNVPMatrix");
}
void OpenGLWnd::setViewMatrix(){
	ViewMatrix.translate(-1*QVector3D(CameraPosition));

	//stTransformations.vm.Translatef(-viewer_pos0.data[0], -viewer_pos0.data[1], -viewer_pos0.data[2]);
	glBindBuffer(GL_UNIFORM_BUFFER, TransBufferHandle);
	glBufferSubData(GL_UNIFORM_BUFFER, TransformUniformLocations[View], 16 * sizeof(GLfloat), ViewMatrix.data());
	glBufferSubData(GL_UNIFORM_BUFFER, TransformUniformLocations[Camera], 4 * sizeof(GLfloat), &CameraPosition[0]);
	checkGLErrors("InitViewMatrix");
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
	checkGLErrors("SetLightParam");
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
	checkGLErrors("ToggleLight");
}
////////////////////////////////////////////////////////

void OpenGLWnd::paintGL(void){

	glClearColor(0.0, 0.0, 1.0, 1.0);
	// Clear color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	//setStage(VIEW);
	//glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);

	//glDepthRange(0.01, 1.0);
	// Paint 3D view

	paintView();
	//setStage(GUI);
	//glDepthRange(0, 0.01);
	//paintGUI();
	glFlush();

}										// Paint objects on screen, QT calls this every time when the app sends repaint message
void OpenGLWnd::resizeGL(int width, int height){
	glViewport(0, 0, width, height);
	PrepareVBO();
	ExitIfGLError("ResizeGL");
	fFrustrumScale = ASPECT*(float)width / (float)height;
	fOrthoScale = fFrustrumScale;
	gldOrthoNearVal = gldFrustrumNearVal = 5.0;
	gldOrthoFarVal = gldFrustrumFarVal = 20.0;
	stTransformations.pmF.SetFrustrumf(-fFrustrumScale, fFrustrumScale, -0.5533, 0.5533, 5.0, 20.0);
	stTransformations.pmO.SetOtrhof(0, width, height, 0, 5.0, 20.0);
	setStage(VIEW);
}					// Resize window, Qt calls it when the window is resized

/*INPUT DEVICES EVENTS*/
void OpenGLWnd::mousePressEvent(QMouseEvent *event){}				// Mouse button press event
void OpenGLWnd::mouseReleaseEvent(QMouseEvent *event){}				// Mouse button release event
void OpenGLWnd::mouseMoveEvent(QMouseEvent *event){}				// Mouse movement event
void OpenGLWnd::keyPressEvent(QKeyEvent *event){}					// Keyboard key press event
void OpenGLWnd::keyReleaseEvent(QKeyEvent *event){}					// Keyboard key release event