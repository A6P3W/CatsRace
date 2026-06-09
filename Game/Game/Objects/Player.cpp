#define NOMINMAX
#include "Player.h"
#include "InputMapper.h"
#include "InputManager.h"
#include "ResourceManager.h"
#include "SpriteComponent.h"
#include "CameraComponent.h"
#include "SceneManager.h"
#include "Objects/GameScene01.h"
#include <DxLib.h>
#include "ObjectManager.h"
#include "MovementComponent.h"
#include <algorithm>
#include "CircleCollisionComponent.h"
#include "Log.h"
#include <EnhancedInputComponent.h>
#include <EasyShakeComponent.h>

APlayer::APlayer(FVector2D location, FRotator rotation)
{
    SetActorLocation(location);
    SetActorRotation(rotation);

    m_walkAnimHandles[0] = ResourceManager::GetInstance().LoadResourceGraph("images/cat_walk_1.png");
    m_walkAnimHandles[1] = ResourceManager::GetInstance().LoadResourceGraph("images/cat_walk_2.png");
    m_walkAnimHandles[2] = ResourceManager::GetInstance().LoadResourceGraph("images/cat_walk_3.png");
    m_walkAnimHandles[3] = ResourceManager::GetInstance().LoadResourceGraph("images/cat_walk_4.png");
    m_walkAnimHandles[4] = ResourceManager::GetInstance().LoadResourceGraph("images/cat_walk_5.png");

    auto sprite = std::make_unique<MSpriteComponent>(0, RenderSpace::World);
    m_sprite = sprite.get();
    sprite->SubmitGraph( m_walkAnimHandles[0]);
    AddComponent(std::move(sprite));


    auto col = std::make_unique<MCircleCollisionComponent>(32.0f);
    col->SetParentComponent(GetRootComponent());
    col->SetCollisionType(ECollisionType::Block);
    col->SetStatic(false);
    AddComponent(std::move(col));

    auto movement = std::make_unique<MMovementComponent>();
    m_movement = movement.get();
    AddComponent(std::move(movement));

	auto shake = std::make_unique<MEasyShakeComponent>();
	m_shake = shake.get();
	AddComponent(std::move(shake));
    // カメラ
    auto camera = std::make_unique<MCameraComponent>();
    m_camera = camera.get();
    AddComponent(std::move(camera));
    m_camera->SetActiveCamera();
    m_camera->SetFOV(0.2f);
	m_camera->SetParentComponent(m_shake);

	m_camera->AddLocalOffset({ 0.0f, -600.0f });
    // 走行音をループ再生開始・最初は無音
    // carsound.mp3 をプロジェクトの sounds/ フォルダに置いてください

}

void APlayer::OnUpdate(float DeltaTime)
{
    const float MaxSpeed = 70.0f;
    const float AccelForce = 3.5f;
    const float BrakeForce = 6.0f;
    const float MaxSteer = 2.5f;

    FVector2D v = m_movement->GetVelocity();
    float speed = std::sqrt(v.SizeSquared());

    if (m_accelInput > 0.0f) {
        float speedRatio = std::clamp(speed / MaxSpeed, 0.0f, 1.0f);
        float force = AccelForce * m_accelInput * (1.0f - speedRatio * 0.8f);
        m_movement->AddLocalForce({ 0.0f, -force });
    }
    else if (m_accelInput < 0.0f) {
        float speedRatio = std::clamp(speed / MaxReverseSpeed, 0.0f, 1.0f);
        float force = ReverseForce * (-m_accelInput) * (1.0f - speedRatio * 0.8f);
        m_movement->AddLocalForce({ 0.0f, force });
    }
    // ---- ステアリング ----
    float steerAbility = std::clamp(speed / 3.0f, 0.0f, 1.0f);
    float sliderDir = (m_accelInput < 0.0f) ? -m_slider : m_slider;
    float steerAngle = MaxSteer * m_slider * steerAbility;
    AddActorRotation(steerAngle);
    m_movement->AddVelocityRotation(steerAngle);

    m_accelInput = 0.0f;
    m_slider = 0.0f;

    // ---- アニメーション ----
    if (m_sprite) {
        const float MoveAnimSpeedMin = 0.1f;
        if (speed > MoveAnimSpeedMin) {
            float speedRate = std::clamp(speed / 5.0f, 0.0f, 1.0f);
            float frameTime = 0.16f - speedRate * 0.07f;
            m_moveAnimTime += DeltaTime;
            while (m_moveAnimTime >= frameTime) {
                m_moveAnimTime -= frameTime;
                m_walkAnimFrame = (m_walkAnimFrame + 1) % static_cast<int>(m_walkAnimHandles.size());
            }
            m_sprite->SubmitGraph( m_walkAnimHandles[m_walkAnimFrame]);
        }
        else {
            m_moveAnimTime = 0.0f;
            m_walkAnimFrame = 0;
            m_sprite->SubmitGraph( m_walkAnimHandles[m_walkAnimFrame]);
        }
    }
}

void APlayer::OnPossesed()
{
    m_camera->SetActiveCamera();
    m_camera->SetFOV(0.2);
}

void APlayer::SetupPlayerInputComponent(MEnhancedInputComponent* PlayerInputComponent)
{
    PlayerInputComponent->BindAction(InputAction::Interact, ETriggerEvent::Started, this, &APlayer::OnRestartPressed);
    PlayerInputComponent->BindAction(InputAction::Move, ETriggerEvent::Triggered, this, &APlayer::OnMove);
    //PlayerInputComponent->BindAction(InputActionMouse::Wheel, ETriggerEvent::Triggered, this, &APlayer::OnWheel);
}

void APlayer::OnMove(const FInputActionValue& Value)
{
    if (!CanMove) return;

    m_accelInput = Value.Axis2D.Y;

    m_slider = -Value.Axis2D.X;
}

void APlayer::OnRestartPressed()
{
    SceneManager::GetInstance().OpenScene<AGameScene01>();
}

void APlayer::OnWheel(const FInputActionValue& Value)
{
	float fov = m_camera->GetFOV();
	m_camera->SetFOV(fov*=1+Value.Axis1D*0.1);
}

void APlayer::BeginOverlap(AActor* OtherActor)
{
	M_LOG("Player BeginOverlap with " + OtherActor->GetActorClassName());
    m_shake->StartShake(m_crashshake, {15,15},2011);
}


void APlayer::EndOverlap(AActor * OtherActor)
{
	M_LOG("Player EndOverlap with " + OtherActor->GetActorClassName());
	m_shake->EndShake(m_crashshake, false);
}




//{
//	if (Scale > 0) {
//		m_movement->AddLocalForce({ 0.0f, -2.0f });
//	}
//	else {
//		m_movement->AddLocalForce({ 0.0f, 2.0f });
//	}
//}
//
//
//{
//	if (Scale > 0) {
//		m_slider -= 0.1;
//	}
//	else {
//		m_slider += 0.1;
//	}
//}
