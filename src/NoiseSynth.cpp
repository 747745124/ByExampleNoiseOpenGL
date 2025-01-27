#if defined(win32)
#include <Windows.h>
#endif
#include "../external/tiny_obj_loader/tiny_obj_loader.h"
#include "../external/imgui/imgui.h"
#include "../external/imgui/imgui_impl_glfw.h"
#include "../external/imgui/imgui_impl_opengl3.h"
#include "./NoiseSynth.h"
#include "./GUI.hpp"
#include "./Setup.hpp"


// main renderloop
void NoiseSynth::renderFrame()
{
	// some options related to imGUI
	static bool wireframe = false;

	showFpsInWindowTitle();

	glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	// draw_debug_pass();
	draw_blend_pass();
	if(!hideGUI)
		drawLightGUI();
}


// configure manually which texture to visualize here
// since all these buffer are not inherited from the same base class
void NoiseSynth::draw_debug_pass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, _windowWidth, _windowHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_debugShader->use();
	_debugShader->setVec2("R",glm::vec2(this->_windowWidth,this->_windowHeight));

	glActiveTexture(GL_TEXTURE0);
	_noiseTexture->bind();

	rq->renderQuad();
}

void NoiseSynth::draw_blend_pass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, _windowWidth, _windowHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_synthShader->use();
	_synthShader->setVec2("R",glm::vec2(this->_windowWidth,this->_windowHeight));
	_synthShader->setInt("blendMode",blendMode);

	glActiveTexture(GL_TEXTURE0);
	_noiseTexture->bind();
	glActiveTexture(GL_TEXTURE1);
	_gaussianTexture->bind();
	rq->renderQuad();
}

void NoiseSynth::handleInput()
{
	// handle input
	if (_keyboardInput.keyStates[GLFW_KEY_SPACE] == GLFW_PRESS)
	{
		hideGUI=!hideGUI;
		_keyboardInput.keyStates[GLFW_KEY_SPACE] = GLFW_RELEASE;
		return;
	}

	if (_keyboardInput.keyStates[GLFW_KEY_TAB] == GLFW_PRESS)
	{	
		std::string path = "../blended/";
		path += input_buffer;
		path += ".png";

		// If you need char*:
		const char* c_path = path.c_str();

		saveScreenshot(c_path, this->_windowWidth,this->_windowHeight);
		_keyboardInput.keyStates[GLFW_KEY_TAB] = GLFW_RELEASE;
		return;
	}
};