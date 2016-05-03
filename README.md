# Chromium (Slim branch)

Forked with intent to slim down distribution size by removing functionality

# Building

- git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git and add it to your path

Put the following in ‘.gclient’

```json
solutions = [
  {
    "name"        : "src",
    "url"         : "https://github.com/zackslash/chromium.src.git@origin/slim”,
    "deps_file"   : "DEPS",
    "managed"     : True,
    "custom_deps" : {
      "src/third_party/WebKit/LayoutTests": None,
      "src/chrome_frame/tools/test/reference_build/chrome": None,
      "src/chrome_frame/tools/test/reference_build/chrome_win": None,
      "src/chrome/tools/test/reference_build/chrome": None,
      "src/chrome/tools/test/reference_build/chrome_linux": None,
      "src/chrome/tools/test/reference_build/chrome_mac": None,
      "src/chrome/tools/test/reference_build/chrome_win": None,
    },
    "safesync_url": "",
  }
]
```

- Clone [V8](https://github.com/zackslash/v8) into /chromium/src/v8
- Clone [nw](https://github.com/zackslash/nw.js) into /chromium/src/content/nw
- Clone [node](https://github.com/zackslash/node) into /chromium/src/third_party/node

- gclient sync --with_branch_heads —verbose

- pip install -U pyobjc

- ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)" < /dev/null 2> /dev/null
- brew install ninja

- cd src
- ninja -C out/Debug nwjs
Or
- ninja -C out/Release nwjs

Forked from nw14
