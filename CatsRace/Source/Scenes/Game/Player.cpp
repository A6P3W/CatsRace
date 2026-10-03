#define NOMINMAX
#include "Player.h"

#include <EasyShakeComponent.h>
#include <EnhancedInputComponent.h>
#include <PlayerController.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <random>

#include "ActorManager.h"
#include "CameraComponent.h"
#include "CircleCollisionComponent.h"
#include "Core/GI_main.h"
#include "InputManager.h"
#include "InputMapper.h"
#include "LapCheckpoint.h"
#include "Log.h"
#include "NetMovementComponent.h"
#include "Objects/Items/HeldSpeedItem.h"
#include "Objects/Items/SpeedDownstage.h"
#include "RectangleCollisionComponent.h"
#include "RenderSystem.h"
#include "ResourceManager.h"
#include "SceneManager.h"
#include "Scenes/Game/GameSceneBase.h"
#include "Scenes/Practice/PracticeGameMode.h"
#include "SpriteComponent.h"
namespace {
enum : FNetworkRPCId {
  RPC_ServerSetDrift = 1,
  RPC_ServerNotifyGoal = 2,
  RPC_ServerUseHeldItem = 3,
  RPC_ServerSyncDriftState = 4,
  RPC_MulticastUpdateLap = 5,

};
}

REGISTER_ACTOR(APlayer)

APlayer::APlayer(FVector2D location, FRotator rotation) {
  bReplicates = true;
  RegisterReplicatedProperty(&m_isDrifting);
  RegisterReplicatedProperty(&m_driftDirection);
  RegisterReplicatedProperty(&CanMove);
  RegisterReplicatedProperty(&m_hasHeldItem);
  RegisterReplicatedProperty(&m_PlayerName);
  RegisterReplicatedProperty(&PlayerColorIndex, this, &APlayer::OnRepPlayerColorIndex);
  RegisterReplicatedProperty(&ReplicatedDriftGaugeRatio);
  RegisterReplicatedProperty(&RaceProgress.GetReplicatedLap());
  RegisterReplicatedProperty(&SpeedMultiplier);
  RegisterRPC(RPC_ServerSetDrift, ENetRPCType::Server, this, &APlayer::Server_SetDrift);
  RegisterRPC(RPC_ServerNotifyGoal, ENetRPCType::Server, this, &APlayer::Server_NotifyGoal);
  RegisterRPC(RPC_ServerUseHeldItem, ENetRPCType::Server, this, &APlayer::Server_UseHeldItem);
  RegisterRPC(RPC_ServerSyncDriftState, ENetRPCType::Server, this, &APlayer::Server_SyncDriftState);
  RegisterRPC(RPC_MulticastUpdateLap, ENetRPCType::Multicast, this, &APlayer::Multicast_UpdateLap);
  SetActorLocation(location);
  SetActorRotation(rotation);
  m_PlayerNameFontHandle = ResourceManager::GetInstance().GetFont(20, 5);

  m_walkAnimHandles[0] =
      ResourceManager::GetInstance().LoadResourceGraph("/Game/images/cat_walk_1_bw.png");
  m_walkAnimHandles[1] =
      ResourceManager::GetInstance().LoadResourceGraph("/Game/images/cat_walk_2_bw.png");
  m_walkAnimHandles[2] =
      ResourceManager::GetInstance().LoadResourceGraph("/Game/images/cat_walk_3_bw.png");
  m_walkAnimHandles[3] =
      ResourceManager::GetInstance().LoadResourceGraph("/Game/images/cat_walk_4_bw.png");
  m_walkAnimHandles[4] =
      ResourceManager::GetInstance().LoadResourceGraph("/Game/images/cat_walk_5_bw.png");
  m_sprite = NewObject<MSpriteComponent>(this);
  m_sprite->SetRenderSettings(50, RenderSpace::World);
  m_sprite->SubmitGraph(m_walkAnimHandles[0]);
  ApplyPlayerColor();
  m_sprite->AttachToComponent(GetRootComponent());
  m_sprite->RegisterComponent();

  SetActorScale(FScale(0.4f));
  auto* col = NewObject<MCircleCollisionComponent>(this);
  col->SetRadius(52.0f);
  col->AttachToComponent(GetRootComponent());
  col->SetCollisionType(ECollisionType::Block);
  col->SetStatic(false);
  col->RegisterComponent();

  Movement = NewObject<MNetMovementComponent>(this);
  Movement->RegisterComponent();

  m_shake = NewObject<MEasyShakeComponent>(this);
  m_shake->RegisterComponent();

  // カメラ
  m_camera = NewObject<MCameraComponent>(this);
  m_camera->SetFOV(1);
  m_camera->AttachToComponent(m_shake);
  m_camera->RegisterComponent();

  m_camera->AddLocalOffset({0.0f, -400.0f});
}

APlayer::~APlayer() {
  if (m_sound) {
    m_sound->StopAll();
  }
}

void APlayer::Draw() {
  AActor::Draw();

  if (bIsLocallyControlled || m_PlayerName.empty() || m_PlayerNameFontHandle == -1) {
    return;
  }

  auto& renderSystem = RenderSystem::GetInstance();
  FVector2D labelPos = renderSystem.WorldToScreen(GetActorLocation());
  const int textWidth =
      ResourceManager::GetInstance().GetTextWidth(m_PlayerName, m_PlayerNameFontHandle);
  labelPos.X -= textWidth * 0.5f;
  labelPos.Y -= 72.0f;

  renderSystem.SubmitText(
      {labelPos.X + 1.0f, labelPos.Y + 1.0f},
      m_PlayerName,
      m_PlayerNameFontHandle,
      FColor{0, 0, 0, 180},
      RenderSpace::Screen,
      2
  );
  renderSystem.SubmitText(
      labelPos, m_PlayerName, m_PlayerNameFontHandle, FColor{255, 255, 255}, RenderSpace::Screen, 3
  );
}

void APlayer::SetPlayerName(const std::string& PlayerName) {
  if (m_PlayerName == PlayerName) {
    return;
  }

  m_PlayerName = PlayerName;
  MarkReplicatedStateDirty();
}

void APlayer::SetSpeedMultiplier(float InMultiplier) {
  const float NewMultiplier = std::clamp(InMultiplier, MinSpeedMultiplier, MaxSpeedMultiplier);
  if (SpeedMultiplier == NewMultiplier) {
    return;
  }

  SpeedMultiplier = NewMultiplier;
  MarkReplicatedStateDirty();
}

void APlayer::SetPlayerColorIndex(uint8_t InColorIndex) {
  if (PlayerColorIndex == InColorIndex) {
    return;
  }

  PlayerColorIndex = InColorIndex;
  ApplyPlayerColor();
  MarkReplicatedStateDirty();
}

void APlayer::ApplyPlayerColor() {
  if (!m_sprite) {
    return;
  }

  if (PlayerColorIndex < PlayerColorPalette.size()) {
    CurrentPlayerColor = PlayerColorPalette[PlayerColorIndex];
  } else {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, PlayerColorPalette.size() - 1);
    CurrentPlayerColor = PlayerColorPalette[dis(gen)];
  }
  m_sprite->SetTint(CurrentPlayerColor);
}

void APlayer::OnRepPlayerColorIndex(uint8_t OldColorIndex) {
  (void)OldColorIndex;
  ApplyPlayerColor();
}

void APlayer::OnUpdate(float DeltaTime) {
  const float EffectiveMaxSpeed = BaseMaxSpeed * SpeedMultiplier;
  const float EffectiveMaxReverseSpeed = BaseMaxReverseSpeed * SpeedMultiplier;

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
      Movement->SetWorldVelocity(Movement->GetVelocity() * decayPerFrame);
    }
    if (m_accelInput > 0.0f) {
      float speedRatio = std::clamp(speed / EffectiveMaxSpeed, 0.0f, 1.0f);
      float force = AccelForce * m_accelInput * (1.0f - speedRatio * 0.8f);
      Movement->AddLocalForce({0.0f, -force});
    } else if (m_accelInput < 0.0f) {
      float speedRatio = std::clamp(speed / EffectiveMaxReverseSpeed, 0.0f, 1.0f);
      float force = ReverseForce * (-m_accelInput) * (1.0f - speedRatio * 0.8f);
      Movement->AddLocalForce({0.0f, force});
    }
    // ---- ステアリング ----
    float steerAbility = std::clamp(speed / 3.0f, 0.0f, 1.0f);
    float sliderDir = (m_accelInput < 0.0f) ? -m_slider : m_slider;
    UpdateDrift(DeltaTime, speed);
    if (!bHasAuthority) {
      const float driftGaugeRatio = m_isDrifting
                                        ? std::clamp(m_driftGauge / MaxDriftGauge, 0.0f, 1.0f)
                                        : DriftVisuals.GetReleaseGaugeRatio();
      InvokeRPC(
          RPC_ServerSyncDriftState,
          ENetRPCType::Server,
          ENetPacketReliability::Unreliable,
          m_isDrifting,
          m_driftDirection,
          driftGaugeRatio
      );
    }
    float steerMultiplier = m_isDrifting ? DriftSteerMultiplier : 0.7f;
    float steerAngle = BaseMaxSteer * m_slider * steerAbility * steerMultiplier;
    AddActorRotation(FRotator(steerAngle));
    Movement->AddVelocityRotation(FRotator(steerAngle));

  } else if (bIsLocallyControlled) {
    UpdateLocalDriftVisual(DeltaTime, speed);
  }

  DriftVisuals.Update(
      DeltaTime, GetActorLocation(), GetActorRotation(), CurrentPlayerColor, m_isDrifting
  );
  DriftVisuals.Draw(GetDriftGaugeRatioForVisuals());
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
    float t = std::clamp(speed / EffectiveMaxSpeed, 0.0f, 1.0f);
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
    const float GaugeY = 1000.0f;
    const float GaugeWidth = 400.0f;
    const float GaugeHeight = 24.0f;

    float gaugeRatio = std::clamp(m_driftGauge / MaxDriftGauge, 0.0f, 1.0f);

    // 背景（枠）
    RenderSystem::GetInstance().SubmitBox(
        {GaugeX, GaugeY},
        {GaugeWidth, GaugeHeight},
        FRotator(0.0f),
        FColor{68, 68, 68, 200},
        1,
        RenderSpace::Screen,
        250
    );

    // ゲージ本体（溜まり具合に応じて色を変える）
    FColor gaugeColor = (gaugeRatio >= 1.0f) ? FColor{255, 68, 68} : FColor{68, 204, 255};
    RenderSystem::GetInstance().SubmitBox(
        {GaugeX, GaugeY},
        {GaugeWidth * gaugeRatio, GaugeHeight},
        FRotator(0.0f),
        gaugeColor,
        1,
        RenderSpace::Screen,
        251
    );

    // 1/5（20%）の位置にしきい値ラインを表示
    float thresholdX = GaugeX + GaugeWidth * 0.2f;
    RenderSystem::GetInstance().SubmitLine(
        {thresholdX, GaugeY},
        {thresholdX, GaugeY + GaugeHeight},
        FColor::Yellow,
        RenderSpace::Screen,
        253
    );

    // 枠線
    RenderSystem::GetInstance().SubmitBox(
        {GaugeX, GaugeY},
        {GaugeWidth, GaugeHeight},
        FRotator(0.0f),
        FColor{255, 255, 255},
        0,
        RenderSpace::Screen,
        252
    );
  }

  if (!bRotateCamera) {
    m_camera->SetWorldRotation(FRotator{0.0f});
  }
}

void APlayer::OnPossessedBy(APlayerController* NewController) {
  APawn::OnPossessedBy(NewController);
  if (bIsLocallyControlled) {
    if (auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
      SetRotateCamera(gi->bRotateCamera);
    }
  }
  if (bIsLocallyControlled || (bHasAuthority && OwnerConnectionId == 0)) {
    m_camera->SetActiveCamera();
  }
  m_camera->SetFOV(1);
}

void APlayer::SetupPlayerInputComponent(MEnhancedInputComponent* PlayerInputComponent) {
  //PlayerInputComponent->BindAction(
  //    InputAction::Interact, ETriggerEvent::Started, this, &APlayer::OnRestartPressed
  //);
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
  if (!GetWorld() || !GetWorld()->GetActorManager()) {
    return bPressed;
  }

  for (const auto& actorPtr : GetWorld()->GetActorManager()->GetAllActors()) {
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

  m_accelInput = clampedInput.Y;
  m_slider = -clampedInput.X;
}

void APlayer::Server_SetDrift(bool bDriftHeld) { m_driftKeyPressed = bDriftHeld; }

void APlayer::Server_SyncDriftState(bool bDrifting, float driftDirection, float driftGaugeRatio) {
  ReplicatedDriftGaugeRatio = std::clamp(driftGaugeRatio, 0.0f, 1.0f);
  if (m_isDrifting && !bDrifting) {
    BeginSkidReleaseTrail();
  }
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
    InvokeRPC(RPC_ServerNotifyGoal, ENetRPCType::Server, ENetPacketReliability::Reliable);
  }
}
//void APlayer::OnRestartPressed() {
//  if (!bIsLocallyControlled) return;
//  if (auto* gameScene = dynamic_cast<AGameSceneBase*>(GetWorld()->GetGameMode())) {
//   gameScene->RestartGame();
// }
//}
//
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

  M_LOG(Log, "Player BeginOverlap with " + OtherActor->GetActorClassName());
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
    m_shake->StartShake(m_crashshake, {2, 2}, 2011);
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
  M_LOG(Log, "Player EndOverlap with " + OtherActor->GetActorClassName());

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
  if (dynamic_cast<APracticeGameMode*>(GetWorld()->GetGameMode())) {
    SetCanMove(true);
  }

  if (bIsLocallyControlled) {
    m_sound = NewObject<MSoundComponent>(this);
    m_sound->RegisterComponent();
    m_engineIdleHandle = m_sound->PlaySE("/Game/images/cat5.mp3", true);
    m_engineRunHandle = m_sound->PlaySE("/Game/images/moving-v2.mp3", true);
    if (auto* sm = GetWorld()->GetSoundManager()) {
      int bgmHandle = sm->PlaySE("/Game/images/Neon_Velocity_3.mp3", true);

      if (bgmHandle != -1) {
        sm->SetVolume(bgmHandle, 0.3f);
      }
    }
  }
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

  const auto now = std::chrono::steady_clock::now().time_since_epoch();
  rng.seed(
      static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(now).count())
  );

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
        {sx, sy},
        {ex, ey},
        FColor{255, 255, 255, static_cast<uint8_t>(alpha)},
        RenderSpace::Screen,
        200
    );
  }
}
bool APlayer::CanStartDrift(float speed) const {
  return m_accelInput >= 0.0f && m_driftKeyPressed && std::abs(m_slider) > 0.3f &&
         (speed > DriftMinSpeed || std::abs(m_accelInput) > 0.1f);
}

bool APlayer::WantsToContinueDrift() const {
  const float CurrentDirection = (m_slider > 0.0f) ? 1.0f : -1.0f;
  return m_accelInput >= 0.0f && m_driftKeyPressed && std::abs(m_slider) > 0.1f &&
         CurrentDirection == m_driftDirection;
}

void APlayer::UpdateLocalDriftVisual(float DeltaTime, float speed) {
  const bool bWantsDrift = m_isDrifting ? WantsToContinueDrift() : CanStartDrift(speed);

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
    BeginSkidReleaseTrail();
    m_isDrifting = false;
    m_driftGauge = 0.0f;
  }
}

void APlayer::UpdateDrift(float DeltaTime, float speed) {
  const bool bWantsDrift = m_isDrifting ? WantsToContinueDrift() : CanStartDrift(speed);

  if (bWantsDrift) {
    if (!m_isDrifting) {
      // ドリフト開始：最初に入力した方向を固定
      m_isDrifting = true;
      m_driftGauge = 0.0f;
      m_driftDirection = (m_slider > 0.0f) ? 1.0f : -1.0f;
      M_LOG(Log, "Drift Start", 0);
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
    Movement->SetWorldVelocity(Movement->GetVelocity() * decay);
  } else {
    if (m_isDrifting) {
      BeginSkidReleaseTrail();
      // ドリフト終了 → ブースト
      float boostRatio = m_driftGauge / MaxDriftGauge;
      if (boostRatio > 0.2f) {
        float boostForce = DriftBoostForce * boostRatio;
        Movement->AddLocalForce({0.0f, -boostForce});
        M_LOG(Log, "Drift Boost! ratio={}", boostRatio);
        ApplyFOVEffect(0.9f, 0.5f, true);
      }
      m_driftGauge = 0.0f;
    }

    m_isDrifting = false;
  }
}
void APlayer::BeginSkidReleaseTrail() {
  if (!m_isDrifting) {
    return;
  }

  DriftVisuals.BeginRelease(m_isDrifting, GetDriftGaugeRatioForVisuals());
}

float APlayer::GetDriftGaugeRatioForVisuals() const {
  if (bIsLocallyControlled || (bHasAuthority && OwnerConnectionId == 0)) {
    return std::clamp(m_driftGauge / MaxDriftGauge, 0.0f, 1.0f);
  }
  return ReplicatedDriftGaugeRatio;
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
void APlayer::GrantHeldItem() {
  if (!bHasAuthority) {
    return;
  }
  m_hasHeldItem = true;
  MarkReplicatedStateDirty();
}
void APlayer::UseHeldItem() {
  M_LOG(Log, "UseHeldItem called. hasItem={}, isLocal={}", m_hasHeldItem, bIsLocallyControlled);

  // 自身が操作していないプレイヤー、またはアイテムを所持していない場合は何もしない
  if (!bIsLocallyControlled || !m_hasHeldItem) {
    return;
  }

  if (bHasAuthority) {
    Server_UseHeldItem();
  } else {
    m_hasHeldItem = false;

    ApplyHeldItemEffect();

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
    // 拾った時に鳴らしていたSEを指定
    m_sound->PlaySE("/Game/images/cat2d.mp3", false);
  }
  M_LOG(Log, "Held item used: speed boost applied");
}
void APlayer::RemoveSlowSource(ASlowFloor2* source) { m_slowSources.erase(source); }

void APlayer::AddSlowSource(ASlowFloor2* source, float strength) {
  if (!source) return;
  // strength: 値が小さいほど強い減速（Player.cpp の更新ロジックに合わせる）
  m_slowSources[source] = strength;
}
void APlayer::OnLapLineCrossed(int totalCheckpoints) {
  if (!bHasAuthority) return;

  M_LOG(
      Log,
      "LapLine: lap={}, cooldown={}, lastCP={}, totalCP={}",
      RaceProgress.GetCurrentLap(),
      m_lapLineCooldown,
      RaceProgress.GetLastPassedCheckpoint(),
      totalCheckpoints
  );

  // if (m_lapLineCooldown > 0.0f) {
  //   M_LOG("LapLine: ignored by cooldown");
  //   return;
  // }

  if (!RaceProgress.CanCompleteLap(totalCheckpoints)) {
    M_LOG(Log, "LapLine: ignored by checkpoint incomplete");
    return;
  }

  const int CurrentLap = RaceProgress.CompleteLap();
  m_lapLineCooldown = 5.0f;
  MarkReplicatedStateDirty();
  InvokeRPC(
      RPC_MulticastUpdateLap, ENetRPCType::Multicast, ENetPacketReliability::Reliable, CurrentLap
  );

  M_LOG(Log, "Lap {} / {} completed!", CurrentLap, TotalLaps);

  if (CurrentLap >= TotalLaps) {
    NotifyGoalReached();
  }
}
void APlayer::Multicast_UpdateLap(int newLap) {
  RaceProgress.SetReplicatedLap(newLap, TotalLaps);
  M_LOG(
      Log,
      "Multicast_UpdateLap received: lap={}, isLocal={}, hasAuthority={}",
      RaceProgress.GetCurrentLap(),
      bIsLocallyControlled,
      bHasAuthority
  );
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
