# Remove all usbmodem network interfaces on macOS

Run:
```
node clean
sudo cp preferences.plist /Library/Preferences/SystemConfiguration/preferences.plist
```

In case of problems, recover by running:
```
sudo cp old-preferences.plist /Library/Preferences/SystemConfiguration/preferences.plist
```
