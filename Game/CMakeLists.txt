# Game Project (WIN32 Application)

# ソースおよびヘッダーファイルを再帰的に収集
file(GLOB_RECURSE GAME_SOURCES
  "Game/*.cpp"
  "Game/*.h"
)



# IDE上の表示フォルダ構造を物理フォルダ階層にマッピングする設定
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Game" PREFIX "Game" FILES ${GAME_SOURCES})

add_executable(Game WIN32 ${GAME_SOURCES})


# インクルードパスの設定
target_include_directories(Game PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/Game"
)


# 依存プロジェクトのリンク
target_link_libraries(Game PRIVATE BroccoliEngine)

# 静的ライブラリ内のグローバルオブジェクト強制初期化のための WHOLEARCHIVE 指定
if(MSVC)
  target_link_options(Game PRIVATE "/WHOLEARCHIVE:BroccoliEngine")
endif()

# ライブラリディレクトリの設定 (DxLib 用)
target_link_directories(Game PRIVATE "${DXLIB_DIR}")



# MSVC 固有のコンパイラ警告/動作設定
if(MSVC)
  target_compile_options(Game PRIVATE 
    "/MP"       # マルチプロセッサビルド
    "/wd4819"   # 特定の警告コードの抑制 (改行コード/コードページ関連)
    "/wd4828"   # 文字コードページ警告の抑制
  )
  # Visual Studio 上でのデバッグ時の作業ディレクトリを実行ファイル出力先ディレクトリに設定
  set_target_properties(Game PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Game>"
    VS_CHAR_SET "MultiByte"
  )
endif()



# ビルド後にアセットディレクトリ全体を実行ファイル出力先へ自動コピーする設定
add_custom_command(
  TARGET Game POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/Resources"
    "$<TARGET_FILE_DIR:Game>/Resources"
  COMMENT "Copying Resources directory to output directory"
)

# Game直下にある特定の.jsonファイルを実行ファイル出力先へ自動コピーする設定
file(GLOB GAME_ROOT_JSON
  "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

foreach(JSON_PATH ${GAME_ROOT_JSON})
  get_filename_component(JSON_NAME "${JSON_PATH}" NAME)
  add_custom_command(
    TARGET Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${JSON_PATH}"
      "$<TARGET_FILE_DIR:Game>/${JSON_NAME}"
    COMMENT "Copying ${JSON_NAME} to output directory"
  )
endforeach()

# EOS SDK DLLのコピー
add_custom_command(
  TARGET Game POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/../BroccoliEngine/Engine/ThirdParty/EOS/SDK/Bin/EOSSDK-Win64-Shipping.dll"
    "$<TARGET_FILE_DIR:Game>/EOSSDK-Win64-Shipping.dll"
  COMMENT "Copying EOS SDK DLL to output directory"
)
