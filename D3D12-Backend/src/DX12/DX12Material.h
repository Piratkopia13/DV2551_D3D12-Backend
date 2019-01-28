#pragma once
#include "../Material.h"
#include "DX12.h"
#include <GL/glew.h>
#include <vector>
//#include "DX12ConstantBuffer.h"

class DX12Renderer;

//#define DBOUTW( s )\
//{\
//std::wostringstream os_;\
//os_ << s;\
//OutputDebugStringW( os_.str().c_str() );\
//}
//
//#define DBOUT( s )\
//{\
//std::ostringstream os_;\
//os_ << s;\
//OutputDebugString( os_.str().c_str() );\
//}
//
//// use X = {Program or Shader}
//#define INFO_OUT(S,X) { \
//char buff[1024];\
//memset(buff, 0, 1024);\
//glGet##X##InfoLog(S, 1024, nullptr, buff);\
//DBOUTW(buff);\
//}
//
//// use X = {Program or Shader}
//#define COMPILE_LOG(S,X,OUT) { \
//char buff[1024];\
//memset(buff, 0, 1024);\
//glGet##X##InfoLog(S, 1024, nullptr, buff);\
//OUT=std::string(buff);\
//}


class DX12Material : public Material {
	friend DX12Renderer;

public:
	DX12Material(const std::string& name);
	~DX12Material();


	void setShader(const std::string& shaderFileName, ShaderType type);
	void removeShader(ShaderType type);
	int compileMaterial(std::string& errString);
	int enable();
	void disable();
	//GLuint getProgram() { return program; };
	void setDiffuse(Color c);

	// location identifies the constant buffer in a unique way
	void updateConstantBuffer(const void* data, size_t size, unsigned int location);
	// slower version using a string
	void addConstantBuffer(std::string name, unsigned int location);
	//std::map<unsigned int, DX12ConstantBuffer*> constantBuffers;


	// DX12 specifics
	ID3DBlob* getShaderBlob(Material::ShaderType type);
	D3D12_INPUT_LAYOUT_DESC getInputLayoutDesc();

private:
	int compileShader(ShaderType type, std::string& errString);
	std::string expandShaderText(std::string& shaderText, ShaderType type);

private:
	std::string m_materialName;
	Material::ShaderType m_shaderTypes[4];
	std::string m_shaderNames[4];
	// Compiled shader blobs
	wComPtr<ID3DBlob> m_shaderBlobs[4];
	D3D12_INPUT_LAYOUT_DESC m_inputLayoutDesc;

};

