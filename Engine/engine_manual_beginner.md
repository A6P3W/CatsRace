\---

## 目次

1. [エンジンの全体像を理解しよう](#1-エンジンの全体像を理解しよう)
2. [ゲームオブジェクト（Actor）を作ろう](#2-ゲームオブジェクトactorを作ろう)
3. [見た目を付けよう（SpriteComponent）](#3-見た目を付けようspritecomponent)
4. [シーンとオブジェクトを管理しよう](#4-シーンとオブジェクトを管理しよう)
5. [キー入力を受け取ろう](#5-キー入力を受け取ろう)
6. [カメラと描画の仕組み](#6-カメラと描画の仕組み)
7. [便利な型・ユーティリティ](#7-便利な型ユーティリティ)

\---

## 1\. エンジンの全体像を理解しよう

### ゲームはループで動いている

ゲームは「毎フレーム同じ処理を繰り返す」ことで動いています。  
このエンジンでは、1フレームの中で次の順番に処理が走ります。

```
① キー入力を読み取る        （InputManager）
② シーン切り替えがあれば実行  （SceneManager）
③ 全オブジェクトを更新する   （ObjectManager）
④ 現在のシーンを更新する     （GameModeBase）
⑤ 全オブジェクトを描画する   （RenderSystem）
```

これが60回/秒のペースで繰り返されます。

### エンジンの主要クラス一覧

|クラス名|役割|
|-|-|
|`AActor`|ゲームに登場するもの（キャラ・弾・背景など）の基底クラス|
|`MSceneComponent`|Actorの位置・回転・大きさを管理するコンポーネント|
|`MSpriteComponent`|画像やテキストなどを描画するコンポーネント|
|`MCameraComponent`|カメラ（画面の見え方）を制御するコンポーネント|
|`AGameModeBase`|シーン（ステージ）の基底クラス|
|`ObjectManager`|Actorの生成・更新・削除を管理するシステム|
|`SceneManager`|シーンの切り替えを管理するシステム|
|`RenderSystem`|描画命令をまとめて実行するシステム|
|`InputManager`|キーボードの生の入力状態を管理するシステム|
|`InputMapper`|「UP」「DOWN」などのアクション名でキー入力を取得するシステム|

### 「コンポーネント」って何？

コンポーネントとは、**機能の部品**のことです。  
たとえば「画像を表示する機能」「当たり判定の機能」などを、Actor に部品として取り付けます。

```
AActor（ゲームオブジェクト）
├── MSceneComponent（ルート：位置・回転を管理、自動で作成される）
├── MSpriteComponent（追加：画像を表示する）
└── MCameraComponent（追加：カメラ機能）
```

Actor はコンストラクタで `MSceneComponent`（ルートコンポーネント）を自動的に持ちます。  
それ以外のコンポーネントは、自分で `AddComponent` を使って追加します。

\---

## 2\. ゲームオブジェクト（Actor）を作ろう

### 最小構成の Actor

まず `AActor` を継承したクラスを作ります。

```cpp
// MyActor.h
#pragma once
#include "Actor.h"

class AMyActor : public AActor  // AActor を継承
{
public:
    AMyActor();                          // コンストラクタ（初期化）
protected:
    void OnUpdate(float DeltaTime) override; // 毎フレーム呼ばれる処理
};
```

```cpp
// MyActor.cpp
#include "MyActor.h"

AMyActor::AMyActor()
{
    // 生成時の座標を設定する
    SetActorLocation({ 400.0f, 300.0f });
}

void AMyActor::OnUpdate(float DeltaTime)
{
    // ここに毎フレームの処理を書く
    // DeltaTime = 前のフレームから経過した時間（秒）
}
```

> \*\*`OnUpdate` の `DeltaTime` について\*\*  
> フレームレートが多少ブレても動作が一定になるよう、移動量などに `DeltaTime` をかけるのが基本です。  
> 例：`speed \* DeltaTime` → 「1秒あたり speed ピクセル移動する」という意味になります。

### Actor の主なメソッド

#### 座標・回転・スケールの操作

```cpp
// 座標
SetActorLocation({ 100.0f, 200.0f }); // ワールド座標を直接セット
GetActorLocation();                    // 現在のワールド座標を取得

// ワールド座標でオフセット（画面の右方向が +X, 下方向が +Y）
AddActorWorldOffset({ 5.0f, 0.0f });

// ローカル座標でオフセット（Actorが向いている方向が前）
AddActorLocalOffset({ 0.0f, -5.0f }); // 自分の向きで「前へ進む」

// 回転（度数で指定）
SetActorRotation(FRotator{ 90.0f });  // 90度に設定
AddActorRotation(FRotator{ 1.0f });   // 1度ずつ回転する

// 大きさ
SetActorScale(2.0f); // 2倍サイズに
```

> \*\*ワールド座標 vs ローカル座標\*\*  
> - \*\*ワールド座標\*\*：画面全体を基準にした絶対的な座標  
> - \*\*ローカル座標\*\*：自分自身が向いている方向を基準にした相対的な座標  
> 斜めを向いているキャラを「前に進ませる」には `AddActorLocalOffset` が便利です。

#### Actor の削除

```cpp
this->Destroy(); // 次のフレームに削除される
```

`Destroy()` を呼んだ瞬間に消えるのではなく、そのフレームの処理が終わった後に `ObjectManager` が削除します。

\---

## 3\. 見た目を付けよう（SpriteComponent）

### 画像を表示する

```cpp
#include "SpriteComponent.h"
#include "ResourceManager.h"

AMyActor::AMyActor()
{
    // ① 画像をロードしてハンドル（ID番号）を取得
    int handle = ResourceManager::GetInstance().LoadResourceGraph("images/player.png");

    // ② SpriteComponent を作成
    //    第1引数: 描画の優先度（小さいほど先に描画される）
    //    第2引数: RenderSpace::World = カメラの影響を受ける
    auto sprite = std::make\_unique<MSpriteComponent>(0, RenderSpace::World);

    // ③ 描画する内容を設定（画像）
    //    第1引数: 表示倍率（1.0 = 等倍）
    //    第2引数: 画像ハンドル
    sprite->SubmitGraph(1.0, handle);

    // ④ コンポーネントを Actor に追加
    AddComponent(std::move(sprite));
}
```

> \*\*`std::make\_unique` について\*\*  
> コンポーネントはメモリを自動管理する `unique\_ptr` で作ります。  
> `AddComponent` に渡すと所有権がエンジン側に移るため、以降は直接ポインタを保持する必要はありません。  
> もし追加後も操作したい場合は、`AddComponent` の前にポインタを保存してください。
>
> ```cpp
> auto sprite = std::make\_unique<MSpriteComponent>(0, RenderSpace::World);
> MSpriteComponent\* spritePtr = sprite.get(); // 追加前にポインタを保存
> AddComponent(std::move(sprite));
> spritePtr->SubmitGraph(1.0, handle);        // 後から操作できる
> ```

### 描画できるもの一覧

|メソッド|説明|
|-|-|
|`SubmitGraph(倍率, ハンドル, 透明度)`|画像|
|`SubmitRectGraph(切出しX, 切出しY, 幅, 高さ, ハンドル, 透明度)`|画像の一部（スプライトシートに使用）|
|`SubmitBox(幅, 高さ, 色, 塗りつぶし, 透明度)`|矩形|
|`SubmitCircle(半径, 色, 塗りつぶし, 透明度)`|円|
|`SubmitLine(終点X, 終点Y, 色, 透明度)`|線分（始点はコンポーネントの座標）|
|`SubmitText(文字列, 色, フォントハンドル, 透明度)`|テキスト|

`透明度` は 0（完全透明）〜 255（不透明）で指定します。省略すると 255 になります。

### UI（画面に固定表示）したい場合

体力バーや残機表示など、カメラが動いても画面上の同じ位置に表示したいものは `RenderSpace::Screen` を使います。

```cpp
// UI用 SpriteComponent はカメラの影響を受けない
auto ui = std::make\_unique<MSpriteComponent>(100, RenderSpace::Screen);
```

### コンポーネントの位置をずらす

コンポーネントを Actor の中心からずらして配置できます。

```cpp
auto sprite = std::make\_unique<MSpriteComponent>(0, RenderSpace::World);
sprite->SubmitGraph(1.0, handle);
sprite->SetParentComponent(this->GetRootComponent()); // 親を Actor のルートに設定
sprite->SetRelativeLocation({ 50.0f, 0.0f });         // 親から右に 50px
AddComponent(std::move(sprite));
```

\---

## 4\. シーンとオブジェクトを管理しよう

### シーンを作る

シーン（ステージや画面）は `AGameModeBase` を継承して作ります。

```cpp
// GameScene.h
#pragma once
#include "GameModeBase.h"

class AGameScene : public AGameModeBase
{
public:
    AGameScene();
    void OnUpdate(float DeltaTime) override;
};
```

```cpp
// GameScene.cpp
#include "GameScene.h"
#include "ObjectManager.h"
#include "MyActor.h"

AGameScene::AGameScene()
{
    // シーン開始時に Actor をスポーン（生成）する
    ObjectManager::GetInstance().SpawnObject<AMyActor>(
        FVector2D{ 400.0f, 300.0f }, // 生成する座標
        FRotator{ 0.0f }             // 生成時の回転
    );
}

void AGameScene::OnUpdate(float DeltaTime)
{
    // シーン全体のロジック（ゲームクリア判定など）をここに書く
}
```

### ゲーム起動時の初期シーンを設定する

`SetupGame()` 関数はエンジンの外で定義し、起動直後に呼ばれます。  
ここで最初に表示するシーンを設定します。

```cpp
// SetupGame.cpp（自分で作成するファイル）
#include "SceneManager.h"
#include "GameScene.h"

void SetupGame()
{
    SceneManager::GetInstance().OpenScene<AGameScene>();
}
```

### シーンを切り替える

```cpp
// どこからでもシーンを切り替えられる
// 切り替えは次のフレームの開始時に実行される
// 切り替え時に ObjectManager の全 Actor が自動削除される
SceneManager::GetInstance().OpenScene<ANextScene>();
```

### Actor をスポーンする

```cpp
// 座標・回転を指定してスポーン
AMyActor\* actor = ObjectManager::GetInstance().SpawnObject<AMyActor>(
    FVector2D{ 100.0f, 200.0f },
    FRotator{ 0.0f }
);

// デフォルトコンストラクタがある場合は引数なしでもOK
ObjectManager::GetInstance().SpawnObject<AMyActor>();
```

返ってきたポインタは参照用です。オブジェクトの**所有権**は `ObjectManager` にあるため、自分でdeleteしてはいけません。

\---

## 5\. キー入力を受け取ろう

### InputMapper を使う（推奨）

キーコードを直接書かずに「UP」「DOWN」などのアクション名で入力を取得できます。

```cpp
#include "InputMapper.h"

void AMyActor::OnUpdate(float DeltaTime)
{
    float speed = 300.0f;

    // 押している間ずっと反応する
    if (InputMapper::GetInstance().GetKeyPressing(E\_INPUT\_ACTION::UP)) {
        AddActorLocalOffset({ 0.0f, -speed \* DeltaTime });
    }
    if (InputMapper::GetInstance().GetKeyPressing(E\_INPUT\_ACTION::DOWN)) {
        AddActorLocalOffset({ 0.0f, speed \* DeltaTime });
    }

    // 押した瞬間だけ反応する（ジャンプや攻撃など）
    if (InputMapper::GetInstance().GetKeyPressStart(E\_INPUT\_ACTION::INTERACT)) {
        // アクションを実行
    }
}
```

**デフォルトのキー割り当て**

|アクション|割り当てキー|
|-|-|
|`UP`|↑ キー, W キー|
|`DOWN`|↓ キー, S キー|
|`LEFT`|← キー, A キー|
|`RIGHT`|→ キー, D キー|
|`CANCEL`|Escape キー|

**3つの取得方法**

|メソッド|いつ `true` になるか|
|-|-|
|`GetKeyPressStart(action)`|押した瞬間の 1フレームだけ|
|`GetKeyPressing(action)`|押している間ずっと|
|`GetKeyRelease(action)`|離した瞬間の 1フレームだけ|

### InputManager を使う（DxLibのキーコードで直接取得）

`INTERACT` などエンジンに定義されていないキーを使いたい場合は、`InputManager` で DxLib のキーコードを直接指定します。

```cpp
#include "InputManager.h"
#include <DxLib.h>

if (InputManager::GetInstance().GetKeyPressStart(KEY\_INPUT\_SPACE)) {
    // スペースキーを押した瞬間
}
```

\---

## 6\. カメラと描画の仕組み

### カメラを設定する

シーンを作ったとき、`AGameModeBase` のコンストラクタで自動的にカメラが作成されます。  
カメラを自分のキャラクターに追従させたい場合は、自分の Actor にカメラを追加します。

```cpp
#include "CameraComponent.h"

AMyActor::AMyActor()
{
    // カメラコンポーネントを作成して追加
    auto camera = std::make\_unique<MCameraComponent>();
    MCameraComponent\* camPtr = camera.get(); // 追加前にポインタを保存
    AddComponent(std::move(camera));
    camPtr->SetActiveCamera(); // このカメラをアクティブにする
}
```

`SetActiveCamera()` を呼ぶと、ゲーム全体でこのカメラが使われるようになります。  
カメラは Actor と一緒に動くため、Actor を動かすだけでカメラも追従します。

### 描画の仕組み（読むだけでOK）

`MSpriteComponent` の `Draw()` が呼ばれると、内容が `RenderSystem` に **描画コマンド** として積まれます。  
フレームの最後に `RenderSystem::Draw()` が呼ばれ、`priority` の小さい順にソートされてまとめて描画されます。

```
各 SpriteComponent::Draw()
    → RenderSystem に描画コマンドを追加
        → RenderSystem::Draw() で一括描画（priority 順）
```

`RenderSpace::World` のコンポーネントは、カメラの位置・回転・FOVに基づいて座標変換されてから描画されます。

### リソース（画像・フォント）の管理

同じパスの画像を何度ロードしても、内部でキャッシュされるため無駄なロードは発生しません。

```cpp
// 画像のロード（同じパスなら2回目以降はキャッシュから返る）
int imgHandle = ResourceManager::GetInstance().LoadResourceGraph("images/enemy.png");

// フォントの取得（サイズ・太さの組み合わせでキャッシュされる）
int fontHandle = ResourceManager::GetInstance().GetFont(16, 3);
```

\---

## 7\. 便利な型・ユーティリティ

### FVector2D（2D座標・ベクトル）

```cpp
FVector2D pos = { 100.0f, 200.0f }; // X=100, Y=200
FVector2D a = { 10.0f, 0.0f };
FVector2D b = { 5.0f, 3.0f };
FVector2D c = a + b; // { 15.0f, 3.0f }

FVector2D zero = FVector2D::ZeroVector; // { 0, 0 }
```

### FRotator（回転値）

```cpp
FRotator rot = FRotator{ 90.0f }; // 90度
float degrees = rot.Rotation;     // 度数で取得
```

### FScale（スケール）

```cpp
FScale scale = FScale{ 2.0f }; // 2倍
float value = scale.Scale;
```

### UMath（数学ユーティリティ）

```cpp
#include "Utils/UMath.h"

float rad = UMath::DegToRad(180.0f); // 度 → ラジアン変換
float deg = UMath::RadToDeg(3.14f);  // ラジアン → 度変換
```

### デバッグログ

Visual Studio の「出力ウィンドウ」にメッセージを表示できます。

```cpp
#include "Utils/Log.h"

// 数値や文字列を出力できる
M\_LOG("プレイヤーのX座標: {}", GetActorLocation().X);
M\_LOG("状態: {}, HP: {}", "走り中", 100);
```

\---

## まとめ：よくある作業のチェックリスト

### 新しいゲームオブジェクトを作るとき

* \[ ] `AActor` を継承したクラスを作る（`.h` と `.cpp`）
* \[ ] コンストラクタで `SetActorLocation` と `AddComponent` を記述
* \[ ] `OnUpdate` に毎フレームの処理を書く
* \[ ] `ObjectManager::SpawnObject<T>()` でシーンから生成する

### 画像を表示するとき

* \[ ] `ResourceManager::LoadResourceGraph` で画像をロードしてハンドルを取得
* \[ ] `MSpriteComponent` を作って `SubmitGraph` を呼ぶ
* \[ ] `AddComponent` で Actor に追加する

### 新しいシーンを作るとき

* \[ ] `AGameModeBase` を継承したクラスを作る
* \[ ] コンストラクタで必要な Actor をスポーン
* \[ ] `SceneManager::OpenScene<T>()` で遷移する

