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
#include <VertexTypes.h>
#include <ShaderMaterial.h>
#include <RendererComponent.h>
#include <TextureCubeMap.h>
#include <TextureCubeMapData.h>

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

#include "Graphics/UIComponent.h"

#include "Utilities/Globals.h"

#include "Utilities/Trigger.h"
#include "Triggers/CoinTrigger.h"
#include "Triggers/EnemyTrigger.h"
#include "Triggers/ExitTrigger.h"

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

		Shader::sptr uiShader = Shader::Create();
		uiShader->LoadShaderPartFromFile("shaders/ui_vert.glsl", GL_VERTEX_SHADER);
		uiShader->LoadShaderPartFromFile("shaders/ui_frag.glsl", GL_FRAGMENT_SHADER);
		uiShader->Link();

		Shader::sptr shader = Shader::Create();
		shader->LoadShaderPartFromFile("shaders/vertex_shader.glsl", GL_VERTEX_SHADER);
		shader->LoadShaderPartFromFile("shaders/frag_blinn_phong_textured.glsl", GL_FRAGMENT_SHADER);
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

		PostEffect* testBuffer;
		PostEffect* uiBuffer;

		int activeEffect = 0;
		std::vector<GameObject> effects;
		
		GreyscaleEffect* greyscaleEffect;
		BloomEffect* bloomEffect;
		PixelationEffect* pixelEffect;

		int activePost = 1;
		std::vector<PostEffect*> post;
		bool ramp=0;
	
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
				if (ImGui::SliderInt("Active Post Effect", &activePost, 0, 2));

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
				if (activePost == 2 && ImGui::CollapsingHeader("Pixelation Effect controls")) {
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
		Texture2D::sptr rattex = Texture2D::LoadFromFile("images/f.png");

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

		Texture2D::sptr testUI = Texture2D::LoadFromFile("images/testUI.png");
		Texture2D::sptr title = Texture2D::LoadFromFile("images/title_transparent.png");
		Texture2D::sptr spcelement = Texture2D::LoadFromFile("images/spcelement.png");
		Texture2D::sptr endelement = Texture2D::LoadFromFile("images/endelement.png");


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
		stoneMat->Shader = shader;
		stoneMat->Set("s_Diffuse", stone);
		stoneMat->Set("s_Specular", stoneSpec);
		stoneMat->Set("u_Shininess", 2.0f);
		stoneMat->Set("u_TextureMix", 0.0f); 

		ShaderMaterial::sptr metalMat = ShaderMaterial::Create();
		metalMat->Shader = shader;
		metalMat->Set("s_Diffuse", metal);
		metalMat->Set("s_Specular", noSpec);
		metalMat->Set("u_Shininess", 10.0f);
		metalMat->Set("u_TextureMix", 0.0f);

		ShaderMaterial::sptr grassMat = ShaderMaterial::Create();
		grassMat->Shader = shader;
		grassMat->Set("s_Diffuse", grass);
		grassMat->Set("s_Specular", noSpec);
		grassMat->Set("u_Shininess", 2.0f);
		grassMat->Set("u_TextureMix", 0.0f);

		ShaderMaterial::sptr boxMat = ShaderMaterial::Create();
		boxMat->Shader = shader;
		boxMat->Set("s_Diffuse", box);
		boxMat->Set("s_Specular", boxSpec);
		boxMat->Set("u_Shininess", 8.0f);
		boxMat->Set("u_TextureMix", 0.0f);

		ShaderMaterial::sptr simpleFloraMat = ShaderMaterial::Create();
		simpleFloraMat->Shader = shader;
		simpleFloraMat->Set("s_Diffuse", simpleFlora);
		simpleFloraMat->Set("s_Specular", noSpec);
		simpleFloraMat->Set("u_Shininess", 8.0f);
		simpleFloraMat->Set("u_TextureMix", 0.0f);

		ShaderMaterial::sptr ratMat = ShaderMaterial::Create();
		ratMat->Shader = shader;
		ratMat->Set("s_Diffuse", rattex);
		ratMat->Set("s_Specular", noSpec);
		ratMat->Set("u_Shininess", 8.0f);
		ratMat->Set("u_TextureMix", 0.0f);

		ShaderMaterial::sptr testuiMat = ShaderMaterial::Create();
		simpleFloraMat->Shader = uiShader;
		simpleFloraMat->Set("s_Diffuse", testUI);
		simpleFloraMat->Set("s_Specular", noSpec);
		simpleFloraMat->Set("u_Shininess", 8.0f);
		simpleFloraMat->Set("u_TextureMix", 0.0f);
		
		VertexArrayObject::sptr coinvao = ObjLoader::LoadFromFile("models/coin.obj");
		std::vector<GameObject> coins;
		ShaderMaterial::sptr coinMat = ShaderMaterial::Create();
		coinMat->Shader = shader;
		coinMat->Set("s_Diffuse", coin);
		coinMat->Set("s_Specular", noSpec);
		coinMat->Set("u_Shininess", 8.0f);
		coinMat->Set("u_TextureMix", 0.0f);
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
		std::vector<GameObject> tubes;

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
					tMat->Shader = shader;
					tMat->Set("u_Shininess", 20.0f);
					tMat->Set("u_TextureMix", 0.0f);

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
						tubee.emplace<RendererComponent>().SetMesh(tubestr).SetMaterial(tMat);
						rvte.emplace<RendererComponent>().SetMesh(rvtstr).SetMaterial(metalMat);
						break;
					case 2:
						tMat->Set("s_Specular", tlbw);
						tubee.emplace<RendererComponent>().SetMesh(tubelbw).SetMaterial(tMat);
						rvte.emplace<RendererComponent>().SetMesh(rvtlbw).SetMaterial(metalMat);
						break;
					case 3:
						tMat->Set("s_Specular", ttee);
						tubee.emplace<RendererComponent>().SetMesh(tubetee).SetMaterial(tMat);
						rvte.emplace<RendererComponent>().SetMesh(rvttee).SetMaterial(metalMat);
						break;
					case 4:
						tMat->Set("s_Specular", noSpec);
						tubee.emplace<RendererComponent>().SetMesh(tubeqd).SetMaterial(tMat);
						rvte.emplace<RendererComponent>().SetMesh(rvtqd).SetMaterial(metalMat);
						break;
					default:
						tMat->Set("s_Specular", noSpec);
						tubee.emplace<RendererComponent>().SetMesh(tubend).SetMaterial(tMat);
						Manager.saferooms.push_back(glm::vec2(coord1, coord2));
						pspawn = true;
						canspawn = false;
						break;
					}
					
					//Set tubing orientation based on coordinates and rotation data given by the manager
					auto& tubeT = tubee.get<Transform>();
					tubeT.SetLocalPosition(coord1, 0, coord2);
					tubeT.SetLocalRotation(glm::vec3(0.0f, tubeData.y, 0.0f));

					auto& rvtT = rvte.get<Transform>();
					rvtT.SetLocalPosition(coord1, 0, coord2);
					rvtT.SetLocalRotation(glm::vec3(0.0f, tubeData.y, 0.0f));

					int r = rand() % 5;
					if (r == 3 && canspawn) {
						GameObject coine = scene->CreateEntity("Coin");
						coins.push_back(coine);
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
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/rat_head_on_clown_body_test_head_attached.obj");
			enemy.emplace<RendererComponent>().SetMesh(vao).SetMaterial(ratMat);
			auto& enemyCol = enemy.emplace<Collision2D>(pworld->World());
			enemyCol.CreateDynamicBox(enemySpawn, glm::vec2(1, 1), ENEMY, PLAYER);
			enemyCol.getBody()->SetUserData(&enemy);
			enemyCol.getFixture()->SetSensor(true);
			enemyCol.getFixture()->SetEntity(enemy.entity());
			enemy.get<Transform>().SetLocalScale(0.15f, 0.15f, 0.15f);
			BehaviourBinding::Bind<EnemyBehaviour>(enemy);
			BehaviourBinding::Get<EnemyBehaviour>(enemy)->SetTarget(player);
			TriggerBinding::Bind<EnemyTrigger>(enemy);
		}
		
		GameObject exit = scene->CreateEntity("Exit");
		{
			//VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/plane.obj");
			//player.emplace<RendererComponent>().SetMesh(vao).SetMaterial(stoneMat);
			auto& exitCol = exit.emplace<Collision2D>(pworld->World());
			exitCol.CreateStaticBox(exitloc, glm::vec2(2, 2), TRIGGER, PLAYER);
			exitCol.getFixture()->SetSensor(true);
			exitCol.getFixture()->SetEntity(exit.entity());

		}
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
		GameObject menucameraObject = menuscene->CreateEntity("Camera");
		{
			menucameraObject.get<Transform>().SetLocalPosition(0, 1, 1).LookAt(glm::vec3(0, 0, 0));

			// We'll make our camera a component of the camera object
			Camera& menucamera = menucameraObject.emplace<Camera>();// Camera::Create();
			menucamera.SetPosition(glm::vec3(0, 0, 0));
			menucamera.SetUp(glm::vec3(0, 0, 1));
			menucamera.LookAt(glm::vec3(0, 0, 0));
			menucamera.SetFovDegrees(90.0f); // Set an initial FOV
			menucamera.SetOrthoHeight(0.0f);
		}

		GameObject framebufferObject = scene->CreateEntity("Basic Buffer");
		{
			testBuffer = &framebufferObject.emplace<PostEffect>();
			testBuffer->Init(width, height);
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
			//BehaviourBinding::Bind<GameBehaviour>(gameController);
		}
		GameObject menuController = menuscene->CreateEntity("MenuController");
		{
			BehaviourBinding::Bind<MenuBehaviour>(menuController);
		}
		GameObject pauseController = pausescene->CreateEntity("PauseController");
		{
			//BehaviourBinding::Bind<PauseBehaviour>(pauseController);
		}
		#pragma endregion 
		//////////////////////////////////////////////////////////////////////////////////////////

		/////////////////////////////////// SKYBOX ///////////////////////////////////////////////
		{
			// Load our shaders
			Shader::sptr skybox = std::make_shared<Shader>();
			skybox->LoadShaderPartFromFile("shaders/skybox-shader.vert.glsl", GL_VERTEX_SHADER);
			skybox->LoadShaderPartFromFile("shaders/skybox-shader.frag.glsl", GL_FRAGMENT_SHADER);
			skybox->Link();

			ShaderMaterial::sptr skyboxMat = ShaderMaterial::Create();
			skyboxMat->Shader = skybox;  
			skyboxMat->Set("s_Environment", environmentMap);
			skyboxMat->Set("u_EnvironmentRotation", glm::mat3(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1, 0, 0))));
			skyboxMat->RenderLayer = 1;

			MeshBuilder<VertexPosNormTexCol> mesh;
			MeshFactory::AddIcoSphere(mesh, glm::vec3(0.0f), 1.0f);
			MeshFactory::InvertFaces(mesh);
			VertexArrayObject::sptr meshVao = mesh.Bake();
			
			GameObject skyboxObj = scene->CreateEntity("skybox");  
			skyboxObj.get<Transform>().SetLocalPosition(0.0f, 0.0f, 0.0f);
			skyboxObj.get_or_emplace<RendererComponent>().SetMesh(meshVao).SetMaterial(skyboxMat);
		}

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
			titleCard.get<Transform>().SetLocalPosition(0, 0, 0.25);
			titleCard.get<Transform>().SetLocalScale(1, 1, 1);
		}

		ShaderMaterial::sptr spcelMat = ShaderMaterial::Create();
		spcelMat->Shader = uiShader;
		spcelMat->Set("s_UiTexture", spcelement);
		spcelMat->RenderLayer = -1;

		GameObject spaceElement = menuscene->CreateEntity("Ui");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/plane.obj");
			spaceElement.emplace<UIComponent>().SetMesh().SetMaterial(spcelMat);
			spaceElement.get<Transform>().SetLocalRotation(-45, 180, 0);
			spaceElement.get<Transform>().SetLocalPosition(0, 0, -1.5);
			spaceElement.get<Transform>().SetLocalScale(2, 2, 2);
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

		// We'll use a vector to store all our key press events for now (this should probably be a behaviour eventually)
		std::vector<KeyPressWatcher> keyToggles;
		{
			// This is an example of a key press handling helper. Look at InputHelpers.h an .cpp to see
			// how this is implemented. Note that the ampersand here is capturing the variables within
			// the scope. If you wanted to do some method on the class, your best bet would be to give it a method and
			// use std::bind
			keyToggles.emplace_back(GLFW_KEY_T, [&]() { cameraObject.get<Camera>().ToggleOrtho(); });

			keyToggles.emplace_back(GLFW_KEY_ESCAPE, [&]() { BehaviourBinding::Get<FirstPersonBehaviour>(cameraObject)->ToggleMouse(); });

			keyToggles.emplace_back(GLFW_KEY_1, [&]() {
				mode = 1;
				});
			keyToggles.emplace_back(GLFW_KEY_2, [&]() {
				mode = 2;
				});
			keyToggles.emplace_back(GLFW_KEY_3, [&]() {
				mode = 3;
				});
			keyToggles.emplace_back(GLFW_KEY_4, [&]() {
				mode = 0;
				});
			keyToggles.emplace_back(GLFW_KEY_5, [&]() {
				mode = 4;
				});
			keyToggles.emplace_back(GLFW_KEY_6, [&]() {
				mode = 5;
				});
			keyToggles.emplace_back(GLFW_KEY_7, [&]() {
				mode = 6;
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

		// Initialize our timing instance and grab a reference for our use
		Timing& time = Timing::Instance();
		time.LastFrame = glfwGetTime();
		float ambientTimer = 20.0f;

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

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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
				camTransform = cameraObject.get<Transform>();
				view = glm::inverse(camTransform.LocalTransform());
				projection = cameraObject.get<Camera>().GetProjection();
				viewProjection = projection * view;
			}
			else {
				camTransform = menucameraObject.get<Transform>();
				view = glm::inverse(camTransform.LocalTransform());
				projection = menucameraObject.get<Camera>().GetProjection();
				viewProjection = projection * view;
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

			testBuffer->BindBuffer(0);

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
				// Render the mesh
				BackendHandler::RenderVAO(renderer.Material->Shader, renderer.Mesh, viewProjection, transform);
			});

			testBuffer->UnbindBuffer();

			//testBuffer->DrawToBackbuffer();
			current = nullptr;
			currentMat = nullptr;

			post[activePost]->ApplyEffect(testBuffer);
			post[activePost]->DrawToScreen();

			uiGroup.each([&](entt::entity e, UIComponent& renderer, Transform& transform) {
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
				// Render the mesh
				BackendHandler::RenderVAO(renderer.Material->Shader, renderer.Mesh, viewProjection, transform);
				});

			PostEffect* currentEffect = &effects[activeEffect].get<ColorCorrection>();
			currentEffect->ApplyEffect(post[activePost]);
			currentEffect->DrawToScreen();
			currentEffect->UnbindBuffer();
			
			// Draw our ImGui content
			BackendHandler::RenderImGui();
			engine.Update();

			Application::Instance().ActiveScene->Poll();
			glfwSwapBuffers(BackendHandler::window);
			time.LastFrame = time.CurrentFrame;
		}

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