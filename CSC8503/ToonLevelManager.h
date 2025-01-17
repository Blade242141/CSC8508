#pragma once
#include "ToonAssetManager.h"
#include "PaintableObject.h"
#include "ToonGameWorld.h"
#include "ToonScreen.h"
#include "Player.h"
#include "PlayerNPC.h"
#include "PaintBallProjectile.h"
#include "HitSphere.h"
#include <reactphysics3d/reactphysics3d.h>

namespace NCL
{
	namespace CSC8503
	{
		enum Axes
		{
			None = 0,
			X = 1,
			Y = 2,
			Z = 4
		};

		enum BasicCollisionShapeType
		{
			Shape_Box,
			Shape_Sphere,
			Shape_Capsule
		};

		struct ObjectCollisionShapeData
		{
			BasicCollisionShapeType shapeType;
			reactphysics3d::Transform collisionTransform;
		};

		struct BoxCollisionShapeData : ObjectCollisionShapeData
		{
			Vector3 boxHalfSize;			
			BoxCollisionShapeData(const Vector3& bHSize, reactphysics3d::Transform colTransform = reactphysics3d::Transform::identity()) : boxHalfSize(bHSize)
			{ 
				shapeType = BasicCollisionShapeType::Shape_Box; 
				collisionTransform = colTransform;
			}
		};

		struct SphereCollisionShapeData : ObjectCollisionShapeData
		{
			float radius;
			SphereCollisionShapeData(const float& r, reactphysics3d::Transform colTransform = reactphysics3d::Transform::identity()) : radius(r)
			{ 
				shapeType = BasicCollisionShapeType::Shape_Sphere; 
				collisionTransform = colTransform;
			}
		};

		struct CapsuleCollisionShapeData : ObjectCollisionShapeData
		{
			float radius, height;
			CapsuleCollisionShapeData(const float& r, const float& h, reactphysics3d::Transform colTransform = reactphysics3d::Transform::identity()) : radius(r), height(h)
			{ 
				shapeType = BasicCollisionShapeType::Shape_Capsule; 
				collisionTransform = colTransform;
			}
		};

		class ToonLevelManager
		{
		public:
			ToonLevelManager(ToonGameWorld* gameWorld);
			~ToonLevelManager();

			Player* AddPlayerToWorld(Team* team);
			PlayerNPC* AddPlayerNPCToWorld(Team* team);
			PaintBallProjectile* AddPaintBallProjectileToWorld(const reactphysics3d::Vector3& position,
				const reactphysics3d::Vector3& rotationEuler, const float& radius, const float& _impactSize, Team* team, std::string objectName = "");
			HitSphere* AddHitSphereToWorld(const reactphysics3d::Vector3& position, const float radius, Team* team);

			Player* GetPlayer() { return player; }

			void ResetLevel(std::vector<ToonNetworkObject*>* networkObjectList = nullptr);

			MeshGeometry* GetMesh(std::string meshName) const { 
				if (meshMap.count(meshName) == 0) {
					std::cout << "ERROR: Attempting to get Mesh that isn't loaded\n";
					std::cout << "Mesh Name: " << meshName << std::endl;
					return nullptr;
				}
				else return meshMap.at(meshName); 
			}
			TextureBase* GetTexture(std::string textureName) const {
				if (textureMap.count(textureName) == 0) {
					std::cout << "ERROR: Attempting to get Texture that isn't loaded\n";
					std::cout << "Texture Name: " << textureName << std::endl;
					return nullptr;
				}
				else return textureMap.at(textureName);
			}
			ShaderBase* GetShader(std::string shaderName) const {
				if (shaderMap.count(shaderName) == 0) {
					std::cout << "ERROR: Attempting to get Shader that isn't loaded\n";
					std::cout << "Shader Name: " << shaderName << std::endl;
					return nullptr;
				}
				else return shaderMap.at(shaderName);
			}
			ToonMeshMaterial* GetMaterial(std::string materialName) const {
				if (materialMap.count(materialName) == 0) {
					std::cout << "ERROR: Attempting to get Material that isn't loaded\n";
					std::cout << "Material Name: " << materialName << std::endl;
					return nullptr;
				}
				else return materialMap.at(materialName);
			}

			void Update(float dt);

		protected:
			bool LoadAssets();
			bool LoadModel(const std::string meshName);
			bool LoadTexture(const std::string textureName);
			bool LoadShader(const std::string shaderName);
			bool LoadMaterial(const std::string materialName);
			bool LoadPrototypeLevel(std::vector<ToonNetworkObject*>* networkObjectList = nullptr);
			bool LoadArenaLevel(std::vector<ToonNetworkObject*>* networkObjectList = nullptr);

			reactphysics3d::ConcaveMeshShape* CreateConcaveMeshShape(MeshGeometry* mesh, const Vector3& scaling);
			reactphysics3d::ConvexMeshShape* CreateConvexMeshShape(MeshGeometry* mesh, const Vector3& scaling);

			Axes selectedAxes = Axes::None;

			inline void SetSelectedAxes(Axes axes) {
				selectedAxes = axes;
			}

			inline bool IsXSelected() {
				return (selectedAxes & Axes::X) == Axes::X;
			}

			inline bool IsYSelected() {
				return (selectedAxes & Axes::Y) == Axes::Y;
			}

			inline bool IsZSelected() {
				return (selectedAxes & Axes::Z) == Axes::Z;
			}

			PaintableObject* AddCubeToWorld(const Vector3& position, const Vector3& rotationEuler, const Vector3& scale, TextureBase* cubeTex, Vector4 minimapColour, float mass = 1.0f, bool isFloor = false);
			PaintableObject* AddTSCubeToWorld(const Vector3& position, const Vector3& rotationEuler, const Vector3& scale, TextureBase* cubeTex, Vector4 minimapColour, float mass = 1.0f, bool isFloor = false);
			PaintableObject* AddSphereToWorld(const Vector3& position, const Vector3& rotationEuler, const float& radius, TextureBase* sphereTex, Vector4 minimapColour, float mass = 1.0f);
			void AddGridWorld(Axes axes, const Vector3& gridSize, const float& gridSpacing, const Vector3& gridPosition,
							const Vector3& cubeScale, const float& cubeMass, TextureBase* cubeTex, 
							Vector4 minimapColour = Vector4(0,0,0,1), bool isFloor = false);
			
			PaintableObject* AddConcaveObjectToWorld(MeshGeometry* mesh, const Vector3& position, const Vector3& rotationEuler, const Vector3& scale, TextureBase* cubeTex, Vector4 minimapColour, float mass = 1.0f, float addAsPaintable = true, float addAsFloor = true);
			PaintableObject* AddConcaveObjectToWorld(MeshGeometry* mesh, const Vector3& position, const Vector3& rotationEuler, const Vector3& scale, ToonMeshMaterial* mat, Vector4 minimapColour, float mass = 1.0f, float addAsPaintable = true, float addAsFloor = true);
			PaintableObject* AddConcaveObjectToWorld(MeshGeometry* mesh, MeshGeometry* collisionHullMesh, const Vector3& position, const Vector3& rotationEuler, const Vector3& scale, ToonMeshMaterial* mat, Vector4 minimapColour, float mass = 1.0f, float addAsPaintable = true, float addAsFloor = true);
			
			PaintableObject* AddConvexObjectToWorld(MeshGeometry* mesh, const Vector3& position, const Vector3& rotationEuler, const Vector3& scale, ToonMeshMaterial* mat, Vector4 minimapColour, float mass = 1.0f, float addAsPaintable = true, float addAsFloor = true);

			PaintableObject* AddPropObject(MeshGeometry* mesh, ObjectCollisionShapeData* collisionData, reactphysics3d::BodyType rigidbodyType, const Vector3& position, const Vector3& rotationEuler, const Vector3& scale, ToonMeshMaterial* mat, Vector4 minimapColour, float mass = 1.0f, float addAsPaintable = true, float addAsFloor = true, std::vector<ToonNetworkObject*>* networkObjectList = nullptr);

			ToonScreen* AddPropScreen(MeshGeometry* mesh, ObjectCollisionShapeData* collisionData, reactphysics3d::BodyType rigidbodyType, const Vector3& position, const Vector3& rotationEuler, const Vector3& scale, ToonMeshMaterial* mat, Vector4 minimapColour, float mass = 1.0f, float addAsPaintable = true, float addAsFloor = true);

		private:
			std::map<std::string, MeshGeometry*> meshMap;
			std::map<std::string, TextureBase*> textureMap;
			std::map<std::string, ShaderBase*> shaderMap;
			std::map<std::string, ToonMeshMaterial*> materialMap;

			GameTechRenderer* gameRenderer;		
			ToonGameWorld* gameWorld;

			PaintableObject* axisObject;
			Player* player = nullptr;

			static ToonLevelManager* instance;
		};
	}
}
