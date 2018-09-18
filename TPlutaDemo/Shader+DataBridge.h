#include <QtWidgets>
#include <cuda_gl_interop.h>

void checkGLErrors(QString msg, QWidget* Wnd);


class CShader;
class CUniformBlock;

class CShaderProgram : public QOpenGLExtraFunctions{
public:
	CShaderProgram(QWidget* p, uint ShadersNumber, CShader** Shaders);
	CUniformBlock* BindNewUniformBlock(int nNamesInBlock, const GLchar** UniformDataNames);
	void BindToUniformBlock(CUniformBlock* UniformBlock);
protected:
	void AttachShader(CShader &Shaders);
	void LinkShaderProgram();
	GLuint NewUniformBindingPoint(void);
	QWidget* parent;
	GLuint Handle;
	CShader **Shaders;
	uint ShadersCount;
	static GLint MaximumUniformBindingPoints;
	static GLuint NextUniformBindingPoint;
};

class CShader : public QOpenGLExtraFunctions{
public:
	CShader::CShader(QWidget* p, GLenum ShaderType, uint nFiles, const QString *FileNames);
	GLuint getHandle() const { return Handle; }
	friend class CShaderProgram;
protected:
	GLuint Handle;
	QWidget* parent;
	GLenum Type;
private:
	int LinksCounter;
	
protected:
	bool CompileShaderFiles(uint nShaderFilesCount,const QString *ShadersFilesNames);
	bool CompileShaderStrings(uint nShaderFilesCount, const GLchar **sourceLines);


};

class CUniformBlock : public QOpenGLExtraFunctions{
public:
	friend class CShaderProgram;
	CUniformBlock(QWidget* p, int blockVariablesNumber){ parent = p; VariablesLocation = new GLint[blockVariablesNumber]; }
	~CUniformBlock(){ delete[] VariablesLocation; }
protected:
	void SetUniformBuffer();
	GLuint nBindingPoint;
	GLint nBlockSize;
	GLint * VariablesLocation;
	GLuint Buffer;
	QWidget* parent;
};



