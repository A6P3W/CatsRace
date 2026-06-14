#include "Goal.h"
#include "RectAngleCollisionComponent.h"
#include "SceneManager.h"
#include "Objects/Player.h" // �v���C���[����p
#include "Log.h"
#include "ClearScene.h"
// #include "ClearScene.h"  // �N���A��ʂ�GameMode���ł�����C���N���[�h

AGoalActor::AGoalActor(FVector2D location, FRotator rotation)
{
    SetActorLocation(location);

    // ���a50�̉~�`�R���W������ǉ�
    auto collision = std::make_unique<MRectangleCollisionComponent>(20000.0f);
    collision->SetParentComponent(GetRootComponent());
    AddComponent(std::move(collision));
}

void AGoalActor::BeginOverlap(AActor* OtherActor)
{
    // �����������肪�v���C���[�iAPlayer�j���ǂ�����`�F�b�N
    if (dynamic_cast<APlayer*>(OtherActor)) {
        // �V�[���J�ځi��F�N���A�V�[���ցj
        SceneManager::GetInstance().OpenScene<AClearScene>();

        // �e�X�g�p�Ƀ��O�o��
        M_LOG("Goal Reached! Transitioning to Clear Scene...");
    }
}