# Create a real-time interactive application, that meets following requirements. The final score sets the exam grade. Maximum of two people per project. 

START = 100 points

## ESSENTIALS: Each missing (non-functional) Essential = -25 points (partial functionality => partial decrement)

- [ ]  realtime 2D raster processing = tracker, threads (can be separate project)
- [x]  3D GL Core profile + shaders version 4.6, GL debug 
- [x]  high performance => at least 60 FPS
- [x]  allow VSync control, fullscreen vs. windowed switching (restore window position & size)
- [x]  event processing (camera, object, app behaviour...): mouse (both axes, wheel), keyboard
- [x]  multiple different independently moving 3D models, at leats two loaded from file
- [x]  at least three different textures (or subtextures from texture atlas etc.)
- [ ]  lighting model, at least 3 lights (1x ambient, 1x directional, 1x reflector: at least one is moving; + possible other lights)
- [ ]  correct full scale transparency (at least one transparent object; NOT if(alpha<0.1) {discard;} )
- [ ]  audio (better than just background)

## EXTRAS: Each working Extra = +10 points

- [x]  height map textured by height, proper player height coords
- [ ]  particles
- [ ]  scripting (useful)
- [x]  correct collisions
- [ ]  some other nice complicated effect...

## INSTAFAIL

Using GLUT, GL compatible profile

NOTE: Hardware limitation might apply (eg. no mouse wheel on notebook), in that case subtask can be ignored.

|   Grade   |   Point range ||
|-----------|:-----:|:-----:|
| A = "1"   |   91  |   100 |
| B = "1-"  |   81  |   90  |
| C = "2"   |   71  |   80  |
| D = "2-"  |   61  |   70  |
| E = "3"   |   51  |   60  |
| F = "4"   |   0   |   50  |

Send full project and installation procedures in advance (use gitlab.tul.cz, github, gitlab, etc.)
