#pragma once
#include "GameTechRenderer.h"
#include "PaintableZone.h"
#include "PaintableObject.h"

#include "Player.h"
#include "PaintBallProjectile.h"
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

		class ToonLevelManager
		{
		public:
			Player* AddPlayerToWorld(const Vector3& position);
			PaintBallProjectile* MakeBullet(const Vector3& position);
			ShaderBase* GetBasicShader()  { return basicShader; }
			MeshGeometry* GetSphereMesh() { return sphereMesh; }
			ToonLevelManager(GameTechRenderer& renderer);
			~ToonLevelManager();

			static ToonLevelManager* Get() { return instance; }

			void Update(float dt);

		protected:
			bool LoadAssets();
			bool LoadModel(MeshGeometry** mesh, const std::string& meshFileName);
			bool LoadTexture(TextureBase** tex, const std::string& textureFileName, const bool& invert = false);
			bool LoadShader(ShaderBase** shader, const std::string& shaderVertexShader, const std::string& shaderFragmentShader);
			bool LoadLevel();

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

			ToonGameObject* AddCubeToWorld(const Vector3& position, const Vector3& rotationEuler, const Vector3& scale, TextureBase* cubeTex, float mass = 1.0f);
			ToonGameObject* AddSphereToWorld(const Vector3& position, const Vector3& rotationEuler, const float& radius, TextureBase* sphereTex, float mass = 1.0f);
			void AddGridWorld(Axes axes, const Vector3& gridSize, const float& gridSpacing, const Vector3& gridPosition, const Vector3& cubeScale, const float& cubeMass, TextureBase* cubeTex);

			MeshGeometry* GetSphereMesh() const { return sphereMesh; }
			ShaderBase* GetBasicShader() const { return basicShader; }


		private:
			MeshGeometry* charMesh = nullptr;
			MeshGeometry* cubeMesh;
			MeshGeometry* sphereMesh;
			TextureBase* checkTex;
			TextureBase* basicTex;
			TextureBase* basicTexPurple;
			ShaderBase* basicShader;

			GameTechRenderer& gameRenderer;			

			ToonGameObject* axisObject;

			static ToonLevelManager* instance;
		};
	}
}
