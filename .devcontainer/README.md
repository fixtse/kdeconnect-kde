# Notification thumbnail development

The Arch container matches the deployment host's distribution. Build and test on
`devserv`, in an isolated checkout (see `/home/fixt/AGENTS.md`). No credentials
are required. Pairing certificates and user configuration stay on the desktop.

```sh
docker build -t kdeconnect-thumbnail -f .devcontainer/Dockerfile .
docker run --rm -v "$PWD:/work" -w /work kdeconnect-thumbnail sh -c '
  cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON \
    -DWITH_PULSEAUDIO=OFF -DBLUETOOTH_ENABLED=OFF -DWITH_X11=OFF &&
  cmake --build build --target kdeconnect_sendnotifications notificationimagetest -j4 &&
  ctest --test-dir build -R notificationimagetest --output-on-failure
'
```

Only deploy `build/bin/kdeconnect/kdeconnect_sendnotifications.so` (locate with `find build`
if the generator layout changes), not the newly built core library. Install it
under `~/.local/lib/qt6/plugins/kdeconnect/` and add that parent plugin directory
to `QT_PLUGIN_PATH` for the KDE Connect user service. Restart that service and
verify the loaded plugin in `/proc/<pid>/maps` and an HA notification trace.
Also override `~/.local/share/dbus-1/services/org.kde.kdeconnect.service` to run
`/usr/bin/env QT_PLUGIN_PATH=<home>/.local/lib/qt6/plugins /usr/bin/kdeconnectd`;
D-Bus activation can race the autostart service during restarts.
Rollback: remove both service overrides and the user plugin, then restart KDE Connect.
System package upgrades do not update this user override; remove it when the
packaged version contains the fix. No Home Assistant restart is needed.
