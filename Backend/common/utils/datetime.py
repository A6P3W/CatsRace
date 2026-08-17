from datetime import datetime, timezone

def get_current_time_iso8601():
    now_utc = datetime.now(timezone.utc).isoformat(timespec='seconds').replace('+00:00', 'Z')
    return now_utc  


def relative_time(iso8601_utc: str) -> str:
    dt = datetime.fromisoformat(
        iso8601_utc.replace("Z", "+00:00")
    )

    now = datetime.now(timezone.utc)

    diff_seconds = (now - dt).total_seconds()

    if diff_seconds < 0:
        return "E"

    minutes = diff_seconds / 60

    # 60分未満
    if minutes < 60:
        return f"{int(minutes)}分前"

    hours = minutes / 60

    # 48時間未満
    if hours < 48:
        return f"{hours:.1f}時間前"

    days = hours / 24

    return f"{int(days)}日前"