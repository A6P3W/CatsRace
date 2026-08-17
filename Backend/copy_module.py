"""
モジュールコピーツール

使い方:
    python copy_module.py

実行すると対象フォルダのパスを入力するよう求められます。
module.yaml に記載されたフォルダ（commonなど）を対象フォルダ内にコピーします。
"""

import shutil
import sys
from pathlib import Path


def load_yaml_paths(yaml_path: Path) -> list:
    """module.yaml からフォルダパスのリストを読み込む。"""
    try:
        import yaml

        with open(yaml_path, "r", encoding="utf-8") as f:
            content = yaml.safe_load(f)
        if isinstance(content, str):
            return [content]
        elif isinstance(content, list):
            return [str(item) for item in content if item]
        return []
    except ImportError:
        # PyYAML が未インストールの場合はシンプルな行読み込みで代替
        paths = []
        with open(yaml_path, "r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if line and not line.startswith("#"):
                    paths.append(line)
        return paths


def copy_dir(src: Path, dst: Path) -> None:
    """src フォルダを dst へコピーする（dst が既存でも上書きマージ）。"""
    shutil.copytree(src, dst, dirs_exist_ok=True)


def merge_requirements(sources: list[Path], dest: Path) -> None:
    """複数の requirements.txt を重複排除して統合し dest に書き出す。"""
    lines: list[str] = []
    seen: set[str] = set()
    for req in sources:
        if req.exists():
            for line in req.read_text(encoding="utf-8").splitlines():
                normalized = line.strip()
                if normalized and normalized not in seen:
                    seen.add(normalized)
                    lines.append(normalized)
            print(f"[統合] {req}")
    if lines:
        dest.write_text("\n".join(lines) + "\n", encoding="utf-8")
        print(f"[書き出し] {dest}")


def main() -> None:
    # 引数があればそれを使用し、なければ input() で求める
    if len(sys.argv) > 1:
        raw = sys.argv[1]
    else:
        raw = input("対象フォルダのパスを入力してください: ").strip()

    target: Path = Path(raw).resolve()

    # module.yaml は対象フォルダ内、パスの基準はプロジェクトルート（スクリプトの場所）
    module_yaml: Path = target / "module.yaml"
    base_dir: Path = Path(__file__).parent

    # --- 入力検証 ---
    if not target.exists():
        print(f"エラー: 対象フォルダが存在しません: {target}", file=sys.stderr)
        sys.exit(1)
    if not target.is_dir():
        print(f"エラー: 対象はフォルダである必要があります: {target}", file=sys.stderr)
        sys.exit(1)

    # --- module.yaml に記載されたフォルダを対象フォルダ内にコピー ---
    extra_dirs: list[Path] = []
    if module_yaml.exists():
        extra_paths = load_yaml_paths(module_yaml)
        print(
            f"module.yaml から {len(extra_paths)} 件のパスを読み込みました: {module_yaml}"
        )

        # 対象フォルダ内の既存commonフォルダを削除
        common_dst = target / "src" / "common"
        if common_dst.exists():
            shutil.rmtree(common_dst)
            print(f"[削除] {common_dst}")

        # コピー実行
        for rel_path in extra_paths:
            extra_src = (base_dir / rel_path).resolve()
            if extra_src.is_dir():
                extra_dirs.append(extra_src)
                extra_dst = target / "src" / rel_path
                copy_dir(extra_src, extra_dst)
                print(f"[コピー] {extra_src}\n     -> {extra_dst}")
            else:
                print(f"[警告] パスが見つかりません (スキップ): {extra_src}")
                raise FileNotFoundError(
                    f"module.yaml に記載されたパスが見つかりません: {extra_src}"
                )
    else:
        print(f"[警告] module.yaml が見つかりません: {module_yaml}")
        raise FileNotFoundError(f"module.yaml が見つかりません: {module_yaml}")

    # --- requirements.txt の統合 ---
    req_sources = [target / "src" / "main.requirements.txt"] + [
        d / "requirements.txt" for d in extra_dirs
    ]
    if any(r.exists() for r in req_sources):
        print("\n[requirements.txt 統合]")
        merge_requirements(req_sources, target / "src" / "requirements.txt")

    print("\n処理が完了しました。")


if __name__ == "__main__":
    main()
