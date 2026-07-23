"""Keep esptool uploads ASCII-only on Windows PlatformIO consoles."""

Import("env")  # type: ignore[name-defined]  # PlatformIO / SCons built-in


def _disable_progress_output() -> None:
    flags = list(env.get("UPLOADERFLAGS", []))  # type: ignore[name-defined]
    if "write-flash" not in flags or "--no-progress" in flags:
        return
    command_index = flags.index("write-flash")
    flags.insert(command_index + 1, "--no-progress")
    env.Replace(UPLOADERFLAGS=flags)  # type: ignore[name-defined]


_disable_progress_output()
