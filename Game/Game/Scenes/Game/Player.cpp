#define NOMINMAX
#include "Player.h"

#include <DxLib.h>
#include <EasyShakeComponent.h>
#include <EnhancedInputComponent.h>
#include <PlayerController.h>

#include <algorithm>
#include <random>

#include "CameraComponent.h"
#include "CircleCollisionComponent.h"
#include "Core/GI_main.h"
#include "InputManager.h"
#include "InputMapper.h"
#include "Log.h"
#include "NetMovementComponent.h"
#include "ActorManager.h"
#include "RectangleCollisionComponent.h"
#include "RenderSystem.h"
#include "ResourceManager.h"
#include "SceneManager.h"
#include "Scenes/Game/GameSceneBase.h"
#include "SpriteComponent.h"
#include "Objects/Items/SpeedDownstage.h"
#include "Objects/Items/HeldSpeedItem.h"
#include "LapCheckpoint.h"
namespace {
enum : FNetworkRPCId { RPC_ServerSetDrift = 1, RPC_ServerNotifyGoal = 2, RPC_ServerUseHeldItem = 3, RPC_ServerSyncDriftState = 4, RPC_MulticastUpdateLap = 5
};
}

REGISTER_ACTOR(APlayer)

APlayer::APlayer(FVector2D location, FRotator rotation) {
  bReplicates = true;
  RegisterReplicatedProperty(&m_isDrifting);
  RegisterReplicatedProperty(&m_driftDirection);
  RegisterReplicatedProperty(&CanMove);
  RegisterReplicatedProperty(&m_hasHeldItem);
  RegisterRPC(RPC_ServerSetDrift, ENetRPCType::Server, this, &APlayer::Server_SetDrift);
  RegisterRPC(RPC_ServerNotifyGoal, ENetRPCType::Server, this, &APlayer::Server_NotifyGoal);
  RegisterRPC(RPC_ServerUseHeldItem, ENetRPCType::Server, this, &APlayer::Server_UseHeldItem);
  RegisterRPC(RPC_ServerSyncDriftState, ENetRPCType::Server, this, &APlayer::Server_SyncDriftState);
  RegisterRPC(RPC_MulticastUpdateLap, ENetRPCType::Multicast, this, &APlayer::Multicast_UpdateLap);

  SetActorLocation(location);
  SetActorRotation(rotation);

  m_walkAnimHandles[0] =
      ResourceManager::GetInstance().LoadResourceGraph("Resources/images/cat_walk_1.png");
  m_walkAnimHandles[1] =
      ResourceManager::GetInstance().LoadResourceGraph("Resources/images/cat_walk_2.png");
  m_walkAnimHandles[2] =
      ResourceManager::GetInstance().LoadResourceGraph("Resources/images/cat_walk_3.png");
  m_walkAnimHandles[3] =
      ResourceManager::GetInstance().LoadResourceGraph("Resources/images/cat_walk_4.png");
  m_walkAnimHandles[4] =
      ResourceManager::GetInstance().LoadResourceGraph("Resources/images/cat_walk_5.png");

  auto sprite = std::make_unique<MSpriteComponent>(0, RenderSpace::World);
  m_sprite = sprite.get();
  sprite->SubmitGraph(m_walkAnimHandles[0]);
  AddComponent(std::move(sprite));

  SetActorScale(FScale(0.4f));
  auto col = std::make_unique<MCircleCollisionComponent>(32.0f);
  col->SetParentComponent(GetRootComponent());
  col->SetCollisionType(ECollisionType::Block);
  col->SetStatic(false);
  AddComponent(std::move(col));

  auto movement = std::make_unique<MNetMovementComponent>();
  Movement = movement.get();
  AddComponent(std::move(movement));

  auto shake = std::make_unique<MEasyShakeComponent>();
  m_shake = shake.get();
  AddComponent(std::move(shake));
  // カメラ
  auto camera = std::make_unique<MCameraComponent>();
  m_camera = camera.get();
  AddComponent(std::move(camera));
  m_camera->SetFOV(1);
  m_camera->SetParentComponent(m_shake);

  m_camera->AddLocalOffset({0.0f, -750.0f});
  // 走行音をループ再生開始・最初は無音
  // carsound.mp3 をプロジェクトの sounds/ フォルダに置いてください

  auto sound = std::make_unique<MSoundComponent>();
  m_sound = sound.get();
  AddComponent(std::move(sound));
}

APlayer::~APlayer() {
  if (m_sound) {
    m_sound->StopAll();
  }
}
void APlayer::OnUpdate(float DeltaTime) {
  const float MaxSpeed = 10.0f;
  const float AccelForce = 3.5f;
  const float BrakeForce = 6.0f;
  const float MaxSteer = 2.5f;

  FVector2D v = Movement->GetVelocity();
  float speed = std::sqrt(v.SizeSquared());

  if (bIsLocallyControlled) {
    if (!m_slowSources.empty()) {
      float strongest = 1.0f;
      for (auto& [src, strength] : m_slowSources) {
        if (strength < strongest) {  // 値が小さいほど強い減速
          strongest = strength;
        }
      }
      float decayPerFrame = std::pow(strongest, DeltaTime * 60.0f);
      Movement->SetWorldForce(Movement->GetVelocity() * decayPerFrame);
    }
    if (m_accelInput > 0.0f) {
      float speedRatio = std::clamp(speed / MaxSpeed, 0.0f, 1.0f);
      float force = AccelForce * m_accelInput * (1.0f - speedRatio * 0.8f);
      Movement->AddLocalForce({0.0f, -force});
    } else if (m_accelInput < 0.0f) {
      float speedRatio = std::clamp(speed / MaxReverseSpeed, 0.0f, 1.0f);
      float force = ReverseForce * (-m_accelInput) * (1.0f - speedRatio * 0.8f);
      Movement->AddLocalForce({0.0f, force});
    }
    // ---- ステアリング ----
    float steerAbility = std::clamp(speed / 3.0f, 0.0f, 1.0f);
    float sliderDir = (m_accelInput < 0.0f) ? -m_slider : m_slider;
    UpdateDrift(DeltaTime, speed);
    if (!bHasAuthority) {
      InvokeRPC(
          RPC_ServerSyncDriftState,
          ENetRPCType::Server,
          ENetPacketReliability::Unreliable,
          m_isDrifting,
          m_driftDirection
      );
    }
    UpdateDriftEffect(DeltaTime);
    DrawDriftEffect();
    float steerMultiplier = m_isDrifting ? DriftSteerMultiplier : 0.7f;
    float steerAngle = MaxSteer * m_slider * steerAbility * steerMultiplier;
    AddActorRotation(FRotator(steerAngle));
    Movement->AddVelocityRotation(FRotator(steerAngle));

  } else if (bIsLocallyControlled) {
    UpdateLocalDriftVisual(DeltaTime, speed);
  }

  UpdateDriftEffect(DeltaTime);
  DrawDriftEffect();
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
      m_sprite->SubmitGraph(m_walkAnimHandles[m_walkAnimFrame]);
    } else {
      m_moveAnimTime = 0.0f;
      m_walkAnimFrame = 0;
      m_sprite->SubmitGraph(m_walkAnimHandles[m_walkAnimFrame]);
    }
  }
  // ---- ドリフト時のスプライト傾き ----
  {
    float targetTilt = 0.0f;
    if (m_isDrifting) {
      targetTilt = MaxDriftTiltAngle * m_driftDirection;
    }
    // なめらかに補間
    m_spriteTiltAngle +=
        (targetTilt - m_spriteTiltAngle) * std::clamp(TiltLerpSpeed * DeltaTime, 0.0f, 1.0f);
    if (m_sprite) {
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
  } else {
    // タイマーが0以下の時は必ずフラグをリセット
    m_isSpeedUp = false;
  }

  // ---- 走行音 ----
  if (m_sound) {
    float t = std::clamp(speed / MaxSpeed, 0.0f, 1.0f);
    m_sound->SetVolume(m_engineIdleHandle, 1.0f - t);  // 速いほど小さく
    m_sound->SetVolume(m_engineRunHandle, t);          // 速いほど大きく
  }
  // スピードアップ中のみ加速線を表示
  if (m_isSpeedUp) {
    DrawSpeedLines(speed);
  }
  // ---- ドリフトゲージ表示 ----
  if (m_isDrifting && (bIsLocallyControlled || (bHasAuthority && OwnerConnectionId == 0))) {
    const float GaugeX = 760.0f;
    const float GaugeY = 50.0f;
    const float GaugeWidth = 400.0f;
    const float GaugeHeight = 24.0f;

    float gaugeRatio = std::clamp(m_driftGauge / MaxDriftGauge, 0.0f, 1.0f);

    // 背景（枠）
    RenderSystem::GetInstance().SubmitBox(
        {GaugeX, GaugeY},
        {GaugeWidth, GaugeHeight},
        FRotator(0.0f),
        0x444444,
        1,
        RenderSpace::Screen,
        250,
        200
    );

    // ゲージ本体（溜まり具合に応じて色を変える）
    int gaugeColor = (gaugeRatio >= 1.0f) ? 0xFF4444 : 0x44CCFF;
    RenderSystem::GetInstance().SubmitBox(
        {GaugeX, GaugeY},
        {GaugeWidth * gaugeRatio, GaugeHeight},
        FRotator(0.0f),
        gaugeColor,
        1,
        RenderSpace::Screen,
        251,
        255
    );

    // 1/5（20%）の位置にしきい値ラインを表示
    float thresholdX = GaugeX + GaugeWidth * 0.2f;
    RenderSystem::GetInstance().SubmitLine(
        {thresholdX, GaugeY},
        {thresholdX, GaugeY + GaugeHeight},
        0xFFFF00,
        RenderSpace::Screen,
        253,
        255
    );

    // 枠線
    RenderSystem::GetInstance().SubmitBox(
        {GaugeX, GaugeY},
        {GaugeWidth, GaugeHeight},
        FRotator(0.0f),
        0xFFFFFF,
        0,
        RenderSpace::Screen,
        252,
        255
    );
  }
}

void APlayer::OnPossessedBy(APlayerController* NewController) {
  APawn::OnPossessedBy(NewController);
  if (bIsLocallyControlled || (bHasAuthority && OwnerConnectionId == 0)) {
    m_camera->SetActiveCamera();
  }
  m_camera->SetFOV(1);
}

void APlayer::SetupPlayerInputComponent(MEnhancedInputComponent* PlayerInputComponent) {
  PlayerInputComponent->BindAction(
      InputAction::Interact, ETriggerEvent::Started, this, &APlayer::OnRestartPressed
  );
  PlayerInputComponent->BindAction(
      InputAction::Move, ETriggerEvent::Triggered, this, &APlayer::OnMove
  );
  PlayerInputComponent->BindAction(
      InputAction::Move, ETriggerEvent::Completed, this, &APlayer::OnMove
  );
  PlayerInputComponent->BindAction("USE_ITEM", ETriggerEvent::Started, this, &APlayer::UseHeldItem);
  PlayerInputComponent->BindAction("DRIFT", ETriggerEvent::Started, this, &APlayer::OnDriftPressed);
  PlayerInputComponent->BindAction(
      "DRIFT", ETriggerEvent::Completed, this, &APlayer::OnDriftReleased
  );
  PlayerInputComponent->BindAction("USE_ITEM", ETriggerEvent::Started, this, &APlayer::UseHeldItem);
}

bool APlayer::IsDriftInputPressed() {
  bool bPressed = m_driftKeyPressed;
  if (!GetWorld() || !GetWorld()->GetObjectManager()) {
    return bPressed;
  }

  for (const auto& actorPtr : GetWorld()->GetObjectManager()->GetAllActors()) {
    auto* controller = dynamic_cast<APlayerController*>(actorPtr.get());
    if (!controller || controller->GetPawn() != this || !controller->GetInputMapper()) {
      continue;
    }
    bPressed = bPressed || controller->GetInputMapper()->GetPressing("DRIFT");
  }
  return bPressed;
}
void APlayer::OnMove(const FInputActionValue& Value) {
  if (!CanMove || !bIsLocallyControlled) return;
  const FVector2D clampedInput{
      std::clamp(Value.Axis2D.X, -1.0f, 1.0f), std::clamp(Value.Axis2D.Y, -1.0f, 1.0f)
  };
  // ★重要: クライアント側でのドリフト判定やアニメーション用にローカル変数に代入
  m_accelInput = clampedInput.Y;
  m_slider = -clampedInput.X;
}

void APlayer::Server_SetDrift(bool bDriftHeld) { m_driftKeyPressed = bDriftHeld; }

void APlayer::Server_SyncDriftState(bool bDrifting, float driftDirection) {
  m_isDrifting = bDrifting;
  m_driftDirection = driftDirection;
}

void APlayer::Server_NotifyGoal() { NotifyGoalReached(); }

void APlayer::NotifyGoalReached() {
  if (bHasAuthority) {
    if (auto* gameScene = dynamic_cast<AGameSceneBase*>(GetWorld()->GetGameMode())) {
      gameScene->NotifyPlayerFinished(this);
    }
    return;
  }

  if (bIsLocallyControlled) {
    if (auto* gameScene = dynamic_cast<AGameSceneBase*>(GetWorld()->GetGameMode())) {
      if (auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
        gi->ClearTime = gameScene->GetRaceTime();
        gi->map_id = gameScene->GetMapId();
      }
    }
    InvokeRPC(RPC_ServerNotifyGoal, ENetRPCType::Server, ENetPacketReliability::Reliable);
  }
}
void APlayer::OnRestartPressed() {
  if (!bIsLocallyControlled) return;
  if (auto* gameScene = dynamic_cast<AGameSceneBase*>(GetWorld()->GetGameMode())) {
    gameScene->RestartGame();
  }
}

void APlayer::OnWheel(const FInputActionValue& Value) {
  float fov = m_camera->GetFOV();
  m_camera->SetFOV(fov *= 1 + Value.Axis1D * 0.1);
}

void APlayer::BeginOverlap(AActor* OtherActor) {
  // 【超重要】クライアント側での衝突によるバグ・クラッシュを完全に防ぐため、
  // サーバーではない（クライアントである）場合は、Overlap処理を一切行わずに即終了させます。
  if (!GetWorld() || !GetWorld()->IsServer()) {
    return;
  }

  if (!OtherActor || OtherActor->IsPendingDestroy()) {
    return;
  }

  M_LOG("Player BeginOverlap with " + OtherActor->GetActorClassName());
  if (dynamic_cast<ASlowFloor2*>(OtherActor)) {
    return;
  }

  if (dynamic_cast<ALapCheckpoint*>(OtherActor)) {
    return;
  }
  if (dynamic_cast<AHeldSpeedItem*>(OtherActor)) {
    return;  // アイテム自体の処理はアイテム側の BeginOverlap で行うため、ここでは何もしない
  }

  // 壁や障害物に当たった時の処理（サーバーのみ実行されるので安全）
  if (m_shake) {
    m_shake->StartShake(m_crashshake, {45, 45}, 2011);
  }
  m_accelInput *= 0.5f;
}

void APlayer::OnDriftPressed() {
  if (!bIsLocallyControlled) return;

  m_driftKeyPressed = true;

  if (bHasAuthority) {
    Server_SetDrift(true);
  } else {
    InvokeRPC(RPC_ServerSetDrift, ENetRPCType::Server, ENetPacketReliability::Reliable, true);
  }
}

void APlayer::OnDriftReleased() {
  if (!bIsLocallyControlled) return;

  m_driftKeyPressed = false;

  if (bHasAuthority) {
    Server_SetDrift(false);
  } else {
    InvokeRPC(RPC_ServerSetDrift, ENetRPCType::Server, ENetPacketReliability::Reliable, false);
  }
}
void APlayer::EndOverlap(AActor* OtherActor) {
  // 【超重要】EndOverlap もクライアント側は完全に無視させます。
  // これにより、アイテムが Destroy された瞬間に発生する不正な EndOverlap で落ちるのを防ぎます。
  if (!GetWorld() || !GetWorld()->IsServer()) {
    return;
  }

  if (!OtherActor || OtherActor->IsPendingDestroy()) {
    return;
  }
  M_LOG("Player EndOverlap with " + OtherActor->GetActorClassName());

  if (dynamic_cast<ASlowFloor2*>(OtherActor)) {
    return;
  }
  if (dynamic_cast<ALapCheckpoint*>(OtherActor)) {
    return;
  }
  if (dynamic_cast<AHeldSpeedItem*>(OtherActor)) {
    return;
  }

  if (m_shake) {
    m_shake->EndShake(m_crashshake, false);
  }
}

void APlayer::BeginPlay() {
  if (!dynamic_cast<AGameSceneBase*>(GetWorld()->GetGameMode())) {
    return;
  }
  m_engineIdleHandle = m_sound->PlaySE("Resources/images/cat5.mp3", true);
  m_engineRunHandle = m_sound->PlaySE("Resources/images/moving-v2.mp3", true);
}

void APlayer::DrawSpeedLines(float speed) {
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
  std::uniform_real_distribution<float> distLen(0.05f, 0.25f);  // 中心方向に何割進むか

  rng.seed(static_cast<uint32_t>(GetNowCount()));

  for (int i = 0; i < lineCount; ++i) {
    // 始点を画面端付近に配置（端20%の帯の中）
    float sx, sy;
    int edge = i % 4;
    switch (edge) {
      case 0:
        sx = distX(rng) * 0.35f;
        sy = distY(rng) * 0.35f;
        break;  // 左上
      case 1:
        sx = 1920.0f - distX(rng) * 0.35f;
        sy = distY(rng) * 0.35f;
        break;  // 右上
      case 2:
        sx = distX(rng) * 0.35f;
        sy = 1080.0f - distY(rng) * 0.35f;
        break;  // 左下
      case 3:
        sx = 1920.0f - distX(rng) * 0.35f;
        sy = 1080.0f - distY(rng) * 0.35f;
        break;  // 右下
    }

    // 中心方向のベクトルを作り、その途中まで伸ばす
    float dx = CenterX - sx;
    float dy = CenterY - sy;
    float len = distLen(rng) * t;

    float ex = sx + dx * len;
    float ey = sy + dy * len;

    RenderSystem::GetInstance().SubmitLine(
        {sx, sy}, {ex, ey}, 0xFFFFFF, RenderSpace::Screen, 200, alpha
    );
  }
}
void APlayer::UpdateLocalDriftVisual(float DeltaTime, float speed) {
  // ドリフト開始条件と継続条件を分離
  bool bCanStartDrift = m_driftKeyPressed && (std::abs(m_slider) > 0.3f) &&
                        (speed > DriftMinSpeed || std::abs(m_accelInput) > 0.1f);

  bool bWantsDrift = bCanStartDrift;
  if (m_isDrifting) {
    float currentDir = (m_slider > 0.0f) ? 1.0f : -1.0f;
    // ドリフトボタン押下状態、ステアリング入力あり、かつ方向が同じ場合のみ継続
    bWantsDrift =
        m_driftKeyPressed && (std::abs(m_slider) > 0.1f) && (currentDir == m_driftDirection);
  }

  if (bWantsDrift) {
    if (m_driftGauge <= 0.0f) {
      m_driftGauge = 0.001f;  // 0より少し大きくして開始の目印にする
      m_driftDirection = (m_slider > 0.0f) ? 1.0f : -1.0f;
    }
    m_isDrifting = true;

    if (std::abs(m_slider) > 0.1f) {
      const float currentDir = (m_slider > 0.0f) ? 1.0f : -1.0f;
      if (currentDir == m_driftDirection) {
        m_driftGauge = std::min(m_driftGauge + DeltaTime * 40.0f, MaxDriftGauge);
      }
    }
  } else {
    m_isDrifting = false;
    m_driftGauge = 0.0f;
  }
}

void APlayer::UpdateDrift(float DeltaTime, float speed) {
  // ドリフト開始条件と継続条件を分離
  bool bCanStartDrift = m_driftKeyPressed && (std::abs(m_slider) > 0.3f) &&
                        (speed > DriftMinSpeed || std::abs(m_accelInput) > 0.1f);

  bool bWantsDrift = bCanStartDrift;
  if (m_isDrifting) {
    float currentDir = (m_slider > 0.0f) ? 1.0f : -1.0f;
    // ドリフトボタン押下状態、ステアリング入力あり、かつ方向が同じ場合のみ継続
    bWantsDrift =
        m_driftKeyPressed && (std::abs(m_slider) > 0.1f) && (currentDir == m_driftDirection);
  }

  if (bWantsDrift) {
    if (!m_isDrifting) {
      // ドリフト開始：最初に入力した方向を固定
      m_isDrifting = true;
      m_driftGauge = 0.0f;
      m_driftDirection = (m_slider > 0.0f) ? 1.0f : -1.0f;
      M_LOG("Drift Start", 0);
    }

    // ドリフト中は固定方向のステア入力のみ受け付ける
    if (std::abs(m_slider) > 0.1f) {
      float currentDir = (m_slider > 0.0f) ? 1.0f : -1.0f;
      if (currentDir == m_driftDirection) {
        m_driftGauge = std::min(m_driftGauge + DeltaTime * 40.0f, MaxDriftGauge);
      } else {
        m_slider = 0.0f;
      }
    }

    // ドリフト中は少し速度を落とす
    float decay = std::pow(DriftSpeedDecay, DeltaTime * 60.0f);
    Movement->SetWorldForce(Movement->GetVelocity() * decay);
  } else {
    if (m_isDrifting) {
      // ドリフト終了 → ブースト
      float boostRatio = m_driftGauge / MaxDriftGauge;
      if (boostRatio > 0.2f) {
        float boostForce = DriftBoostForce * boostRatio;
        Movement->AddLocalForce({0.0f, -boostForce});
        M_LOG("Drift Boost! ratio={}", boostRatio);
      }
      m_driftGauge = 0.0f;
    }

    m_isDrifting = false;
  }
}
void APlayer::ApplyFOVEffect(float targetFOV, float duration, bool showSpeedLines) {
  if (!bIsLocallyControlled && !(bHasAuthority && OwnerConnectionId == 0)) return;
  m_fovBase = 1.0f;
  m_fovTarget = targetFOV;
  m_fovEffectTimer = duration;
  m_fovEffectDuration = duration;
  if (m_camera) m_camera->SetFOV(targetFOV);

  m_isSpeedUp = showSpeedLines;
}
void APlayer::UpdateDriftEffect(float DeltaTime) {
  // ----- タイヤ痕の生成 -----
  if (m_isDrifting) {
    m_skidTimer += DeltaTime;
    if (m_skidTimer >= SkidInterval) {
      m_skidTimer = 0.0f;
      SpawnSkidMark();
    }
  } else {
    m_skidTimer = 0.0f;
  }

  // タイヤ痕フェードアウト＆削除
  for (auto& mark : m_skidMarks) {
    mark.Alpha -= SkidFadeSpeed * DeltaTime;
  }
  m_skidMarks.erase(
      std::remove_if(
          m_skidMarks.begin(), m_skidMarks.end(), [](const FSkidMark& m) { return m.Alpha <= 0.0f; }
      ),
      m_skidMarks.end()
  );

  // ----- パーティクルの生成 -----
  if (m_isDrifting) {
    m_particleTimer += DeltaTime;
    if (m_particleTimer >= ParticleInterval) {
      m_particleTimer = 0.0f;
      SpawnDriftParticles();
    }
  } else {
    m_particleTimer = 0.0f;
  }

  // パーティクルの移動・寿命・サイズ更新
  for (auto& p : m_driftParticles) {
    p.Location = p.Location + p.Velocity * DeltaTime;
    p.Life -= DeltaTime;
    if (!p.IsSpark) {
      p.Radius += 25.0f * DeltaTime;  // 煙は膨らむ
    }
  }
  m_driftParticles.erase(
      std::remove_if(
          m_driftParticles.begin(),
          m_driftParticles.end(),
          [](const FDriftParticle& p) { return p.Life <= 0.0f; }
      ),
      m_driftParticles.end()
  );
}

void APlayer::SpawnSkidMark() {
  if (m_skidMarks.size() >= 300) {
    m_skidMarks.erase(m_skidMarks.begin());
  }
  // 左右タイヤそれぞれ1つずつ生成
  const float TireOffset = 18.0f;
  for (int side : {-1, 1}) {
    FVector2D offset = FVector2D(TireOffset * side, 10.0f).RotateVector(GetActorRotation());
    FSkidMark mark;
    mark.Location = GetActorLocation() + offset;
    mark.Rotation = GetActorRotation();
    mark.Alpha = 1.0f;
    m_skidMarks.push_back(mark);
  }
}

void APlayer::SpawnDriftParticles() {
  static std::mt19937 rng{std::random_device{}()};
  std::uniform_real_distribution<float> distAngle(0.0f, 360.0f);
  std::uniform_real_distribution<float> distSmoke(20.0f, 80.0f);
  std::uniform_real_distribution<float> distSpark(100.0f, 280.0f);
  std::uniform_real_distribution<float> distLife(0.25f, 0.55f);
  std::uniform_real_distribution<float> distSparkLife(0.05f, 0.12f);
  std::uniform_real_distribution<float> distOfs(-15.0f, 15.0f);

  FVector2D base = GetActorLocation();

  // 煙 2つ
  for (int i = 0; i < 2; ++i) {
    float a = UMath::DegToRad(distAngle(rng));
    float s = distSmoke(rng);
    float life = distLife(rng);
    FDriftParticle p;
    p.Location = {base.X + distOfs(rng), base.Y + distOfs(rng)};
    p.Velocity = {std::cos(a) * s, std::sin(a) * s};
    p.Life = life;
    p.MaxLife = life;
    p.Radius = 6.0f;
    p.IsSpark = false;
    m_driftParticles.push_back(p);
  }

  // 火花 3つ
  for (int i = 0; i < 3; ++i) {
    float a = UMath::DegToRad(distAngle(rng));
    float s = distSpark(rng);
    float life = distSparkLife(rng);
    FDriftParticle p;
    p.Location = {base.X + distOfs(rng), base.Y + distOfs(rng)};
    p.Velocity = {std::cos(a) * s, std::sin(a) * s};
    p.Life = life;
    p.MaxLife = life;
    p.Radius = 0.0f;
    p.IsSpark = true;
    m_driftParticles.push_back(p);
  }
}

void APlayer::DrawDriftEffect() {
  auto& rs = RenderSystem::GetInstance();

  // ----- タイヤ痕 -----
  for (const auto& mark : m_skidMarks) {
    int alpha = static_cast<int>(mark.Alpha * 160.0f);
    FVector2D topLeft = FVector2D(-3.0f, -10.0f).RotateVector(mark.Rotation);
    rs.SubmitBox(
        mark.Location + topLeft,
        {6.0f, 20.0f},
        mark.Rotation,
        0x111111,
        true,
        RenderSpace::World,
        -1,
        alpha
    );
  }

  // ----- 煙・火花パーティクル -----
  for (const auto& p : m_driftParticles) {
    float lifeRatio = p.Life / p.MaxLife;
    int alpha = static_cast<int>(lifeRatio * 190.0f);

    if (p.IsSpark) {
      FVector2D tip = p.Location + p.Velocity * 0.025f;
      rs.SubmitLine(p.Location, tip, 0xFFCC00, RenderSpace::World, 3, alpha);
    } else {
      rs.SubmitCircle(p.Location, p.Radius, 0xBBBBBB, true, RenderSpace::World, 3, alpha);
    }
  }
}


void APlayer::GrantHeldItem() {
  if (!bHasAuthority) {
    return;  
  }
  m_hasHeldItem = true;
  MarkReplicatedStateDirty();
}
void APlayer::UseHeldItem() {
  M_LOG("UseHeldItem called. hasItem={}, isLocal={}", m_hasHeldItem, bIsLocallyControlled);  
  if (!bIsLocallyControlled) {
    return;
  }
  if (!m_hasHeldItem) {
    return;
  }
  if (!bIsLocallyControlled) {
    return;
  }
  if (!m_hasHeldItem) {
    return; 
  }
  if (bHasAuthority) {
    Server_UseHeldItem();
  } else {
    m_hasHeldItem = false;
    InvokeRPC(RPC_ServerUseHeldItem, ENetRPCType::Server, ENetPacketReliability::Reliable);
  }
}
void APlayer::Server_UseHeldItem() {
  if (!m_hasHeldItem) {
    return; 
  }
  m_hasHeldItem = false;
  MarkReplicatedStateDirty();
  ApplyHeldItemEffect();
}
void APlayer::ApplyHeldItemEffect() {
  if (Movement) {
    Movement->AddLocalForce({0, -25.0f});
  }
  ApplyFOVEffect(0.7f, 2.0f, true);
  if (m_sound) {
    m_sound->PlaySE("images/somekinoko", false);
  }
  M_LOG("Held item used: speed boost applied");
}

void APlayer::RemoveSlowSource(ASlowFloor2* source) { m_slowSources.erase(source); }

void APlayer::AddSlowSource(ASlowFloor2* source, float strength) {
  if (!source) return;
  // strength: 値が小さいほど強い減速（Player.cpp の更新ロジックに合わせる）
  m_slowSources[source] = strength;
}
void APlayer::OnLapLineCrossed(int totalCheckpoints) {
  if (!bHasAuthority) return;

  if (m_lapLineCooldown > 0.0f) return;

  if (totalCheckpoints > 0 && m_lastPassedCheckpoint < totalCheckpoints - 1) {
    M_LOG(
        "Lap line crossed but checkpoints incomplete: {}/{}",
        m_lastPassedCheckpoint + 1,
        totalCheckpoints
    );
    return;
  }

  m_lastPassedCheckpoint = -1;
  m_currentLap++;
  m_lapLineCooldown = 3.0f;

  // 全クライアントに周回数を通知
  InvokeRPC(
      RPC_MulticastUpdateLap, ENetRPCType::Multicast, ENetPacketReliability::Reliable, m_currentLap
  );

  M_LOG("Lap {} / {} completed!", m_currentLap, TotalLaps);

  if (m_currentLap >= TotalLaps) {
    NotifyGoalReached();
  }
}

void APlayer::Multicast_UpdateLap(int newLap) {
  m_currentLap = newLap;
  M_LOG("Lap updated to {} (multicast)", m_currentLap);
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
