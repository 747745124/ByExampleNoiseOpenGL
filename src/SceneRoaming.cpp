#if defined(win32)
#include <Windows.h>
#endif
#include "../external/tiny_obj_loader/tiny_obj_loader.h"
#include "../external/imgui/imgui.h"
#include "../external/imgui/imgui_impl_glfw.h"
#include "../external/imgui/imgui_impl_opengl3.h"
#include "./SceneRoaming.h"
#include "./GUI.hpp"
#include "./Setup.hpp"


// main renderloop
void SceneRoaming::renderFrame()
{
	// some options related to imGUI
	static bool wireframe = false;

	showFpsInWindowTitle();

	glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	draw_debug_pass();

	draw_blend_pass();
}


// configure manually which texture to visualize here
// since all these buffer are not inherited from the same base class
void SceneRoaming::draw_debug_pass()
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

void SceneRoaming::draw_blend_pass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, _windowWidth, _windowHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_synthShader->use();
	_synthShader->setVec2("R",glm::vec2(this->_windowWidth,this->_windowHeight));

	glActiveTexture(GL_TEXTURE0);
	_noiseTexture->bind();

	rq->renderQuad();
}

void SceneRoaming::handleInput()
{
	// handle input
};