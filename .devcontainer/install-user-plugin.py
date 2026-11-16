"""Install a user-only KDE Connect notification plugin, preserving package files."""
from pathlib import Path
import shutil
import subprocess
import sys

home = Path.home()
plugin = home / '.local/lib/qt6/plugins/kdeconnect/kdeconnect_sendnotifications.so'
override = home / '.config/systemd/user/app-org.kde.kdeconnect.daemon@autostart.service.d/thumbnail.conf'
dbus_service = home / '.local/share/dbus-1/services/org.kde.kdeconnect.service'
if plugin.exists() or override.exists() or dbus_service.exists():
    raise SystemExit('An override already exists; refusing to overwrite it')
plugin.parent.mkdir(parents=True, exist_ok=True)
shutil.copy2(sys.argv[1], plugin)
result = subprocess.run(['ldd', str(plugin)], capture_output=True, text=True, check=True)
if 'not found' in result.stdout:
    plugin.unlink()
    raise SystemExit(result.stdout)
override.parent.mkdir(parents=True, exist_ok=True)
override.write_text('[Service]\nEnvironment=QT_PLUGIN_PATH=' + str(home / '.local/lib/qt6/plugins') + '\n')
dbus_service.parent.mkdir(parents=True, exist_ok=True)
dbus_service.write_text('[D-BUS Service]\nName=org.kde.kdeconnect\nExec=/usr/bin/env QT_PLUGIN_PATH=' + str(home / '.local/lib/qt6/plugins') + ' /usr/bin/kdeconnectd\n')
activation.parent.mkdir(parents=True, exist_ok=True)
activation.write_text('[D-BUS Service]\nName=org.kde.kdeconnect\nExec=/usr/bin/env QT_PLUGIN_PATH=' + str(home / '.local/lib/qt6/plugins') + ' /usr/bin/kdeconnectd\n')
try:
    subprocess.run(['systemctl', '--user', 'daemon-reload'], check=True)
    subprocess.run(['systemctl', '--user', 'restart', 'app-org.kde.kdeconnect.daemon@autostart.service'], check=True)
except Exception:
    dbus_service.unlink()
    activation.unlink()
    override.unlink()
    plugin.unlink()
    subprocess.run(['systemctl', '--user', 'daemon-reload'])
    subprocess.run(['systemctl', '--user', 'restart', 'app-org.kde.kdeconnect.daemon@autostart.service'])
    raise
print('Installed user notification plugin and restarted KDE Connect')
