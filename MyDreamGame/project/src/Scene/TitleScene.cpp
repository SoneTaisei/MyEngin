#include "TitleScene.h"
#include "SceneManager.h"
#include "Input/KeyboardInput.h"
#include "../externals/imgui/imgui.h"
#include "Sprite/SpriteCommon.h"
#include "Model/ModelCommon.h"
#include "Graphics/TextureManager.h"
#include "Core/TimeManager.h"
#include "StageSelectScene.h"
#include <wrl.h>

TitleScene::~TitleScene() {
}

void TitleScene::Initialize(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList) {
	commandList_ = commandList;

	// 1. 画像をロードしてハンドルを取得 (TextureManagerに任せる)
	uint32_t uvCheckerIndex = TextureManager::GetInstance()->Load("resources/uvChecker.png", commandList_.Get());
	D3D12_GPU_DESCRIPTOR_HANDLE textureHandle = TextureManager::GetInstance()->GetGpuHandle(uvCheckerIndex);

	std::unique_ptr<Model> playerModel = std::make_unique<Model>();
	playerModel->Initialize(modelCommon_, "resources/plane", "plane.obj");
	playerModel->SetTextureHandle(textureHandle);
	playerModel->SetRotation({ 0.0f,0.0f,0.0f });
	playerModel_ = playerModel.get();
	models_.push_back(std::move(playerModel));
}

void TitleScene::Update(SceneManager *sceneManager) {
	// スペースキーが押されたらステージセレクトシーンへ
	if(KeyboardInput::GetInstance()->IsKeyPressed(DIK_SPACE)) {
		sceneManager->ChangeScene(new StageSelectScene());
	}
}

void TitleScene::Draw(const Matrix4x4 &viewProjectionMatrix) {
	textureHandle_ = TextureManager::GetInstance()->Load("resources/uvChecker.png",commandList_);
	D3D12_GPU_DESCRIPTOR_HANDLE planeGpuHandle = TextureManager::GetInstance()->GetGpuHandle(textureHandle_);

	// そのまま呼べる
	modelCommon_->DrawAll(viewProjectionMatrix);

	if(spriteCommon_) {
		spriteCommon_->PreDraw(commandList_.Get());
		spriteCommon_->DrawAll();
	}
}