#pragma once

#include "ofMain.h"
#include "../helpers/Vectors.h"
#include "KinoGlobals.hpp"
#include <string>

class DistortionManager {
public:

	struct VertexDataScene
	{
		Vector3 position;
		Vector2 texCoord;
	};

	struct VertexDataLens
	{
		Vector2 position;
		Vector2 texCoordRed;
		Vector2 texCoordGreen;
		Vector2 texCoordBlue;
	};

	struct FramebufferDesc
	{
		GLuint _nDepthBufferId;
		GLuint _nRenderTextureId;
		GLuint _nRenderFramebufferId;
		GLuint _nResolveTextureId;
		GLuint _nResolveFramebufferId;
	};
	FramebufferDesc leftEyeDesc;
	FramebufferDesc rightEyeDesc;

	bool _bIsGLInit;
	ofShader _lensShader;
	GLuint _unLensVAO;
	GLuint _glIDVertBuffer;
	GLuint _glIDIndexBuffer;
	unsigned int _uiIndexSize;

	//GLuint leftTexId;
	//GLuint rightTexId;

	uint32_t _nRenderWidth; //window width
	uint32_t _nRenderHeight; //window height

	~DistortionManager();

	void Init();

	bool initGL();

	void Begin();

	void End();

	bool SetupStereoRenderTargets();

	bool SetupDistortionFromFiles();

	bool CreateFrameBuffer(int nWidth, int nHeight, FramebufferDesc & framebufferDesc);

	void RenderDistortion(GLuint& left, GLuint& right);

	void RenderDistortion(ofFbo & leftFbo, ofFbo & rightFbo);

	void RenderStereoTargets(ofFbo& leftFbo, ofFbo& rightFbo);

	void StoreDistortion(std::vector<VertexDataLens>& vVerts, std::vector<GLushort>& vIndices);

	bool GetStoredDistortion(std::vector<VertexDataLens>& vVerts, std::vector<GLushort>& vIndices);

	bool BindShaders();

	std::string GetVertexShader();
	std::string GetFragmentShader();

private:
	void split(const std::string &s, char delim, std::vector<std::string> &elems) {
		std::stringstream ss;
		ss.str(s);
		std::string item;
		while (std::getline(ss, item, delim)) {
			elems.push_back(item);
		}
	}


	std::vector<std::string> split(const std::string &s, char delim) {
		std::vector<std::string> elems;
		split(s, delim, elems);
		return elems;
	}

};