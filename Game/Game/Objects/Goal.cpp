#include "Goal.h"
#include "RectAngleCollisionComponent.h"
#include "SceneManager.h"
#include "Objects/Player.h" // プレイヤー判定用
#include "Log.h"
#include "ClearScene.h"
// #include "ClearScene.h"  // クリア画面のGameModeができたらインクルード

AGoalActor::AGoalActor(FVector2D location, FRotator rotation)
{
    SetActorLocation(location);

    // 半径50の円形コリジョンを追加
    auto collision = std::make_unique<MRectangleCollisionComponent>(20000.0f);
    collision->SetParentComponent(GetRootComponent());
    AddComponent(std::move(collision));
}

void AGoalActor::BeginOverlap(AActor* OtherActor)
{
    // 当たった相手がプレイヤー（APlayer）かどうかをチェック
    if (dynamic_cast<APlayer*>(OtherActor)) {
        // シーン遷移（例：クリアシーンへ）
        SceneManager::GetInstance().OpenScene<AClearScene>();

        // テスト用にログ出力
        M_LOG("Goal Reached! Transitioning to Clear Scene...");
    }
}