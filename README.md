# max2sphere

maxsSphere Batch takes raw GoPro .360 frames (two strips of cube map projections encoded as EAC) and converts them to a more widely recognised equirectangular projection.

## READ BEFORE YOU BEGIN

* the resulting image frames from max2sphere will not have any metadata. max2sphere is implemented in our GoPro Frame Maker script. [GoPro Frame Maker adds metadata once fusion2sphere has finished processing (and offers a few other features) which could be better suited to your requirements](https://github.com/trek-view/gopro-frame-maker/).
* If you want to convert .360 EAC videos to a single equirectangular video (not frames), [you can follow the steps in this blog post to do so](https://www.trekview.org/blog/2022/using-ffmpeg-process-gopro-raw-360).
* If you're using a GoPro Fusion, [check out fusion2sphere](https://github.com/trek-view/fusion2sphere).

## Installation

The max2sphere command line utility should build out of the box on Linux using the simple Makefile provided. The only external dependency is the standard jpeg library (libjpeg), the lib and include directories need to be on the gcc build path. The same applies to MacOS except Xcode and command line tools need to be installed.

```
$ git clone https://github.com/trek-view/max2sphere
$ make -f Makefile
$ @SYSTEM_PATH/max2sphere
```

## Usage

### Preparation

This script is designed to be used with frames.

You will need to first convert a `.360` video to frames and then pass the two corresponding frames to the script ([2 frames because .360's use 2 video tracks](https://www.trekview.org/blog/2021/reverse-engineering-gopro-360-file-format-part-1/)).

You can use ffmpeg to split your `.360` video into frames (below at a rate of 1 FPS).

```
$ ffmpeg -i INPUT.360 -map 0:0 -r 1 -q:v 1 track0/img%d.jpg -map 0:5 -r 1 -q:v 1 track5/img%d.jpg
```

Note: this assumes video tracks are `0:0` and `0:5`. If timelapse mode is used, the tracks are different:

* Regular video = `-map 0:0` and `-map 0:5`
* Timewarp video = `-map 0:0` and `-map 0:4`

### Script

```shell
$ fusion2sphere [options] DIRECTORY_%d/img_%4d.jpg
```

Variables:

The `DIRECTORY_%d` should contain two `%d` entries, one for each video track (e.g. `DIRECTORY_1` and `DIRECTORY_2`).

The `img_%4` should contain sequentially numbered frames from each track with matching numbers (e.g. `DIRECTORY_1/img_1.jpg` and `DIRECTORY_1/img_2.jpg`).

Options:

* `-w` n sets the output image width, default: -1
* `-a` n sets antialiasing level, default = 2
* `-o` s specify the output filename, default is based on track0 name. If specified then it should contain one `%d` field for the frame number
* `-n` n Start index for the sequence, default: 0
* `-m` n End index for the sequence, default: 100000
* `-d` enable debug mode, default: off

## How lookup tables are handled

max2sphere will first look for a lookup table (these are stored in the root directory once the script has successfully run once).

If it finds one it will read it and use it during the current processing run.

If it doesn't find a lookup table (or if the read above fails) it will create one and save it to disk and then use it during the current processing run.

Lookup tables rely on four values: the template number, the output width and height, and the antialising value.

e.g. Template 0, width 5376, height 2688 and antialising of 2.

In the case above where the output image width was autodetermined the lookup table is called `0_5376_2688_2.data`

The template refers to the recording mode, the template defines the various geometric values the code needs in order to extract out the parts correctly.

Currently two recording modes are supported. At the top of the max2sphere.c file you will see the line referencing them:

```
FRAMESPECS template[NTEMPLATE] = {{4096,1344,1376,1344,32,5376},{2272,736,768,736,16,2944}};
```

The lookup table does take almost no time to read, compared to calculating the lookup table. However it does end up taking a decent about of disk space, almost 700MB in this case.

For us, this is acceptable as we only ever have 2 static lookup tables using default settings for 5.6k and 3k videos.

#### Examples (MacOS)

##### Use a GoPro Max 3K video (input width = 2272 // recommended output width = 3072)

**Single image**

```shell
$ /Users/dgreenwood/max2sphere/max2sphere -w 3072 -n 1 -m 1 -o testframes/3k/single/STITCHED/GS018423_%d.jpg testframes/3k/single/track%d/GS018423_%d.jpg
```

**Directory of images**

```shell
$ /Users/dgreenwood/max2sphere/max2sphere -w 3072 -n 1 -m 4 -o testframes/3k/directory/STITCHED/GS018423_%d.jpg testframes/3k/directory/track%d/GS018423_%d.jpg
```

##### Use a GoPro Max 5.6K video (input width = 4096 // recommended output width = 5376)

**Single image**

```shell
$ /Users/dgreenwood/max2sphere/max2sphere -w 5376 -n 1 -m 1 -o testframes/5_6k/single/STITCHED/GS018421_%d.jpg testframes/5_6k/single/track%d/GS018421_%d.jpg
```

**Directory of images**

```shell
$ /Users/dgreenwood/max2sphere/max2sphere -w 5376 -n 1 -m 4 -o testframes/5_6k/directory/STITCHED/GS018421_%d.jpg testframes/5_6k/directory/track%d/GS018421_%d.jpg
```

## Debugging

#### Failed to open warning

If you see the warning:

```
ReadFrame() - Failed to open "testframes/5_6k/track0/GS018421_11.jpg"
```

It is likely the value for `-m` used does not match the number of frames in the directories used.

This is not a critical error, and you should find all processed frames.

## Support

Join our Discord community and get in direct contact with the Trek View team, and the wider Trek View community.

[Join the Trek View Discord server](https://discord.gg/ZVk7h9hCfw).

## License

The code of this site is licensed under an [MIT License](/LICENSE).