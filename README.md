`impload` is a program to download image and video files from a digital camera, giving them systematic names and creating a year/date folder structure.

It currently supports JPEG, Canon raw CR2 and CR3, Olympus raw ORF, and MP4, because these are what the cameras I have present. There's no obvious reason why it should not be possible to support everything in the Exiv2 and MediaInfo libraries, but the logic for choosing formats is a bit limited right now and could do with some rework.

As the foregoing implies, you will need to have Exiv2 and MediaInfo development packages, as well as GPhoto2. There's a good chance you will need to compile the former two from source, as BMFF (Canon CR3) support is not yet the default in Exiv2, and MediaInfo defaults to Windows-style wide-character Unicode, which is not compatible with the UTF-8 that almost everything else in Linuxland uses.

The build system is qmake, and you are advised to install Qt6 and the latest version of QtCreator if you don't already have it. The C++ version specified is C++23, and although there are probably few if any features that depend on it, I make absolutely no promises that any earlier standard will not be missing something. I'm pretty much certain that anything older than C++17 won't work, but it's not something I care about.

Regardless of what the files might be called by the camera, and how they are structured into folders, the output format is fixed. Files are written to a user-specified location in the form `YYYY/YYYY-MM-DD/YYMMDD_HHMMSS.mmm_<tag>_NNNNNN.<ext>` where `mmm` is millisecond time if available, else 000, `<tag>` is a user-chosen tag to identify the particular camera and `NNNNNNNN` is an absolute sequence number that is incremented for every transferred file. If the same file is transferred more than once, the two instances will have different sequence numbers and the sequence will never go backwards. If I ever hit a million transfers, I will probably just let it wrap. The file extension, `<ext>` is just carried over from whatever the camera has called it. In the case of known raw files, the extensions (.CR2, .CR3, .ORF) appear fixed, but for JPEGs it's a touch more fluid. I don't make any attempt to unify .JPEG .JPG, .jpg, .jpeg, etc, I just copy what's there.

I haven't tried building on anything but Linux, and while I've not deliberately shunned Windows, I'd be mildly surprised if it works.

I'm sure there is more I should say here. Perhaps in time I will.
