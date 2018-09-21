
//#include "Shader+DataBridge.h"
#include "OpenGLWnd.h"

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

CShader::CShader(OpenGLWnd* p, GLenum ShaderType, uint nFiles, const QString *FileNames){
	parent = p;
	Type = ShaderType;

	CompileShaderFiles(nFiles, FileNames);
	LinksCounter = 0;
	
}
CShader::CShader(OpenGLWnd* p, GLenum ShaderType, uint nFiles, const char **FileNames){
	parent = p;
	Type = ShaderType;

	CompileShaderFiles(nFiles, FileNames);
	LinksCounter = 0;

}

bool CShader::CompileShaderFiles(uint nShaderFilesCount, const char **ShadersFilesNames){
	
	FILE *file;
	int i;
	GLint *fileSize = NULL, nTotalSize;
	GLchar *source = NULL, **sourceLines = NULL;
	try{
		if (!(fileSize = new GLint[nShaderFilesCount]) || !(sourceLines = new GLchar*[nShaderFilesCount]))
			throw;
		for (i = 0, nTotalSize = nShaderFilesCount; i < nShaderFilesCount; i++) {
			if (!(file = fopen(ShadersFilesNames[i], "rb")))
				throw;
			fseek(file, 0, SEEK_END);
			fileSize[i] = ftell(file);
			nTotalSize += fileSize[i];
			fclose(file);
		}
		if (!(source = new char[nTotalSize]))
			throw;
		for (i = 0, nTotalSize = 0; i < nShaderFilesCount; i++) {
			if (!(file = fopen(ShadersFilesNames[i], "rb")))
				throw;
			sourceLines[i] = &source[nTotalSize];
			if (fread(sourceLines[i], sizeof(char), fileSize[i], file) != fileSize[i]){
				fclose(file);
				throw;
			}
			sourceLines[i][fileSize[i]] = 0;
			nTotalSize += fileSize[i] + 1;
			fclose(file);
		}
		 CompileShaderStrings( nShaderFilesCount, (const GLchar**)sourceLines);
	}
	catch (...){
		fprintf(stderr, "Error: exception in CompileShaderFiles\n");
	};

	if (fileSize) delete[](fileSize);
	if (sourceLines) delete[](sourceLines);
	if (source)  delete[](source);
	
	return true;
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
			if (!ShaderFile.exists()) {
				QString errorMsg = "Shader File not exist path: " + QDir::currentPath() + ShadersFilesNames[i];
				QMessageBox::information(
					 parent,
					"Compile Shaders",
					errorMsg);
			
				throw;
			}
			if (!(ShaderFile.open(QIODevice::ReadOnly))){
				QString errorMsg = "Shader File open return an error: " + ShaderFile.errorString();
				QMessageBox::information(
					 parent,
					"Compile Shaders",
					errorMsg);
				
				throw;

			}

			Blob += ShaderFile.readAll();
			Blob += '\0';					//adding 0 byte at the end of file data to create shader source with last null param
			ShaderFile.close();
		}
		cSourceData[0] = (GLchar*)Blob.constData();
		for (i = 1; i < nShaderFilesCount - 1; i++) {
			cSourceData[i] = (GLchar*)Blob.constData() + nFileStartPosition[i];
		}
		/*QMessageBox::information(
			parent,
			"Compile Shaders",
			cSourceData[0]);*/
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
	//QMessageBox::information(
	//parent,
	//"Compile Shaders",
	//sourceLines[0]);
	GLint logsize;
	GLchar *log = nullptr;

	if ((Handle = parent->glCreateShader(Type)) != 0) {
		parent->glShaderSource(Handle, nShaderFilesCount, sourceLines, NULL);
		parent->glCompileShader(Handle);
		parent->glGetShaderiv(Handle, GL_INFO_LOG_LENGTH, &logsize);
		if (logsize > 1) {
			if ((log = new GLchar[logsize + 1]) != 0) {
				parent->glGetShaderInfoLog(Handle, logsize, &logsize, log);
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


CShaderProgram::CShaderProgram(OpenGLWnd *p, uint ShadersNumber, CShader** Shaders){
	parent = p;
	this->Shaders = Shaders;
	this->ShadersCount = ShadersNumber;
	LinkShaderProgram();
}
void CShaderProgram::LinkShaderProgram()
{
	
	GLint logsize;
	GLchar *log;

	if ((Handle = parent->glCreateProgram())) {
		for (int i = 0; i < ShadersCount; i++)
			AttachShader(*Shaders[i]);
		parent->glLinkProgram(Handle);
		parent->glGetProgramiv(Handle, GL_INFO_LOG_LENGTH, &logsize);
		if (logsize > 1) {
			if ((log = new char[logsize + 1])) {
				parent->glGetProgramInfoLog(Handle, logsize, &logsize, log);
				QMessageBox::information(
					parent,
					"Linking Shaders",
					log);
				delete[] log;
			}
		}
	}
	checkGLErrors("Linking Shader Program", parent);
}
void CShaderProgram::AttachShader(CShader &Shaders){
	Shaders.LinksCounter++;				//accessing protected member 
	parent->glAttachShader(Handle, Shaders.getHandle());
}

CUniformBlock* CShaderProgram::BindNewUniformBlock(int nNamesInBlock, const GLchar** UniformDataNames){
	GLuint ufi[32];
	CUniformBlock* UniformBlock = new CUniformBlock(parent, nNamesInBlock);
	
	UniformBlock->name = UniformDataNames[0];

	GLuint nUniBlockIndex = parent->glGetUniformBlockIndex(Handle, UniformBlock->name);
	parent->glGetActiveUniformBlockiv(Handle, nUniBlockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &(UniformBlock->nBlockSize));
	if (nNamesInBlock > 0) {
		parent->glGetUniformIndices(Handle, nNamesInBlock, &UniformDataNames[1], ufi);
		parent->glGetActiveUniformsiv(Handle, nNamesInBlock, ufi, GL_UNIFORM_OFFSET, UniformBlock->VariablesLocation);
	}

	UniformBlock->nBindingPoint = NewUniformBindingPoint();
	parent->glUniformBlockBinding(Handle, nUniBlockIndex, UniformBlock->nBindingPoint);
	checkGLErrors("GetAccessToUniformBlock", parent);

	UniformBlock->SetUniformBuffer();
	return UniformBlock;
}

void CShaderProgram::BindToUniformBlock(CUniformBlock* UniformBlock){
	GLuint index;

	index = parent->glGetUniformBlockIndex(Handle, UniformBlock->name);
	parent->glUniformBlockBinding(Handle, index, UniformBlock->nBindingPoint);
	checkGLErrors("BindToUniformBlock", parent);

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
void CShaderProgram::Bind(){
	parent->glUseProgram(Handle);
}
void CUniformBlock::SetUniformBuffer(){
	
	parent->glGenBuffers(1, &Buffer);
	parent->glBindBufferBase(GL_UNIFORM_BUFFER, nBindingPoint, Buffer);
	parent->glBufferData(GL_UNIFORM_BUFFER, nBlockSize, NULL, GL_DYNAMIC_DRAW);
	checkGLErrors("SetUniformBuffer", parent);

}

void CUniformBlock::SetUniformData(const void* data, qopengl_GLsizeiptr size, int index, int column){
	//GLint offset = column * (VariablesLocation[Distance + ElementsCounter] - VariablesLocation[Distance]) + VariablesLocation[index];
	parent->glBindBuffer(GL_UNIFORM_BUFFER, Buffer);
	//parent->glBufferSubData(GL_UNIFORM_BUFFER, offset, size, &data);
	parent->glBufferSubData(GL_UNIFORM_BUFFER, VariablesLocation[index], size, &data);
	checkGLErrors("SetUniformData", parent);
}