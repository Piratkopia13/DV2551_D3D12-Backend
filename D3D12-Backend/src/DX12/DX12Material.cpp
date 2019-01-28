#include <windows.h>
#include <streambuf>
#include <sstream>
#include <istream>
#include <fstream>
#include <vector>
#include <set>
#include <assert.h>

#include "DX12Material.h"

typedef unsigned int uint;

// recursive function to split a string by a delimiter
// easier to read than all that crap using STL...
//void split(const char* text, std::vector<std::string>* const temp, const char delim = ' ') {
//
//	unsigned int delimPos = strcspn(text, (const char*) &delim);
//	if (delimPos == strlen(text)) {
//		temp->push_back(std::string(text));
//	} else {
//		temp->push_back(std::string(text, delimPos));
//		split(text + delimPos, temp, delim);
//	}
//}

/*
	vtx_shader is the basic shader text coming from the .vs file.
	strings will be added to the shader in this order:
	// - version of GLSL
	// - defines from map
	// - existing shader code
*/
std::vector<std::string> DX12Material::expandShaderText(std::string& shaderSource, ShaderType type) {

	//std::vector<std::string> result{ "\n\n #version 450\n\0" };
	//for (auto define : shaderDefines[type])
	//	result.push_back(define);
	//result.push_back(shaderSource);

	std::vector<std::string> result{ "not implemented" };
	return result;
};

DX12Material::DX12Material(const std::string& _name) {
	isValid = false;
	name = _name;
	/*mapShaderEnum[(uint)ShaderType::VS] = GL_VERTEX_SHADER;
	mapShaderEnum[(uint)ShaderType::PS] = GL_FRAGMENT_SHADER;
	mapShaderEnum[(uint)ShaderType::GS] = GL_GEOMETRY_SHADER;
	mapShaderEnum[(uint)ShaderType::CS] = GL_COMPUTE_SHADER;*/
};

DX12Material::~DX12Material() {
	// delete attached constant buffers
	//for (auto buffer : constantBuffers) {
	//	if (buffer.second != nullptr) {
	//		delete(buffer.second);
	//		buffer.second = nullptr;
	//	}
	//}
	//// delete shader objects and program.
	//for (auto shaderObject : shaderObjects) {
	//	glDeleteShader(shaderObject);
	//};
	//glDeleteProgram(program);
};

void DX12Material::setDiffuse(Color c) {

}

void DX12Material::setShader(const std::string& shaderFileName, ShaderType type) {
	if (shaderFileNames.find(type) != shaderFileNames.end()) {
		removeShader(type);
	}
	shaderFileNames[type] = shaderFileName;
};

// this constant buffer will be bound every time we bind the material
void DX12Material::addConstantBuffer(std::string name, unsigned int location) {
	//constantBuffers[location] = new ConstantBufferGL(name, location);
}

// location identifies the constant buffer in a unique way
void DX12Material::updateConstantBuffer(const void* data, size_t size, unsigned int location) {
	//constantBuffers[location]->setData(data, size, this, location);
}

void DX12Material::removeShader(ShaderType type) {
	//GLuint shader = shaderObjects[(GLuint)type];
	//// check if name exists (if it doesn't there should not be a shader here.
	//if (shaderFileNames.find(type) == shaderFileNames.end()) {
	//	assert(shader == 0);
	//	return;
	//} else if (shader != 0 && program != 0) {
	//	glDetachShader(program, shader);
	//	glDeleteShader(shader);
	//};
};

int DX12Material::compileShader(ShaderType type, std::string& errString) {


	std::string str("asd");

	//D3DCompile(str.c_str(), str.length(), nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0, &vsBlob, nullptr);


	//// index in the the array "shaderObject[]";
	//GLuint shaderIdx = (GLuint)type;

	//// open the file and read it to a string "shaderText"
	//std::ifstream shaderFile(shaderFileNames[type]);
	//std::string shaderText;
	//if (shaderFile.is_open()) {
	//	shaderText = std::string((std::istreambuf_iterator<char>(shaderFile)), std::istreambuf_iterator<char>());
	//	shaderFile.close();
	//} else {
	//	errString = "Cannot find file: " + shaderNames[shaderIdx];
	//	return -1;
	//}

	//// make final vector<string> with shader source + defines + GLSL version
	//// in theory this uses move semantics (compiler does it automagically)
	//std::vector<std::string> shaderLines = expandShaderText(shaderText, type);

	//// debug
	//for (auto ex : shaderLines)
	//	DBOUTW(ex.c_str());

	//// OpenGL wants an array of GLchar* with null terminated strings 
	//const GLchar** tempShaderLines = new const GLchar*[shaderLines.size()];
	//int i = 0;
	//for (std::string& text : shaderLines)
	//	tempShaderLines[i++] = text.c_str();

	//GLuint newShader = glCreateShader(mapShaderEnum[shaderIdx]);
	//glShaderSource(newShader, shaderLines.size(), tempShaderLines, nullptr);
	//// ask GL to compile this
	//glCompileShader(newShader);
	//// print error or anything...
	//INFO_OUT(newShader, Shader);
	//std::string err2;
	//COMPILE_LOG(newShader, Shader, err2);
	//shaderObjects[shaderIdx] = newShader;
	return 0;
}

int DX12Material::compileMaterial(std::string& errString) {
	// remove all shaders.
	removeShader(ShaderType::VS);
	removeShader(ShaderType::PS);

	// compile shaders
	std::string err;
	if (compileShader(ShaderType::VS, err) < 0) {
		errString = err;
		exit(-1);
	};
	if (compileShader(ShaderType::PS, err) < 0) {
		errString = err;
		exit(-1);
	};

	//// try to link the program
	//// link shader program (connect vs and ps)
	//if (program != 0)
	//	glDeleteProgram(program);

	//program = glCreateProgram();
	//glAttachShader(program, shaderObjects[(GLuint)ShaderType::VS]);
	//glAttachShader(program, shaderObjects[(GLuint)ShaderType::PS]);
	//glLinkProgram(program);

	//std::string err2;
	//INFO_OUT(program, Program);
	//COMPILE_LOG(program, Program, err2);
	//isValid = true;
	return 0;
};

int DX12Material::enable() {
	/*if (program == 0 || isValid == false)
		return -1;
	glUseProgram(program);

	for (auto cb : constantBuffers) {
		cb.second->bind(this);
	}*/
	return 0;
};

void DX12Material::disable() {
	//glUseProgram(0);
};

//int DX12Material::updateAttribute(
//	ShaderType type,
//	std::string &attribName,
//	void* data,
//	unsigned int size)
//{
//	return 0;
//}