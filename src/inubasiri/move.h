#pragma once
#include <complex>
using Vector2D = std::complex<float>;

class Mover {
public:
    Mover(float x, float y, float vx, float vy);

    // 更新処理
    void Update();

    // 複素数を使った回転処理
    void RotateVelocity(float angle);

    // ゲッター
    float GetX() const { return pos.real(); }
    float GetY() const { return pos.imag(); }

private:
    Vector2D pos;      // 位置
    Vector2D velocity; // 速度（移動ベクトル）
};