#include <glad/glad.h>
#include <glad/glad.c>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/trigonometric.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <stb/stb_image.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <iostream>
#include <random>
#include <unordered_map>
#include <fileSystem>
#include <fileSystem/fileSystem.h>


#include "Engine/Application/Application.h"
#include "Engine/GUI/guiMain.h"

#include "Engine/Application/PickingTexture.h"
#include "Engine/Components/Core/Camera.h"
#include "Engine/Components/Core/Mesh.h"
#include "Engine/Components/Core/GameObject.h"
#include "Engine/Shaders/Shader.h"

#include "Engine/GUI/Style/EditorStyle.h"
#include "Engine/Core/Skybox.h"
#include "Engine/Core/Time.h"
#include "Engine/Core/Renderer.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Application/Window.h"
#include "Engine/Application/Callbacks/CallbacksHeader.h"
#include "Engine/Editor/Outline/Outline.h"
#include "Engine/Editor/Editor.h"

#include "Engine/Application/EntryPoint.h"
//Engine::ApplicationClass* Application;

void blurFramebuffer1();
void blurFramebuffer2();
void blurFramebuffer3();

using namespace Engine;
/// ---------------------------------------------------------------------

//std::list<Model> models;

EditorStyle editorStyle;

using namespace Engine::Editor;

Engine::ApplicationClass::ApplicationClass() {
	editorCamera = new Engine::Camera(glm::vec3(0.0f, 0.0f, 5.0f));
	activeCamera = editorCamera;
}
//ApplicationClass* Application = nullptr;
void combineBuffers() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, Application->frameBuffer);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilMask(0xFF);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glViewport(Application->appSizes->sceneStart.x, Application->appSizes->sceneStart.y, Application->appSizes->sceneSize.x, Application->appSizes->sceneSize.y);

	//glClearColor(0.1f, 0.3f, 0.4f, 1.0f);
	//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Application->textureColorbuffer);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, Application->edgeDetectionColorBuffer);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, Application->rbo);
	// Load depth data into the texture

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, Application->blurDepthStencilRBO);

	Application->combiningShader->use();
	Application->combiningShader->setInt("buffer1", 0);
	Application->combiningShader->setInt("buffer2", 1);
	Application->combiningShader->setInt("Depth0", 2);
	Application->combiningShader->setInt("Depth1", 3);
	//glUniform1i(glGetUniformLocation(Application->edgeDetectionShader->ID, "screenTexture"), 0);

	glBindVertexArray(Application->blurVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	//Renderer::Render(*Application->shader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ApplicationClass::CreateApplication() {
	// Initialize GLFW (Window)
	Application->Window = new Engine::WindowClass();
	// Initialize OpenGL, Shaders and Skybox
	InitOpenGL();
	// Initialize Shaders
	Application->shader = new Shader("C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\1.model_loadingVertex.glsl", "C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\1.model_loadingFragment.glsl");
	Application->pickingShader = new Shader("C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\1.model_loadingVertex.glsl", "C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\objectPickingFragment.glsl");
	Application->outlineShader = new Shader("C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\outlining\\outliningVertex.glsl", "C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\outlining\\outliningFragment.glsl");
	Application->outlineBlurShader = new Shader("C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\blur\\blurVertex.glsl", "C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\blur\\blurFragment.glsl");
	Application->combiningShader = new Shader("C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\combining\\combiningVertex.glsl", "C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\combining\\combiningFragment.glsl");
	Application->edgeDetectionShader = new Shader("C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\edgeDetection\\edgeDetectionVertex.glsl", "C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\edgeDetection\\edgeDetectionFragment.glsl");
	Application->singleColorShader = new Shader("C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\singleColor\\singleColorVertex.glsl", "C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\singleColor\\singleColorFragment.glsl");
	Skybox::skyboxShader = new Shader("C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\skybox\\skyboxVertex.glsl", "C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\skybox\\skyboxFragment.glsl");
	InitBlur();

	//Application->InitSkybox();
	Skybox::Init();
	//Initialize ImGui
	Editor::Gui::Init(Application->Window->glfwWindow);
}

void ApplicationClass::Loop() {
	while (!glfwWindowShouldClose(Application->Window->glfwWindow)) {
		// Update time
		Time::Update();
		float currentFrame = static_cast<float>(glfwGetTime());

		// Update Buffers
		if (Application->appSizes->sceneSize != Application->lastAppSizes->sceneSize || Application->appSizes->sceneStart != Application->lastAppSizes->sceneStart) {
			Application->updateBuffers(Application->textureColorbuffer, Application->rbo);
			Application->pickingTexture->updateSize(Application->appSizes->sceneSize);
		}

		// Update inputs
		Callbacks::processInput(Application->Window->glfwWindow);
		glDisable(GL_BLEND);

		Application->pickingTexture->enableWriting();
		//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// Render with Picking Shader
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
		Renderer::Render(*Application->pickingShader);
		Application->pickingTexture->disableWriting();
		glEnable(GL_BLEND);

		// Imgui New Frame
		Gui::NewFrame();

		// Clear buffers
		glBindFramebuffer(GL_FRAMEBUFFER, Application->frameBuffer);
		//glClearColor(0.1f, 0.3f, 0.4f, 1.0f);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// Draw GameObjects
		Renderer::Render(*Application->shader);

		// Draw Shadows

		// Update Skybox
		glBindFramebuffer(GL_FRAMEBUFFER, Application->frameBuffer);
		Skybox::Update();

		//  Draw Outline
		if (Editor::selectedGameObject != nullptr)
		{
			Renderer::RenderOutline(*Application->outlineShader);
			combineBuffers();
		}








		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// Update ImGui
		Gui::Update();

		// Update last frame

		Time::lastFrame = currentFrame;

		// GLFW
		glfwSwapBuffers(Application->Window->glfwWindow);
		glfwPollEvents();

		// Update lastSizes
		Application->lastAppSizes = Application->appSizes;
	}
}

void ApplicationClass::Terminate() {
	Skybox::Terminate();
	Gui::Delete();
	glfwTerminate();
}

void ApplicationClass::InitBlur() {
	float quadVertices[] = {
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};
	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};

	unsigned int& quadVAO = Application->blurVAO;
	unsigned int& quadVBO = Application->blurVBO;
	unsigned int EBO = 0;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

}

void ApplicationClass::InitOpenGL() {
	ApplicationSizes& appSizes = *Application->appSizes;

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	// Init some starting shaders
	Application->shader = new Shader("C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\1.model_loadingVertex.glsl", "C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\1.model_loadingFragment.glsl");
	Application->pickingShader = new Shader("C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\1.model_loadingVertex.glsl", "C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\objectPickingFragment.glsl");
	Application->outlineShader = new Shader("C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\outlining\\outliningVertex.glsl", "C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\outlining\\outliningFragment.glsl");
	Application->outlineBlurShader = new Shader("C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\blur\\blurVertex.glsl", "C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\blur\\blurFragment.glsl");
	Application->edgeDetectionShader = new Shader("C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\edgeDetection\\edgeDetectionVertex.glsl", "C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\edgeDetection\\edgeDetectionFragment.glsl");

	// Create buffers

			/* Edge Detection Framebuffer */
	blurFramebuffer1();
	blurFramebuffer2();
	blurFramebuffer3();
	Application->Shadows = new ShadowsClass();
	Application->Shadows->Init();

#pragma region Framebuffer
	unsigned int& frameBuffer = Application->frameBuffer;
	//unsigned int& textureColorbuffer = Application->textureColorbuffer;
	glGenFramebuffers(1, &Application->frameBuffer);	
	glBindFramebuffer(GL_FRAMEBUFFER, Application->frameBuffer);
	// create a color attachment texture
	glGenTextures(1, &Application->textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, Application->textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, appSizes.sceneSize.x, appSizes.sceneSize.y, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Application->textureColorbuffer, 0);

	// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
	glGenRenderbuffers(1, &Application->rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, Application->rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, appSizes.sceneSize.x, appSizes.sceneSize.y); // use a single renderbuffer object for both a depth AND stencil buffer.
	glViewport(0, 0, appSizes.sceneSize.x, appSizes.sceneSize.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, Application->rbo); // now actually attach it

	GLenum attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_RED_INTEGER };
	glDrawBuffers(3, attachments);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, appSizes.sceneSize.x, appSizes.sceneSize.y);

	Application->pickingTexture = new PickingTexture();
	Application->pickingTexture->init(appSizes.sceneSize.x, appSizes.sceneSize.y);
	glViewport(0, 0, appSizes.sceneSize.x, appSizes.sceneSize.y);
#pragma endregion




	glViewport(0, 0, appSizes.sceneSize.x, appSizes.sceneSize.y);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, appSizes.sceneSize.x, appSizes.sceneSize.y);

	Application->shader->use();
	Application->shader->setInt("texture1", 0);
}

void blurFramebuffer1() {
	ApplicationSizes& appSizes = *Application->appSizes;
	unsigned int& frameBuffer = Application->edgeDetectionFramebuffer;
	unsigned int& textureColorbuffer = Application->edgeDetectionColorBuffer;
	unsigned int& rbo = Application->edgeDetectionDepthStencilRBO;
	//unsigned int& textureColorbuffer = Application->textureColorbuffer;
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	// create a color attachment texture
	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, appSizes.sceneSize.x, appSizes.sceneSize.y, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

	// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, appSizes.sceneSize.x, appSizes.sceneSize.y); // use a single renderbuffer object for both a depth AND stencil buffer.
	glViewport(0, 0, appSizes.sceneSize.x, appSizes.sceneSize.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it

	GLenum attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_RED_INTEGER };
	glDrawBuffers(3, attachments);
	glViewport(0, 0, appSizes.sceneSize.x, appSizes.sceneSize.y);

	Application->edgeDetectionShader->setInt("sceneBuffer", 0);
}

void blurFramebuffer2() {
	ApplicationSizes& appSizes = *Application->appSizes;
	unsigned int& frameBuffer = Application->blurFramebuffer;
	unsigned int& textureColorbuffer = Application->blurColorBuffer;
	unsigned int& rbo = Application->blurDepthStencilRBO;

	//unsigned int& textureColorbuffer = Application->textureColorbuffer;
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	// create a color attachment texture
	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, appSizes.sceneSize.x, appSizes.sceneSize.y, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

	// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, appSizes.sceneSize.x, appSizes.sceneSize.y); // use a single renderbuffer object for both a depth AND stencil buffer.
	glViewport(0, 0, appSizes.sceneSize.x, appSizes.sceneSize.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it

	GLenum attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_RED_INTEGER };
	glDrawBuffers(3, attachments);
	glViewport(0, 0, appSizes.sceneSize.x, appSizes.sceneSize.y);

	Application->outlineBlurShader->setInt("sceneBuffer", 0);
}
void blurFramebuffer3() {
	ApplicationSizes& appSizes = *Application->appSizes;
	unsigned int& frameBuffer = Application->selectedFramebuffer;
	unsigned int& textureColorbuffer = Application->selectedColorBuffer;
	unsigned int& rbo = Application->selectedDepthStencilRBO;

	//unsigned int& textureColorbuffer = Application->textureColorbuffer;
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	// create a color attachment texture
	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, appSizes.sceneSize.x, appSizes.sceneSize.y, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

	// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, appSizes.sceneSize.x, appSizes.sceneSize.y); // use a single renderbuffer object for both a depth AND stencil buffer.
	glViewport(0, 0, appSizes.sceneSize.x, appSizes.sceneSize.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it

	GLenum attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_RED_INTEGER };
	glDrawBuffers(3, attachments);
	glViewport(0, 0, appSizes.sceneSize.x, appSizes.sceneSize.y);
}

/* Updates ? ------------------------------------------------------------------- MUST REVIEW IT*/
void ApplicationClass::updateBuffers(GLuint textureColorBuffer, GLuint rbo) {
	{
		ApplicationSizes& appSizes = *Application->appSizes;
		glBindTexture(GL_TEXTURE_2D, Application->textureColorbuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, appSizes.sceneSize.x, appSizes.sceneSize.y, 0, GL_RGB, GL_FLOAT, nullptr);

		glBindTexture(GL_TEXTURE_2D, Application->edgeDetectionColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, appSizes.sceneSize.x, appSizes.sceneSize.y, 0, GL_RGB, GL_FLOAT, nullptr);

		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, appSizes.sceneSize.x, appSizes.sceneSize.y);

		glBindRenderbuffer(GL_RENDERBUFFER, Application->edgeDetectionDepthStencilRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, appSizes.sceneSize.x, appSizes.sceneSize.y);

		glViewport(appSizes.sceneStart.x, appSizes.sceneStart.y, appSizes.sceneSize.x, appSizes.sceneSize.y);
	}
}
