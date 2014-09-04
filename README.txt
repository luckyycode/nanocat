Sky cat engine, still in development. VERY raw version.
It's a 3d/2d engine, cross-platform. No 3rd libs/tools used. 

Platforms ( todo ):
- Mac OS X ( 10.9.2 Mavericks ) - works, native
- Linux Mint 16 - native, everything works, but 3d isn't. ( todo )
- Windows - works, native

To render terrain/water/sky/models/2d stuff .. EVERYTHING - you can toggle in renderer.c in r_render function.

Some stuff I have done/still todo:
- sky - done
- font system ( loads from 2d texture ) - done
- networking - server-client - 65% done 
- secure packet sending - TODO
- console - done
- console variables - done
- native ogl context creation. ( no glut used, i don't want to ). Mac OS X uses Cocoa, Linux - X11, Windows - native. 
- environenment ( fog, water, msampling ) - a lot to do ( rain, etc )
- asset loader - done
- shader loader - done
- map loader  - done
- MODEL ( obj for now ) - DONE 
- custom model format - still making
- image loader ( bmp, tga ) - done
- input ( mouse, keyboard ) - done
- terrain generation from a file ( also multitexturing )  - done
- also - no, it isn't idTech engine, I just like how functions are named :D

To launch the map, simply click '~' and type 'map demo'

More description soon.


I am doing this on my MacBook Air 2012 ( OS X 10.9.2 (13C48), 1.7 ghz, 4 gb of ram ) - takes ~4-7% of CPUand ~110MB of ram when map created.
Created by NekoCode ( nekocode@icloud.com )

If you want to use some code, please mail me first.

Thanks for reading.

Some screenshots available:
http://imageshack.com/a/img855/4489/4ziz.png - multitexturing
http://imageshack.com/a/img593/1886/kiv7.jpg - underwater
http://s018.radikal.ru/i509/1403/6c/63f4a3a8f616.png - models
