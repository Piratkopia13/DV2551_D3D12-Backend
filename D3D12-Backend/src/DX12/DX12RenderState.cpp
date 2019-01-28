#include <GL/glew.h>
#include "DX12RenderState.h"

DX12RenderState::DX12RenderState() {
	m_wireframe = false;
}

DX12RenderState::~DX12RenderState() {
}

void DX12RenderState::set() {
	// was wireframe mode already set?
	if (*m_globalWireFrame == m_wireframe)
		return;
	else
		*m_globalWireFrame = m_wireframe;

	//if (m_wireframe)
	//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // change to wireframe
	//else
	//	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// change to solid
}

/*
	Keep a pointer to the global wireframe state
*/
void DX12RenderState::setGlobalWireFrame(bool* global) {
	this->m_globalWireFrame = global;
}

bool DX12RenderState::wireframeEnabled() {
	return m_wireframe;
}

/*
 set wireframe mode for this Render state
*/
void DX12RenderState::setWireFrame(bool enabled) {
	m_wireframe = enabled;

}
