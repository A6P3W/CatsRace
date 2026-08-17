"""Run one or more Cloud Functions cells locally behind Caddy."""

from __future__ import annotations

import argparse
import os
import re
import shutil
import signal
import socket
import subprocess
import sys
import threading
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Sequence

PROJECT_ROOT = Path(__file__).resolve().parent
CELLS_ROOT = PROJECT_ROOT / "cells"
COPY_MODULE_SCRIPT = PROJECT_ROOT / "copy_module.py"
LOCAL_ROOT = PROJECT_ROOT / ".local"
CADDYFILE = LOCAL_ROOT / "Caddyfile"
ENTRY_POINT = "cell_entry_point"
SOURCE_FILE = "main.py"
FUNCTION_HOST = "127.0.0.1"
PROXY_HOST = "0.0.0.0"
CELL_NAME_PATTERN = re.compile(r"^[A-Za-z0-9_-]+$")
STARTUP_TIMEOUT_SECONDS = 15.0
SHUTDOWN_TIMEOUT_SECONDS = 5.0


class LocalRunError(RuntimeError):
    """An expected local-run validation or startup error."""


@dataclass(frozen=True)
class CellRoute:
    name: str
    directory: Path
    port: int


@dataclass
class ManagedProcess:
    name: str
    process: subprocess.Popen[str]
    log_thread: threading.Thread


def parse_args(argv: Sequence[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Run Cloud Functions cells locally behind a Caddy proxy."
    )
    selection = parser.add_mutually_exclusive_group(required=True)
    selection.add_argument("--cells", nargs="+", metavar="CELL")
    selection.add_argument("--all", action="store_true", dest="all_cells")
    parser.add_argument("--proxy-port", type=int, default=8080, metavar="PORT")
    return parser.parse_args(argv)


def validate_cell_name(cell_name: str) -> None:
    if not CELL_NAME_PATTERN.fullmatch(cell_name):
        raise LocalRunError(
            f"Invalid cell name: {cell_name!r}. "
            "Use only letters, numbers, hyphens, and underscores."
        )


def resolve_cells(
    requested_cells: Sequence[str] | None, all_cells: bool
) -> list[str]:
    if all_cells:
        if not CELLS_ROOT.is_dir():
            raise LocalRunError(f"Cells directory not found: {CELLS_ROOT}")
        cell_names = sorted(path.name for path in CELLS_ROOT.iterdir() if path.is_dir())
        if not cell_names:
            raise LocalRunError(f"No cells found under: {CELLS_ROOT}")
    else:
        cell_names = list(requested_cells or [])
        if not cell_names:
            raise LocalRunError(
                'CELLS is required. Example: make local CELLS="sample-1 sample-2"'
            )

    seen: set[str] = set()
    duplicates: set[str] = set()
    for name in cell_names:
        if name in seen:
            duplicates.add(name)
        seen.add(name)
    if duplicates:
        raise LocalRunError(f"Duplicate cell name: {', '.join(sorted(duplicates))}")
    for cell_name in cell_names:
        validate_cell_name(cell_name)
    return cell_names


def validate_cell(cell_name: str) -> Path:
    cell_dir = CELLS_ROOT / cell_name
    required_paths = (
        (cell_dir, "Cell directory", True),
        (cell_dir / "module.yaml", "module.yaml", False),
        (cell_dir / "src", "src directory", True),
        (cell_dir / "src" / SOURCE_FILE, SOURCE_FILE, False),
        (cell_dir / "src" / "main.requirements.txt", "main.requirements.txt", False),
    )
    for path, label, must_be_directory in required_paths:
        exists = path.is_dir() if must_be_directory else path.is_file()
        if not exists:
            raise LocalRunError(f"{label} not found: {path}")
    return cell_dir


def assign_ports(
    cell_names: Sequence[str], proxy_port: int
) -> list[CellRoute]:
    highest_port = proxy_port + len(cell_names)
    if proxy_port < 1 or highest_port > 65535:
        raise LocalRunError(
            "Ports must be between 1 and 65535, including all cell ports "
            f"(requested range: {proxy_port}-{highest_port})."
        )
    return [
        CellRoute(name, CELLS_ROOT / name, proxy_port + index)
        for index, name in enumerate(cell_names, start=1)
    ]


def check_port_available(host: str, port: int) -> None:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        try:
            sock.bind((host, port))
        except OSError as exc:
            raise LocalRunError(f"Port {port} is already in use.") from exc


def check_ports(proxy_port: int, routes: Sequence[CellRoute]) -> None:
    check_port_available(PROXY_HOST, proxy_port)
    for route in routes:
        check_port_available(FUNCTION_HOST, route.port)


def run_checked(command: Sequence[str], description: str) -> None:
    print(f"[local] {description}", flush=True)
    try:
        subprocess.run(command, cwd=PROJECT_ROOT, check=True)
    except subprocess.CalledProcessError as exc:
        raise LocalRunError(f"Failed to {description.lower()}.") from exc


def prepare_cell(route: CellRoute) -> None:
    run_checked(
        [sys.executable, str(COPY_MODULE_SCRIPT), str(route.directory)],
        f"Preparing cell: {route.name}",
    )


def install_requirements(route: CellRoute) -> None:
    requirements = route.directory / "src" / "requirements.txt"
    if not requirements.is_file():
        raise LocalRunError(f"requirements.txt was not generated: {requirements}")
    run_checked(
        [
            sys.executable,
            "-m",
            "pip",
            "install",
            "--disable-pip-version-check",
            "-r",
            str(requirements),
        ],
        f"Installing requirements for: {route.name}",
    )


def generate_caddyfile(routes: Sequence[CellRoute], proxy_port: int) -> Path:
    lines = [f":{proxy_port} {{"]
    for route in routes:
        lines.extend(
            [
                f"\thandle /{route.name} {{",
                "\t\trewrite * /",
                f"\t\treverse_proxy {FUNCTION_HOST}:{route.port}",
                "\t}",
                "",
                f"\thandle_path /{route.name}/* {{",
                f"\t\treverse_proxy {FUNCTION_HOST}:{route.port}",
                "\t}",
                "",
            ]
        )
    lines.pop()
    lines.append("}")
    LOCAL_ROOT.mkdir(parents=True, exist_ok=True)
    CADDYFILE.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return CADDYFILE


def _stream_logs(prefix: str, stream: object) -> None:
    if stream is None:
        return
    for line in stream:  # type: ignore[union-attr]
        print(f"[{prefix}] {line.rstrip()}", flush=True)


def start_process(
    name: str, command: Sequence[str], working_directory: Path
) -> ManagedProcess:
    popen_options: dict[str, object] = {}
    if os.name == "nt":
        popen_options["creationflags"] = subprocess.CREATE_NEW_PROCESS_GROUP
    else:
        popen_options["start_new_session"] = True
    process = subprocess.Popen(
        command,
        cwd=working_directory,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        encoding="utf-8",
        errors="replace",
        bufsize=1,
        **popen_options,
    )
    thread = threading.Thread(
        target=_stream_logs,
        args=(name, process.stdout),
        daemon=True,
        name=f"{name}-log",
    )
    thread.start()
    return ManagedProcess(name, process, thread)


def start_function(route: CellRoute) -> ManagedProcess:
    command = [
        sys.executable,
        "-u",
        "-m",
        "functions_framework",
        f"--target={ENTRY_POINT}",
        f"--source={SOURCE_FILE}",
        f"--host={FUNCTION_HOST}",
        f"--port={route.port}",
        "--debug",
    ]
    return start_process(route.name, command, route.directory / "src")


def start_caddy(caddyfile: Path) -> ManagedProcess:
    caddy = shutil.which("caddy")
    if not caddy:
        raise LocalRunError(
            "Caddy executable was not found. Rebuild the Dev Container."
        )
    command = [caddy, "run", "--config", str(caddyfile), "--adapter", "caddyfile"]
    return start_process("proxy", command, PROJECT_ROOT)


def wait_for_port(
    host: str,
    port: int,
    process: ManagedProcess,
    timeout: float = STARTUP_TIMEOUT_SECONDS,
) -> None:
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        return_code = process.process.poll()
        if return_code is not None:
            raise LocalRunError(
                f"Process failed to start: {process.name} (exit code {return_code})."
            )
        try:
            with socket.create_connection((host, port), timeout=0.2):
                return
        except OSError:
            time.sleep(0.1)
    raise LocalRunError(f"Timed out waiting for {process.name} on port {port}.")


def terminate_process(managed: ManagedProcess) -> None:
    if managed.process.poll() is not None:
        return
    try:
        if os.name == "nt":
            managed.process.terminate()
        else:
            os.killpg(managed.process.pid, signal.SIGTERM)
    except ProcessLookupError:
        pass


def kill_process(managed: ManagedProcess) -> None:
    if managed.process.poll() is not None:
        return
    try:
        if os.name == "nt":
            managed.process.kill()
        else:
            os.killpg(managed.process.pid, signal.SIGKILL)
    except ProcessLookupError:
        pass


def shutdown(processes: Sequence[ManagedProcess]) -> None:
    if not processes:
        return
    print("[local] Stopping all processes...", flush=True)
    for managed in reversed(processes):
        terminate_process(managed)

    deadline = time.monotonic() + SHUTDOWN_TIMEOUT_SECONDS
    for managed in reversed(processes):
        remaining = max(0.0, deadline - time.monotonic())
        try:
            managed.process.wait(timeout=remaining)
        except subprocess.TimeoutExpired:
            kill_process(managed)

    for managed in processes:
        try:
            managed.process.wait(timeout=1.0)
        except subprocess.TimeoutExpired:
            pass
        managed.log_thread.join(timeout=1.0)
    print("[local] All processes stopped.", flush=True)


def monitor(processes: Sequence[ManagedProcess]) -> None:
    while True:
        for managed in processes:
            return_code = managed.process.poll()
            if return_code is not None:
                raise LocalRunError(
                    f"Process exited unexpectedly: {managed.name} "
                    f"(exit code {return_code})."
                )
        time.sleep(0.25)


def run(args: argparse.Namespace) -> None:
    cell_names = resolve_cells(args.cells, args.all_cells)
    for cell_name in cell_names:
        validate_cell(cell_name)
    routes = assign_ports(cell_names, args.proxy_port)
    check_ports(args.proxy_port, routes)
    if not shutil.which("caddy"):
        raise LocalRunError(
            "Caddy executable was not found. Rebuild the Dev Container."
        )

    for route in routes:
        prepare_cell(route)
        install_requirements(route)
    caddyfile = generate_caddyfile(routes, args.proxy_port)

    processes: list[ManagedProcess] = []
    try:
        for route in routes:
            processes.append(start_function(route))
        for route, process in zip(routes, processes, strict=True):
            wait_for_port(FUNCTION_HOST, route.port, process)

        proxy = start_caddy(caddyfile)
        processes.append(proxy)
        wait_for_port(FUNCTION_HOST, args.proxy_port, proxy)

        print(f"[local] Proxy: http://localhost:{args.proxy_port}", flush=True)
        for route in routes:
            print(
                f"[local] {route.name}: "
                f"http://localhost:{args.proxy_port}/{route.name} "
                f"-> {FUNCTION_HOST}:{route.port}",
                flush=True,
            )
        print("[local] Press Ctrl+C to stop.", flush=True)
        monitor(processes)
    finally:
        shutdown(processes)


def main(argv: Sequence[str] | None = None) -> int:
    try:
        run(parse_args(argv))
    except KeyboardInterrupt:
        print("\n[local] Interrupted.", flush=True)
        return 0
    except LocalRunError as exc:
        print(f"[local] Error: {exc}", file=sys.stderr, flush=True)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
