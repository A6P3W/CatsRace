#define NOMINMAX
#include "Player.h"
#include "InputMapper.h"
#include "InputManager.h"
#include "ResourceManager.h"
#include "SpriteComponent.h"
#include "CameraComponent.h"
#include "SceneManager.h"
#include "Scenes/Game/GameScene01.h"
#include <DxLib.h>
#include "ObjectManager.h"
#include "MovementComponent.h"
#include <algorithm>
#include "CircleCollisionComponent.h"
#include "RectangleCollisionComponent.h"
#include "Log.h"
#include <EnhancedInputComponent.h>
#include <EasyShakeComponent.h>
#include "RenderSystem.h"
#include <random>



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

	SetActorScale(FScale(0.4f));
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
    m_camera->SetFOV(1);
	m_camera->SetParentComponent(m_shake);

	m_camera->AddLocalOffset({ 0.0f, -750.0f });
    // 走行音をループ再生開始・最初は無音
    // carsound.mp3 をプロジェクトの sounds/ フォルダに置いてください

    auto sound = std::make_unique<MSoundComponent>();
    m_sound = sound.get();
    AddComponent(std::move(sound));

}

void APlayer::OnUpdate(float DeltaTime)
{
    const float MaxSpeed = 10.0f;
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
    UpdateDrift(DeltaTime, speed);
    float steerMultiplier = m_isDrifting ? DriftSteerMultiplier : 0.7f;
    float steerAngle = MaxSteer * m_slider * steerAbility * steerMultiplier;
    AddActorRotation(FRotator(steerAngle));
    m_movement->AddVelocityRotation(FRotator(steerAngle));

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
    // ---- ドリフト時のスプライト傾き ----
{
    float targetTilt = 0.0f;
    if (m_isDrifting)
    {
        targetTilt = MaxDriftTiltAngle * m_driftDirection;
    }
    // なめらかに補間
    m_spriteTiltAngle += (targetTilt - m_spriteTiltAngle) * std::clamp(TiltLerpSpeed * DeltaTime, 0.0f, 1.0f);
    if (m_sprite)
    {
        m_sprite->SetWorldRotation(FRotator(GetActorRotation().Rotation + m_spriteTiltAngle));
    }
}
    // ---- FOVエフェクト補間 ----
    if (m_fovEffectTimer > 0.0f) {
        m_fovEffectTimer -= DeltaTime;
        // alphaは0→1（タイマー終了に近づくほど1.0fに戻る）
        float alpha = 1.0f - std::clamp(m_fovEffectTimer / m_fovEffectDuration, 0.0f, 1.0f);
        float currentFOV = m_fovTarget + (m_fovBase - m_fovTarget) * alpha;
        if (m_fovEffectTimer <= 0.0f) {
            currentFOV = 1.0f;
            m_isSpeedUp = false;
        }
        if (m_camera) m_camera->SetFOV(currentFOV);
    }
    else {
        // タイマーが0以下の時は必ずフラグをリセット
        m_isSpeedUp = false;
    }

    // ---- 走行音 ----
    if (m_sound) {
        float t = std::clamp(speed / MaxSpeed, 0.0f, 1.0f);
        m_sound->SetVolume(m_engineIdleHandle, 1.0f - t); // 速いほど小さく
        m_sound->SetVolume(m_engineRunHandle, t);         // 速いほど大きく
    }
    // スピードアップ中のみ加速線を表示
    if (m_isSpeedUp) {
        DrawSpeedLines(speed);
    }
    // ---- ドリフトゲージ表示 ----
    if (m_isDrifting)
    {
        const float GaugeX = 760.0f;
        const float GaugeY = 50.0f;
        const float GaugeWidth = 400.0f;
        const float GaugeHeight = 24.0f;

        float gaugeRatio = std::clamp(m_driftGauge / MaxDriftGauge, 0.0f, 1.0f);

        // 背景（枠）
        RenderSystem::GetInstance().SubmitBox(
            { GaugeX, GaugeY }, { GaugeWidth, GaugeHeight }, FRotator(0.0f),
            0x444444, 1, RenderSpace::Screen, 250, 200
        );

        // ゲージ本体（溜まり具合に応じて色を変える）
        int gaugeColor = (gaugeRatio >= 1.0f) ? 0xFF4444 : 0x44CCFF;
        RenderSystem::GetInstance().SubmitBox(
            { GaugeX, GaugeY }, { GaugeWidth * gaugeRatio, GaugeHeight }, FRotator(0.0f),
            gaugeColor, 1, RenderSpace::Screen, 251, 255
        );

        // 1/5（20%）の位置にしきい値ラインを表示
        float thresholdX = GaugeX + GaugeWidth * 0.2f;
        RenderSystem::GetInstance().SubmitLine(
            { thresholdX, GaugeY }, { thresholdX, GaugeY + GaugeHeight },
            0xFFFF00, RenderSpace::Screen, 253, 255
        );

        // 枠線
        RenderSystem::GetInstance().SubmitBox(
            { GaugeX, GaugeY }, { GaugeWidth, GaugeHeight }, FRotator(0.0f),
            0xFFFFFF, 0, RenderSpace::Screen, 252, 255
        );
    }
   
}

void APlayer::OnPossesed()
{
    m_camera->SetActiveCamera();
    m_camera->SetFOV(1);
}

void APlayer::SetupPlayerInputComponent(MEnhancedInputComponent* PlayerInputComponent)
{
    PlayerInputComponent->BindAction(InputAction::Interact, ETriggerEvent::Started, this, &APlayer::OnRestartPressed);
    PlayerInputComponent->BindAction(InputAction::Move, ETriggerEvent::Triggered, this, &APlayer::OnMove);
    PlayerInputComponent->BindAction("DRIFT", ETriggerEvent::Started, this, &APlayer::OnDriftPressed);
    PlayerInputComponent->BindAction("DRIFT", ETriggerEvent::Completed, this, &APlayer::OnDriftReleased);
}

void APlayer::OnMove(const FInputActionValue& Value)
{
    if (!CanMove) return;
    m_accelInput = std::clamp(Value.Axis2D.Y, -1.0f, 1.0f);
    m_slider = -std::clamp(Value.Axis2D.X, -1.0f, 1.0f);
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
    m_shake->StartShake(m_crashshake, {45,45},2011);
	m_accelInput *= 0.5f;
}

void APlayer::OnDriftPressed()
{
    m_driftKeyPressed = true;
}

void APlayer::OnDriftReleased()
{
    m_driftKeyPressed = false;
}
void APlayer::EndOverlap(AActor * OtherActor)
{
	M_LOG("Player EndOverlap with " + OtherActor->GetActorClassName());
	m_shake->EndShake(m_crashshake, false);
}

void APlayer::BeginPlay()
{
    m_engineIdleHandle = m_sound->PlaySE("images/cat5.mp3", true);
    m_engineRunHandle = m_sound->PlaySE("images/moving-v2.mp3", true);
}
void APlayer::DrawSpeedLines(float speed)
{
    const float MaxSpeed = 30.0f;
    float t = std::clamp(speed / MaxSpeed, 0.0f, 1.0f);
    if (t < 0.1f) return;

    int lineCount = static_cast<int>(t * 60);
    int alpha = static_cast<int>(t * 1800);

    const float CenterX = 960.0f;
    const float CenterY = 540.0f;

    static std::mt19937 rng(12345);
    // 画面端付近にランダムな始点を置くための分布
    std::uniform_real_distribution<float> distX(0.0f, 1920.0f);
    std::uniform_real_distribution<float> distY(0.0f, 1080.0f);
    std::uniform_real_distribution<float> distLen(0.05f, 0.25f); // 中心方向に何割進むか

    rng.seed(static_cast<uint32_t>(GetNowCount()));

    for (int i = 0; i < lineCount; ++i)
    {
        // 始点を画面端付近に配置（端20%の帯の中）
        float sx, sy;
        int edge = i % 4;
        switch (edge)
        {
        case 0: sx = distX(rng) * 0.35f;               sy = distY(rng) * 0.35f;               break; // 左上
        case 1: sx = 1920.0f - distX(rng) * 0.35f;     sy = distY(rng) * 0.35f;               break; // 右上
        case 2: sx = distX(rng) * 0.35f;               sy = 1080.0f - distY(rng) * 0.35f;     break; // 左下
        case 3: sx = 1920.0f - distX(rng) * 0.35f;     sy = 1080.0f - distY(rng) * 0.35f;     break; // 右下
        }

        // 中心方向のベクトルを作り、その途中まで伸ばす
        float dx = CenterX - sx;
        float dy = CenterY - sy;
        float len = distLen(rng) * t;

        float ex = sx + dx * len;
        float ey = sy + dy * len;

        RenderSystem::GetInstance().SubmitLine(
            { sx, sy }, { ex, ey },
            0xFFFFFF, RenderSpace::Screen, 200, alpha
        );
    }
}
void APlayer::UpdateDrift(float DeltaTime, float speed)
{
    bool bWantsDrift = m_driftKeyPressed && (std::abs(m_slider) > 0.3f) && (speed > DriftMinSpeed);

    if (bWantsDrift)
    {
        if (!m_isDrifting)
        {
            // ドリフト開始：最初に入力した方向を固定
            m_isDrifting = true;
            m_driftGauge = 0.0f;
            m_driftDirection = (m_slider > 0.0f) ? 1.0f : -1.0f;
            M_LOG("Drift Start", 0);
        }

        // ドリフト中は固定方向のステア入力のみ受け付ける
        float currentDir = (m_slider > 0.0f) ? 1.0f : -1.0f;
        if (currentDir == m_driftDirection)
        {
            m_driftGauge = std::min(m_driftGauge + DeltaTime * 40.0f, MaxDriftGauge);
        }
        else
        {
            // 逆方向の入力は無視する（ステア自体を固定方向にする）
            m_slider = 0.0f;
        }

        // ドリフト中は少し速度を落とす
        float decay = std::pow(DriftSpeedDecay, DeltaTime * 60.0f);
        m_movement->SetWorldForce(m_movement->GetVelocity() * decay);
    }
    else
    {
        if (m_isDrifting)
        {
            // ドリフト終了 → ブースト
            float boostRatio = m_driftGauge / MaxDriftGauge;
            if (boostRatio > 0.2f)
            {
                float boostForce = DriftBoostForce * boostRatio;
                m_movement->AddLocalForce({ 0.0f, -boostForce });
                M_LOG("Drift Boost! ratio={}", boostRatio);
            }
            m_driftGauge = 0.0f;
        }
        m_isDrifting = false;
    }
}
void APlayer::ApplyFOVEffect(float targetFOV, float duration, bool showSpeedLines) {
    m_fovBase = 1.0f;
    m_fovTarget = targetFOV;
    m_fovEffectTimer = duration;
    m_fovEffectDuration = duration;
    if (m_camera) m_camera->SetFOV(targetFOV);

    m_isSpeedUp = showSpeedLines;
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
