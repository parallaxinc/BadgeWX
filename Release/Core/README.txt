These are the core binary images for the BadgeWX's ESP8266 module firmware and well as the initial User Filesystem.


IMPORTANT:
  To optimize repository size, the binary files in this folder are managed by Git LFS.  You MUST have Git LFS installed and initialized before checking out this commit in order
  to retrieve these assets.  https://git-lfs.github.com

  If you don't need the binary assets, you don't need to install Git LFS, just don't change the binary contents of this folder and all should be well for further commits.


FILES:
  httpd.ota   - Over-The-Air firmware image.  This can be used to update the firmware wirelessly via the Badge's configuration web pages (does not affect user filesystem).
  <most> .bin - Individual binaries that make up the whole firmware.  Each is written to specific locations within the ESP.
  fsimage.bin - Initial User Filesystem image.  This may include Parallax general Wi-Fi Module examples (.html and .png files) as well as the autorunxx.bin files specific to the BadgeWX.
  .out & .a   - (ignore these) compiler temporaries.
