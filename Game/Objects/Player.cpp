#define NOMINMAX
#include "Player.h"
#include "InputMapper.h"
#include "InputManager.h"
#include "ResourceManager.h"
#include "SpriteComponent.h"
#include "CameraComponent.h"
#include "SceneManager.h"
#include "Objects/DefaultScene.h"
#include <DxLib.h>
#include "ObjectManager.h"
#include "MovementComponent.h"
#include <algorithm>
#include "CircleCollisionComponent.h"
#include "Utils/Log.h"
#include <EnhancedInputComponent.h>

APlayer::APlayer(FVector2D location, FRotator rotation)
{
    SetActorLocation(location);
    SetActorRotation(rotation);

    // グラフィック
    m_walkAnimHandles[0] = ResourceManager::GetInstance().LoadResourceGraph("images/cat_walk_1.png");
    m_walkAnimHandles[1] = ResourceManager::GetInstance().LoadResourceGraph("images/cat_walk_2.png");
    m_walkAnimHandles[2] = ResourceManager::GetInstance().LoadResourceGraph("images/cat_walk_3.png");
    m_walkAnimHandles[3] = ResourceManager::GetInstance().LoadResourceGraph("images/cat_walk_4.png");
    m_walkAnimHandles[4] = ResourceManager::GetInstance().LoadResourceGraph("images/cat_walk_5.png");
    auto sprite = std::make_unique<MSpriteComponent>(0, RenderSpace::World);
    m_sprite = sprite.get();
    sprite->SubmitGraph(1.0, m_walkAnimHandles[0]);
    AddComponent(std::move(sprite));
	auto col = std::make_unique<MCircleCollisionComponent>(32.0f);
	col->SetParentComponent(GetRootComponent());
	col->SetCollisionType(ECollisionType::Block);
	col->SetStatic(false);
	AddComponent(std::move(col));


    // 移動
    auto movement = std::make_unique<MMovementComponent>();
    m_movement = movement.get();
    AddComponent(std::move(movement));

    // カメラ
    auto camera = std::make_unique<MCameraComponent>();
    m_camera = camera.get();
    AddComponent(std::move(camera));
    m_camera->SetActiveCamera();
    m_camera->SetFOV(0.2f);

    // 走行音をループ再生開始・最初は無音
    // carsound.mp3 をプロジェクトの sounds/ フォルダに置いてください

}


void APlayer::OnUpdate(float DeltaTime)
{

    float moveSpeed = 1000.0f;
    float rotationSpeed = 180.0f;

    m_slider = std::clamp(m_slider, -1.0f, 1.0f);
    float velocity = std::clamp(m_movement->GetVelocitySizeSquared() - 0.9f, 0.0f, 1.0f);
    AddActorRotation(rotationSpeed * m_slider * velocity * DeltaTime);
    m_movement->AddVelocityRotation(rotationSpeed * m_slider * velocity * DeltaTime);


    // ---------- パラメータ ----------
    const float AccelPower = 5.0f;
    const float HandleSpeed = 12.0f;
    const float HandleReturn = 10.0f;
    const float MaxRot = 150.0f;


    // ---------- ステアリング ----------
    bool bIn = false;


    // ---------- 旋回計算 ----------
    FVector2D v = m_movement->GetVelocity();
    float s = std::sqrt(v.SizeSquared());
    float ability = std::clamp(s / 1.5f, 0.0f, 1.0f);

    // ---------- 移動アニメーション ----------
    if (m_sprite) {
        const float MoveAnimSpeedMin = 0.1f;
        if (s > MoveAnimSpeedMin) {
            float speedRate = std::clamp(s / 5.0f, 0.0f, 1.0f);
            float frameTime = 0.16f - speedRate * 0.07f;
            m_moveAnimTime += DeltaTime;
            while (m_moveAnimTime >= frameTime) {
                m_moveAnimTime -= frameTime;
                m_walkAnimFrame = (m_walkAnimFrame + 1) % static_cast<int>(m_walkAnimHandles.size());
            }
            m_sprite->SubmitGraph(1.0, m_walkAnimHandles[m_walkAnimFrame]);
        }
        else {
            m_moveAnimTime = 0.0f;
            m_walkAnimFrame = 0;
            m_sprite->SubmitGraph(1.0, m_walkAnimHandles[m_walkAnimFrame]);
        }
    }

    // ---------- 走行音：速度 → 音量 ----------
    {
        float currentSpeed = std::sqrt(m_movement->GetVelocity().SizeSquared());

        // 速度を 0〜255 の音量にマッピング
        // SpeedMax のとき最大音量になる（値はお好みで調整してください）
        const float SpeedMin = 0.5f;    // この速度以下は無音（停車中のノイズ防止）
        const float SpeedMax = 10.0f;   // この速度で最大音量

        float t = std::clamp((currentSpeed - SpeedMin) / (SpeedMax - SpeedMin), 0.0f, 1.0f);
        int volume = static_cast<int>(t * 255.0f);

    }

	//if (InputManager::GetInstance().GetKeyPressStart(KEY_INPUT_SPACE)) {
	//	int handle = ResourceManager::GetInstance().LoadResourceGraph("BaseFile/texture_Checker_64px.png");
	//	auto sprite = std::make_unique<MSpriteComponent>(0, RenderSpace::World);
	//	sprite->SubmitGraph(1.0, handle);
	//	sprite->SetParentComponent(this->GetRootComponent());
	//	sprite->AddWorldOffset({ 220.0f, -100.0f });
	//	sprite->SetScale(0.5f);
	//	AddComponent(std::move(sprite));
	//	ObjectManager::GetInstance().SpawnObject<ACSVMap>(GetActorLocation(), FRotator{ 0 });
	//}
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
	PlayerInputComponent->BindAction(InputActionMouse::Wheel, ETriggerEvent::Triggered, this, &APlayer::OnWheel);

}

void APlayer::OnMove(const FInputActionValue& Value)
{
	m_movement->AddLocalForce({ 0.0f, Value.Axis2D.Y*-2});
	m_slider -= Value.Axis2D.X*0.1;
    
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
