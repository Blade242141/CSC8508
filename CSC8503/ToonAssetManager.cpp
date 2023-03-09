#include "ToonAssetManager.h"
#include <stb/stb_image.h>
#include "ToonDebugManager.h"

using namespace NCL;

ToonAssetManager* ToonAssetManager::instance = NULL;

ToonAssetManager::ToonAssetManager(void) {
	file = std::ifstream(Assets::DATADIR + "ItemsToLoad.csv");
	if (!file.is_open()) {
		std::cerr << "Failed to open the file\n";
	}
	string line;
	while (getline(file, line)) {
		loadingData.assetCountTotal++;
	}
	file.clear();
	file.seekg(0, std::ios::beg);
}

ToonAssetManager::~ToonAssetManager(void) {
	for (auto& [name, texture] : textures)
		delete texture;
	for (auto& [name, mesh] : meshes)
		delete mesh;
	for (auto& [name, shader] : shaders)
		delete shader;
	for (auto& [name, animation] : animations)
		delete animation;
	for (auto& [name, mat] : materials)
		delete mat;
	for (auto& [name, sound] : sounds)
		Audio::RemoveSound(sound);
}

void ToonAssetManager::LoadAssets(void) {
	ToonDebugManager::Instance().StartLoad();
	for (auto& [name, texture] : textures)
		delete texture;
	for (auto& [name, mesh] : meshes)
		delete mesh;
	for (auto& [name, shader] : shaders)
		delete shader;
	for (auto& [name, animation] : animations)
		delete animation;
	for (auto& [name, mat] : materials)
		delete mat;
	for (auto& [name, sound] : sounds)
		Audio::RemoveSound(sound);
	textures.clear();
	meshes.clear();
	shaders.clear();
	animations.clear();
	materials.clear();
	sounds.clear();
	//-----------------------------------------------------------
	//		Textures
	AddTexture("mesh", "checkerboard.png");
	AddTexture("basic", "Prefab_Grey50.png", true);
	AddTexture("basicPurple", "Prefab_Purple.png", true);
	AddTexture("crosshair", "crosshair2.png",true);
	AddTexture("screenTex", "RB_Animated_Screen.png", true);
	/*AddTexture("player", "Boss_diffuse.png", true);
	AddTexture("tex_arena_wall", "RB_Level_Arena_Wall.png", true);
	AddTexture("tex_arena_wall2", "RB_Level_Arena_Wall2.png", true);
	AddTexture("tex_arena_lights", "RB_Level_Arena_Lights.png", true);*/

	//-----------------------------------------------------------
	//		Meshes
	AddMesh("cube", "cube.msh");
	AddMesh("arrow", "Minimap_Arrow.msh");
	AddMesh("player", "Character_Boss.msh");
	AddMesh("sphere", "sphere.msh");

	//AddMesh("arena_main", "Level_Arena.msh");
	AddMesh("arena_floor_part_a", "Arena_Part_A_Floor.msh");
	AddMesh("arena_floor_part_b", "Arena_Part_B_Floor.msh");
	AddMesh("arena_floor_part_c", "Arena_Part_C_Floor.msh");
	AddMesh("arena_floor_part_d", "Arena_Part_D_Floor.msh");

	AddMesh("arena_wall_part_a", "Arena_Part_A_Wall.msh");
	AddMesh("arena_wall_part_b", "Arena_Part_B_Wall.msh");
	AddMesh("arena_wall_part_c", "Arena_Part_C_Wall.msh");
	AddMesh("arena_wall_part_d", "Arena_Part_D_Wall.msh");

	AddMesh("arena_platform_floor_high_l", "Arena_Platform_High_L_Floor.msh");
	AddMesh("arena_platform_floor_high_r", "Arena_Platform_High_R_Floor.msh");
	AddMesh("arena_platform_wall_high_l", "Arena_Platform_High_L_Wall.msh");
	AddMesh("arena_platform_wall_high_r", "Arena_Platform_High_R_Wall.msh");

	AddMesh("arena_platform_floor_low_top", "Arena_Platform_Low_Top_Floor.msh");
	AddMesh("arena_platform_floor_low_bottom", "Arena_Platform_Low_Bottom_Floor.msh");
	AddMesh("arena_platform_wall_low_top", "Arena_Platform_Low_Top_Wall.msh");
	AddMesh("arena_platform_wall_low_bottom", "Arena_Platform_Low_Bottom_Wall.msh");

	AddMesh("arena_lights", "Level_Arena_Lights.msh");

	AddMesh("arena_deco_cats", "Level_Arena_Cats.msh");
	AddMesh("arena_deco_cats_hull", "Level_Arena_Cats_Hull.msh");

	AddMesh("arena_deco_chair", "Level_Deco_Chair.msh");	
	AddMesh("arena_deco_chair_throne", "Level_Deco_Throne_Chair.msh");
	AddMesh("arena_deco_table", "Level_Deco_Table.msh");
	AddMesh("arena_deco_screen", "Level_Deco_Screen.msh");

	AddMesh("arena_obstacles", "Level_Arena_Obstables.msh");
	AddMesh("arena_obstacles_hulls", "Level_Arena_Obstacles_Hulls.msh");
	AddMesh("arena_ramps", "Level_Arena_Ramps.msh");
	AddMesh("arena_decos", "Level_Arena_Decos.msh");
	AddMesh("arena_decos_hulls", "Level_Arena_Decos_Hulls.msh");
	AddMesh("arena_border_wall", "Level_Arena_Border.msh");
	AddMesh("arena_cameras", "Level_Arena_Cameras.msh");
	AddMesh("arena_cameras_hulls", "Level_Arena_Cameras_Convex_Hulls.msh");
	AddMesh("arena_toonsplat_Box", "Level_ToonSplat_Box.msh");

	AddMesh("player_mesh_1", CreateCharacterTeamMesh("Character_Boss.msh", Vector4(Team::T_GREEN_GOBLINS, 1.0f)));
	AddMesh("player_mesh_2", CreateCharacterTeamMesh("Character_Boss.msh", Vector4(Team::T_PURPLE_PRAWNS, 1.0f)));
	AddMesh("player_mesh_3", CreateCharacterTeamMesh("Character_Boss.msh", Vector4(Team::T_BLUE_BULLDOGS, 1.0f)));
	AddMesh("player_mesh_4", CreateCharacterTeamMesh("Character_Boss.msh", Vector4(Team::T_ORANGE_OTTERS, 1.0f)));
	//AddMesh("floorMain", "FloorsMain.msh");
	//AddMesh("platformMain", "Level_Platform.msh");
	
	//-----------------------------------------------------------
	//		Shaders
	AddShader("debug", "debug.vert", "debug.frag");
	AddShader("shadow", "shadowSkin.vert", "shadow.frag");
	AddShader("minimap", "minimap.vert", "minimap.frag");
	AddShader("texture", "Texture.vert", "Texture.frag");
	AddShader("scene", "scene.vert", "scene.frag");
	AddShader("sceneScreen", "sceneScreen.vert", "scene.frag");
	AddShader("scoreBar", "ScoreBar.vert", "ScoreBar.frag");
	AddShader("fullMap", "map.vert", "map.frag");
void ToonAssetManager::LoadLoadingScreenAssets(void) {
	AddShader("debug", "debug.vert", "debug.frag");
	AddShader("skybox", "skybox.vert", "skybox.frag");
}

	//-----------------------------------------------------------
	//		Animations
	AddAnimation("Player_Idle", "Boss_Gun_Idle2.anm");
	AddAnimation("Player_Idle_Aim", "Boss_Gun_Idle_Aim.anm");
	AddAnimation("Player_Run", "Boss_Gun_Run.anm");
	AddAnimation("Player_Run_Aim_F", "Boss_Gun_Run_Aim_F.anm");
	AddAnimation("Player_Run_Aim_FL", "Boss_Gun_Run_Aim_FL.anm");
	AddAnimation("Player_Run_Aim_FR", "Boss_Gun_Run_Aim_FR.anm");
	AddAnimation("Player_Run_Aim_L", "Boss_Gun_Run_Aim_L.anm");
	AddAnimation("Player_Run_Aim_R", "Boss_Gun_Run_Aim_R.anm");
	AddAnimation("Player_Run_Aim_B", "Boss_Gun_Run_Aim_B.anm");
	AddAnimation("Player_Run_Aim_BL", "Boss_Gun_Run_Aim_BL.anm");
	AddAnimation("Player_Run_Aim_BR", "Boss_Gun_Run_Aim_BR.anm");
	AddAnimation("Player_Jump", "Boss_Gun_Jump.anm");
	AddAnimation("Player_Defeat", "Boss_Gun_Defeated.anm");
	AddAnimation("Player_Dance", "Boss_Gun_Dance.anm");

	//-----------------------------------------------------------
	//		Materials
	AddMaterial("mat_player", "Character_Boss.mat", GetMesh("player")->GetSubMeshCount());

	//AddMaterial("mat_arena", "Level_Arena.mat", GetMesh("arena_main")->GetSubMeshCount());
	AddMaterial("mat_arena_floor_part_a", "Arena_Part_A_Floor.mat", GetMesh("arena_floor_part_a")->GetSubMeshCount());
	AddMaterial("mat_arena_floor_part_b", "Arena_Part_B_Floor.mat", GetMesh("arena_floor_part_b")->GetSubMeshCount());
	AddMaterial("mat_arena_floor_part_c", "Arena_Part_C_Floor.mat", GetMesh("arena_floor_part_c")->GetSubMeshCount());
	AddMaterial("mat_arena_floor_part_d", "Arena_Part_D_Floor.mat", GetMesh("arena_floor_part_d")->GetSubMeshCount());
	
	AddMaterial("mat_arena_wall_part_a", "Arena_Part_A_Wall.mat", GetMesh("arena_wall_part_a")->GetSubMeshCount());
	AddMaterial("mat_arena_wall_part_b", "Arena_Part_B_Wall.mat", GetMesh("arena_wall_part_b")->GetSubMeshCount());
	AddMaterial("mat_arena_wall_part_c", "Arena_Part_C_Wall.mat", GetMesh("arena_wall_part_c")->GetSubMeshCount());
	AddMaterial("mat_arena_wall_part_d", "Arena_Part_D_Wall.mat", GetMesh("arena_wall_part_d")->GetSubMeshCount());
	
	AddMaterial("mat_arena_platform_floor_high_l", "Arena_Platform_High_L_Floor.mat", GetMesh("arena_platform_floor_high_l")->GetSubMeshCount());
	AddMaterial("mat_arena_platform_floor_high_r", "Arena_Platform_High_R_Floor.mat", GetMesh("arena_platform_floor_high_r")->GetSubMeshCount());
	AddMaterial("mat_arena_platform_wall_high_l", "Arena_Platform_High_L_Wall.mat", GetMesh("arena_platform_wall_high_l")->GetSubMeshCount());
	AddMaterial("mat_arena_platform_wall_high_r", "Arena_Platform_High_R_Wall.mat", GetMesh("arena_platform_wall_high_r")->GetSubMeshCount());
	
	AddMaterial("mat_arena_platform_floor_low_top", "Arena_Platform_Low_Top_Floor.mat", GetMesh("arena_platform_floor_low_top")->GetSubMeshCount());
	AddMaterial("mat_arena_platform_floor_low_bottom", "Arena_Platform_Low_Bottom_Floor.mat", GetMesh("arena_platform_floor_low_bottom")->GetSubMeshCount());
	AddMaterial("mat_arena_platform_wall_low_top", "Arena_Platform_Low_Top_Wall.mat", GetMesh("arena_platform_wall_low_top")->GetSubMeshCount());
	AddMaterial("mat_arena_platform_wall_low_bottom", "Arena_Platform_Low_Bottom_Wall.mat", GetMesh("arena_platform_wall_low_bottom")->GetSubMeshCount());

	AddMaterial("mat_arena_obstacles", "Level_Arena_Obstables.mat", GetMesh("arena_obstacles")->GetSubMeshCount());
	AddMaterial("mat_arena_obstacles_hulls", "Level_Arena_Obstacles_Hulls.mat", GetMesh("arena_obstacles_hulls")->GetSubMeshCount());
	AddMaterial("mat_arena_ramps", "Level_Arena_Ramps.mat", GetMesh("arena_ramps")->GetSubMeshCount());
	AddMaterial("mat_arena_lights", "Level_Arena_Lights.mat", GetMesh("arena_lights")->GetSubMeshCount());
	
	AddMaterial("mat_arena_deco_cats", "Level_Arena_Cats.mat", GetMesh("arena_deco_cats")->GetSubMeshCount());
	AddMaterial("mat_arena_deco_cats_hull", "Level_Arena_Cats_Hull.mat", GetMesh("arena_deco_cats_hull")->GetSubMeshCount());
	
	AddMaterial("mat_arena_deco_chair", "Level_Deco_Chair.mat", GetMesh("arena_deco_chair")->GetSubMeshCount());
	AddMaterial("mat_arena_deco_chair_throne", "Level_Deco_Throne_Chair.mat", GetMesh("arena_deco_chair_throne")->GetSubMeshCount());
	AddMaterial("mat_arena_deco_table", "Level_Deco_Table.mat", GetMesh("arena_deco_table")->GetSubMeshCount());
	AddMaterial("mat_arena_deco_screen", "Level_Deco_Screen.mat", GetMesh("arena_deco_screen")->GetSubMeshCount());
	
	AddMaterial("mat_arena_decos", "Level_Arena_Decos.mat", GetMesh("arena_decos")->GetSubMeshCount());
	AddMaterial("mat_arena_decos_hulls", "Level_Arena_Decos_Hulls.mat", GetMesh("arena_decos_hulls")->GetSubMeshCount());

	AddMaterial("mat_arena_border_wall", "Level_Arena_Border.mat", GetMesh("arena_border_wall")->GetSubMeshCount());

	AddMaterial("mat_arena_cameras", "Level_Arena_Cameras.mat", GetMesh("arena_cameras")->GetSubMeshCount());
	AddMaterial("mat_arena_cameras_hulls", "Level_Arena_Cameras_Convex_Hulls.mat", GetMesh("arena_cameras_hulls")->GetSubMeshCount());
	
	AddMaterial("mat_arena_toonsplat_Box", "Level_ToonSplat_Box.mat", GetMesh("arena_toonsplat_Box")->GetSubMeshCount());


	//-----------------------------------------------------------
	//		Sounds
	AddSound("splatter", "splatter.wav");
	AddSound("gameMusic", "gameTune.wav");
	AddSound("menuMusic", "menuTune.wav");
	AddSound("splash", "splash.wav");
	AddSound("click", "click.wav");

	ToonDebugManager::Instance().EndLoad();
void ToonAssetManager::LoadNextAsset(void) {
	vector<string> tokens = SplitLine();
	
	if (tokens[0] == "Texture")
		AddTexture(tokens);
	else if (tokens[0] == "Mesh")
		AddMesh(tokens);
	else if (tokens[0] == "Shader")
		AddShader(tokens);
	else if (tokens[0] == "Animation")
		AddAnimation(tokens);
	else if (tokens[0] == "Material")
		AddMaterial(tokens);
	else
		std::cout << "Error: Unknown asset type\n";
}

vector<string> ToonAssetManager::SplitLine() {
	vector<string> result;
	size_t pos = 0;
	while (pos != string::npos) {
		size_t next_pos = currentLine.find(',', pos);
		if (next_pos == string::npos) {
			result.push_back(currentLine.substr(pos));
			break;
		}
		result.push_back(currentLine.substr(pos, next_pos - pos));
		pos = next_pos + 1;
	}
	return result;
}

Rendering::TextureBase* ToonAssetManager::GetTexture(const string& name) {

	map<string, Rendering::TextureBase*>::iterator i = textures.find(name);

	if (i != textures.end())
		return i->second;
	return nullptr;
}

Rendering::TextureBase* ToonAssetManager::AddTexture(vector<string> tokens) {
	string name, fileName;
	bool invert;

	name = tokens[1];
	fileName = tokens[2];
	if (tokens.size() >= 4)
		invert = (tokens[3] == "TRUE");
	else invert = false;

	return AddTexture(name, fileName, invert);
}

Rendering::TextureBase* ToonAssetManager::AddTexture(const string& name, const string& fileName, const bool& invert) {

	Rendering::TextureBase* texture = GetTexture(name);

	if (texture != nullptr) return texture;

	if (invert) {
		stbi_set_flip_vertically_on_load(true);
		texture = TextureLoader::LoadAPITexture(fileName);
		stbi_set_flip_vertically_on_load(false);
	}
	else
		texture = TextureLoader::LoadAPITexture(name);

	textures.emplace(name, texture);

	return texture;
}

MeshGeometry* ToonAssetManager::GetMesh(const string& name) {

	map<string, MeshGeometry*>::iterator i = meshes.find(name);

	if (i != meshes.end())
		return i->second;
	return nullptr;
}

MeshGeometry* ToonAssetManager::AddMesh(vector<string> tokens) {
	string name, fileName;
	if (tokens[1] != "Character") {

		name = tokens[1];
		fileName = tokens[2];
		// For now not doing type... but it can be done

		return AddMesh(name, fileName);
	}
	else {
		float r, g, b;
		name = tokens[2];
		fileName = tokens[3];
		r = stof(tokens[4]);
		g = stof(tokens[5]);
		b = stof(tokens[6]);
		return AddMesh(name, CreateCharacterTeamMesh(fileName, Vector4(r, g, b, 1.0f)));
	}
}


MeshGeometry* ToonAssetManager::AddMesh(const string& name, const string& fileName, const GeometryPrimitive& type) {

	MeshGeometry* mesh = GetMesh(name);

	if (mesh != nullptr) return mesh;

	mesh = new Rendering::OGLMesh(fileName);
	mesh->SetPrimitiveType(type);
	mesh->UploadToGPU();

	meshes.emplace(name, mesh);

	return mesh;
}

MeshGeometry* NCL::ToonAssetManager::AddMesh(const string& name, MeshGeometry* newMesh)
{
	if (newMesh == nullptr)
		return nullptr;

	meshes.emplace(name, newMesh);

	return newMesh;
}

Rendering::OGLShader* ToonAssetManager::GetShader(const string& name) {

	map<string, Rendering::OGLShader*>::iterator i = shaders.find(name);

	if (i != shaders.end())
		return i->second;
	return nullptr;
}

Rendering::OGLShader* ToonAssetManager::AddShader(vector<string> tokens) {
	string name, vertex, fragment, geometry, domain, hull;
	name = tokens[1];
	vertex = tokens[2];
	fragment = tokens[3];
	if (tokens.size() >= 5)
		geometry = tokens[4];
	if (tokens.size() >= 6)
		domain = tokens[5];
	if (tokens.size() >= 7)
		hull = tokens[6];
	// For now not doing type... but it can be done

	return AddShader(name, vertex, fragment, geometry, domain, hull);
}

Rendering::OGLShader* ToonAssetManager::AddShader(const string& name, const string& vertexShader, const string& fragmentShader,
	const string& geometryShader, const string& domainShader, const string& hullShader) {

	Rendering::OGLShader* shader = GetShader(name);

	if (shader != nullptr) return shader;

	shader = new Rendering::OGLShader(vertexShader, fragmentShader, geometryShader, domainShader, hullShader);

	shaders.emplace(name, shader);

	return shader;
}

MeshAnimation* ToonAssetManager::GetAnimation(const string& name) {

	map<string, MeshAnimation*>::iterator i = animations.find(name);

	if (i != animations.end())
		return i->second;
	return nullptr;
}

MeshAnimation* ToonAssetManager::AddAnimation(vector<string> tokens) {
	string name, fileName;

	name = tokens[1];
	fileName = tokens[2];

	return AddAnimation(name, fileName);
}


MeshAnimation* ToonAssetManager::AddAnimation(const string& name, const string& fileName) {
	
	MeshAnimation* animation = GetAnimation(name);
	if (animation != nullptr) return animation;

	animation = new MeshAnimation(fileName);

	animations.emplace(name, animation);

	return animation;
}

ToonMeshMaterial* ToonAssetManager::GetMaterial(const string& name)
{
	map<string, ToonMeshMaterial*>::iterator i = materials.find(name);

	if (i != materials.end())
		return i->second;
	return nullptr;
}

ToonMeshMaterial* NCL::ToonAssetManager::AddMaterial(vector<string> tokens) {
	string name, fileName, mesh;

	name = tokens[1];
	fileName = tokens[2];
	mesh = tokens[3];

	return AddMaterial(name, fileName, GetMesh(mesh)->GetSubMeshCount());
}

ToonMeshMaterial* NCL::ToonAssetManager::AddMaterial(const string& name, const string& fileName, const unsigned int& subMeshCount)
{
	ToonMeshMaterial* mat = GetMaterial(name);
	if (mat != nullptr) return mat;

	mat = new ToonMeshMaterial(fileName, subMeshCount);
	materials.emplace(name, mat);

	return mat;
}

CSC8503::Sound* ToonAssetManager::GetSound(const string& name)
{
	map<string, Sound*>::iterator i = sounds.find(name);

	if (i != sounds.end())
		return i->second;
	return nullptr;
}

CSC8503::Sound* ToonAssetManager::AddSound(const string& name, const string& fileName)
{
	CSC8503::Sound* sound = GetSound(name);
	if (sound != nullptr) return sound;

	sound = Audio::AddSound(fileName.c_str());
	sounds.emplace(fileName, sound); // Filename is placeholder

	return sound;
}

MeshGeometry* ToonAssetManager::CreateCharacterTeamMesh(const std::string& fileName, const Vector4& teamColor)
{
	MeshGeometry* copyPlayerMesh = new OGLMesh(fileName);
	if (copyPlayerMesh == nullptr) return nullptr;

	std::vector<Vector4> vertexColours;
	const std::vector<unsigned int> indices = copyPlayerMesh->GetIndexData();
	for (size_t i = 0; i < copyPlayerMesh->GetVertexCount(); i++)
		vertexColours.emplace_back(Debug::WHITE);

	const SubMesh* clothesSubMesh = copyPlayerMesh->GetSubMesh(4);		//4 = Clothes!
	if (clothesSubMesh == nullptr) return nullptr;

	int start = clothesSubMesh->start;
	int end = start + clothesSubMesh->count;

	for (int i = start; i < end; i += 3)
	{
		int A = indices[i + 0];
		int B = indices[i + 1];
		int C = indices[i + 2];

		vertexColours[A] = teamColor;
		vertexColours[B] = teamColor;
		vertexColours[C] = teamColor;
	}

	copyPlayerMesh->SetVertexColours(vertexColours);
	copyPlayerMesh->UploadToGPU();

	return copyPlayerMesh;
}
