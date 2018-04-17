#include "OpenGLWnd.h"


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
	memset(&stTransformations, 0, sizeof(TransBl));
	memset(&stLights, 0, sizeof(LightBl));
	resetRotationAndZoom();
	SetupModelMatrix(model_rot_axis, model_rot_angle0);
	InitViewMatrix();
	InitLights();
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

	
	static const QString TransormUniformNames[5] = { "Trans.VM", "Trans.MM", "Trans.PM","Trans.MVPM","Trans.Cam" }
	static const QString LightsUniformNames[5] = { "Trans.VM", "Trans.MM", "Trans.PM", "Trans.MVPM", "Trans.Cam" }


	View = 0,
		Model = 1,
		Perspective = 2,
		MVP = 3,
		Camera = 4
}