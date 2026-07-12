#pragma once
#include "Actor.h"
#include "UMath.h"

class MRectangleCollisionComponent;
class MSpriteComponent;

// コース上に置くチェックポイント。
// SetIsLapLine(true) にしたものがスタート/ゴールライン。
// プレイヤーは全チェックポイントを順番に通過してからラインを踏むと1周カウント。
class ALapCheckpoint : public AActor {
 public:
  DEFINE_ACTOR_CLASS(ALapCheckpoint);
  ALapCheckpoint(FVector2D location = FVector2D::ZeroVector(), FRotator rotation = FRotator(0));
  const std::string& GetImagePath() const { return m_imagePath; }
  void SetImagePath(const std::string& path) { m_imagePath = path; }
  // チェックポイント番号（0始まりで昇順に設定する）
  void SetCheckpointIndex(int index) { m_index = index; }
  int GetCheckpointIndex() const { return m_index; }
  void SetSize(float width, float height);
  // true にするとスタート/ゴールライン扱い
  void SetIsLapLine(bool b) { m_bIsLapLine = b; }
  bool IsLapLine() const { return m_bIsLapLine; }

  // コース上の通常チェックポイント総数（ゴールライン側に設定）
  void SetTotalCheckpoints(int total) { m_totalCheckpoints = total; }

  void BeginOverlap(AActor* OtherActor) override;

 private:
  void BeginPlay() override;
  std::string m_imagePath;
  int m_index = 0;
  bool m_bIsLapLine = false;
  int m_totalCheckpoints = 0;

  MRectangleCollisionComponent* m_collision = nullptr;
  MSpriteComponent* m_sprite = nullptr;
};
