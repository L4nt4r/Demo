
#include "OpenGLWnd.h"


bool CShader::CompileShaderFiles(GLenum eShaderType, uint nShaderFilesCount, const char **ShadersFilesNames){

	Handle = 0;
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
		if (!CompileShaderStrings(eShaderType, nShaderFilesCount, (const GLchar**)sourceLines))
			throw;
	}
	catch (...){
		fprintf(stderr, "Error: exception in CompileShaderFiles\n");
	};

	if (fileSize) delete[](fileSize);
	if (sourceLines) delete[](sourceLines);
	if (source)  delete[](source);
}/*CompileShaderFiles*/

bool CShader::CompileShaderStrings(GLenum eShaderType, int nShaderFilesCount, const GLchar **sourceLines)
{
	GLint logsize;
	GLchar *log = nullptr;

	if ((Handle = glCreateShader(eShaderType)) != 0) {
		glShaderSource(Handle, nShaderFilesCount, sourceLines, NULL);
		glCompileShader(Handle);
		glGetShaderiv(Handle, GL_INFO_LOG_LENGTH, &logsize);
		if (logsize > 1) {
			if ((log = new GLchar[logsize + 1]) != 0) {
				glGetShaderInfoLog(Handle, logsize, &logsize, log);
				QMessageBox::information(
					Globals::Data().MAINHWND,
					"Compile Shaders",
					log);
				delete log;
			}
		}
	}
	if (glGetError() != GL_NO_ERROR) return false;
	else return true;
}/*CompileShaderStrings*/