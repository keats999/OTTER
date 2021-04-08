//Just a simple handler for simple initialization stuffs
#include "Utilities/BackendHandler.h"
#include "Utilities/Util.h"

#include <filesystem>
#include <json.hpp>
#include <fstream>

#include <Texture2D.h>
#include <Texture2DData.h>
#include <MeshBuilder.h>
#include <MeshFactory.h>
#include <NotObjLoader.h>
#include <ObjLoader.h>
#include "Utilities/ObjAnimation.h"
#include <VertexTypes.h>
#include <ShaderMaterial.h>
#include <RendererComponent.h>
#include <TextureCubeMap.h>
#include <TextureCubeMapData.h>

#include <DirectionalLight.h>
#include <PointLight.h>
#include <UniformBuffer.h>

#include <Timing.h>
#include <GameObjectTag.h>
#include <InputHelpers.h>

#include <IBehaviour.h>
#include <CameraControlBehaviour.h>
#include <FollowPathBehaviour.h>
#include <SimpleMoveBehaviour.h>
#include "Behaviours/PlayerBehaviour.h"
#include "Behaviours/FirstPersonBehaviour.h"
#include "Behaviours/EnemyBehaviour.h"
#include "Behaviours/CoinBehaviour.h"
#include "Behaviours/GameBehaviour.h"
#include "Behaviours/MenuBehaviour.h"
#include "Behaviours/PauseBehaviour.h"
#include "Behaviours/UIBehaviour.h"
#include "Behaviours/ExitBehaviour.h"
#include "Behaviours/SafeRoomBehaviour.h"
#include "Behaviours/EndBehaviour.h"

#include "Graphics/UIComponent.h"

#include "Utilities/Globals.h"

#include "Utilities/Trigger.h"
#include "Triggers/CoinTrigger.h"
#include "Triggers/EnemyTrigger.h"
#include "Triggers/ExitTrigger.h"
#include "Triggers/SafeRoomTrigger.h"

#include "Utilities/MapManager.h"
#include "Utilities/Collision2D.h"
#include "Utilities/PhysicsWorld.h"

#define NUM_TREES 300
#define NUM_ROCKS 40
#define PLANE_X 19.0f
#define PLANE_Y 19.0f
#define DNS_X 3.0f
#define DNS_Y 3.0f

int main() {
	int frameIx = 0;
	float fpsBuffer[128];
	float minFps, maxFps, avgFps;
	int selectedVao = 0; // select cube by default
	std::vector<GameObject> controllables;
	bool drawGBuffer = false;
	bool drawIllumBuffer = false;

	BackendHandler::InitAll();

	// Let OpenGL know that we want debug output, and route it to our handler function
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(BackendHandler::GlDebugMessage, nullptr);

	// Enable texturing
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);

	// Push another scope so most memory should be freed *before* we exit the app
	{
		#pragma region Shader and ImGui
		// Load our shaders

		Shader::sptr passthroughShader = Shader::Create();
		passthroughShader->LoadShaderPartFromFile("shaders/passthrough_vert.glsl", GL_VERTEX_SHADER);
		passthroughShader->LoadShaderPartFromFile("shaders/passthrough_frag.glsl", GL_FRAGMENT_SHADER);
		passthroughShader->Link();

		Shader::sptr colorCorrectionShader = Shader::Create();
		colorCorrectionShader->LoadShaderPartFromFile("shaders/passthrough_vert.glsl", GL_VERTEX_SHADER);
		colorCorrectionShader->LoadShaderPartFromFile("shaders/Post/color_correction_frag.glsl", GL_FRAGMENT_SHADER);
		colorCorrectionShader->Link();

		Shader::sptr simpleDepthShader = Shader::Create();
		simpleDepthShader->LoadShaderPartFromFile("shaders/simple_depth_vert.glsl", GL_VERTEX_SHADER);
		simpleDepthShader->LoadShaderPartFromFile("shaders/simple_depth_frag.glsl", GL_FRAGMENT_SHADER);
		simpleDepthShader->Link();

		Shader::sptr gBufferShader = Shader::Create();
		gBufferShader->LoadShaderPartFromFile("shaders/vertex_shader.glsl", GL_VERTEX_SHADER);
		gBufferShader->LoadShaderPartFromFile("shaders/gBuffer_pass_frag.glsl", GL_FRAGMENT_SHADER);
		gBufferShader->Link();

		Shader::sptr uiShader = Shader::Create();
		uiShader->LoadShaderPartFromFile("shaders/ui_vert.glsl", GL_VERTEX_SHADER);
		uiShader->LoadShaderPartFromFile("shaders/ui_frag.glsl", GL_FRAGMENT_SHADER);
		uiShader->Link();

		// Load our shaders
		Shader::sptr shader = Shader::Create();
		shader->LoadShaderPartFromFile("shaders/vertex_shader.glsl", GL_VERTEX_SHADER);
		//Directional Light Shader
		shader->LoadShaderPartFromFile("shaders/directional_blinn_phong_frag.glsl", GL_FRAGMENT_SHADER);
		shader->Link();

		glm::vec3 lightPos = glm::vec3(0.0f, -1.0f, 0.0f);
		glm::vec3 lightCol = glm::vec3(0.9f, 0.85f, 0.6f);
		float     lightAmbientPow = 0.05f;
		float     lightSpecularPow = 1.0f;
		glm::vec3 ambientCol = glm::vec3(0.5f);
		float     ambientPow = 0.025f;
		float     lightLinearFalloff = 0.9f;
		float     lightQuadraticFalloff = 0.032f;
		int		  mode = 0;
		int		  textures = 1;

		Texture2DData::sptr rampImage = Texture2DData::LoadFromFile("images/Ramp.png");
		Texture2D::sptr texRamp = Texture2D::Create();
		texRamp->LoadData(rampImage);

		
		// These are our application / scene level uniforms that don't necessarily update
		// every frame
		shader->SetUniform("u_LightPos", lightPos);
		shader->SetUniform("u_LightCol", lightCol);
		shader->SetUniform("u_AmbientLightStrength", lightAmbientPow);
		shader->SetUniform("u_SpecularLightStrength", lightSpecularPow);
		shader->SetUniform("u_AmbientCol", ambientCol);
		shader->SetUniform("u_AmbientStrength", ambientPow);
		shader->SetUniform("u_LightAttenuationConstant", 1.0f);
		shader->SetUniform("u_LightAttenuationLinear", lightLinearFalloff);
		shader->SetUniform("u_LightAttenuationQuadratic", lightQuadraticFalloff);
		shader->SetUniform("u_Mode", mode);
		shader->SetUniform("u_Textures", textures);
		
		shader->SetUniform("s_RampTexture", 1);
		
		//Creates our directional Light
		DirectionalLight theSun;
		theSun._lightCol = glm::vec4(0.9f, 0.85f, 0.6f, 0.0f);
		theSun._ambientCol = glm::vec4(0.5f, 0.5f, 0.5f, 0.0f);
		theSun._lightDirection = glm::vec4(0.f, 10.f, 0.f, 0.0f);
		theSun._shadowBias = 0.05;
		//theSun._ambientPow = 0.05f;
		//theSun._lightAmbientPow = 0.09f;
		//theSun._lightSpecularPow = 1.5f;

		PointLight playerLight;
		playerLight._lightPos = glm::vec4(0.f, 0.f, 0.f, 0.0f);

		UniformBuffer directionalLightBuffer;

		//Allocates enough memory for one directional light (we can change this easily, but we only need 1 directional light)
		directionalLightBuffer.AllocateMemory(sizeof(DirectionalLight));
		//Casts our sun as "data" and sends it to the shader
		directionalLightBuffer.SendData(reinterpret_cast<void*>(&theSun), sizeof(DirectionalLight));

		directionalLightBuffer.Bind(0);

		UniformBuffer pointLightBuffer;

		//Allocates enough memory for one directional light (we can change this easily, but we only need 1 directional light)
		pointLightBuffer.AllocateMemory(sizeof(PointLight));
		//Casts our sun as "data" and sends it to the shader
		pointLightBuffer.SendData(reinterpret_cast<void*>(&playerLight), sizeof(PointLight));

		pointLightBuffer.Bind(1);

		PostEffect* testBuffer;
		PostEffect* uiBuffer;

		Framebuffer* shadowBuffer;
		GBuffer* gBuffer;
		IlluminationBuffer* illuminationBuffer;

		int activeDef = 0;

		int activeEffect = 0;
		std::vector<GameObject> effects;
		
		GreyscaleEffect* greyscaleEffect;
		BloomEffect* bloomEffect;
		FilmGrainEffect* filmGrainEffect;
		PixelationEffect* pixelEffect;

		int activePost = 2;
		std::vector<PostEffect*> post;
		bool ramp=0;
		
		std::vector<GameObject> enemies;

		int width, height;
		glfwGetWindowSize(BackendHandler::window, &width, &height);

		// We'll add some ImGui controls to control our shader
		BackendHandler::imGuiCallbacks.push_back([&]() {
			if (ImGui::CollapsingHeader("basic lighting"))
			{
				if (ImGui::Button("No Lighting")) {
					mode = 1;
					shader->SetUniform("u_Mode", mode);
					activePost = 0;

				}

				if (ImGui::Button("Ambient Only")) {
					mode = 2;
					shader->SetUniform("u_Mode", mode);
					activePost = 0;

				}

				if (ImGui::Button("Specular Only")) {
					mode = 3;
					shader->SetUniform("u_Mode", mode);
					activePost = 0;

				}

				if (ImGui::Button("Ambient + Specular")) {
					mode = 0;
					shader->SetUniform("u_Mode", mode);
					activePost = 0;

				}

				if (ImGui::Button("Ambient + Specular + Custom")) {
					mode = 4;
					shader->SetUniform("u_Mode", mode);
					activePost = 1;

				}
				if (ImGui::Button("Ambient + Specular + Bloom")) {
					mode = 7;
					shader->SetUniform("u_Mode", mode);
					activePost = 1;
				}
				if (ImGui::Button("Diffuse Ramp")) {
					mode = 5;
					shader->SetUniform("u_Mode", mode);
					shader->SetUniform("s_RampTexture", 1);
					activePost = 0;

				}

				if (ImGui::Button("Specular Ramp")) {
					mode = 6;
					shader->SetUniform("u_Mode", mode);
					shader->SetUniform("s_RampTexture", 1);
					activePost = 0;

				}
			}
			if (ImGui::CollapsingHeader("Assignment 3"))
			{
				if (ImGui::SliderInt("Active Post Effect", &activePost, 0, 3));

				if (activePost == 1 && ImGui::CollapsingHeader("Bloom Effect controls")) {
					BloomEffect* temp = (BloomEffect*)post[activePost];
					float threshold = temp->GetThreshold();
					int pass = temp->GetPasses();

					if (ImGui::SliderFloat("Brightness Threshold", &threshold, 0.0f, 1.0f))
					{
						temp->SetThreshold(threshold);
					}

					if (ImGui::SliderInt("Blur", &pass, 0, 10))
					{
						temp->SetPasses(pass);
					}
				}
				if (activePost == 2 && ImGui::CollapsingHeader("Film Grain Effect controls")) {
					FilmGrainEffect* temp = (FilmGrainEffect*)post[activePost];
					float intensity = temp->GetIntensity();

					if (ImGui::SliderFloat("Intensity", &intensity, 0.0f, 1.0f))
					{
						temp->SetIntensity(intensity);
					}
				}
				if (activePost == 3 && ImGui::CollapsingHeader("Pixelation Effect controls")) {
					PixelationEffect* temp = (PixelationEffect*)post[activePost];
					float percent = temp->GetPercentOfPixels();

					if (ImGui::SliderFloat("percent of pixels", &percent, 0.0f, 1.0f))
					{
						temp->SetPercentOfPixels(percent);
					}
				}
			}
			if (ImGui::CollapsingHeader("colour correction"))
			{
				if (ImGui::Button("Color Grading Warm")) {
					if (activeEffect == 1)
						activeEffect = 0;
					else
						activeEffect = 1;
				}

				if (ImGui::Button("Color Grading Cool")) {
					if (activeEffect == 2)
						activeEffect = 0;
					else
						activeEffect = 2;
				}

				if (ImGui::Button("Color Grading Custom")) {
					if (activeEffect == 3)
						activeEffect = 0;
					else
						activeEffect = 3;
				}

				if (ImGui::Button("Toggle Textures")) {
					if (textures == 0)
						textures = 1;
					else
						textures = 0;

					shader->SetUniform("u_Textures", textures);
				}
			}
			if (ImGui::DragFloat4("Light Dir", glm::value_ptr(theSun._lightDirection), 0.01f, -10.0f, 10.0f)) {
				directionalLightBuffer.SendData(reinterpret_cast<void*>(&theSun), sizeof(DirectionalLight));
			}
			auto behaviour = BehaviourBinding::Get<EnemyBehaviour>(enemies[0]);
			ImGui::Checkbox("Enemy Active", &behaviour->Enabled);
			/*if (ImGui::CollapsingHeader("Scene Level Lighting Settings"))
			{
				if (ImGui::ColorPicker3("Ambient Color", glm::value_ptr(ambientCol))) {
					shader->SetUniform("u_AmbientCol", ambientCol);
				}
				if (ImGui::SliderFloat("Fixed Ambient Power", &ambientPow, 0.01f, 1.0f)) {
					shader->SetUniform("u_AmbientStrength", ambientPow);
				}
			}
			if (ImGui::CollapsingHeader("Light Level Lighting Settings"))
			{
				if (ImGui::DragFloat3("Light Pos", glm::value_ptr(lightPos), 0.01f, -10.0f, 10.0f)) {
					shader->SetUniform("u_LightPos", lightPos);
				}
				if (ImGui::ColorPicker3("Light Col", glm::value_ptr(lightCol))) {
					shader->SetUniform("u_LightCol", lightCol);
				}
				if (ImGui::SliderFloat("Light Ambient Power", &lightAmbientPow, 0.0f, 1.0f)) {
					shader->SetUniform("u_AmbientLightStrength", lightAmbientPow);
				}
				if (ImGui::SliderFloat("Light Specular Power", &lightSpecularPow, 0.0f, 1.0f)) {
					shader->SetUniform("u_SpecularLightStrength", lightSpecularPow);
				}
				if (ImGui::DragFloat("Light Linear Falloff", &lightLinearFalloff, 0.01f, 0.0f, 1.0f)) {
					shader->SetUniform("u_LightAttenuationLinear", lightLinearFalloff);
				}
				if (ImGui::DragFloat("Light Quadratic Falloff", &lightQuadraticFalloff, 0.01f, 0.0f, 1.0f)) {
					shader->SetUniform("u_LightAttenuationQuadratic", lightQuadraticFalloff);
				}
			}

			/*auto name = controllables[selectedVao].get<GameObjectTag>().Name;
			ImGui::Text(name.c_str());
			auto behaviour = BehaviourBinding::Get<SimpleMoveBehaviour>(controllables[selectedVao]);
			ImGui::Checkbox("Relative Rotation", &behaviour->Relative);

			ImGui::Text("Q/E -> Yaw\nLeft/Right -> Roll\nUp/Down -> Pitch\nY -> Toggle Mode");
			*/
			minFps = FLT_MAX;
			maxFps = 0;
			avgFps = 0;
			for (int ix = 0; ix < 128; ix++) {
				if (fpsBuffer[ix] < minFps) { minFps = fpsBuffer[ix]; }
				if (fpsBuffer[ix] > maxFps) { maxFps = fpsBuffer[ix]; }
				avgFps += fpsBuffer[ix];
			}
			ImGui::PlotLines("FPS", fpsBuffer, 128);
			ImGui::Text("MIN: %f MAX: %f AVG: %f", minFps, maxFps, avgFps / 128.0f);
			});

		#pragma endregion 

		// GL states
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDepthFunc(GL_LEQUAL); // New 
		
		#pragma region TEXTURE LOADING

		// Load some textures from files
		Texture2D::sptr stone = Texture2D::LoadFromFile("images/Stone_001_Diffuse.png");
		Texture2D::sptr stoneSpec = Texture2D::LoadFromFile("images/Stone_001_Specular.png");
		Texture2D::sptr grass = Texture2D::LoadFromFile("images/grass.jpg");
		Texture2D::sptr noSpec = Texture2D::LoadFromFile("images/grassSpec.png");
		Texture2D::sptr box = Texture2D::LoadFromFile("images/box.bmp");
		Texture2D::sptr boxSpec = Texture2D::LoadFromFile("images/box-reflections.bmp");
		Texture2D::sptr simpleFlora = Texture2D::LoadFromFile("images/SimpleFlora.png");
		Texture2D::sptr coin = Texture2D::LoadFromFile("images/coin.png");
		Texture2D::sptr rattex = Texture2D::LoadFromFile("images/ratTex.png");

		Texture2D::sptr tstr = Texture2D::LoadFromFile("images/tubestr.png");
		Texture2D::sptr ttee = Texture2D::LoadFromFile("images/tubetee.png");
		Texture2D::sptr tlbw = Texture2D::LoadFromFile("images/tubelbw.png");
		Texture2D::sptr tqd = Texture2D::LoadFromFile("images/tubeqd.png");

		Texture2D::sptr metal = Texture2D::LoadFromFile("images/metal.png");

		Texture2D::sptr blue = Texture2D::LoadFromFile("images/blue.png");
		Texture2D::sptr green = Texture2D::LoadFromFile("images/green.png"); 
		Texture2D::sptr orange = Texture2D::LoadFromFile("images/orange.png");
		Texture2D::sptr red = Texture2D::LoadFromFile("images/red.png");
		Texture2D::sptr yellow = Texture2D::LoadFromFile("images/yellow.png");

		Texture2D::sptr glass = Texture2D::LoadFromFile("images/glass.png");

		Texture2D::sptr testUI = Texture2D::LoadFromFile("images/testUI.png");
		Texture2D::sptr title = Texture2D::LoadFromFile("images/title_transparent_info.png");
		Texture2D::sptr pauseTex = Texture2D::LoadFromFile("images/pauseelement.png");
		Texture2D::sptr endelement = Texture2D::LoadFromFile("images/endelement.png");

		Texture2D::sptr dep = Texture2D::LoadFromFile("images/deposit.png");
		Texture2D::sptr scrn = Texture2D::LoadFromFile("images/screens/screen9.png");
		Texture2D::sptr disp = Texture2D::LoadFromFile("images/display.png");

		
		// Load the cube map
		TextureCubeMap::sptr environmentMap = TextureCubeMap::LoadFromImages("images/cubemaps/skybox/dark.jpg");
		Texture2D::sptr uiTex = Texture2D::LoadFromFile("images/testUI.png");
		//TextureCubeMap::sptr environmentMap = TextureCubeMap::LoadFromImages("images/cubemaps/skybox/ToonSky.jpg"); 

		// Creating an empty texture
		Texture2DDescription desc = Texture2DDescription();  
		desc.Width = 1;
		desc.Height = 1;
		desc.Format = InternalFormat::RGB8;
		Texture2D::sptr texture2 = Texture2D::Create(desc);
		// Clear it with a white colour
		texture2->Clear();

		#pragma endregion

		///////////////////////////////////// Scene Generation //////////////////////////////////////////////////
		#pragma region Scene Generation
		
		// We need to tell our scene system what extra component types we want to support
		GameScene::RegisterComponentType<RendererComponent>();
		GameScene::RegisterComponentType<BehaviourBinding>();
		GameScene::RegisterComponentType<TriggerBinding>();
		GameScene::RegisterComponentType<Collision2D>();
		GameScene::RegisterComponentType<Camera>();

		// Create a scene, and set it to be the active scene in the application
		GameScene::sptr scene = GameScene::Create("test");
		PhysicsWorld::sptr pworld = PhysicsWorld::Create(scene);

		GameScene::sptr menuscene = GameScene::Create("MainMenu");
		GameScene::sptr pausescene = GameScene::Create("PauseMenu");
		GameScene::sptr winscene = GameScene::Create("Win");
		GameScene::sptr endscene = GameScene::Create("GameOver");

		Application::Instance().ActiveScene = menuscene;

		Globals::Instance().scenes.push_back(menuscene);	//0
		Globals::Instance().scenes.push_back(scene);		//1
		Globals::Instance().scenes.push_back(pausescene);	//2
		Globals::Instance().scenes.push_back(winscene);		//3
		Globals::Instance().scenes.push_back(endscene);		//4

		// We can create a group ahead of time to make iterating on the group faster
		

		// Create a material and set some properties for it
		ShaderMaterial::sptr stoneMat = ShaderMaterial::Create();  
		stoneMat->Shader = gBufferShader;
		stoneMat->Set("s_Diffuse", stone);
		stoneMat->Set("s_Specular", stoneSpec);
		stoneMat->Set("u_Shininess", 2.0f);
		stoneMat->Set("u_TextureMix", 0.0f); 
		stoneMat->Set("u_Emission", 0.0f);

		ShaderMaterial::sptr metalMat = ShaderMaterial::Create();
		metalMat->Shader = gBufferShader;
		metalMat->Set("s_Diffuse", metal);
		metalMat->Set("s_Specular", noSpec);
		metalMat->Set("u_Shininess", 10.0f);
		metalMat->Set("u_TextureMix", 0.0f);
		metalMat->Set("u_Emission", 0.0f);

		ShaderMaterial::sptr grassMat = ShaderMaterial::Create();
		grassMat->Shader = gBufferShader;
		grassMat->Set("s_Diffuse", grass);
		grassMat->Set("s_Specular", noSpec);
		grassMat->Set("u_Shininess", 2.0f);
		grassMat->Set("u_TextureMix", 0.0f);
		grassMat->Set("u_Emission", 0.0f);

		ShaderMaterial::sptr boxMat = ShaderMaterial::Create();
		boxMat->Shader = gBufferShader;
		boxMat->Set("s_Diffuse", box);
		boxMat->Set("s_Specular", boxSpec);
		boxMat->Set("u_Shininess", 8.0f);
		boxMat->Set("u_TextureMix", 0.0f);
		boxMat->Set("u_Emission", 0.0f);

		ShaderMaterial::sptr depositMat = ShaderMaterial::Create();
		depositMat->Shader = gBufferShader;
		depositMat->Set("s_Diffuse", dep);
		depositMat->Set("s_Specular", noSpec);
		depositMat->Set("u_Shininess", 2.0f);
		depositMat->Set("u_TextureMix", 0.0f);
		depositMat->Set("u_Emission", 0.0f);
		
		ShaderMaterial::sptr displayMat = ShaderMaterial::Create();
		displayMat->Shader = gBufferShader;
		displayMat->Set("s_Diffuse", disp);
		displayMat->Set("s_Specular", noSpec);
		displayMat->Set("u_Shininess", 2.0f);
		displayMat->Set("u_TextureMix", 0.0f);
		displayMat->Set("u_Emission", 0.0f);

		ShaderMaterial::sptr screenMat = ShaderMaterial::Create();
		screenMat->Shader = gBufferShader;
		screenMat->Set("s_Diffuse", scrn);
		screenMat->Set("s_Specular", noSpec);
		screenMat->Set("u_Shininess", 2.0f);
		screenMat->Set("u_TextureMix", 0.0f);
		screenMat->Set("u_Emission", 1.0f);

		ShaderMaterial::sptr doorMat = ShaderMaterial::Create();
		doorMat->Shader = gBufferShader;
		doorMat->Set("s_Diffuse", glass);
		doorMat->Set("s_Specular", noSpec);
		doorMat->Set("u_Shininess", 2.0f);
		doorMat->Set("u_TextureMix", 0.0f);
		doorMat->Set("u_Emission", 0.0f);

		ShaderMaterial::sptr simpleFloraMat = ShaderMaterial::Create();
		simpleFloraMat->Shader = gBufferShader;
		simpleFloraMat->Set("s_Diffuse", simpleFlora);
		simpleFloraMat->Set("s_Specular", noSpec);
		simpleFloraMat->Set("u_Shininess", 8.0f);
		simpleFloraMat->Set("u_TextureMix", 0.0f);
		simpleFloraMat->Set("u_Emission", 0.0f);

		ShaderMaterial::sptr ratMat = ShaderMaterial::Create();
		ratMat->Shader = gBufferShader;
		ratMat->Set("s_Diffuse", rattex);
		ratMat->Set("s_Specular", noSpec);
		ratMat->Set("u_Shininess", 8.0f);
		ratMat->Set("u_TextureMix", 0.0f);
		ratMat->Set("u_Emission", 0.0f);

		ShaderMaterial::sptr testuiMat = ShaderMaterial::Create();
		simpleFloraMat->Shader = uiShader;
		simpleFloraMat->Set("s_Diffuse", testUI);
		simpleFloraMat->Set("s_Specular", noSpec);
		simpleFloraMat->Set("u_Shininess", 8.0f);
		simpleFloraMat->Set("u_TextureMix", 0.0f);
		//simpleFloraMat->Set("u_Emission", 0.0f);
		
		VertexArrayObject::sptr coinvao = ObjLoader::LoadFromFile("models/coin.obj");
		ShaderMaterial::sptr coinMat = ShaderMaterial::Create();
		coinMat->Shader = gBufferShader;
		coinMat->Set("s_Diffuse", coin);
		coinMat->Set("s_Specular", noSpec);
		coinMat->Set("u_Shininess", 8.0f);
		coinMat->Set("u_TextureMix", 0.0f);
		coinMat->Set("u_Emission", 0.0f);
		int coincount = 0;
		

		VertexArrayObject::sptr rvtstr = ObjLoader::LoadFromFile("models/rvtstr.obj");
		VertexArrayObject::sptr rvtlbw = ObjLoader::LoadFromFile("models/rvtlbw.obj");
		VertexArrayObject::sptr rvttee = ObjLoader::LoadFromFile("models/rvttee.obj");
		VertexArrayObject::sptr rvtqd = ObjLoader::LoadFromFile("models/rvtqd.obj");
		std::vector<GameObject> rivets;

		VertexArrayObject::sptr tubestr = ObjLoader::LoadFromFile("models/tubestr.obj");
		VertexArrayObject::sptr tubelbw = ObjLoader::LoadFromFile("models/tubelbw.obj");
		VertexArrayObject::sptr tubetee = ObjLoader::LoadFromFile("models/tubetee.obj");
		VertexArrayObject::sptr tubeqd = ObjLoader::LoadFromFile("models/tubeqd.obj");
		VertexArrayObject::sptr tubend = ObjLoader::LoadFromFile("models/tubend.obj");
		VertexArrayObject::sptr escdoor = ObjLoader::LoadFromFile("models/escdoor.obj");
		std::vector<GameObject> tubes;
		std::vector<GameObject> srooms;

		VertexArrayObject::sptr wallobj = ObjLoader::LoadFromFile("models/wall.obj");
		std::vector<GameObject> walls;

		glm::vec2 spawn = glm::vec2(0, 0);
		glm::vec2 exitloc = glm::vec2(0, 0);
		glm::vec2 enemySpawn = glm::vec2(0, 0);

		//Call map managing tool and pass level data
		MapManager& Manager = MapManager::Instance();
		Manager.LoadFromFile("level.lvl");

		//Get map data from the manager
		auto& map = Manager.GetMap();
		int x = Manager.GetRows();
		int y = Manager.GetColumns();
		int unitsize = Manager.GetUnitS();
		bool endcreated = false;

		//Nested loop to cycle through the 2D array for the map
		for (int i = 0; i < x; i++) {
			for (int j = 0; j < y; j++) {
				//Use i and j to get coordinates
				int coord1 = j * unitsize;
				int coord2 = i * unitsize;
				//Check to see the map array has a 1 to spawn a tube
				if (map[i][j] == 1) {
					bool canspawn = true; //Variable to check for valid coin spawn location
					bool pspawn = false; //Variable to check for valid player spawn location (Safe rooms)
					
					//Create tube material
					ShaderMaterial::sptr tMat = ShaderMaterial::Create();
					tMat->Shader = gBufferShader;
					tMat->Set("u_Shininess", 20.0f);
					tMat->Set("u_TextureMix", 0.0f);
					tMat->Set("u_Emission", 1.0f);

					//Randomly pick a color and set the material diffuse
					int c = rand() % 5;
					switch (c) {
					case 1:
						tMat->Set("s_Diffuse", green);
						break;
					case 2:
						tMat->Set("s_Diffuse", orange);
						break;
					case 3:
						tMat->Set("s_Diffuse", red);
						break;
					case 4:
						tMat->Set("s_Diffuse", yellow);
						break;
					default:
						tMat->Set("s_Diffuse", blue);
						break;
					}

					//Create tube object
					GameObject tubee = scene->CreateEntity("Tube");
					tubes.push_back(tubee);
					GameObject rvte = scene->CreateEntity("Rivet");
					rivets.push_back(rvte);

					//Pass the location in the array to the manager to get appropriate piece
					glm::vec2 tubeData = Manager.GetTube(glm::vec2(i, j));
					switch (int(tubeData.x)) {
					case 1:
						tMat->Set("s_Specular", tstr);
						tubee.emplace<RendererComponent>().SetMesh(tubestr).SetMaterial(tMat).SetCastShadow(false);
						rvte.emplace<RendererComponent>().SetMesh(rvtstr).SetMaterial(metalMat).SetCastShadow(false);
						break;
					case 2:
						tMat->Set("s_Specular", tlbw);
						tubee.emplace<RendererComponent>().SetMesh(tubelbw).SetMaterial(tMat).SetCastShadow(false);
						rvte.emplace<RendererComponent>().SetMesh(rvtlbw).SetMaterial(metalMat).SetCastShadow(false);
						break;
					case 3:
						tMat->Set("s_Specular", ttee);
						tubee.emplace<RendererComponent>().SetMesh(tubetee).SetMaterial(tMat).SetCastShadow(false);
						rvte.emplace<RendererComponent>().SetMesh(rvttee).SetMaterial(metalMat).SetCastShadow(false);
						break;
					case 4:
						tMat->Set("s_Specular", noSpec);
						tubee.emplace<RendererComponent>().SetMesh(tubeqd).SetMaterial(tMat).SetCastShadow(false);
						rvte.emplace<RendererComponent>().SetMesh(rvtqd).SetMaterial(metalMat).SetCastShadow(false);
						break;
					default:
						tMat->Set("s_Specular", noSpec);
						tubee.emplace<RendererComponent>().SetMesh(tubend).SetMaterial(tMat).SetCastShadow(false);
						Manager.saferooms.push_back(glm::vec2(coord1, coord2));
						Manager.safeindexes.push_back(tubes.size()-1);
						
						GameObject sroome = scene->CreateEntity("SafeRoom");
						srooms.push_back(sroome);
						sroome.emplace<RendererComponent>().SetMesh(escdoor).SetMaterial(doorMat);
						auto& srCol = sroome.emplace<Collision2D>(pworld->World());
						srCol.CreateStaticBox(glm::vec2(coord1, coord2), glm::vec2(unitsize / 4, unitsize / 4), TRIGGER, PLAYER);
						srCol.getFixture()->SetSensor(true);
						srCol.getFixture()->SetEntity(sroome.entity());
						srCol.SetAngle(glm::radians(-tubeData.y));
						auto& srT = sroome.get<Transform>();
						srT.SetLocalPosition(coord1, 0, coord2);
						//srT.SetLocalRotation(glm::vec3(0.0f, tubeData.y, 0.0f));
						BehaviourBinding::Bind<SafeRoomBehaviour>(sroome);
						BehaviourBinding::Get<SafeRoomBehaviour>(sroome)->SetRoom(i, j);
						TriggerBinding::Bind<SafeRoomTrigger>(sroome);
						TriggerBinding::Get<SafeRoomTrigger>(sroome)->SetRoom(i, j);
						if (!endcreated) {
							auto& exitCol = tubee.emplace<Collision2D>(pworld->World());
							exitCol.CreateStaticBox(glm::vec2(coord1, coord2), glm::vec2(0.5, 0.5), TRIGGER, PLAYER);
							exitCol.getFixture()->SetSensor(true);
							exitCol.getFixture()->SetEntity(tubee.entity());
							exitCol.SetAngle(glm::radians(-tubeData.y));
							TriggerBinding::BindDisabled<ExitTrigger>(tubee);
							BehaviourBinding::Bind<ExitBehaviour>(tubee);
							endcreated = true;
						}

						pspawn = true;
						canspawn = false;
						break;
					}
					
					//Set tubing orientation based on coordinates and rotation data given by the manager
					auto& tubeT = tubee.get<Transform>();
					tubeT.SetLocalPosition(coord1, 0, coord2);
					tubeT.SetLocalRotation(glm::vec3(0.0f, tubeData.y, 0.0f));
					tubeT.SetLocalScale(glm::vec3(1.05f, 1.0f, 1.05f));

					auto& rvtT = rvte.get<Transform>();
					rvtT.SetLocalPosition(coord1, 0, coord2);
					rvtT.SetLocalRotation(glm::vec3(0.0f, tubeData.y, 0.0f));
					rvtT.SetLocalScale(glm::vec3(1.05f, 1.0f, 1.05f));

					int r = rand() % 5;
					if (r == 3 && canspawn) {
						GameObject coine = scene->CreateEntity("Coin");
						Globals::Instance().coinArray.push_back(coine);
						coine.emplace<RendererComponent>().SetMesh(coinvao).SetMaterial(coinMat);
						auto& coinCol = coine.emplace<Collision2D>(pworld->World());
						coinCol.CreateStaticBox(glm::vec2(coord1, coord2), glm::vec2(unitsize / 2, unitsize / 2), PICKUP, PLAYER);
						coinCol.getFixture()->SetSensor(true);
						coinCol.getFixture()->SetEntity(coine.entity());
						auto& coinT = coine.get<Transform>();
						coinT.SetLocalPosition(coord1, 0, coord2);
						coinT.SetLocalRotation(90, 0, 90);
						BehaviourBinding::Bind<CoinBehaviour>(coine);
						TriggerBinding::Bind<CoinTrigger>(coine);
						Globals::Instance().coinmax++;
					}
				}
				else if (map[i][j] == 0) {
					GameObject walle = scene->CreateEntity("Wall");
					walls.push_back(walle);
					auto& wallCol = walle.emplace<Collision2D>(pworld->World());
					//wallCol.getBody()->SetUserData(&walle);
					wallCol.CreateStaticBox(glm::vec2(coord1, coord2), glm::vec2(unitsize / 2, unitsize / 2), ENVIRONMENT, PLAYER);
					auto& wallT = walle.get<Transform>();
					wallT.SetLocalPosition(coord1, 0, coord2);
				}
			}
		}

		 exitloc = Manager.saferooms[0];
		 spawn = Manager.saferooms[Manager.saferooms.size() - 1];
		 enemySpawn = exitloc;

		 /*GameObject shade = scene->CreateEntity("shade");
		 {
			 VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/plane.obj");
			 shade.emplace<RendererComponent>().SetMesh(vao).SetMaterial(stoneMat);
			 shade.get<Transform>().SetLocalRotation(0, 90, 0);
			 int spawnindex = Manager.safeindexes[Manager.safeindexes.size() - 1];
			 GameObject spawntube = tubes[spawnindex];
			 shade.get<Transform>().SetLocalPosition(10, 10, 10);
			 shade.get<Transform>().SetLocalScale(15, 15, 15);
		 }*/

		 int endindex = Manager.safeindexes[0];
		/* GameObject exit = scene->CreateEntity("Exit");
		 {
			 VertexArrayObject::sptr newtube = ObjLoader::LoadFromFile("models/tubesc.obj");
			 auto& exitCol = exitObject.emplace<Collision2D>(pworld->World());
			 exitCol.CreateStaticBox(Manager.saferooms[0], glm::vec2(1, 1), TRIGGER, PLAYER);
			 exitCol.getFixture()->SetSensor(true);
			 exitCol.getFixture()->SetEntity(exit.entity());
			 TriggerBinding::BindDisabled<ExitTrigger>(exit);
			 BehaviourBinding::Bind<ExitBehaviour>(exit);
		 }*/
		 
		 GameObject deposit = scene->CreateEntity("Deposit");
		 {
			 VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/deposit.obj");
			 int spawnindex = Manager.safeindexes[Manager.safeindexes.size()-1];
			 GameObject spawntube = tubes[spawnindex];
			deposit.get<Transform>().SetLocalPosition(spawntube.get<Transform>().GetLocalPosition());
			deposit.get<Transform>().SetLocalRotation(spawntube.get<Transform>().GetLocalRotation());
			deposit.emplace<RendererComponent>().SetMesh(vao).SetMaterial(depositMat).SetCastShadow(false);
		 }

		 GameObject display = scene->CreateEntity("Display");
		 {
			 VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/display.obj");
			 int spawnindex = Manager.safeindexes[Manager.safeindexes.size() - 1];
			 GameObject spawntube = tubes[spawnindex];
			 display.get<Transform>().SetLocalPosition(spawntube.get<Transform>().GetLocalPosition());
			 display.get<Transform>().SetLocalRotation(spawntube.get<Transform>().GetLocalRotation());
			 display.emplace<RendererComponent>().SetMesh(vao).SetMaterial(displayMat).SetCastShadow(false);
		 }
		 GameObject displayscreen = scene->CreateEntity("Screen");
		 {
			 VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/screen.obj");
			 int spawnindex = Manager.safeindexes[Manager.safeindexes.size() - 1];
			 GameObject spawntube = tubes[spawnindex];
			 displayscreen.get<Transform>().SetLocalPosition(spawntube.get<Transform>().GetLocalPosition());
			 displayscreen.get<Transform>().SetLocalRotation(spawntube.get<Transform>().GetLocalRotation());
			 displayscreen.get<Transform>().SetLocalScale(1.f, 1.f, 1.f);
			 displayscreen.emplace<RendererComponent>().SetMesh(vao).SetMaterial(screenMat).SetCastShadow(false);
		 }
		 
		GameObject player = scene->CreateEntity("Player");
		{
			//VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/plane.obj");
			//player.emplace<RendererComponent>().SetMesh(vao).SetMaterial(stoneMat);
			auto& playerCol = player.emplace<Collision2D>(pworld->World());
			playerCol.CreateDynamicBox(spawn, glm::vec2(2, 2), PLAYER, ENVIRONMENT|INTERACTABLE|OBJECT|ENEMY|PICKUP|TRIGGER);
			//playerCol.getBody()->SetAngularDamping(1.0);
			playerCol.getBody()->SetLinearDamping(1.0);
			BehaviourBinding::Bind<PlayerBehaviour>(player);
			BehaviourBinding::Get<PlayerBehaviour>(player)->SetShader(shader);
		}

		GameObject enemy = scene->CreateEntity("Enemy");
		{
			enemy.emplace<ObjAnimation>();
			enemy.get<ObjAnimation>().LoadFromFolder("rat", 21);
			enemy.emplace<RendererComponent>().SetMesh(enemy.get<ObjAnimation>().LoadMesh()).SetMaterial(ratMat);
			auto& enemyCol = enemy.emplace<Collision2D>(pworld->World());
			enemyCol.CreateDynamicBox(enemySpawn, glm::vec2(1, 1), ENEMY, PLAYER);
			enemyCol.getBody()->SetUserData(&enemy);
			enemyCol.getFixture()->SetSensor(true);
			enemyCol.getFixture()->SetEntity(enemy.entity());
			enemy.get<Transform>().SetLocalScale(0.15f, 0.15f, 0.15f);
			enemy.get<Transform>().SetLocalPosition(0.f, -0.1f, 0.f);
			BehaviourBinding::Bind<EnemyBehaviour>(enemy);
			BehaviourBinding::Get<EnemyBehaviour>(enemy)->SetTarget(player);
			TriggerBinding::Bind<EnemyTrigger>(enemy);
		}
		enemies.push_back(enemy);
		
	

		// Create an object to be our camera
		GameObject cameraObject = scene->CreateEntity("Camera");
		{
			cameraObject.get<Transform>().SetLocalPosition(0, 3, 3).LookAt(glm::vec3(0, 0, 0));

			// We'll make our camera a component of the camera object
			Camera& camera = cameraObject.emplace<Camera>();// Camera::Create();
			camera.SetPosition(glm::vec3(0, 3, 3));
			camera.SetUp(glm::vec3(0, 1, 0));
			camera.LookAt(glm::vec3(0, 2, 0));
			camera.SetFovDegrees(90.0f); // Set an initial FOV
			camera.SetOrthoHeight(3.0f);
			//BehaviourBinding::Bind<CameraControlBehaviour>(cameraObject);
			BehaviourBinding::Bind<FirstPersonBehaviour>(cameraObject);
			BehaviourBinding::Get<FirstPersonBehaviour>(cameraObject)->SetParent(player);
		}
		GameObject menucameraObject = menuscene->CreateEntity("Menu Camera");
		{
			menucameraObject.get<Transform>().SetLocalPosition(0, 1, 1).LookAt(glm::vec3(0, 0, 0));

			// We'll make our camera a component of the camera object
			Camera& menucamera = menucameraObject.emplace<Camera>();// Camera::Create();
			menucamera.SetPosition(glm::vec3(0, 0, 0));
			menucamera.SetUp(glm::vec3(0, 1, 0));
			menucamera.LookAt(glm::vec3(0, 0, 1));
			menucamera.SetFovDegrees(90.0f); // Set an initial FOV
			menucamera.SetOrthoHeight(0.0f);
		}
		GameObject pausecameraObject = pausescene->CreateEntity("Pause Camera");
		{
			pausecameraObject.get<Transform>().SetLocalPosition(0, 1, 1).LookAt(glm::vec3(0, 0, 0));

			// We'll make our camera a component of the camera object
			Camera& pausecamera = pausecameraObject.emplace<Camera>();// Camera::Create();
			pausecamera.SetPosition(glm::vec3(0, 0, 0));
			pausecamera.SetUp(glm::vec3(0, 1, 0));
			pausecamera.LookAt(glm::vec3(0, 0, 1));
			pausecamera.SetFovDegrees(90.0f); // Set an initial FOV
			pausecamera.SetOrthoHeight(0.0f);
		}
		GameObject diecameraObject = endscene->CreateEntity("Die Camera");
		{
			diecameraObject.get<Transform>().SetLocalPosition(0, 1, 1).LookAt(glm::vec3(0, 0, 0));

			// We'll make our camera a component of the camera object
			Camera& diecamera = diecameraObject.emplace<Camera>();// Camera::Create();
			diecamera.SetPosition(glm::vec3(0, 0, 0));
			diecamera.SetUp(glm::vec3(0, 1, 0));
			diecamera.LookAt(glm::vec3(0, 0, 1));
			diecamera.SetFovDegrees(90.0f); // Set an initial FOV
			diecamera.SetOrthoHeight(0.0f);
		}
		GameObject wincameraObject = winscene->CreateEntity("win Camera");
		{
			wincameraObject.get<Transform>().SetLocalPosition(0, 1, 1).LookAt(glm::vec3(0, 0, 0));

			// We'll make our camera a component of the camera object
			Camera& wincamera = wincameraObject.emplace<Camera>();// Camera::Create();
			wincamera.SetPosition(glm::vec3(0, 0, 0));
			wincamera.SetUp(glm::vec3(0, 1, 0));
			wincamera.LookAt(glm::vec3(0, 0, 1));
			wincamera.SetFovDegrees(90.0f); // Set an initial FOV
			wincamera.SetOrthoHeight(0.0f);
		}
		GameObject gBufferObject = scene->CreateEntity("G Buffer");
		{
			gBuffer = &gBufferObject.emplace<GBuffer>();
			gBuffer->Init(width, height);
		}

		GameObject illuminationBufferObject = scene->CreateEntity("Illumination Buffer");
		{
			illuminationBuffer = &illuminationBufferObject.emplace<IlluminationBuffer>();
			illuminationBuffer->Init(width, height);
		}
		GameObject framebufferObject = scene->CreateEntity("Basic Buffer");
		{
			testBuffer = &framebufferObject.emplace<PostEffect>();
			testBuffer->Init(width, height);
		}
		int shadowWidth = 4096;
		int shadowHeight = 4096;

		GameObject shadowBufferObject = scene->CreateEntity("Shadow Buffer");
		{
			shadowBuffer = &shadowBufferObject.emplace<Framebuffer>();
			shadowBuffer->AddDepthTarget();
			shadowBuffer->Init(shadowWidth, shadowHeight);
		}
		GameObject noColorCorrectionObj = scene->CreateEntity("Color Correct");
		{
			ColorCorrection* noColorCorrectEffect = &noColorCorrectionObj.emplace<ColorCorrection>();
			noColorCorrectEffect->filename = "cubes/no_color_correction.cube";
			noColorCorrectEffect->filename = "cubes/neutral.cube";
			noColorCorrectEffect->Init(width, height);
		}
		effects.push_back(noColorCorrectionObj);

		GameObject warmColorCorrectionObj = scene->CreateEntity("Color Correct Warm");
		{
			ColorCorrection* warmColorCorrectEffect = &warmColorCorrectionObj.emplace<ColorCorrection>();
			warmColorCorrectEffect->filename = "cubes/warm_color_correction.cube";
			warmColorCorrectEffect->Init(width, height);
		}
		effects.push_back(warmColorCorrectionObj);

		GameObject coolColorCorrectionObj = scene->CreateEntity("Color Correct Cold");
		{
			ColorCorrection* coolColorCorrectEffect = &coolColorCorrectionObj.emplace<ColorCorrection>();
			coolColorCorrectEffect->filename = "cubes/cool_color_correction.cube";
			coolColorCorrectEffect->Init(width, height);
		}
		effects.push_back(coolColorCorrectionObj);

		GameObject customColorCorrectionObj = scene->CreateEntity("Color Correct Inverted");
		{
			ColorCorrection* customColorCorrectEffect = &customColorCorrectionObj.emplace<ColorCorrection>();
			customColorCorrectEffect->filename = "cubes/custom.cube";
			customColorCorrectEffect->Init(width, height);
		}
		effects.push_back(customColorCorrectionObj);

		GameObject greyscaleEffectObject = scene->CreateEntity("greyscale Effect");
		{
			greyscaleEffect = &greyscaleEffectObject.emplace<GreyscaleEffect>();
			greyscaleEffect->Init(width, height);
		}
		post.push_back(greyscaleEffect);

		GameObject bloomEffectObject = scene->CreateEntity("Bloom Effect");
		{
			bloomEffect = &bloomEffectObject.emplace<BloomEffect>();
			bloomEffect->Init(width, height);
		}
		post.push_back(bloomEffect);

		GameObject filmGrainEffectObject = scene->CreateEntity("Film Grain Effect Effect");
		{
			filmGrainEffect = &filmGrainEffectObject.emplace<FilmGrainEffect>();
			filmGrainEffect->Init(width, height);
		}
		post.push_back(filmGrainEffect);

		GameObject pixelationEffectObject = scene->CreateEntity("Pixelation Effect");
		{
			pixelEffect = &pixelationEffectObject.emplace<PixelationEffect>();
			pixelEffect->Init(width, height);
		}
		post.push_back(pixelEffect);
		//////////////////////////////////////////////////////////////////////////////////////////

		/////////////////////////////////// CONTROLLERS //////////////////////////////////////////
		GameObject gameController = scene->CreateEntity("GameController");
		{
			BehaviourBinding::Bind<GameBehaviour>(gameController);
		}
		GameObject menuController = menuscene->CreateEntity("MenuController");
		{
			BehaviourBinding::Bind<MenuBehaviour>(menuController);
		}
		GameObject pauseController = pausescene->CreateEntity("PauseController");
		{
			BehaviourBinding::Bind<PauseBehaviour>(pauseController);
		}
		GameObject deadController = endscene->CreateEntity("EndController");
		{
			BehaviourBinding::Bind<EndBehaviour>(deadController);
		}
		GameObject endController = winscene->CreateEntity("EndController");
		{
			BehaviourBinding::Bind<EndBehaviour>(endController);
		}
		#pragma endregion 
		//////////////////////////////////////////////////////////////////////////////////////////

		/////////////////////////////////// SKYBOX ///////////////////////////////////////////////
		
			// Load our shaders
			Shader::sptr skybox = std::make_shared<Shader>();
			skybox->LoadShaderPartFromFile("shaders/skybox-shader.vert.glsl", GL_VERTEX_SHADER);
			skybox->LoadShaderPartFromFile("shaders/skybox-shader.frag.glsl", GL_FRAGMENT_SHADER);
			skybox->Link();

			ShaderMaterial::sptr skyboxMat = ShaderMaterial::Create();
			skyboxMat->Shader = skybox;  
			skyboxMat->Set("s_Environment", environmentMap);
			skyboxMat->Set("u_EnvironmentRotation", glm::mat3(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1, 0, 0))));
			skyboxMat->RenderLayer = 10;

			MeshBuilder<VertexPosNormTexCol> mesh;
			MeshFactory::AddIcoSphere(mesh, glm::vec3(0.0f), 1.0f);
			MeshFactory::InvertFaces(mesh);
			VertexArrayObject::sptr meshVao = mesh.Bake();
			
			GameObject skyboxObj = scene->CreateEntity("skybox");  
			skyboxObj.get<Transform>().SetLocalPosition(0.0f, 0.0f, 0.0f);
			//skyboxObj.get_or_emplace<RendererComponent>().SetMesh(meshVao).SetMaterial(skyboxMat);
		

		////////////////////////////////////////////////////////////////////////////////////////

		/////////////////////////////////// UI ///////////////////////////////////////////////
		
		ShaderMaterial::sptr endelmtMat = ShaderMaterial::Create();
		endelmtMat->Shader = uiShader;
		endelmtMat->Set("s_UiTexture", endelement);
		endelmtMat->RenderLayer = -1;

		GameObject endCard = endscene->CreateEntity("Ui");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/plane.obj");
			endCard.emplace<RendererComponent>().SetMesh(vao).SetMaterial(endelmtMat);
			endCard.get<Transform>().SetLocalRotation(-45, 180, 0);
			endCard.get<Transform>().SetLocalPosition(0.0f, 0.0f, 0.25f);
			endCard.get<Transform>().SetLocalScale(2, 2, 2);
		}
		
		ShaderMaterial::sptr titleMat = ShaderMaterial::Create();
		titleMat->Shader = uiShader;
		titleMat->Set("s_UiTexture", title);
		titleMat->RenderLayer = -1;

		GameObject titleCard = menuscene->CreateEntity("Ui");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/plane.obj");
			titleCard.emplace<RendererComponent>().SetMesh(vao).SetMaterial(titleMat);
			//BehaviourBinding::Bind<UIBehaviour>(titleCard);
			//BehaviourBinding::Get<UIBehaviour>(titleCard)->SetCamera(menucameraObject);
			titleCard.get<Transform>().SetLocalRotation(-45, 180, 0);
			titleCard.get<Transform>().SetLocalPosition(0, 0, 0.25f);
		}

		ShaderMaterial::sptr pauseMat = ShaderMaterial::Create();
		pauseMat->Shader = uiShader;
		pauseMat->Set("s_UiTexture", pauseTex);
		pauseMat->RenderLayer = -1;

		GameObject pauseCard = pausescene->CreateEntity("Ui");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/plane.obj");
			pauseCard.emplace<RendererComponent>().SetMesh(vao).SetMaterial(pauseMat);
			//BehaviourBinding::Bind<UIBehaviour>(titleCard);
			//BehaviourBinding::Get<UIBehaviour>(titleCard)->SetCamera(menucameraObject);
			pauseCard.get<Transform>().SetLocalRotation(-45, 180, 0);
			pauseCard.get<Transform>().SetLocalPosition(0, 0, 0.25f);
		}

		ShaderMaterial::sptr uiMat = ShaderMaterial::Create();
		uiMat->Shader = uiShader;
		uiMat->Set("s_UiTexture", uiTex);
		uiMat->RenderLayer = -1;

		GameObject ui = scene->CreateEntity("Ui");
		{
			//VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/plane.obj");
			//ui.emplace<RendererComponent>().SetMesh(vao).SetMaterial(uiMat);
			ui.emplace<UIComponent>().SetMesh().SetMaterial(uiMat);
		}
		////////////////////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////// Sound Setup //////////////////////////////////////////////
		// Setup FMOD
		AudioEngine& engine = AudioEngine::Instance();
		engine.Init();
		engine.LoadBank("Master");
		engine.LoadBank("Master.strings");

		// Add sound events
		AudioEvent& music = engine.CreateSound("Music", "event:/BGM");// Right-click event in fmod -> copy path
		music.Play();
		AudioEvent& playerThumping = engine.CreateSound("Player Thumping", "event:/Thump");
		playerThumping.Play();
		AudioEvent& enemyScratching = engine.CreateSound("Enemy Scratching", "event:/Scratching");
		enemyScratching.Play();
		AudioEvent& enemyAmbient = engine.CreateSound("Enemy Ambient", "event:/Clown Ambient");

		//add modifiers to the sounds (can be done dynamically)
		playerThumping.SetParameter("Moving", 0);
		enemyScratching.SetParameter("Moving", 1);

		// Get ref to Listener
		AudioListener& listener = engine.GetListener(); // Can use this listener to change the player's 3D position
		listener.SetUp(glm::vec3(0.0f, 1.0f, 0.0f));
		///////////////////////////////////////////////////////////////////////////////////////////////////

		// We'll use a vector to store all our key press events for now (this should probably be a behaviour eventually)
		std::vector<KeyPressWatcher> keyToggles;
		{
			// This is an example of a key press handling helper. Look at InputHelpers.h an .cpp to see
			// how this is implemented. Note that the ampersand here is capturing the variables within
			// the scope. If you wanted to do some method on the class, your best bet would be to give it a method and
			// use std::bind
			keyToggles.emplace_back(GLFW_KEY_T, [&]() { cameraObject.get<Camera>().ToggleOrtho(); });

			keyToggles.emplace_back(GLFW_KEY_ESCAPE, [&]() {
				if (Application::Instance().ActiveScene == Globals::Instance().scenes[1])
				{
					BehaviourBinding::Get<FirstPersonBehaviour>(cameraObject)->ToggleMouse();
					music.SetParameter("Paused", 1);
					playerThumping.SetParameter("Paused", 1);
					enemyScratching.SetParameter("Paused", 1);
					BehaviourBinding::Get<EnemyBehaviour>(enemies[0])->Enabled = !BehaviourBinding::Get<EnemyBehaviour>(enemies[0])->Enabled;
				}
				else if (Application::Instance().ActiveScene == Globals::Instance().scenes[3] || Application::Instance().ActiveScene == Globals::Instance().scenes[4])
				{
					directionalLightBuffer.Unbind(0);
					Application::Instance().ActiveScene = nullptr;
					Globals::Instance().scenes.clear();
					BackendHandler::ShutdownImGui();
					engine.Shutdown();
					Logger::Uninitialize();
					exit(0);
				}
				}
			);

			keyToggles.emplace_back(GLFW_KEY_SPACE, [&]() {
				if (Application::Instance().ActiveScene == Globals::Instance().scenes[2])
				{
					BehaviourBinding::Get<FirstPersonBehaviour>(cameraObject)->ToggleMouse();
					music.SetParameter("Paused", 0);
					playerThumping.SetParameter("Paused", 0);
					enemyScratching.SetParameter("Paused", 0);
					BehaviourBinding::Get<EnemyBehaviour>(enemies[0])->Enabled = !BehaviourBinding::Get<EnemyBehaviour>(enemies[0])->Enabled;
				}
				else if (Application::Instance().ActiveScene == Globals::Instance().scenes[3] || Application::Instance().ActiveScene == Globals::Instance().scenes[4])
				{
					for (int i = 0; i < Globals::Instance().coinArray.size(); i++)
					{
						glm::vec2 coords;
						bool goodPlacement = false;
						while (!goodPlacement)
						{
							goodPlacement = true;
							int tubeIndex = rand() % tubes.size();
							for (int safeIndex : Manager.safeindexes)
							{
								if (tubeIndex == safeIndex)
								{
									goodPlacement = false;
									break;
								}
							}
							if (goodPlacement)
								coords = glm::vec2(tubes[tubeIndex].get<Transform>().GetLocalPosition().x, tubes[tubeIndex].get<Transform>().GetLocalPosition().z);
						}

						GameObject coine = scene->CreateEntity("Coin");
						coine.emplace<RendererComponent>().SetMesh(coinvao).SetMaterial(coinMat);
						auto& coinCol = coine.emplace<Collision2D>(pworld->World());
						coinCol.CreateStaticBox(coords, glm::vec2(unitsize / 2, unitsize / 2), PICKUP, PLAYER);
						coinCol.getFixture()->SetSensor(true);
						coinCol.getFixture()->SetEntity(coine.entity());
						auto& coinT = coine.get<Transform>();
						coinT.SetLocalPosition(coords.x, 0, coords.y);
						coinT.SetLocalRotation(90, 0, 90);
						BehaviourBinding::Bind<CoinBehaviour>(coine);
						TriggerBinding::Bind<CoinTrigger>(coine);

						Globals::Instance().coinArray[i] = coine;
					}

					Globals::Instance().coins = 0;
					b2Vec2 spawnLocation = b2Vec2(spawn.x, spawn.y);
					player.get<Collision2D>().getBody()->SetTransform(spawnLocation, 0.0f);
					spawnLocation = b2Vec2(enemySpawn.x, enemySpawn.y);
					enemy.get<Collision2D>().getBody()->SetTransform(spawnLocation, 0.0f);
					BehaviourBinding::Get<EnemyBehaviour>(enemies[0])->ResetAStar(enemySpawn, player);
				}
				}
			);


			keyToggles.emplace_back(GLFW_KEY_1, [&]() {
				activeDef = 0;
				});
			keyToggles.emplace_back(GLFW_KEY_2, [&]() {
				activeDef = 1;
				});
			keyToggles.emplace_back(GLFW_KEY_3, [&]() {
				activeDef = 2;
				});
			keyToggles.emplace_back(GLFW_KEY_4, [&]() {
				activeDef = 3;
				});
			keyToggles.emplace_back(GLFW_KEY_5, [&]() {
				activeDef = 4;
				});

			/*controllables.push_back(obj2);

			keyToggles.emplace_back(GLFW_KEY_KP_ADD, [&]() {
				BehaviourBinding::Get<SimpleMoveBehaviour>(controllables[selectedVao])->Enabled = false;
				selectedVao++;
				if (selectedVao >= controllables.size())
					selectedVao = 0;
				BehaviourBinding::Get<SimpleMoveBehaviour>(controllables[selectedVao])->Enabled = true;
				});
			keyToggles.emplace_back(GLFW_KEY_KP_SUBTRACT, [&]() {
				BehaviourBinding::Get<SimpleMoveBehaviour>(controllables[selectedVao])->Enabled = false;
				selectedVao--;
				if (selectedVao < 0)
					selectedVao = controllables.size() - 1;
				BehaviourBinding::Get<SimpleMoveBehaviour>(controllables[selectedVao])->Enabled = true;
				});

			keyToggles.emplace_back(GLFW_KEY_Y, [&]() {
				auto behaviour = BehaviourBinding::Get<SimpleMoveBehaviour>(controllables[selectedVao]);
				behaviour->Relative = !behaviour->Relative;
				});*/
		}

		// Initialize our timing instance and grab a reference for our use
		Timing& time = Timing::Instance();
		time.LastFrame = glfwGetTime();
		float ambientTimer = 20.0f;

		glClearColor(1.0f, 1.0f, 1.0f, 0.3f);

		///// Game loop /////
		while (!glfwWindowShouldClose(BackendHandler::window)) {
			glfwPollEvents();

			// Update the timing
			time.CurrentFrame = glfwGetTime();
			time.DeltaTime = static_cast<float>(time.CurrentFrame - time.LastFrame);

			time.DeltaTime = time.DeltaTime > 1.0f ? 1.0f : time.DeltaTime;
			
			//BehaviourBinding::Get<EnemyBehaviour>(enemy)->Update(enemy);

			// Update our FPS tracker data
			fpsBuffer[frameIx] = 1.0f / time.DeltaTime;
			frameIx++;
			if (frameIx >= 128)
				frameIx = 0;

			// We'll make sure our UI isn't focused before we start handling input for our game
			if (!ImGui::IsAnyWindowFocused()) {
				// We need to poll our key watchers so they can do their logic with the GLFW state
				// Note that since we want to make sure we don't copy our key handlers, we need a const
				// reference!
				for (const KeyPressWatcher& watcher : keyToggles) {
					watcher.Poll(BackendHandler::window);
				}
			}
			entt::basic_group<entt::entity, entt::exclude_t<>, entt::get_t<Transform>, RendererComponent> renderGroup =
				Application::Instance().ActiveScene->Registry().group<RendererComponent>(entt::get_t<Transform>());

			entt::basic_group<entt::entity, entt::exclude_t<>, entt::get_t<Transform>, UIComponent> uiGroup =
				Application::Instance().ActiveScene->Registry().group<UIComponent>(entt::get_t<Transform>());

			// Iterate over all the behaviour binding components
			Application::Instance().ActiveScene->Registry().view<BehaviourBinding>().each([&](entt::entity entity, BehaviourBinding& binding) {
				// Iterate over all the behaviour scripts attached to the entity, and update them in sequence (if enabled)
				for (const auto& behaviour : binding.Behaviours) {
					if (behaviour->Enabled) {
						behaviour->Update(entt::handle(Application::Instance().ActiveScene->Registry(), entity));
					}
				}
			});
			playerLight._lightPos = glm::vec4(cameraObject.get<Transform>().GetLocalPosition(), 0.0f);
			ui.get<Transform>().SetLocalPosition(cameraObject.get<Transform>().GetLocalPosition() + (glm::vec3(-0.11f, 0.0f, -0.11f) * glm::vec3(sin(glm::radians((int(cameraObject.get<Transform>().GetLocalRotation().x) != 180) ? cameraObject.get<Transform>().GetLocalRotation().y : -cameraObject.get<Transform>().GetLocalRotation().y + cameraObject.get<Transform>().GetLocalRotation().x)), 0.0f, cos(glm::radians((int(cameraObject.get<Transform>().GetLocalRotation().x) != 180) ? cameraObject.get<Transform>().GetLocalRotation().y : -cameraObject.get<Transform>().GetLocalRotation().y + cameraObject.get<Transform>().GetLocalRotation().x)))));
			ui.get<Transform>().SetLocalRotation(90.0f, (int(cameraObject.get<Transform>().GetLocalRotation().x) != 180) ? cameraObject.get<Transform>().GetLocalRotation().y : -cameraObject.get<Transform>().GetLocalRotation().y + cameraObject.get<Transform>().GetLocalRotation().x, 0.0f);
			ui.get<Transform>().SetLocalScale(0.11f * BackendHandler::aspectRatio, 0.11f, 0.11f);

			//update sound
			listener.SetForward(glm::normalize(cameraObject.get<Transform>().GetLocalPosition() - ui.get<Transform>().GetLocalPosition()));
			listener.SetPosition(player.get<Transform>().GetLocalPosition());

			music.SetPosition(listener.GetPosition());

			playerThumping.SetPosition(player.get<Transform>().GetLocalPosition());
			enemyScratching.SetPosition(enemy.get<Transform>().GetLocalPosition());
			enemyAmbient.SetPosition(enemy.get<Transform>().GetLocalPosition());
			if (ambientTimer <= 0.0f)
			{
				ambientTimer = 20.0f;
				enemyAmbient.Play();
			}
			else
				ambientTimer -= time.DeltaTime;

			pworld->Update(time.DeltaTime);
			
			
			// Clear the screen
			testBuffer->Clear();
			for (int i = 0; i < effects.size(); i++)
			{
				effects[i].get<ColorCorrection>().Clear();
			}
			for (int i = 0; i < post.size(); i++)
			{
				post[i]->Clear();
			}
			shadowBuffer->Clear();
			gBuffer->Clear();
			illuminationBuffer->Clear();

			glEnable(GL_DEPTH_TEST);
			glClearDepth(1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Update all world matrices for this frame
			Application::Instance().ActiveScene->Registry().view<Transform>().each([](entt::entity entity, Transform& t) {
				t.UpdateWorldMatrix();
			});

			Transform& camTransform = cameraObject.get<Transform>();
			glm::mat4 view;
			glm::mat4 projection;
			glm::mat4 viewProjection;

			if (Application::Instance().ActiveScene == scene) {
				// Grab out camera info from the camera object
				illuminationBuffer->EnableSun(true);
				camTransform = cameraObject.get<Transform>();
				view = glm::inverse(camTransform.LocalTransform());
				projection = cameraObject.get<Camera>().GetProjection();
				viewProjection = projection * view;
			}
			else {
				illuminationBuffer->EnableSun(false);
				camTransform = menucameraObject.get<Transform>();
				view = glm::inverse(camTransform.LocalTransform());
				projection = menucameraObject.get<Camera>().GetProjection();
				viewProjection = projection * view;
			}
			
			//Set up light space matrix
			glm::mat4 lightProjectionMatrix = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, -30.0f, 30.0f);
			glm::mat4 lightViewMatrix = glm::lookAt(glm::vec3(-illuminationBuffer->GetSunRef()._lightDirection), glm::vec3(), glm::vec3(0.0f, 0.0f, 1.0f));
			glm::mat4 lightSpaceViewProj = lightProjectionMatrix * lightViewMatrix;

			illuminationBuffer->SetLightSpaceViewProj(lightSpaceViewProj);
			glm::vec3 camPos = glm::inverse(view) * glm::vec4(0, 0, 0, 1);
			illuminationBuffer->SetCamPos(camPos);
			
			//Animations
			if (Application::Instance().ActiveScene == Globals::Instance().scenes[1])
			{
				enemy.get<ObjAnimation>().UpdateAnimation(time.DeltaTime);
				enemy.get<RendererComponent>().SetMesh(enemy.get<ObjAnimation>().LoadMesh()).SetMaterial(ratMat);
			}

			// Sort the renderers by shader and material, we will go for a minimizing context switches approach here,
			// but you could for instance sort front to back to optimize for fill rate if you have intensive fragment shaders
			renderGroup.sort<RendererComponent>([](const RendererComponent& l, const RendererComponent& r) {
				// Sort by render layer first, higher numbers get drawn last
				if (l.Material->RenderLayer < r.Material->RenderLayer) return true;
				if (l.Material->RenderLayer > r.Material->RenderLayer) return false;

				// Sort by shader pointer next (so materials using the same shader run sequentially where possible)
				if (l.Material->Shader < r.Material->Shader) return true;
				if (l.Material->Shader > r.Material->Shader) return false;

				// Sort by material pointer last (so we can minimize switching between materials)
				if (l.Material < r.Material) return true;
				if (l.Material > r.Material) return false;
				
				return false;
			});

			// Start by assuming no shader or material is applied
			Shader::sptr current = nullptr;
			ShaderMaterial::sptr currentMat = nullptr;

			glViewport(0, 0, shadowWidth, shadowHeight);
			shadowBuffer->Bind();

			renderGroup.each([&](entt::entity e, RendererComponent& renderer, Transform& transform) {
				// Render the mesh
				if (renderer.CastShadows)
				{
					BackendHandler::RenderVAO(simpleDepthShader, renderer.Mesh, viewProjection, transform, lightSpaceViewProj);
				}
				});

			shadowBuffer->Unbind();

			glfwGetWindowSize(BackendHandler::window, &width, &height);

			glViewport(0, 0, width, height);
			gBuffer->Bind();
			// Iterate over the render group components and draw them
			renderGroup.each( [&](entt::entity e, RendererComponent& renderer, Transform& transform) {
				// If the shader has changed, set up it's uniforms
				if (current != renderer.Material->Shader) {
					current = renderer.Material->Shader;
					current->Bind();
					BackendHandler::SetupShaderForFrame(current, view, projection);
				}
				// If the material has changed, apply it
				if (currentMat != renderer.Material) {
					currentMat = renderer.Material;
					currentMat->Apply();
				}

				shadowBuffer->BindDepthAsTexture(30);
				// Render the mesh
				BackendHandler::RenderVAO(renderer.Material->Shader, renderer.Mesh, viewProjection, transform, lightSpaceViewProj);
			});

			gBuffer->Unbind();

			illuminationBuffer->BindBuffer(0);
			skybox->Bind();
			BackendHandler::SetupShaderForFrame(skybox, view, projection);
			skyboxMat->Apply();
			BackendHandler::RenderVAO(skybox, meshVao, viewProjection, skyboxObj.get<Transform>(), lightSpaceViewProj);
			skybox->UnBind();

			illuminationBuffer->UnbindBuffer();

			shadowBuffer->BindDepthAsTexture(30);

			illuminationBuffer->ApplyEffect(gBuffer);

			/*if (drawGBuffer)
			{
				gBuffer->DrawBuffersToScreen();
			}
			else if (drawIllumBuffer)
			{
				illuminationBuffer->DrawIllumBuffer();
			}
			else
			{
				illuminationBuffer->DrawToScreen();
			}*/

			if (post[activePost] == filmGrainEffect)
			{
				FilmGrainEffect* temp = (FilmGrainEffect*)post[activePost];
				temp->SetTime(float(glfwGetTime()));
			}

			switch (activeDef) {
			case 1: gBuffer->DrawBuffer(3); break;
			case 2: gBuffer->DrawBuffer(1); break;
			case 3: gBuffer->DrawBuffer(0); break;
			case 4: illuminationBuffer->DrawIllumBuffer(); break;
			default: illuminationBuffer->DrawToScreen();
				post[activePost]->ApplyEffect(illuminationBuffer);
				post[activePost]->DrawToScreen();

				PostEffect* currentEffect = &effects[activeEffect].get<ColorCorrection>();
				currentEffect->ApplyEffect(post[activePost]);
				currentEffect->DrawToScreen();
				currentEffect->UnbindBuffer(); break;
			}

			//testBuffer->DrawToBackbuffer();
			

			
			current = nullptr;
			currentMat = nullptr;
			/*uiGroup.each([&](entt::entity e, UIComponent& renderer, Transform& transform) {
				// If the shader has changed, set up it's uniforms
				if (current != renderer.Material->Shader) {
					current = renderer.Material->Shader;
					current->Bind();
					BackendHandler::SetupShaderForFrame(current, view, projection);
				}
				// If the material has changed, apply it
				if (currentMat != renderer.Material) {
					currentMat = renderer.Material;
					currentMat->Apply();
				}
				BackendHandler::RenderVAO(renderer.Material->Shader, renderer.Mesh, viewProjection, transform);
				});*/
			
			// Draw our ImGui content
			BackendHandler::RenderImGui();
			engine.Update();

			Application::Instance().ActiveScene->Poll();
			glfwSwapBuffers(BackendHandler::window);
			time.LastFrame = time.CurrentFrame;
		}
		directionalLightBuffer.Unbind(0);
		// Nullify scene so that we can release references
		Application::Instance().ActiveScene = nullptr;
		Globals::Instance().scenes.clear();
		BackendHandler::ShutdownImGui();
		engine.Shutdown();
	}	

	// Clean up the toolkit logger so we don't leak memory
	Logger::Uninitialize();
	return 0;
}