#include "GameTechRenderer.h"
#include "GameObject.h"
#include "ToonGameObject.h"
#include "RenderObject.h"
#include "ToonRenderObject.h"
#include "ToonFollowCamera.h"
#include "ToonLevelManager.h"
#include "ToonUtils.h"
#include "TextureLoader.h"
#include "ImpactPoint.h"
#include "PaintableObject.h"
#include "ToonUtils.h"
#include <iostream>
#include <algorithm>
#include "ToonAssetManager.h"

#include "../ThirdParty/imgui/imgui.h"
#include "../ThirdParty/imgui/imgui_impl_opengl3.h"
#include "../ThirdParty/imgui/imgui_impl_win32.h"

using namespace NCL;
using namespace Rendering;
using namespace CSC8503;

#define SHADOWSIZE 4096



Matrix4 biasMatrix = Matrix4::Translation(Vector3(0.5f, 0.5f, 0.5f)) * Matrix4::Scale(Vector3(0.5f, 0.5f, 0.5f));

GameTechRenderer::GameTechRenderer() : OGLRenderer(*Window::GetWindow())
{
	ToonAssetManager::Instance().LoadAssets();
	SetupStuffs();
	team1Percentage = 0;
	team2Percentage = 0;
	team3Percentage = 0;
	team4Percentage = 0;
}

GameTechRenderer::~GameTechRenderer()	{
	glDeleteTextures(1, &shadowTex);
	glDeleteFramebuffers(1, &shadowFBO);
}

void NCL::CSC8503::GameTechRenderer::SetupStuffs()
{
	glEnable(GL_DEPTH_TEST);
	debugShader = ToonAssetManager::Instance().GetShader("debug");
	shadowShader = ToonAssetManager::Instance().GetShader("shadow");
	minimapShader = ToonAssetManager::Instance().GetShader("minimap");
	textureShader = ToonAssetManager::Instance().GetShader("texture");
	sceneShader = ToonAssetManager::Instance().GetShader("scene");
	scoreBarShader = ToonAssetManager::Instance().GetShader("scoreBar");
	mapShader = ToonAssetManager::Instance().GetShader("fullMap");

	GenerateShadowFBO();
	GenerateSceneFBO(windowWidth, windowHeight);
	GenerateMinimapFBO(windowWidth, windowHeight);
	GenerateMapFBO(windowWidth, windowHeight);
	GenerateAtomicBuffer();
	glClearColor(1, 1, 1, 1);

	//Set up the light properties
	lightColour = Vector4(0.8f, 0.8f, 0.5f, 1.0f);
	lightRadius = 1000.0f;
	lightPosition = Vector3(-200.0f, 60.0f, -200.0f);

	//Skybox!
	skyboxShader = ToonAssetManager::Instance().GetShader("skybox");
	skyboxMesh = new OGLMesh();
	skyboxMesh->SetVertexPositions({ Vector3(-1, 1,-1), Vector3(-1,-1,-1) , Vector3(1,-1,-1) , Vector3(1,1,-1) });
	skyboxMesh->SetVertexIndices({ 0,1,2,2,3,0 });
	skyboxMesh->UploadToGPU();

	fullScreenQuad = new OGLMesh();
	fullScreenQuad->SetVertexPositions({ Vector3(-1, 1,1), Vector3(-1,-1,1) , Vector3(1,-1,1) , Vector3(1,1,1) });
	fullScreenQuad->SetVertexTextureCoords({ Vector2(0.0f,1.0f), Vector2(0.0f,0.0f), Vector2(1.0f,0.0f), Vector2(1.0f,1.0f) });
	fullScreenQuad->SetVertexIndices({ 0,1,2,2,3,0 });
	fullScreenQuad->UploadToGPU();
	
	minimapQuad = new OGLMesh();
	minimapQuad->SetVertexPositions({ Vector3(-1, 1,-1), Vector3(-1,-1,-1) , Vector3(1,-1,-1) , Vector3(1,1,-1) });
	minimapQuad->SetVertexTextureCoords({ Vector2(0.0f,1.0f), Vector2(0.0f,0.0f), Vector2(1.0f,0.0f), Vector2(1.0f,1.0f) });
	minimapQuad->SetVertexIndices({ 0,1,2,2,3,0 });
	minimapQuad->UploadToGPU();

	minimapStencilQuad = new OGLMesh();
	minimapStencilQuad->SetVertexPositions({ Vector3(-0.5f, 0.8f, -1.0f), Vector3(-0.5f, -0.8f, -1.0f) , Vector3(0.5f, -0.8f, -1.0f) , Vector3(0.5f, 0.8f, -1.0f) });
	minimapStencilQuad->SetVertexIndices({ 0,1,2,2,3,0 });
	minimapStencilQuad->UploadToGPU();

	scoreQuad = new OGLMesh();
	scoreQuad->SetVertexPositions({ Vector3(-1, 1, 1), Vector3(-1, -1, 1), Vector3(1, -1, 1), Vector3(1, 1, 1) });
	scoreQuad->SetVertexTextureCoords({ Vector2(0.0f,1.0f), Vector2(0.0f,0.0f), Vector2(1.0f,0.0f), Vector2(1.0f,1.0f) });
	scoreQuad->SetVertexIndices({ 0,1,2,2,3,0 });
	scoreQuad->UploadToGPU();
	
	LoadSkybox();

	glGenVertexArrays(1, &lineVAO);
	glGenVertexArrays(1, &textVAO);

	glGenBuffers(1, &lineVertVBO);
	glGenBuffers(1, &textVertVBO);
	glGenBuffers(1, &textColourVBO);
	glGenBuffers(1, &textTexVBO);

	SetDebugStringBufferSizes(10000);
	SetDebugLineBufferSizes(1000);



}

void NCL::CSC8503::GameTechRenderer::GenerateShadowFBO()
{
	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GameTechRenderer::GenerateSceneFBO(int width, int height)
{
	glGenFramebuffers(1, &sceneFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);

	glGenTextures(1, &sceneColourTexture);
	glBindTexture(GL_TEXTURE_2D, sceneColourTexture);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneColourTexture, 0);
	glObjectLabel(GL_TEXTURE, sceneColourTexture, -1, "Scene Colour Texture");

	glGenTextures(1, &sceneDepthTexture);
	glBindTexture(GL_TEXTURE_2D, sceneDepthTexture);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, sceneDepthTexture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, sceneDepthTexture, 0);

	glObjectLabel(GL_TEXTURE, sceneDepthTexture, -1, "Scene Depth Texture");



	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE || !sceneColourTexture || !sceneColourTexture) {
		return;
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GameTechRenderer::GenerateMinimapFBO(int width, int height)
{
	glGenFramebuffers(1, &minimapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, minimapFBO);

	glGenTextures(1, &minimapColourTexture);
	glBindTexture(GL_TEXTURE_2D, minimapColourTexture);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, minimapColourTexture, 0);
	glObjectLabel(GL_TEXTURE, minimapColourTexture, -1, "Minimap Colour Texture");

	glGenTextures(1, &minimapDepthTexture);
	glBindTexture(GL_TEXTURE_2D, minimapDepthTexture);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, minimapDepthTexture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, minimapDepthTexture, 0);

	glObjectLabel(GL_TEXTURE, minimapDepthTexture, -1, "Minimap Depth Texture");

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE || !minimapColourTexture || !minimapColourTexture) {
		return;
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void GameTechRenderer::GenerateMapFBO(int width, int height)
{
	glGenFramebuffers(1, &mapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, mapFBO);

	glGenTextures(1, &mapColourTexture);
	glBindTexture(GL_TEXTURE_2D, mapColourTexture);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mapColourTexture, 0);
	glObjectLabel(GL_TEXTURE, mapColourTexture, -1, "Mainmap Colour Texture");


	glGenTextures(1, &mapDepthTexture);
	glBindTexture(GL_TEXTURE_2D, mapDepthTexture);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mapDepthTexture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, mapDepthTexture, 0);

	glObjectLabel(GL_TEXTURE, mapDepthTexture, -1, "Mainmap Depth Texture");


	glGenTextures(1, &mapScoreTexture);
	glBindTexture(GL_TEXTURE_2D, mapScoreTexture);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mapScoreTexture, 0);

	glObjectLabel(GL_TEXTURE, mapScoreTexture, -1, "Mainmap Score Texture");


	GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);


	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE || !mapColourTexture || !mapScoreTexture) {
		return;
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void GameTechRenderer::LoadSkybox() {
	string filenames[6] = {
		"/Cubemap/skyrender0004.png",
		"/Cubemap/skyrender0001.png",
		"/Cubemap/skyrender0003.png",
		"/Cubemap/skyrender0006.png",
		"/Cubemap/skyrender0002.png",
		"/Cubemap/skyrender0005.png"
	};

	int width[6] = { 0 };
	int height[6] = { 0 };
	int channels[6] = { 0 };
	int flags[6] = { 0 };

	vector<char*> texData(6, nullptr);

	for (int i = 0; i < 6; ++i) {
		TextureLoader::LoadTexture(filenames[i], texData[i], width[i], height[i], channels[i], flags[i]);
		if (i > 0 && (width[i] != width[0] || height[0] != height[0])) {
			std::cout << __FUNCTION__ << " cubemap input textures don't match in size?\n";
			return;
		}
	}
	glGenTextures(1, &skyboxTex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);

	GLenum type = channels[0] == 4 ? GL_RGBA : GL_RGB;

	for (int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width[i], height[i], 0, type, GL_UNSIGNED_BYTE, texData[i]);
	}

	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void GameTechRenderer::RenderFrame() {
	if (!gameWorld) return; // Safety Check

	DrawMainScene();
	if (gameWorld->GetMapCamera()) {
		DrawMap();

	}
	if (gameWorld->GetMinimapCamera())
	{
		
		DrawMinimap();
	}
	PresentScene();
	
	
	RenderImGUI();

}

void NCL::CSC8503::GameTechRenderer::DrawMainScene()
{
	glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneColourTexture, 0);
	glEnable(GL_CULL_FACE);
	glClearColor(1, 1, 1, 1);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	BuildObjectList();
	RenderShadowMap();
	RenderSkybox();
	
	float screenAspect = (float)windowWidth / (float)windowHeight;
	Matrix4 viewMatrix = gameWorld->GetMainCamera()->BuildViewMatrix();
	Matrix4 projMatrix = gameWorld->GetMainCamera()->BuildProjectionMatrix(screenAspect);
	RenderScene(sceneShader, viewMatrix, projMatrix);

	glDisable(GL_CULL_FACE); //Todo - text indices are going the wrong way...
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	NewRenderLines();
	NewRenderLinesOnOrthographicView();
	NewRenderText();
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void NCL::CSC8503::GameTechRenderer::DrawMinimap()
{
	glBindFramebuffer(GL_FRAMEBUFFER, minimapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, minimapColourTexture, 0);

	glEnable(GL_CULL_FACE);
	glClearColor(1, 1, 1, 1);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	float screenAspect = (float)windowWidth / (float)windowHeight;
	Matrix4 viewMatrix = gameWorld->GetMinimapCamera()->BuildViewMatrix();
	Matrix4 projMatrix = gameWorld->GetMinimapCamera()->BuildProjectionMatrix(screenAspect);
	RenderScene(minimapShader, viewMatrix, projMatrix);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void NCL::CSC8503::GameTechRenderer::RenderImGUI()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Debug Window");
	if (ImGui::CollapsingHeader("Camera"))
	{
		ToonFollowCamera* followCamera = (ToonFollowCamera*)(gameWorld->GetMainCamera());

		Vector3 cPos = gameWorld->GetMainCamera()->GetPosition();
		Vector3 cRot(gameWorld->GetMainCamera()->GetPitch(), gameWorld->GetMainCamera()->GetYaw(), 0);
		Vector3 cFollowOffset = followCamera->GetFollowOffset();
		Vector3 cTargetOffset = followCamera->GetTargetOffset();
		Vector3 cAimOffset = followCamera->GetAimOffset();

		float distance = followCamera->GetFollowDistance();
		float smoothness = followCamera->GetSmoothness();
		float cPitchOffset = followCamera->GetPitchOffset();

		if (ImGui::DragFloat3("Cam Position", (float*)&cPos)) gameWorld->GetMainCamera()->SetPosition(cPos);
		if (ImGui::DragFloat("Cam Pitch", (float*)&cRot.x)) gameWorld->GetMainCamera()->SetPitch(cPos.x);
		if (ImGui::DragFloat("Cam Yaw", (float*)&cRot.y)) gameWorld->GetMainCamera()->SetYaw(cPos.y);
		if (ImGui::DragFloat("Pitch Offset", (float*)&cPitchOffset)) followCamera->SetPitchOffset(cPitchOffset);

		if (ImGui::DragFloat("Follow Distance", (float*)&distance)) followCamera->SetFollowDistance(distance);
		if (ImGui::DragFloat("Follow Smoothness", (float*)&smoothness)) followCamera->SetSmoothness(smoothness);
		if (ImGui::DragFloat3("Follow Offset", (float*)&cFollowOffset)) followCamera->SetFollowOffset(cFollowOffset);
		if (ImGui::DragFloat3("Target Offset", (float*)&cTargetOffset)) followCamera->SetTargetOffset(cTargetOffset);
		if (ImGui::DragFloat3("Aim Offset", (float*)&cAimOffset)) followCamera->SetAimOffset(cAimOffset);
	}
	/*if (ImGui::CollapsingHeader("Player"))
	{
		Player* player = ToonLevelManager::Get()->GetPlayer();
		Vector3 playerPos = ToonUtils::ConvertToNCLVector3(player->GetRigidbody()->getTransform().getPosition());

		ImGui::DragFloat3("Position", (float*)(&playerPos));
	}*/
	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void NCL::CSC8503::GameTechRenderer::DrawMap()
{
	glBindFramebuffer(GL_FRAMEBUFFER, mapFBO);
	
	glEnable(GL_CULL_FACE);
	glClearColor(1, 1, 1, 1);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	float screenAspect = (float)windowWidth / (float)windowHeight;
	Matrix4 viewMatrix = gameWorld->GetMapCamera()->BuildViewMatrix();
	Matrix4 projMatrix = gameWorld->GetMapCamera()->BuildProjectionMatrix(screenAspect);

	

	RenderScene(mapShader, viewMatrix, projMatrix);

	currentAtomicCPU = ((currentAtomicCPU + 1) % 3);
	currentAtomicGPU = ((currentAtomicGPU + 1) % 3);
	curretAtomicReset = ((curretAtomicReset + 1) % 3);
	
	


	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GameTechRenderer::BuildObjectList() {
	activeObjects.clear();

	gameWorld->OperateOnContents(
		[&](ToonGameObject* o) 
		{
			if (o->IsActive()) 
			{
				o->CalculateModelMatrix();
				activeObjects.emplace_back(o);
			}
		}
	);
}


void GameTechRenderer::PresentScene()
{
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glDisable(GL_CULL_FACE); //Todo - text indices are going the wrong way...
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	BindShader(textureShader);
	Matrix4 identityMatrix = Matrix4();
	
	int projLocation = glGetUniformLocation(textureShader->GetProgramID(), "projMatrix");
	int viewLocation = glGetUniformLocation(textureShader->GetProgramID(), "viewMatrix");
	int modelLocation = glGetUniformLocation(textureShader->GetProgramID(), "modelMatrix");

	glUniformMatrix4fv(modelLocation, 1, false, (float*)&identityMatrix);
	glUniformMatrix4fv(viewLocation, 1, false, (float*)&identityMatrix);
	glUniformMatrix4fv(projLocation, 1, false, (float*)&identityMatrix);

	PresentGameScene();
	
	PresentMinimap(modelLocation);

	if (gameWorld->GetMapCamera()) {
		DrawScoreBar();

	}
	
	
}

void NCL::CSC8503::GameTechRenderer::PresentGameScene()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sceneColourTexture);
	glUniform1i(glGetUniformLocation(textureShader->GetProgramID(), "diffuseTex"), 0);
	BindMesh(fullScreenQuad);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

}

void NCL::CSC8503::GameTechRenderer::DrawScoreBar() {
	BindShader(scoreBarShader);

	RetrieveAtomicValues();

	
	glUniform1f(glGetUniformLocation(scoreBarShader->GetProgramID(), "team1PercentageOwned"), team1Percentage);
	glUniform1f(glGetUniformLocation(scoreBarShader->GetProgramID(), "team2PercentageOwned"), team2Percentage);
	glUniform1f(glGetUniformLocation(scoreBarShader->GetProgramID(), "team3PercentageOwned"), team3Percentage);
	glUniform1f(glGetUniformLocation(scoreBarShader->GetProgramID(), "team4PercentageOwned"), team4Percentage);

	glUniform3fv(glGetUniformLocation(scoreBarShader->GetProgramID(), "defaultGray"), 1, defaultColour.array);
	glUniform3fv(glGetUniformLocation(scoreBarShader->GetProgramID(), "team1Colour"), 1, teamColours[0].array);
	glUniform3fv(glGetUniformLocation(scoreBarShader->GetProgramID(), "team2Colour"), 1, teamColours[1].array);
	glUniform3fv(glGetUniformLocation(scoreBarShader->GetProgramID(), "team3Colour"), 1, teamColours[2].array);
	glUniform3fv(glGetUniformLocation(scoreBarShader->GetProgramID(), "team4Colour"), 1, teamColours[3].array);
	
	Matrix4 identityMatrix = Matrix4();

	int projLocation = glGetUniformLocation(scoreBarShader->GetProgramID(), "projMatrix");
	int viewLocation = glGetUniformLocation(scoreBarShader->GetProgramID(), "viewMatrix");
	int modelLocation = glGetUniformLocation(scoreBarShader->GetProgramID(), "modelMatrix");

	glUniformMatrix4fv(modelLocation, 1, false, (float*)&identityMatrix);
	glUniformMatrix4fv(viewLocation, 1, false, (float*)&identityMatrix);
	glUniformMatrix4fv(projLocation, 1, false, (float*)&identityMatrix);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	Matrix4 scoreBarModelMatrix = Matrix4::Translation(Vector3(0, 0.85f, 0)) * Matrix4::Scale(Vector3(0.4f, 0.035f, 1));
	glUniformMatrix4fv(modelLocation, 1, false, (float*)&scoreBarModelMatrix);

	BindMesh(scoreQuad);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

void NCL::CSC8503::GameTechRenderer::CalculatePercentages(const int& totalPixels, const int& team1Pixels, const int& team2Pixels, const int& team3Pixels, const int& team4Pixels) {
	float totalPaintedPixels = (float)team1Pixels + (float)team2Pixels + (float)team3Pixels + (float)team4Pixels;
	if (totalPaintedPixels != 0) {
		team1Percentage = (float)team1Pixels / totalPaintedPixels;
		team2Percentage = (float)team2Pixels / totalPaintedPixels;
		team3Percentage = (float)team3Pixels / totalPaintedPixels;
		team4Percentage = (float)team4Pixels / totalPaintedPixels;
	}
	else {
		team1Percentage = 0;
		team2Percentage = 0;
		team3Percentage = 0;
		team4Percentage = 0;
	}
}

void NCL::CSC8503::GameTechRenderer::PresentMinimap(int modelLocation)
{
	if (!gameWorld->GetMinimapCamera()) return;
	glEnable(GL_STENCIL_TEST);

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glStencilFunc(GL_ALWAYS, 2, ~0);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	Matrix4 minimapModelMatrix = Matrix4::Translation(Vector3(-0.8f, -0.7f, 0.0f)) * Matrix4::Scale(Vector3(0.3f, 0.3f, 1.0f));
	glUniformMatrix4fv(modelLocation, 1, false, (float*)&minimapModelMatrix);
	
	glBindTexture(GL_TEXTURE_2D, minimapColourTexture);
	glUniform1i(glGetUniformLocation(textureShader->GetProgramID(), "diffuseTex"), 0);
	
	BindMesh(minimapStencilQuad);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glStencilFunc(GL_EQUAL, 2, ~0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glDisable(GL_DEPTH_TEST);
	BindMesh(minimapQuad);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_STENCIL_TEST);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GameTechRenderer::SortObjectList() {

}

void GameTechRenderer::RenderShadowMap() {
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);

	glCullFace(GL_FRONT);

	BindShader(shadowShader);
	int mvpLocation = glGetUniformLocation(shadowShader->GetProgramID(), "mvpMatrix");
	int hasSkinLocation = glGetUniformLocation(shadowShader->GetProgramID(), "hasSkin");

	Matrix4 shadowViewMatrix = Matrix4::BuildViewMatrix(lightPosition, Vector3(0, 0, 0), Vector3(0, 1, 0));
	Matrix4 shadowProjMatrix = Matrix4::Perspective(100.0f, 500.0f, 1, 45.0f);

	Matrix4 mvMatrix = shadowProjMatrix * shadowViewMatrix;

	shadowMatrix = biasMatrix * mvMatrix; //we'll use this one later on

	for (const auto& i : activeObjects)
	{
		/*Quaternion rot;
		reactphysics3d::Quaternion rRot = (*i).GetRigidbody()->getTransform().getOrientation();
		rot.x = rRot.x;
		rot.y = rRot.y;
		rot.z = rRot.z;
		rot.w = rRot.w;
		Matrix4 modelMatrix = Matrix4::Translation((*i).GetRigidbody()->getTransform().getPosition().x,
			(*i).GetRigidbody()->getTransform().getPosition().y,
			(*i).GetRigidbody()->getTransform().getPosition().z) *
			Matrix4(rot) *
			Matrix4::Scale((*i).GetRenderObject()->GetTransform()->GetScale().x, (*i).GetRenderObject()->GetTransform()->GetScale().y, (*i).GetRenderObject()->GetTransform()->GetScale().z);*/

		Matrix4 modelMatrix = (*i).GetModelMatrix();
		Matrix4 mvpMatrix = mvMatrix * modelMatrix;
		glUniformMatrix4fv(mvpLocation, 1, false, (float*)&mvpMatrix);
		glUniform1i(hasSkinLocation, (*i).HasSkin());

		(*i).Draw(*this);
		/*BindMesh((*i).GetRenderObject()->GetMesh());
		int layerCount = (*i).GetRenderObject()->GetMesh()->GetSubMeshCount();
		for (int j = 0; j < layerCount; ++j) {
			i->Draw(j);
		}*/

		/*Jainesh - Moved to ToonGameObject.h Draw() func
		BindMesh((*i).GetRenderObject()->GetMesh());
		int layerCount = (*i).GetRenderObject()->GetMesh()->GetSubMeshCount();
		for (int i = 0; i < layerCount; ++i) {
			DrawBoundMesh(i);
		}*/
	}

	glViewport(0, 0, windowWidth, windowHeight);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);

	glCullFace(GL_BACK);
}

void GameTechRenderer::RenderSkybox() {
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	float screenAspect = (float)windowWidth / (float)windowHeight;
	Matrix4 viewMatrix = gameWorld->GetMainCamera()->BuildViewMatrix();
	Matrix4 projMatrix = gameWorld->GetMainCamera()->BuildProjectionMatrix(screenAspect);

	BindShader(skyboxShader);

	int projLocation = glGetUniformLocation(skyboxShader->GetProgramID(), "projMatrix");
	int viewLocation = glGetUniformLocation(skyboxShader->GetProgramID(), "viewMatrix");
	int texLocation = glGetUniformLocation(skyboxShader->GetProgramID(), "cubeTex");

	glUniformMatrix4fv(projLocation, 1, false, (float*)&projMatrix);
	glUniformMatrix4fv(viewLocation, 1, false, (float*)&viewMatrix);

	glUniform1i(texLocation, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);

	BindMesh(skyboxMesh);
	DrawBoundMesh();

	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

void GameTechRenderer::RenderScene(OGLShader* shader, Matrix4 viewMatrix, Matrix4 projMatrix)
{
	BindShader(shader);
	for (const auto& i : activeObjects) {
		if ((*i).GetRenderObject() == nullptr) continue;
		if (shader != minimapShader && shader != mapShader)
		{
			shader = (OGLShader*)(*i).GetRenderObject()->GetShader();
			BindShader(shader);
		}

		int projLocation = glGetUniformLocation(shader->GetProgramID(), "projMatrix");
		int viewLocation = glGetUniformLocation(shader->GetProgramID(), "viewMatrix");
		int modelLocation = glGetUniformLocation(shader->GetProgramID(), "modelMatrix");
		int colourLocation = glGetUniformLocation(shader->GetProgramID(), "objectColour");
		int hasVColLocation = glGetUniformLocation(shader->GetProgramID(), "hasVertexColours");
		int hasTexLocation = glGetUniformLocation(shader->GetProgramID(), "hasTexture");
		int objectPosLocation = glGetUniformLocation(shader->GetProgramID(), "objectPosition");

		if ((i)->GetRigidbody()->getMass() != 0.0f && shader == mapShader) continue;
		BindTextureToShader((OGLTexture*)(*i).GetRenderObject()->GetDefaultTexture(), "mainTex", 0);

		ToonGameObject* linkedObject = (*i).GetRenderObject()->GetGameObject();
		if (dynamic_cast<PaintableObject*>(linkedObject)) {

			PaintableObject* paintedObject = (PaintableObject*)linkedObject;
			int isFloorLocation = glGetUniformLocation(shader->GetProgramID(), "isFloor");
			glUniform1i(isFloorLocation, paintedObject->IsObjectTheFloor() ? 1 : 0);
			PassImpactPointDetails(paintedObject, shader);
		}
		else {
			int impactPointCountLocation = glGetUniformLocation(shader->GetProgramID(), "impactPointCount");
			glUniform1i(impactPointCountLocation, 0);
		}
		if (shader == mapShader) {
			// MAKE COLOUR WORK
			int atomicLocation = glGetUniformLocation(shader->GetProgramID(), "currentAtomicTarget");
			glUniform1i(atomicLocation, currentAtomicGPU);

			glUniform3fv(glGetUniformLocation(shader->GetProgramID(), "team1Colour"), 1, teamColours[0].array);
			glUniform3fv(glGetUniformLocation(shader->GetProgramID(), "team2Colour"), 1, teamColours[1].array);
			glUniform3fv(glGetUniformLocation(shader->GetProgramID(), "team3Colour"), 1, teamColours[2].array);
			glUniform3fv(glGetUniformLocation(shader->GetProgramID(), "team4Colour"), 1, teamColours[3].array);
		}

		glUniformMatrix4fv(projLocation, 1, false, (float*)&projMatrix);
		glUniformMatrix4fv(viewLocation, 1, false, (float*)&viewMatrix);

		Vector3 objPos = ToonUtils::ConvertToNCLVector3((i)->GetRigidbody()->getTransform().getPosition());
		glUniform3fv(objectPosLocation, 1, objPos.array);


		/*Quaternion rot;
		reactphysics3d::Quaternion rRot = (*i).GetRigidbody()->getTransform().getOrientation();
		rot.x = rRot.x;
		rot.y = rRot.y;
		rot.z = rRot.z;
		rot.w = rRot.w;

		Matrix4 modelMatrix = Matrix4::Translation((*i).GetRigidbody()->getTransform().getPosition().x,
			(*i).GetRigidbody()->getTransform().getPosition().y,
			(*i).GetRigidbody()->getTransform().getPosition().z) *

			Matrix4(rot) *

			Matrix4::Scale((*i).GetRenderObject()->GetTransform()->GetScale().x, (*i).GetRenderObject()->GetTransform()->GetScale().y, (*i).GetRenderObject()->GetTransform()->GetScale().z);*/

		Matrix4 modelMatrix = (*i).GetModelMatrix();
		glUniformMatrix4fv(modelLocation, 1, false, (float*)&modelMatrix);

		Vector4 colour = i->GetRenderObject()->GetColour();
		glUniform4fv(colourLocation, 1, colour.array);

		glUniform1i(hasVColLocation, !(*i).GetRenderObject()->GetMesh()->GetColourData().empty());

		glUniform1i(hasTexLocation, (OGLTexture*)(*i).GetRenderObject()->GetDefaultTexture() ? 1 : 0);

		//Jainesh - This too was moved to Draw func
		/*MeshGeometry* boundMesh = nullptr;
		if (shader == minimapShader && (*i).GetRenderObject()->GetMinimapMesh() != nullptr) {
			boundMesh = (*i).GetRenderObject()->GetMinimapMesh();
		}
		else {
			boundMesh = (*i).GetRenderObject()->GetMesh();
		}
		BindMesh(boundMesh);
		int layerCount = boundMesh->GetSubMeshCount();
		for (int i = 0; i < layerCount; ++i) {
			DrawBoundMesh(i);
		}*/		

		(*i).Draw(*this, shader == minimapShader && (*i).GetRenderObject()->GetMinimapMesh() != nullptr);
	}
}

void GameTechRenderer::PassImpactPointDetails(PaintableObject* const& paintedObject, OGLShader* shader)
{
	int impactPointsLocation = 0;
	int impactPointCountLocation = glGetUniformLocation(shader->GetProgramID(), "impactPointCount");

	std::deque<ImpactPoint>* objImpactPoints = paintedObject->GetImpactPoints(); //change to reference at some point

	glUniform1i(impactPointCountLocation, (GLint)objImpactPoints->size());

	if (objImpactPoints->empty()) return;

	GLuint i = 0;
	for (ImpactPoint& point : *objImpactPoints) {
		char buffer[64];

		sprintf_s(buffer, "impactPoints[%i].position", i);
		impactPointsLocation = glGetUniformLocation(shader->GetProgramID(), buffer);
		Vector3 impactLocation = point.GetImpactLocation();
		glUniform3fv(impactPointsLocation, 1, (float*)&impactLocation);

		sprintf_s(buffer, "impactPoints[%i].colour", i);
		impactPointsLocation = glGetUniformLocation(shader->GetProgramID(), buffer);
		Vector3 impactColour = point.GetImpactColour();
		glUniform3fv(impactPointsLocation, 1, (float*)&impactColour);

		sprintf_s(buffer, "impactPoints[%i].radius", i);
		impactPointsLocation = glGetUniformLocation(shader->GetProgramID(), buffer);
		glUniform1f(impactPointsLocation, point.GetImpactRadius());

		i++;
	}

	
}

void GameTechRenderer::NewRenderLines() {
	const std::vector<Debug::DebugLineEntry>& lines = Debug::GetDebugLines();
	if (lines.empty()) {
		return;
	}
	float screenAspect = (float)windowWidth / (float)windowHeight;
	Matrix4 viewMatrix = gameWorld->GetMainCamera()->BuildViewMatrix();
	Matrix4 projMatrix = gameWorld->GetMainCamera()->BuildProjectionMatrix(screenAspect);

	Matrix4 viewProj = projMatrix * viewMatrix;

	BindShader(debugShader);
	int matSlot = glGetUniformLocation(debugShader->GetProgramID(), "viewProjMatrix");
	GLuint texSlot = glGetUniformLocation(debugShader->GetProgramID(), "useTexture");
	glUniform1i(texSlot, 0);

	glUniformMatrix4fv(matSlot, 1, false, (float*)viewProj.array);

	debugLineData.clear();

	int frameLineCount = (int)lines.size() * 2;

	SetDebugLineBufferSizes(frameLineCount);

	glBindBuffer(GL_ARRAY_BUFFER, lineVertVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, lines.size() * sizeof(Debug::DebugLineEntry), lines.data());


	glBindVertexArray(lineVAO);
	glDrawArrays(GL_LINES, 0, frameLineCount);
	glBindVertexArray(0);
}

void GameTechRenderer::NewRenderLinesOnOrthographicView()
{
	const std::vector<Debug::DebugLineEntry>& lines = Debug::GetOrthographicViewLines();
	if (lines.empty()) {
		return;
	}
	float screenAspect = (float)windowWidth / (float)windowHeight;

	Matrix4 proj = Matrix4::Orthographic(0.0, 100.0f, 100, 0, -1.0f, 1.0f);

	BindShader(debugShader);
	int matSlot = glGetUniformLocation(debugShader->GetProgramID(), "viewProjMatrix");
	GLuint texSlot = glGetUniformLocation(debugShader->GetProgramID(), "useTexture");
	glUniform1i(texSlot, 0);

	glUniformMatrix4fv(matSlot, 1, false, (float*)proj.array);

	debugLineData.clear();

	int frameLineCount = (int)lines.size() * 2;

	SetDebugLineBufferSizes(frameLineCount);

	glBindBuffer(GL_ARRAY_BUFFER, lineVertVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, lines.size() * sizeof(Debug::DebugLineEntry), lines.data());


	glBindVertexArray(lineVAO);
	glDrawArrays(GL_LINES, 0, frameLineCount);
	glBindVertexArray(0);
}

void GameTechRenderer::NewRenderText() {
	const std::vector<Debug::DebugStringEntry>& strings = Debug::GetDebugStrings();
	if (strings.empty()) {
		return;
	}

	BindShader(debugShader);

	OGLTexture* t = (OGLTexture*)Debug::GetDebugFont()->GetTexture();

	if (t) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, t->GetObjectID());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
		BindTextureToShader(t, "mainTex", 0);
	}
	Matrix4 proj = Matrix4::Orthographic(0.0, 100.0f, 100, 0, -1.0f, 1.0f);

	int matSlot = glGetUniformLocation(debugShader->GetProgramID(), "viewProjMatrix");
	glUniformMatrix4fv(matSlot, 1, false, (float*)proj.array);

	GLuint texSlot = glGetUniformLocation(debugShader->GetProgramID(), "useTexture");
	glUniform1i(texSlot, 1);

	debugTextPos.clear();
	debugTextColours.clear();
	debugTextUVs.clear();

	int frameVertCount = 0;
	for (const auto& s : strings) {
		frameVertCount += Debug::GetDebugFont()->GetVertexCountForString(s.data);
	}
	SetDebugStringBufferSizes(frameVertCount);

	for (const auto& s : strings) {
		float size = 20.0f;
		Debug::GetDebugFont()->BuildVerticesForString(s.data, s.position, s.colour, size, debugTextPos, debugTextUVs, debugTextColours);
	}


	glBindBuffer(GL_ARRAY_BUFFER, textVertVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, frameVertCount * sizeof(Vector3), debugTextPos.data());
	glBindBuffer(GL_ARRAY_BUFFER, textColourVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, frameVertCount * sizeof(Vector4), debugTextColours.data());
	glBindBuffer(GL_ARRAY_BUFFER, textTexVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, frameVertCount * sizeof(Vector2), debugTextUVs.data());

	glBindVertexArray(textVAO);
	glDrawArrays(GL_TRIANGLES, 0, frameVertCount);
	glBindVertexArray(0);
}

void GameTechRenderer::GenerateAtomicBuffer()
{
	glGenBuffers(1, &atomicsBuffer[0]);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicsBuffer[0]);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicsBuffer[0]);
	glBufferStorage(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint) * ATOMIC_COUNT, NULL, GL_DYNAMIC_STORAGE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_READ_BIT | GL_MAP_COHERENT_BIT);

	glGenBuffers(1, &atomicsBuffer[1]);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicsBuffer[1]);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 1, atomicsBuffer[1]);
	glBufferStorage(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint) * ATOMIC_COUNT, NULL, GL_DYNAMIC_STORAGE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_READ_BIT | GL_MAP_COHERENT_BIT);
	
	glGenBuffers(1, &atomicsBuffer[2]);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicsBuffer[2]);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 2, atomicsBuffer[2]);
	glBufferStorage(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint) * ATOMIC_COUNT, NULL, GL_DYNAMIC_STORAGE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_READ_BIT | GL_MAP_COHERENT_BIT);
	
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, 0);
	

	currentAtomicCPU = 0;
	curretAtomicReset = 1;
	currentAtomicGPU = 2;
}
	
	
void GameTechRenderer::RetrieveAtomicValues()
{
	GLuint pixelCount[ATOMIC_COUNT];
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicsBuffer[currentAtomicCPU]);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, currentAtomicCPU, atomicsBuffer[currentAtomicCPU]);

	glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint) * ATOMIC_COUNT, pixelCount);

	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
	totalPixelCount = pixelCount[0];
	//std::cout << "TOTAL: " << totalPixelCount << std::endl;
	for (GLuint i = 1; i < ATOMIC_COUNT; i++)
	{
		teamPixelCount[i - 1] = pixelCount[i];
		//std::cout << "Team " << i << "  " << i << std::endl;

	}
	
	CalculatePercentages(totalPixelCount, teamPixelCount[0], teamPixelCount[1], teamPixelCount[2], teamPixelCount[3]);

	ResetAtomicBuffer();
}

void GameTechRenderer::ResetAtomicBuffer()
{
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicsBuffer[currentAtomicCPU]);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, currentAtomicCPU, atomicsBuffer[currentAtomicCPU]);
	GLuint a[ATOMIC_COUNT];
	for (GLuint i = 0; i < ATOMIC_COUNT; i++)
	{
		a[i] = 0;
	}
	glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint) * ATOMIC_COUNT, a);

}

std::map<int, float> GameTechRenderer::GetTeamScores() {
	std::map<int, float> scores;
	scores.emplace(1, team1Percentage);
	scores.emplace(2, team2Percentage);
	scores.emplace(3, team3Percentage);
	scores.emplace(4, team4Percentage);
	return scores;
}


void GameTechRenderer::SetWorld(ToonGameWorld* world)
{
	gameWorld = world;
	std::map<int, Team*> teams = gameWorld->GetTeams();

	int i = 0;
	for (auto& [ID, team] : teams) {
		teamColours[i] = team->GetTeamColour();
		i++;
	}
}

void GameTechRenderer::SetDebugStringBufferSizes(size_t newVertCount) {
	if (newVertCount > textCount) {
		textCount = newVertCount;

		glBindBuffer(GL_ARRAY_BUFFER, textVertVBO);
		glBufferData(GL_ARRAY_BUFFER, textCount * sizeof(Vector3), nullptr, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, textColourVBO);
		glBufferData(GL_ARRAY_BUFFER, textCount * sizeof(Vector4), nullptr, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, textTexVBO);
		glBufferData(GL_ARRAY_BUFFER, textCount * sizeof(Vector2), nullptr, GL_DYNAMIC_DRAW);

		debugTextPos.reserve(textCount);
		debugTextColours.reserve(textCount);
		debugTextUVs.reserve(textCount);

		glBindVertexArray(textVAO);

		glVertexAttribFormat(0, 3, GL_FLOAT, false, 0);
		glVertexAttribBinding(0, 0);
		glBindVertexBuffer(0, textVertVBO, 0, sizeof(Vector3));

		glVertexAttribFormat(1, 4, GL_FLOAT, false, 0);
		glVertexAttribBinding(1, 1);
		glBindVertexBuffer(1, textColourVBO, 0, sizeof(Vector4));

		glVertexAttribFormat(2, 2, GL_FLOAT, false, 0);
		glVertexAttribBinding(2, 2);
		glBindVertexBuffer(2, textTexVBO, 0, sizeof(Vector2));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);
	}
}

void GameTechRenderer::SetDebugLineBufferSizes(size_t newVertCount) {
	if (newVertCount > lineCount) {
		lineCount = newVertCount;

		glBindBuffer(GL_ARRAY_BUFFER, lineVertVBO);
		glBufferData(GL_ARRAY_BUFFER, lineCount * sizeof(Debug::DebugLineEntry), nullptr, GL_DYNAMIC_DRAW);

		debugLineData.reserve(lineCount);

		glBindVertexArray(lineVAO);

		int realStride = sizeof(Debug::DebugLineEntry) / 2;

		glVertexAttribFormat(0, 3, GL_FLOAT, false, offsetof(Debug::DebugLineEntry, start));
		glVertexAttribBinding(0, 0);
		glBindVertexBuffer(0, lineVertVBO, 0, realStride);

		glVertexAttribFormat(1, 4, GL_FLOAT, false, offsetof(Debug::DebugLineEntry, colourA));
		glVertexAttribBinding(1, 0);
		glBindVertexBuffer(1, lineVertVBO, sizeof(Vector4), realStride);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
	}
}


