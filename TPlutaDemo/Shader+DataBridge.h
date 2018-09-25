#include <QtWidgets>
#include <cuda.h>
#include "driver_types.h"

#define MinimumRequireMajor 5
#define MinimumRequireMinor 1

void checkGLErrors(QString msg, QWidget* Wnd);

class OpenGLWnd;
class CShader;
class CUniformBlock;

struct VertexData{
	GLfloat x;
	GLfloat y;
	GLfloat z;
};

class CShaderProgram{
public:
	CShaderProgram(OpenGLWnd* p, uint ShadersNumber, CShader** Shaders);
	~CShaderProgram();
	CUniformBlock* BindNewUniformBlock(int nNamesInBlock, const GLchar** UniformDataNames);
	void BindToUniformBlock(CUniformBlock* UniformBlock);
	void Bind();
	GLuint Handle;
protected:
	void AttachShader(CShader &Shaders);
	void LinkShaderProgram();
	GLuint NewUniformBindingPoint(void);
	OpenGLWnd* parent;
	CShader **Shaders;
	uint ShadersCount;
	static GLint MaximumUniformBindingPoints;
	static GLuint NextUniformBindingPoint;
};

class CShader{
public:
	CShader(OpenGLWnd* p, GLenum ShaderType, uint nFiles, const QString *FileNames);
	CShader(OpenGLWnd* p, GLenum ShaderType, uint nFiles, const char **FileNames);
	~CShader();
	GLuint getHandle() const { return Handle; }
	friend class CShaderProgram;

	GLuint Handle;
protected:
	OpenGLWnd* parent;
	GLenum Type;
private:
	int LinksCounter;
	
protected:
	bool CompileShaderFiles(uint nShaderFilesCount,const QString *ShadersFilesNames);
	bool CompileShaderFiles(uint nShaderFilesCount, const char **ShadersFilesNames);
	bool CompileShaderStrings(uint nShaderFilesCount, const GLchar **sourceLines);


};

class CUniformBlock{
public:
	friend class CShaderProgram;
	CUniformBlock(OpenGLWnd* p, int blockVariablesNumber){
		parent = p; 
		VariablesLocation = new GLint[blockVariablesNumber];
		Distance=0;
		ElementsCounter=0;
	}
	~CUniformBlock();
	void SetStructOffsetParams(int nDistance, int nElements) { OffsetPatern = true; Distance = nDistance; ElementsCounter = nElements; }
	//void SetUniformData(const void* data, qopengl_GLsizeiptr size, int index, int column = 0);
	void SetUniformData(const void* data, const qopengl_GLsizeiptr &size, const int &index, const int &column = 0);
	
	GLint * VariablesLocation;
	GLuint Buffer;
protected:
	void SetUniformBuffer();
	GLuint nBindingPoint;
	GLint nBlockSize;
	
	OpenGLWnd* parent;
	const GLchar *name;
	int Distance;
	int ElementsCounter;
	bool OffsetPatern;
};

struct CudaDevice {
	CUdevice DeviceId;
	int nDeviceVersion;
	bool isInitialized;
	CudaDevice(CUdevice CUdev, int nDV){ DeviceId = CUdev; nDeviceVersion = nDV; isInitialized = false; }
};

typedef std::vector<CudaDevice>::iterator CudaDevIt;

class CDataBridge : public QObject {
	Q_OBJECT
public:
	CDataBridge(OpenGLWnd *p){ 
		parent = p;}
	void paintVAO();
	void PrepareVBO();
	void ConnectToDevice();
	static int InitCudaDevice(OpenGLWnd* wnd);
protected:
	OpenGLWnd* parent;
	static std::vector<CudaDevice> DevicesList;
	GLuint				vbo_id[3];
	GLuint				my_vao[1];
	cudaGraphicsResource *resource1[1];
	cudaGraphicsResource *resource2[1];
	VertexData *vertexData;
	GLubyte* vertexColorData;
	GLuint IndexesNumber;

signals:
	void showStatusText(QString str);
};

