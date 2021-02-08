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
		Shader::sptr shader = Shader::Create();
		shader->LoadShaderPartFromFile("shaders/vertex_shader.glsl", GL_VERTEX_SHADER);
		shader->LoadShaderPartFromFile("shaders/frag_blinn_phong_textured.glsl", GL_FRAGMENT_SHADER);
		shader->Link();

		glm::vec3 lightPos = glm::vec3(0.0f, 0.0f, 5.0f);
		glm::vec3 lightCol = glm::vec3(0.9f, 0.85f, 0.5f);
		float     lightAmbientPow = 0.05f;
		float     lightSpecularPow = 1.0f;
		glm::vec3 ambientCol = glm::vec3(1.0f);
		float     ambientPow = 0.1f;
		float     lightLinearFalloff = 0.09f;
		float     lightQuadraticFalloff = 0.032f;

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

		PostEffect* testBuffer;

		int activeEffect = 0;
		std::vector<PostEffect*> effects;

		ColorCorrection* colorCorrectEffect;
		int width, height;
		glfwGetWindowSize(BackendHandler::window, &width, &height);

		// We'll add some ImGui controls to control our shader
		BackendHandler::imGuiCallbacks.push_back([&]() {

			if (ImGui::Button("Base Lighting")) {

				shader->SetUniform("u_AmbientCol", glm::vec3(0.0f));
				shader->SetUniform("u_AmbientStrength", 0.0f);
				shader->SetUniform("u_AmbientLightStrength", 1.0f);

				shader->SetUniform("u_LightPos", glm::vec3(0.0f, 0.0f, 0.0f));

				shader->SetUniform("u_SpecularLightStrength", 0.0f);

				shader->SetUniform("u_Cel", (int)false);

				colorCorrectEffect->filename = "cubes/test.cube";
				colorCorrectEffect->Init(width, height);
				effects[activeEffect]->ApplyEffect(testBuffer);
				effects[activeEffect]->DrawToScreen();

			}
			if (ImGui::Button("Ambient")) {
				shader->SetUniform("u_AmbientCol", glm::vec3(1.0f));
				shader->SetUniform("u_AmbientStrength", 0.3f);
				shader->SetUniform("u_AmbientLightStrength", 0.5f);

				shader->SetUniform("u_LightPos", glm::vec3(0.0f, 0.0f, 0.0f));

				shader->SetUniform("u_SpecularLightStrength", 0.0f);

				shader->SetUniform("u_Cel", (int)false);

			}
			if (ImGui::Button("Specular")) {
				shader->SetUniform("u_SpecularLightStrength", 1.0f);

				shader->SetUniform("u_LightPos", glm::vec3(0.0f, 0.0f, 0.0f));
				shader->SetUniform("u_AmbientCol", glm::vec3(0.0f));
				shader->SetUniform("u_AmbientStrength", 0.0f);
				shader->SetUniform("u_AmbientLightStrength", 0.0f);

				shader->SetUniform("u_Cel", (int)false);

			}
			if (ImGui::Button("Diffuse")) {
				shader->SetUniform("u_LightPos", glm::vec3(0.0f, 0.0f, 2.0f));

				shader->SetUniform("u_SpecularLightStrength", 0.0f);
				shader->SetUniform("u_AmbientCol", glm::vec3(0.0f));
				shader->SetUniform("u_AmbientStrength", 0.0f);
				shader->SetUniform("u_AmbientLightStrength", 0.0f);

				shader->SetUniform("u_Cel", (int)false);

			}

			if (ImGui::Button("Ambient+Specular+Diffuse")) {
				shader->SetUniform("u_LightPos", glm::vec3(0.0f, 0.0f, 2.0f));

				shader->SetUniform("u_AmbientCol", glm::vec3(1.0f));
				shader->SetUniform("u_AmbientStrength", 0.3f);
				shader->SetUniform("u_AmbientLightStrength", 0.5f);

				shader->SetUniform("u_SpecularLightStrength", 1.0f);

				shader->SetUniform("u_Cel", (int)false);

			}

			if (ImGui::Button("Special")) {
				shader->SetUniform("u_LightPos", glm::vec3(0.0f, 0.0f, 2.0f));

				shader->SetUniform("u_AmbientCol", glm::vec3(1.0f));
				shader->SetUniform("u_AmbientStrength", 0.3f);
				shader->SetUniform("u_AmbientLightStrength", 0.5f);

				shader->SetUniform("u_SpecularLightStrength", 1.0f);

				shader->SetUniform("u_Cel", (int)true);
			}
			if (ImGui::Button("Diffuse Ramp")) {
				shader->SetUniform("u_LightPos", glm::vec3(0.0f, 0.0f, 2.0f));

				shader->SetUniform("u_AmbientCol", glm::vec3(1.0f));
				shader->SetUniform("u_AmbientStrength", 0.3f);
				shader->SetUniform("u_AmbientLightStrength", 0.5f);

				shader->SetUniform("u_SpecularLightStrength", 1.0f);

				shader->SetUniform("u_Cel", (int)false);
			}
			if (ImGui::Button("Specular Ramp")) {
				shader->SetUniform("u_LightPos", glm::vec3(0.0f, 0.0f, 0.0f));

				shader->SetUniform("u_AmbientCol", glm::vec3(0.0f));
				shader->SetUniform("u_AmbientStrength", 0.0f);
				shader->SetUniform("u_AmbientLightStrength", 0.0f);

				shader->SetUniform("u_SpecularLightStrength", 1.0f);

				shader->SetUniform("u_Cel", (int)false);


			}
			if (ImGui::Button("Color Grading Warm")) {
				shader->SetUniform("u_LightPos", glm::vec3(0.0f, 0.0f, 2.0f));

				shader->SetUniform("u_AmbientCol", glm::vec3(1.0f));
				shader->SetUniform("u_AmbientStrength", 0.3f);
				shader->SetUniform("u_AmbientLightStrength", 0.5f);

				shader->SetUniform("u_SpecularLightStrength", 1.0f);

				shader->SetUniform("u_Cel", (int)false);

				colorCorrectEffect->filename= "cubes/test.cube";
				colorCorrectEffect->Init(width, height);
				effects[activeEffect]->ApplyEffect(testBuffer);
				effects[activeEffect]->DrawToScreen();
			}
			if (ImGui::Button("Color Grading Cool")) {
				shader->SetUniform("u_LightPos", glm::vec3(0.0f, 0.0f, 2.0f));

				shader->SetUniform("u_AmbientCol", glm::vec3(1.0f));
				shader->SetUniform("u_AmbientStrength", 0.3f);
				shader->SetUniform("u_AmbientLightStrength", 0.5f);

				shader->SetUniform("u_SpecularLightStrength", 1.0f);

				shader->SetUniform("u_Cel", (int)false);

				colorCorrectEffect->filename = "cubes/BrightenedCorrection.cube";
				colorCorrectEffect->Init(width, height);
				effects[activeEffect]->ApplyEffect(testBuffer);
				effects[activeEffect]->DrawToScreen();

			}
			if (ImGui::Button("Color Grading Custom")) {
				shader->SetUniform("u_LightPos", glm::vec3(0.0f, 0.0f, 2.0f));

				shader->SetUniform("u_AmbientCol", glm::vec3(1.0f));
				shader->SetUniform("u_AmbientStrength", 0.3f);
				shader->SetUniform("u_AmbientLightStrength", 0.5f);

				shader->SetUniform("u_SpecularLightStrength", 1.0f);

				shader->SetUniform("u_Cel", (int)false);


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

		Texture2D::sptr blue = Texture2D::LoadFromFile("images/blue.png");
		Texture2D::sptr green = Texture2D::LoadFromFile("images/green.png");
		Texture2D::sptr orange = Texture2D::LoadFromFile("images/orange.png");
		Texture2D::sptr red = Texture2D::LoadFromFile("images/red.png");
		Texture2D::sptr yellow = Texture2D::LoadFromFile("images/yellow.png");

		// Load the cube map
		//TextureCubeMap::sptr environmentMap = TextureCubeMap::LoadFromImages("images/cubemaps/skybox/sample.jpg");
		TextureCubeMap::sptr environmentMap = TextureCubeMap::LoadFromImages("images/cubemaps/skybox/ToonSky.jpg"); 

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
		GameScene::RegisterComponentType<Collision2D>();
		GameScene::RegisterComponentType<Camera>();

		// Create a scene, and set it to be the active scene in the application
		GameScene::sptr scene = GameScene::Create("test");
		Application::Instance().ActiveScene = scene;
		PhysicsWorld::sptr pworld = PhysicsWorld::Create(scene);

		// We can create a group ahead of time to make iterating on the group faster
		entt::basic_group<entt::entity, entt::exclude_t<>, entt::get_t<Transform>, RendererComponent> renderGroup =
			scene->Registry().group<RendererComponent>(entt::get_t<Transform>());

		// Create a material and set some properties for it
		ShaderMaterial::sptr stoneMat = ShaderMaterial::Create();  
		stoneMat->Shader = shader;
		stoneMat->Set("s_Diffuse", stone);
		stoneMat->Set("s_Specular", stoneSpec);
		stoneMat->Set("u_Shininess", 2.0f);
		stoneMat->Set("u_TextureMix", 0.0f); 

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

		VertexArrayObject::sptr coin = ObjLoader::LoadFromFile("models/coin.obj");
		std::vector<GameObject> coins;
		ShaderMaterial::sptr coinMat = ShaderMaterial::Create();
		coinMat->Shader = shader;
		coinMat->Set("s_Diffuse", yellow);
		coinMat->Set("s_Specular", noSpec);
		coinMat->Set("u_Shininess", 8.0f);
		coinMat->Set("u_TextureMix", 0.0f);
		int coincount = 0;

		VertexArrayObject::sptr tubestr = ObjLoader::LoadFromFile("models/tubestr.obj");
		VertexArrayObject::sptr tubelbw = ObjLoader::LoadFromFile("models/tubelbw.obj");
		VertexArrayObject::sptr tubetee = ObjLoader::LoadFromFile("models/tubetee.obj");
		VertexArrayObject::sptr tubeqd = ObjLoader::LoadFromFile("models/tubeqd.obj");
		VertexArrayObject::sptr tubend = ObjLoader::LoadFromFile("models/tubend.obj");
		std::vector<GameObject> tubes;

		VertexArrayObject::sptr wallobj = ObjLoader::LoadFromFile("models/wall.obj");
		std::vector<GameObject> walls;

		glm::vec2 spawn = glm::vec2(0, 0);

		//Call map managing tool and pass level data
		MapManager::sptr Manager = MapManager::Create();
		Manager->LoadFromFile("level.lvl");

		//Get map data from the manager
		auto& map = Manager->GetMap();
		int x = Manager->GetRows();
		int y = Manager->GetColumns();
		int unitsize = Manager->GetUnitS();

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
					tMat->Set("s_Specular", noSpec);
					tMat->Set("u_Shininess", 8.0f);
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

					//Pass the location in the array to the manager to get appropriate piece
					glm::vec2 tubeData = Manager->GetTube(glm::vec2(i, j));
					switch (int(tubeData.x)) {
					case 1:
						tubee.emplace<RendererComponent>().SetMesh(tubestr).SetMaterial(tMat);
						break;
					case 2:
						tubee.emplace<RendererComponent>().SetMesh(tubelbw).SetMaterial(tMat);
						break;
					case 3:
						tubee.emplace<RendererComponent>().SetMesh(tubetee).SetMaterial(tMat);
						break;
					case 4:
						tubee.emplace<RendererComponent>().SetMesh(tubeqd).SetMaterial(tMat);
						break;
					default:
						tubee.emplace<RendererComponent>().SetMesh(tubend).SetMaterial(tMat);
						pspawn = true;
						canspawn = false;
						break;
					}

					//If valid spot to spawn player, set spawn coordinates to tubing piece
					if (pspawn) {
						spawn = glm::vec2(coord1, coord2);
					}
					
					//Set tubing orientation based on coordinates and rotation data given by the manager
					auto& tubeT = tubee.get<Transform>();
					tubeT.SetLocalPosition(coord1, 0, coord2);
					tubeT.SetLocalRotation(glm::vec3(0.0f, tubeData.y, 0.0f));

					int r = rand() % 5;
					if (r == 3 && canspawn) {
						GameObject coine = scene->CreateEntity("Coin");
						coins.push_back(coine);
						coine.emplace<RendererComponent>().SetMesh(coin).SetMaterial(coinMat);
						auto& coinCol = coine.emplace<Collision2D>(pworld->World());
						coinCol.CreateStaticSensor(glm::vec2(coord1, coord2), glm::vec2(unitsize / 2, unitsize / 2));
						coinCol.getBody()->SetUserData(&coine);
						auto& coinT = coine.get<Transform>();
						coinT.SetLocalPosition(coord1, 0, coord2);
						coinT.SetLocalRotation(90, 0, 90);
						coincount++;
					}
				}
				else if (map[i][j] == 0) {
					GameObject walle = scene->CreateEntity("wall");
					walls.push_back(walle);
					auto& wallCol = walle.emplace<Collision2D>(pworld->World());
					//wallCol.getBody()->SetUserData(&walle);
					wallCol.CreateStaticBox(glm::vec2(coord1, coord2), glm::vec2(unitsize / 2, unitsize / 2));
					auto& wallT = walle.get<Transform>();
					wallT.SetLocalPosition(coord1, 0, coord2);
				}
			}
		}



		GameObject player = scene->CreateEntity("player");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/plane.obj");
			player.emplace<RendererComponent>().SetMesh(vao).SetMaterial(stoneMat);
			auto& playerCol = player.emplace<Collision2D>(pworld->World());
			playerCol.CreateDynamicBox(spawn, glm::vec2(2, 2));
			playerCol.getBody()->SetUserData(&player);
			//playerCol.getBody()->SetAngularDamping(1.0);
			playerCol.getBody()->SetLinearDamping(1.0);
			BehaviourBinding::Bind<PlayerBehaviour>(player);
		}

		GameObject obj2 = scene->CreateEntity("monkey_quads");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/monkey_quads.obj");
			obj2.emplace<RendererComponent>().SetMesh(vao).SetMaterial(stoneMat);
			obj2.get<Transform>().SetLocalPosition(0.0f, 0.0f, 2.0f);
			obj2.get<Transform>().SetLocalRotation(0.0f, 0.0f, -90.0f);
			BehaviourBinding::BindDisabled<SimpleMoveBehaviour>(obj2);
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

		GameObject framebufferObject = scene->CreateEntity("Basic Buffer");
		{
			testBuffer = &framebufferObject.emplace<PostEffect>();
			testBuffer->Init(width, height);
		}

		GameObject colorCorrectionObj = scene->CreateEntity("Color Correct");
		{
			colorCorrectEffect = &colorCorrectionObj.emplace<ColorCorrection>();
			colorCorrectEffect->filename = "cubes/BrightenedCorrection.cube";
			colorCorrectEffect->Init(width, height);
		}

		effects.push_back(colorCorrectEffect);
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
			skyboxMat->RenderLayer = 100;

			MeshBuilder<VertexPosNormTexCol> mesh;
			MeshFactory::AddIcoSphere(mesh, glm::vec3(0.0f), 1.0f);
			MeshFactory::InvertFaces(mesh);
			VertexArrayObject::sptr meshVao = mesh.Bake();
			
			GameObject skyboxObj = scene->CreateEntity("skybox");  
			skyboxObj.get<Transform>().SetLocalPosition(0.0f, 0.0f, 0.0f);
			skyboxObj.get_or_emplace<RendererComponent>().SetMesh(meshVao).SetMaterial(skyboxMat);
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

		///// Game loop /////
		while (!glfwWindowShouldClose(BackendHandler::window)) {
			glfwPollEvents();

			// Update the timing
			time.CurrentFrame = glfwGetTime();
			time.DeltaTime = static_cast<float>(time.CurrentFrame - time.LastFrame);

			time.DeltaTime = time.DeltaTime > 1.0f ? 1.0f : time.DeltaTime;

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

			// Iterate over all the behaviour binding components
			scene->Registry().view<BehaviourBinding>().each([&](entt::entity entity, BehaviourBinding& binding) {
				// Iterate over all the behaviour scripts attached to the entity, and update them in sequence (if enabled)
				for (const auto& behaviour : binding.Behaviours) {
					if (behaviour->Enabled) {
						behaviour->Update(entt::handle(scene->Registry(), entity));
					}
				}
			});
			pworld->Update(time.DeltaTime);
			// Clear the screen
			testBuffer->Clear();
			for (int i = 0; i < effects.size(); i++)
			{
				effects[i]->Clear();
			}

			glClearColor(0.08f, 0.17f, 0.31f, 1.0f);
			glEnable(GL_DEPTH_TEST);
			glClearDepth(1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Update all world matrices for this frame
			scene->Registry().view<Transform>().each([](entt::entity entity, Transform& t) {
				t.UpdateWorldMatrix();
			});
			
			// Grab out camera info from the camera object
			Transform& camTransform = cameraObject.get<Transform>();
			glm::mat4 view = glm::inverse(camTransform.LocalTransform());
			glm::mat4 projection = cameraObject.get<Camera>().GetProjection();
			glm::mat4 viewProjection = projection * view;
						
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
			effects[activeEffect]->ApplyEffect(testBuffer);
			effects[activeEffect]->DrawToScreen();
			// Draw our ImGui content
			BackendHandler::RenderImGui();

			scene->Poll();
			glfwSwapBuffers(BackendHandler::window);
			time.LastFrame = time.CurrentFrame;
		}

		// Nullify scene so that we can release references
		Application::Instance().ActiveScene = nullptr;
		BackendHandler::ShutdownImGui();
	}	

	// Clean up the toolkit logger so we don't leak memory
	Logger::Uninitialize();
	return 0;
}