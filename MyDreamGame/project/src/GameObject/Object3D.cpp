#include "Object3D.h"
#include <DirectXMath.h>

void Object3D::Initialize(ID3D12Device *device, Model *model) {
    model_ = model; // 共有されているモデルをセット
    transform_ = {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};

    // マテリアルと座標変換リソースの作成（自分の分だけ）
    transformResource_ = CreateBufferResource(device, (sizeof(TransformMatrix) + 255) & ~255u);
    transformResource_->Map(0, nullptr, reinterpret_cast<void **>(&mappedTransform_));

    materialResource_ = CreateBufferResource(device, (sizeof(Material) + 255) & ~255u);
    materialResource_->Map(0, nullptr, reinterpret_cast<void **>(&mappedMaterial_));
}

void Object3D::Update(const Matrix4x4 &viewMatrix, const Matrix4x4 &projectionMatrix) {
    *mappedMaterial_ = material_;

    // 自身のワールド行列作成
    Matrix4x4 worldMatrix = TransformFunctions::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

    // モデル側のデータを使って最終的な行列を計算
    Matrix4x4 nodeMatrix = model_->GetModelData().rootNode.localMatrix;
    Matrix4x4 finalWorldMatrix = nodeMatrix * worldMatrix;

    mappedTransform_->World = finalWorldMatrix;
    mappedTransform_->WVP = finalWorldMatrix * viewMatrix * projectionMatrix;

    // ★ ここを修正！ 順序は World * View * Projection
    mappedTransform_->WVP = TransformFunctions::Multiply(TransformFunctions::Multiply(finalWorldMatrix, viewMatrix), projectionMatrix);

    // ★ 追加：法線用行列の計算（これがないとライティングが真っ黒になります）
    mappedTransform_->WorldInverseTranspose = TransformFunctions::Transpose(TransformFunctions::Inverse(finalWorldMatrix));
}

void Object3D::Draw(ID3D12GraphicsCommandList *commandList) {
    // 自分のマテリアルと行列をセット
    commandList->SetGraphicsRootConstantBufferView(0, transformResource_->GetGPUVirtualAddress()); // 行列
    commandList->SetGraphicsRootConstantBufferView(1, materialResource_->GetGPUVirtualAddress());  // マテリアル

    // 実際の描画（バッファのセットやDrawコマンド）はModelに任せる！
    model_->Draw();
}