
#include "DistortionManager.hpp"

#define STRINGIFY(A) #A

DistortionManager::~DistortionManager()
{

	if (_bIsGLInit)
	{
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE);
		glDebugMessageCallback(nullptr, nullptr);
		glDeleteBuffers(1, &_glIDVertBuffer);
		glDeleteBuffers(1, &_glIDIndexBuffer);

		if (_unLensVAO != 0)
		{
			glDeleteVertexArrays(1, &_unLensVAO);
		}

		_lensShader.unload();
	}


}

void DistortionManager::Init() {
	_bIsGLInit = initGL();
	if (!_bIsGLInit)
	{
		printf("%s - Unable to initialize OpenGL!\n", __FUNCTION__);
		//return false;
	}

	//Kino::app_log.AddLog("DistortionManager initialized.\n");

	//return true;
}

bool DistortionManager::initGL() {

	if (!BindShaders()) {
		Kino::app_log.AddLog("initGL(): Could not create shaders. \n");
		return false;
	}

	if (!SetupDistortionFromFiles()) {
		Kino::app_log.AddLog("initGL(): Could not set up distortion from files. \n");
		return false;
	}

	return true;
}

//See void ofxOpenVR::renderDistortion()
void DistortionManager::Begin()
{

}

void DistortionManager::End()
{

}

bool DistortionManager::SetupDistortionFromFiles()
{
	//if (!_pHMD)
	//	return;

	GLushort _iLensGridSegmentCountH = 43;
	GLushort _iLensGridSegmentCountV = 43;

	float w = (float)(1.0 / float(_iLensGridSegmentCountH - 1));
	float h = (float)(1.0 / float(_iLensGridSegmentCountV - 1));

	float u, v = 0;

	std::vector<VertexDataLens> vVerts(0);
	std::vector<GLushort> vIndices;

	GetStoredDistortion(vVerts, vIndices);

	//std::cout << "Vertex length: " << vVerts.size() << "\n";
	//std::cout << "Index length: " << vIndices.size() << "\n";

	_uiIndexSize = vIndices.size();

	glGenVertexArrays(1, &_unLensVAO);
	glBindVertexArray(_unLensVAO);

	glGenBuffers(1, &_glIDVertBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _glIDVertBuffer);
	glBufferData(GL_ARRAY_BUFFER, vVerts.size() * sizeof(VertexDataLens), &vVerts[0], GL_STATIC_DRAW);

	glGenBuffers(1, &_glIDIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _glIDIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vIndices.size() * sizeof(GLushort), &vIndices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataLens), (void *)offsetof(VertexDataLens, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataLens), (void *)offsetof(VertexDataLens, texCoordRed));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataLens), (void *)offsetof(VertexDataLens, texCoordGreen));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataLens), (void *)offsetof(VertexDataLens, texCoordBlue));

	glBindVertexArray(0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return true;
}

void DistortionManager::RenderDistortion(ofFbo& leftFbo, ofFbo& rightFbo) {

	////Pull the textures from the screen
	//ofTexture left;
	//left.allocate(ofGetWindowWidth() / 2, ofGetWindowHeight(), GL_RGBA);
	//left.loadScreenData(0, 0, ofGetWindowWidth() / 2, ofGetWindowHeight());

	///*ofImage leftImg;
	//leftImg.grabScreen(0, 0, ofGetWidth() / 2, ofGetHeight());
	//ofTexture left = leftImg.getTextureReference();*/

	//ofTexture right;
	//right.allocate(ofGetWindowWidth() / 2, ofGetWindowHeight(), GL_RGBA);
	//right.loadScreenData(ofGetWindowWidth() / 2, 0, ofGetWindowWidth() / 2, ofGetWindowHeight());

	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, ofGetWidth(), ofGetHeight()); //Draw at screen space

	glBindVertexArray(_unLensVAO);
	_lensShader.begin();

	//std::cout << leftFbo.getTextureReference().getWidth() << ", " << leftFbo.getTextureReference().getHeight() << "\n";

	//render left lens (first half of index array )
	//glBindTexture(GL_TEXTURE_2D, leftEyeDesc._nResolveTextureId);
	_lensShader.setUniformTexture("tex0", leftFbo.getTextureReference(), leftFbo.getTextureReference().getTextureData().textureID);
	//_lensShader.setUniformTexture("tex0", left, left.getTextureData().textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glDrawElements(GL_TRIANGLES, _uiIndexSize / 2, GL_UNSIGNED_SHORT, 0);

	//render right lens (second half of index array )
	//glBindTexture(GL_TEXTURE_2D, rightEyeDesc._nResolveTextureId);
	_lensShader.setUniformTexture("tex0", rightFbo.getTextureReference(), rightFbo.getTextureReference().getTextureData().textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glDrawElements(GL_TRIANGLES, _uiIndexSize / 2, GL_UNSIGNED_SHORT, (const void *)(_uiIndexSize));

	glBindVertexArray(0);
	_lensShader.end();

}

/**
* Stores the distorted vertices and indices in a format that we can decode later.
*/
void DistortionManager::StoreDistortion(std::vector<VertexDataLens>& vVerts, std::vector<GLushort>& vIndices)
{
	// ---- Serialize vVerts ----

	// Vector2 holds two floats

	// Every row will be:
	// position x, position y, red x, red y, green x, green y, blue x, blue y
	// (each element separated by one space)

	Kino::app_log.AddLog("Writing distortion vertices file at \"distortion_verts.txt\"... \n");
	ofstream vertsFile("data/distortion_verts.txt");

	for (int i = 0; i < vVerts.size(); i++) {
		VertexDataLens vert = vVerts[i];
		vertsFile << vert.position.x << " ";
		vertsFile << vert.position.y << " ";
		vertsFile << vert.texCoordRed.x << " ";
		vertsFile << vert.texCoordRed.y << " ";
		vertsFile << vert.texCoordGreen.x << " ";
		vertsFile << vert.texCoordGreen.y << " ";
		vertsFile << vert.texCoordBlue.x << " ";
		vertsFile << vert.texCoordBlue.y << "\n";
	}
	vertsFile.close();
	Kino::app_log.AddLog("Done writing vertices file.\n");


	// ---- Serialize vIndices ----
	// Save in a file where 1 row = unsigned short
	Kino::app_log.AddLog("Writing distortion indices file at \"distortion_indices.txt\"... \n");
	ofstream indicesFile("distortion_indices.txt");
	for (int j = 0; j < vIndices.size(); j++) {
		indicesFile << vIndices[j] << "\n";
	}

	indicesFile.close();
	Kino::app_log.AddLog("Done writing indices file.\n");


}

/**
* Decodes the distorted vertices and indices created by StoreDistortion().
*/
bool DistortionManager::GetStoredDistortion(std::vector<VertexDataLens>& vVerts, std::vector<GLushort>& vIndices)
{
	//Read in vVerts
	ifstream vertsFile("data/distortion_verts.txt");
	if (vertsFile.is_open()) {
		string line;
		while (getline(vertsFile, line)) {
			//Construct a VertexDataLens from the line.
			VertexDataLens data;

			std::vector<std::string> elems;
			elems = split(line, ' ');

			if (elems.size() >= 8) {
				data.position.x = std::stof(elems[0]);
				data.position.y = std::stof(elems[1]);
				data.texCoordRed.x = std::stof(elems[2]);
				data.texCoordRed.y = std::stof(elems[3]);
				data.texCoordGreen.x = std::stof(elems[4]);
				data.texCoordGreen.y = std::stof(elems[5]);
				data.texCoordBlue.x = std::stof(elems[6]);
				data.texCoordBlue.y = std::stof(elems[7]);

				vVerts.push_back(data);
			}
			else {
				Kino::app_log.AddLog("Error: not enough elements in input string\n");
				return false;
			}

		}

		vertsFile.close();
	}
	else {
		Kino::app_log.AddLog("Could not open vertices file!\n");
		return false;
	}


	//Read in vIndices
	ifstream indicesFile("data/distortion_indices.txt");
	if (indicesFile.is_open()) {
		string line;
		while (getline(indicesFile, line)) {
			GLushort data = std::stoi(line, nullptr, 10);
			vIndices.push_back(data);
		}
		indicesFile.close();
	}
	else {
		Kino::app_log.AddLog("Could not open indices file!\n");
		return false;
	}

	return true;
}

bool DistortionManager::BindShaders()
{
	_lensShader.setupShaderFromSource(GL_VERTEX_SHADER, GetVertexShader());
	_lensShader.setupShaderFromSource(GL_FRAGMENT_SHADER, GetFragmentShader());
	_lensShader.bindDefaults();
	_lensShader.linkProgram();

	return true;
}

std::string DistortionManager::GetVertexShader() {
	std::string vertex = "#version 410\n";
	vertex += STRINGIFY(
		layout(location = 0) in vec4 position;
		layout(location = 1) in vec2 v2UVredIn;
		layout(location = 2) in vec2 v2UVGreenIn;
		layout(location = 3) in vec2 v2UVblueIn;
		noperspective  out vec2 v2UVred;
		noperspective  out vec2 v2UVgreen;
		noperspective  out vec2 v2UVblue;
		void main()
		{
			v2UVred = v2UVredIn;
			v2UVgreen = v2UVGreenIn;
			v2UVblue = v2UVblueIn;
			gl_Position = position;
		}
	);

	return vertex;
}

std::string DistortionManager::GetFragmentShader() {
	std::string fragment = "#version 410\n";
	fragment += STRINGIFY(
		uniform sampler2D tex0;

		noperspective in vec2 v2UVred;
		noperspective in vec2 v2UVgreen;
		noperspective in vec2 v2UVblue;

		out vec4 outputColor;

		void main()
		{
			float fBoundsCheck = (dot(vec2(lessThan(v2UVgreen.xy, vec2(0.05, 0.05))), vec2(1.0, 1.0)) + dot(vec2(greaterThan(v2UVgreen.xy, vec2(0.95, 0.95))), vec2(1.0, 1.0)));
			if (fBoundsCheck > 1.0)
			{
				//Out of bounds, don't display the color
				outputColor = vec4(0, 0, 0, 1.0);
			}
			else
			{
				float red = texture(tex0, vec2(v2UVred.x, 1.0 - v2UVred.y)).x;
				float green = texture(tex0, vec2(v2UVgreen.x, 1.0 - v2UVgreen.y)).y;
				float blue = texture(tex0, vec2(v2UVblue.x, 1.0 - v2UVblue.y)).z;
				outputColor = vec4(red, green, blue, 1.0);

				//Visualize UV position as RG
				//outputColor = vec4(v2UVred.x, v2UVred.y, 0.0, 1.0);
			}
		}
	);

	return fragment;
}
