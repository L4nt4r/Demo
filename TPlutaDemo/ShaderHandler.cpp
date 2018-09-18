
#include "Shader+DataBridge.h"

GLint CShaderProgram::MaximumUniformBindingPoints = 0;
GLuint CShaderProgram::NextUniformBindingPoint = 0;

void checkGLErrors(QString msg, QWidget* Wnd){
	GLenum error;
	if ((error = glGetError()) != GL_NO_ERROR) {
		QMessageBox::information(
			Wnd,
			"GL ERROR",
			msg + " " + QString::number(error));
		exit(1);
	}
}

CShader::CShader(QWidget* p, GLenum ShaderType, uint nFiles,const QString *FileNames){
	parent = p;
	Type = ShaderType;
	CompileShaderFiles(nFiles, FileNames);
	LinksCounter = 0;
}
bool CShader::CompileShaderFiles(uint nShaderFilesCount, const QString *ShadersFilesNames){

	Handle = 0;
	QFile ShaderFile;
	QByteArray Blob;
	int i;
	GLint *nFileStartPosition = NULL;
	GLchar **cSourceData = NULL;
	Blob.clear();
	try{
		if (!(nFileStartPosition = new GLint[nShaderFilesCount]) || !(cSourceData = new GLchar*[nShaderFilesCount]))
			throw;
		for (i = 0; i < nShaderFilesCount; i++) {
			nFileStartPosition[i] = Blob.size();
			ShaderFile.setFileName(ShadersFilesNames[i]);
			if (!(ShaderFile.open(QIODevice::ReadOnly)));
				throw;
			Blob += ShaderFile.readAll();
			Blob += '\0)';					//adding 0 byte at the end of file data to create shader source with last null param
			ShaderFile.close();
		}
		cSourceData[0] = (GLchar*)Blob.constData();
		for (i = 1; i < nShaderFilesCount - 1; i++) {
			cSourceData[i] = (GLchar*)Blob.constData() + nFileStartPosition[i];
		}
		if (!CompileShaderStrings(nShaderFilesCount, (const GLchar**)cSourceData))
			throw;
	}
	catch (...){
		QMessageBox::information(
			parent,
			"Compile Shaders", 
			"Error: exception in CompileShaderFiles\n");
	};
	
	if (nFileStartPosition) delete[](nFileStartPosition);
	if (cSourceData) delete[](cSourceData);
}/*CompileShaderFiles*/

bool CShader::CompileShaderStrings(uint nShaderFilesCount, const GLchar **sourceLines)
{
	GLint logsize;
	GLchar *log = nullptr;

	if ((Handle = glCreateShader(Type)) != 0) {
		glShaderSource(Handle, nShaderFilesCount, sourceLines, NULL);
		glCompileShader(Handle);
		glGetShaderiv(Handle, GL_INFO_LOG_LENGTH, &logsize);
		if (logsize > 1) {
			if ((log = new GLchar[logsize + 1]) != 0) {
				glGetShaderInfoLog(Handle, logsize, &logsize, log);
				QMessageBox::information(
					parent,
					"Compile Shaders",
					log);
				delete log;
			}
		}
	}
	if (glGetError() != GL_NO_ERROR) return false;
	else return true;
}/*CompileShaderStrings*/


CShaderProgram::CShaderProgram(QWidget* p, uint ShadersNumber, CShader** Shaders){
	parent = p;
	this->Shaders = Shaders;
	this->ShadersCount = ShadersNumber;
	LinkShaderProgram();
}
void CShaderProgram::LinkShaderProgram()
{
	
	GLint logsize;
	GLchar *log;

	if ((Handle = glCreateProgram())) {
		for (int i = 0; i < ShadersCount; i++)
			AttachShader(*Shaders[i]);
		glLinkProgram(Handle);
		glGetProgramiv(Handle, GL_INFO_LOG_LENGTH, &logsize);
		if (logsize > 1) {
			if ((log = new char[logsize + 1])) {
				glGetProgramInfoLog(Handle, logsize, &logsize, log);
				QMessageBox::information(
					parent,
					"Linking Shaders",
					log);
				delete[] log;
			}
		}
	}
	if (glGetError() != GL_NO_ERROR)
		QMessageBox::information(
		parent,
		"Linking Shaders",
		"Error during Linking");
}
void CShaderProgram::AttachShader(CShader &Shaders){
	Shaders.LinksCounter++;				//accessing protected member 
	glAttachShader(Handle, Shaders.getHandle());
}

CUniformBlock* CShaderProgram::BindNewUniformBlock(int nNamesInBlock, const GLchar** UniformDataNames){
	GLuint ufi[32];
	CUniformBlock* UniformBlock = new CUniformBlock(parent, nNamesInBlock);
	


	GLuint nUniBlockIndex = glGetUniformBlockIndex(Handle, UniformDataNames[0]);
	glGetActiveUniformBlockiv(Handle, nUniBlockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &(UniformBlock->nBlockSize));
	if (nNamesInBlock > 0) {
		glGetUniformIndices(Handle, nNamesInBlock, &UniformDataNames[1], ufi);
		glGetActiveUniformsiv(Handle, nNamesInBlock, ufi, GL_UNIFORM_OFFSET, UniformBlock->VariablesLocation);
	}

	UniformBlock->nBindingPoint = NewUniformBindingPoint();
	glUniformBlockBinding(Handle, nUniBlockIndex, UniformBlock->nBindingPoint);
	checkGLErrors("GetAccessToUniformBlock", parent);

	UniformBlock->SetUniformBuffer();
	return UniformBlock;
}

void CShaderProgram::BindToUniformBlock(CUniformBlock* UniformBlock){


}

GLuint CShaderProgram::NewUniformBindingPoint(){
	if (!MaximumUniformBindingPoints)
		glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &MaximumUniformBindingPoints);
	if (NextUniformBindingPoint < MaximumUniformBindingPoints)
		return NextUniformBindingPoint++;
	else {
		QMessageBox::information(
			parent,
			"GL ERROR",
			"Uniform binding points limit exhausted\n");
		exit(1);
	}
}

void CUniformBlock::SetUniformBuffer(){
	
	glGenBuffers(1, &Buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, nBindingPoint, Buffer);
	glBufferData(GL_UNIFORM_BUFFER, nBlockSize, NULL, GL_DYNAMIC_DRAW);
	checkGLErrors("NewUniformBlockObject", parent);
}