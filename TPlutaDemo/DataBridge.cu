#include "OpenGLWnd.h"
#include <cuda_runtime_api.h>
#include <drvapi_error_string.h>
#include <cuda_gl_interop.h>




std::vector<CudaDevice> CDataBridge::DevicesList;

inline bool ErrorControl(cudaError_t Status)
{
	if (Status != cudaSuccess)
	{
		qDebug("cuda error");
		return false;
	}
	return true;
}


__global__ void FillVBOTest(GLubyte *Buffer, VertexData *Buffer2);

bool compareByVersion(const CudaDevice &a, const CudaDevice &b)
{
	return a.nDeviceVersion < b.nDeviceVersion;
}

int CDataBridge::InitCudaDevice(OpenGLWnd* wnd){
	int			nDeviceMajor = 0;
	int			nDeviceMinor = 0;
	int			nDeviceCounter = 0;

	CUresult ErrorId = cuInit(0);
	if (ErrorId != CUDA_SUCCESS) {
		QMessageBox::information(
			wnd,
			"Init",
			"Can't Init CUDA Framework");
		return 0;
	}

	ErrorId = cuDeviceGetCount(&nDeviceCounter);
	if (ErrorId != CUDA_SUCCESS) {
		QString str = "cuDeviceGetCount returns an error: " + QString(getCudaDrvErrorString(ErrorId));

		QMessageBox::information(
			wnd,
			"Init",
			str);
		return 0;
	}
	if (nDeviceCounter == 0) {
		QMessageBox::information(
			wnd,
			"Init",
			"Can't find CUDA supported Device");
		return 0;
	}

	for (CUdevice CudaDev = 0; CudaDev < nDeviceCounter; CudaDev++)
	{
		ErrorId = cuDeviceComputeCapability(&nDeviceMajor, &nDeviceMinor, CudaDev);
		if (ErrorId != CUDA_SUCCESS) {
			QString str = "cuDeviceComputeCapability returns an error: " + QString(getCudaDrvErrorString(ErrorId));
			QMessageBox::information(
				wnd,
				"Init",
				str);
			return 0;
		}
		if (nDeviceMajor = MinimumRequireMajor && nDeviceMinor >= MinimumRequireMinor || nDeviceMajor > MinimumRequireMajor)
			DevicesList.push_back(CudaDevice(CudaDev, nDeviceMajor * 100 + nDeviceMinor));
	}
	if (DevicesList.size() == 0){
		QMessageBox::information(
			wnd,
			"Init",
			"Cuda version on device is too low ");
		return 0;
	}

	std::sort(DevicesList.begin(), DevicesList.end(), compareByVersion);

	return 1;
}

void CDataBridge::ConnectToDevice(){
	qDebug("connect to cuda dev");
	CudaDevIt nCurrentDevice;

	if (DevicesList.size() == 0){
		qDebug("Init Devices List");
		if (!InitCudaDevice(parent)) return exit(1);

		qDebug("Init Devices List Done");
	}

	nCurrentDevice = DevicesList.begin();

	cudaSetDevice(nCurrentDevice->DeviceId);
	//cudaGLSetGLDevice(nCurrentDevice->DeviceId);
	char strDeviceName[256];
	CUresult ErrorId = cuDeviceGetName(strDeviceName, 256, nCurrentDevice->DeviceId);
	if (ErrorId != CUDA_SUCCESS) {
		qDebug("Cuda Devices Get Name Error Done");
		QString str = "cuDeviceGetName returns an error: " + QString(getCudaDrvErrorString(ErrorId));
		QMessageBox::information(
			parent,
			"Cuda Init",
			str);
		exit(1);
	}
	QString str = QString(strDeviceName) + " is used by application";
	emit showStatusText(str);

}
void CDataBridge::PrepareVBO(){
	//qDebug("return from PrepareVBO"); return;

	const int width = 2048;
	const int height = 1024;

	int nGridSize = width * height;
	
	

	// wygenerowanie dwÛch identyfikatorÛw obiektÛw buforowych
	//if (vbo_id[0] != -1){
	//	parent->glDeleteBuffers(3, vbo_id);
	//	parent->glDeleteVertexArrays(1, my_vao);
	//}
	parent->glGenVertexArrays(1, my_vao);
	parent->glBindVertexArray(my_vao[0]);
	parent->glGenBuffers(2, vbo_id);

	parent->glBindBuffer(GL_ARRAY_BUFFER, vbo_id[0]);
	parent->glBufferData(GL_ARRAY_BUFFER, nGridSize * 12, NULL, GL_DYNAMIC_DRAW);				/////zmiana

	size_t* size = NULL;
	parent->glEnableVertexAttribArray(0);
	parent->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	ErrorControl(cudaGraphicsGLRegisterBuffer(resource1, vbo_id[0], cudaGraphicsRegisterFlagsWriteDiscard));
	ErrorControl(cudaGraphicsMapResources(1, resource1));
	ErrorControl(cudaGraphicsResourceGetMappedPointer((void**)&vertexData, size, resource1[0]));
	ErrorControl(cudaGraphicsUnmapResources(1, resource1));

	parent->glBindBuffer(GL_ARRAY_BUFFER, vbo_id[1]);
	parent->glBufferData(GL_ARRAY_BUFFER, nGridSize * 4, NULL, GL_DYNAMIC_DRAW);				/////zmiana



	parent->glEnableVertexAttribArray(1);
	parent->glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (GLvoid*)0);

	ErrorControl(cudaGraphicsGLRegisterBuffer(resource2, vbo_id[1], cudaGraphicsRegisterFlagsWriteDiscard));
	ErrorControl(cudaGraphicsMapResources(1, resource2));
	ErrorControl(cudaGraphicsResourceGetMappedPointer((void**)&vertexColorData, size, resource2[0]));
	ErrorControl(cudaGraphicsUnmapResources(1, resource2));
	//	GLvoid * buf = glMapBufferRange(GL_ARRAY_BUFFER, 0, GridSize * 3, GL_MAP_WRITE_BIT);

	// skopiowanie danych do drugiego obiektu buforowego
	//	memcpy(buf, vertcol, GridSize * 3);

	// zwolnienie wskaünika na dane drugiego obiektu buforowego
	//	if (glUnmapBuffer(GL_ARRAY_BUFFER) == GL_FALSE)
	//	{
	//		printf("Niepoprawne odwzorowanie obiektu buforowego\n");
	//		exit(0);
	//	}
	ErrorControl(cudaGraphicsMapResources(1, resource2));
	ErrorControl(cudaGraphicsMapResources(1, resource1));
	FillVBOTest << <width, height, 0 >> >(vertexColorData, vertexData);
	ErrorControl(cudaGraphicsUnmapResources(1, resource2));
	ErrorControl(cudaGraphicsUnmapResources(1, resource1));
	//FillVBOBuffer();

	
	//ExitIfCudaGLError("ConstructVAO");

	
	// FillVBOBuffer();
	//*/
	IndexesNumber = 6 * (width - 1)*(height - 1);
	GLuint* Indexes = new unsigned int[IndexesNumber];

	for (int i = 0; i < (width - 1); i++){
		for (int j = 0; j < (height - 1); j++){
			Indexes[6 * (i*(height - 1) + j)] = j + height * i;
			Indexes[6 * (i*(height - 1) + j) + 1] = j + height * (i + 1);
			Indexes[6 * (i*(height - 1) + j) + 2] = j + 1 + height * (i + 1);
			Indexes[6 * (i*(height - 1) + j) + 3] = j + 1 + height * i;
			Indexes[6 * (i*(height - 1) + j) + 4] = j + height * i;
			Indexes[6 * (i*(height - 1) + j) + 5] = j + 1 + height * (i + 1);
		}
	}
	parent->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_id[2]);
	parent->glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexesNumber * sizeof(GLuint), Indexes, GL_STATIC_DRAW);


	delete[] Indexes;
}
void CDataBridge::paintVAO(){
	parent->glBindVertexArray(my_vao[0]);
	parent->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_id[2]);


	parent->glDrawElements(
		GL_TRIANGLES,      // mode
		IndexesNumber,    // count

		GL_UNSIGNED_INT,   // type
		(void*)0           // element array buffer offset
		);


	parent->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
__global__ void FillVBOTest(GLubyte *Buffer, VertexData *Buffer2){

	int y = threadIdx.x;
	int x = blockIdx.x;
	
	int id = y + x*1024;

	float posx = (float)(x - 1024.0f) / 1024.0f;
	float posy = (float)(y - 512.0f) / 512.0f;

	Buffer2[id].x = posx;
	Buffer2[id].y = posy;
	Buffer2[id].z = -sqrt(posx*posx + posy*posy);

	Buffer[4 * (id)] =  GLubyte((1 - sqrt(posx*posx + posy*posy)) * 255);
	Buffer[4 * (id)+1] = 0;
	Buffer[4 * (id)+2] = 0;
	Buffer[4 * (id)+3] = 255;
}
