# As of last revamp of this file, I didn't put much effort into figuring
# private vs public headers. The codebase seems to be based on all filenames
# being unique across all folders, so it doesn't really matter. The files
# themselves are mostly not separated into header folders, so it would take
# reversing to find public vs private API in each library. If you go to that
# effort, you should move the public header files out of the Source folders.
add_library(shockLG
	LG/Source/LOG/src/log.c
	LG/Source/memall.c
	LG/Source/stack.c
	LG/Source/tmpalloc.c
)
target_include_directories(shockLG PUBLIC H LG/Source LG/Source/LOG/src)

add_library(shockFIX
	FIX/Source/f_exp.c
	FIX/Source/fix.c
	FIX/Source/fix_pow.c
	FIX/Source/fix_sqrt.c
	FIX/Source/MakeTables.c
)
target_include_directories(shockFIX PUBLIC FIX/Source)
target_link_libraries(shockFIX shockLG m)

add_library(shock2D
	2D/Source/bit.c
	2D/Source/bitmap.c
	2D/Source/blend.c
	2D/Source/canvas.c
	2D/Source/close.c
	2D/Source/cnvtab.c
	2D/Source/context.c
	2D/Source/detect.c
	2D/Source/devtab.c
	2D/Source/fcntab.c
	2D/Source/init.c
	2D/Source/lintab.c
	2D/Source/mode.c
	2D/Source/pal.c
	2D/Source/permap.c
	2D/Source/persetup.c
	2D/Source/pertol.c
	2D/Source/rgb.c
	2D/Source/screen.c
	2D/Source/sscrn.c
	2D/Source/StateStk.c
	2D/Source/svgainit.c
	2D/Source/tempbm.c
	2D/Source/temptm.c
	2D/Source/tlucdat.c
	2D/Source/tluctab.c
	2D/Source/valloc.c
	2D/Source/vtab.c
	2D/Source/MacDev.c
	2D/Source/GR/grilin.c
	2D/Source/GR/grmalloc.c
	2D/Source/GR/grnull.c
	2D/Source/GR/gruilin.c
	2D/Source/GR/grd.c
	2D/Source/Gen/gendisk.c
	2D/Source/Gen/gentm.c
	2D/Source/Gen/genwclin.c
	2D/Source/Gen/genuclin.c
	2D/Source/Gen/genrsd8.c
	2D/Source/Gen/genuvlin.c
	2D/Source/Gen/genhlin.c
	2D/Source/Gen/genrsdbm.c
	2D/Source/Gen/genvlin.c
	2D/Source/Gen/genvcply.c
	2D/Source/Gen/genclin.c
	2D/Source/Gen/genfl8c.c
	2D/Source/Gen/genrsdtm.c
	2D/Source/Gen/genel.c
	2D/Source/Gen/genwlin.c
	2D/Source/Gen/genpix.c
	2D/Source/Gen/genuhlin.c
	2D/Source/Gen/genhfl8.c
	2D/Source/Gen/genov.c
	2D/Source/Gen/gente.c
	2D/Source/Gen/genlin.c
	2D/Source/Gen/gencwlin.c
	2D/Source/Gen/genchfl8.c
	2D/Source/Gen/gengfl8.c
	2D/Source/Gen/genbox.c
	2D/Source/Gen/genmono.c
	2D/Source/Gen/genulin.c
	2D/Source/Gen/gencnv.c
	2D/Source/Gen/genuslin.c
	2D/Source/Gen/genvpoly.c
	2D/Source/Gen/general.c
	2D/Source/Gen/gencirc.c
	2D/Source/Gen/genfl8.c
	2D/Source/Gen/genf24.c
	2D/Source/Gen/gentl8.c
	2D/Source/Gen/genrect.c
	2D/Source/Gen/genvrect.c
	2D/Source/Gen/genslin.c
	2D/Source/Flat8/fl8vlin.c
	2D/Source/Flat8/FL8OPL.c
	2D/Source/Flat8/fl8cply.c
	2D/Source/Flat8/fl8clin.c
	2D/Source/Flat8/fl8lp.c
	2D/Source/Flat8/fl8lf.c
	2D/Source/Flat8/fl8ll.c
	2D/Source/Flat8/fl8s.c
	2D/Source/Flat8/fl8hfl8.c
	2D/Source/Flat8/fl8pnt.c
	2D/Source/Flat8/fl8fl8m.c
	2D/Source/Flat8/fl8ltp.c
	2D/Source/Flat8/fl8fl8c.c
	2D/Source/Flat8/fl8w.c
	2D/Source/Flat8/fl8cop.c
	2D/Source/Flat8/fl8wlin.c
	2D/Source/Flat8/fl8lw.c
	2D/Source/Flat8/fl8pix.c
	2D/Source/Flat8/fl8row.c
	2D/Source/Flat8/fl8ft.c
	2D/Source/Flat8/fl8rsd8.c
	2D/Source/Flat8/Fl8F.c
	2D/Source/Flat8/fl8bldbl.c
	2D/Source/Flat8/fl8clear.c
	2D/Source/Flat8/fl8p.c
	2D/Source/Flat8/fl8wclin.c
	2D/Source/Flat8/fl8hlin.c
	2D/Source/Flat8/fl8bl.c
	2D/Source/Flat8/fl8gpix.c
	2D/Source/Flat8/fl8ns.c
	2D/Source/Flat8/fl8lnop.c
	2D/Source/Flat8/fl8dbl.c
	2D/Source/Flat8/fl8clin.h
	2D/Source/Flat8/fl8cnv.c
	2D/Source/Flat8/fl8slin.c
	2D/Source/Flat8/fl8lin.c
	2D/Source/Flat8/fl8tsmap.c
	2D/Source/Flat8/fl8ply.c
	2D/Source/Flat8/fl8sply.c
	2D/Source/Flat8/fl8rect.c
	2D/Source/Flat8/fl8gfl8.c
	2D/Source/Flat8/fl8tl8.c
	2D/Source/Flat8/fl8p24.c
	2D/Source/Flat8/fl8g24.c
	2D/Source/Flat8/fl8ctp.c
	2D/Source/Flat8/fl8mscl.c
	2D/Source/Flat8/fl8lop.c
	2D/Source/Flat8/fl8ntrp2.c
	2D/Source/Flat8/fl8sub.c
	2D/Source/Flat8/fl8tpl.c
	2D/Source/Flat8/fl8wclin.h
	2D/Source/Flat8/fl8nl.c
	2D/Source/Flat8/fl8fltr2.c
	2D/Source/Flat8/fl8mono.c
	2D/Source/Flat8/fl8fl8.c
	2D/Source/Flat8/fl8chfl8.c
	2D/Source/Clip/clpply.c
	2D/Source/Clip/clplin.c
	2D/Source/Clip/clpclin.c
	2D/Source/Clip/clpltab.c
	2D/Source/Clip/clpf24.c
	2D/Source/Clip/clplin2.c
	2D/Source/Clip/clppoly.c
	2D/Source/Clip/clpslin.c
	2D/Source/Clip/clprect.c
	2D/Source/Clip/clpmono.c
	2D/Source/RSD/rsdcvt.c
	2D/Source/RSD/RSDUnpack.c
	2D/Source/string/chrsiz.c
	2D/Source/string/genchr.c
	2D/Source/string/genstr.c
	2D/Source/string/genuchr.c
	2D/Source/string/genustr.c
	2D/Source/string/strscl.c
	2D/Source/string/strsiz.c
	2D/Source/string/struscl.c
	2D/Source/string/strwrap.c
)
target_include_directories(shock2D PUBLIC
    2D/Source
    2D/Source/Clip
    2D/Source/Flat8
    2D/Source/Gen
    2D/Source/GR
    2D/Source/RSD
)
target_link_libraries(shock2D shockLG shockFIX)

add_library(shock3D
	3D/Source/alloc.c
	3D/Source/Bitmap.c
	3D/Source/clip.c
	3D/Source/detail.c
	3D/Source/fov.c
	3D/Source/GlobalV.c
	3D/Source/instance.c
	3D/Source/interp.c
	3D/Source/light.c
	3D/Source/matrix.c
	3D/Source/points.c
	3D/Source/polygon.c
	3D/Source/slew.c
	3D/Source/tmap.c
	3D/Source/vector.c
)
target_include_directories(shock3D PUBLIC 3D/Source)
# a bunch of 3D files have an unconditional `#include "OpenGL.h", and that
# lives in ../MacSrc, gross
target_include_directories(shock3D PRIVATE ../MacSrc)
# FURTHERMORE, that MacSrc code uses SDL, so need to manually propagate that dependency.
target_link_libraries(shock3D dep_SDL2)
target_link_libraries(shock3D shock2D m)

add_library(shockRES
	RES/Source/caseless.c
	RES/Source/lzw.c
	RES/Source/refacc.c
	RES/Source/resacc.c
	RES/Source/resbuild.c
	RES/Source/res.c
	RES/Source/resfile.c
	RES/Source/resformat.c
	RES/Source/resload.c
	RES/Source/resmake.c
	RES/Source/restypes.c
)
target_include_directories(shockRES PUBLIC RES/Source)
target_link_libraries(shockRES shockLG shock2D dep_SDL2)

add_library(shockDSTRUCT
	DSTRUCT/Source/array.c
	DSTRUCT/Source/hash.c
	DSTRUCT/Source/pqueue.c
	DSTRUCT/Source/rect.c
)
target_include_directories(shockDSTRUCT PUBLIC DSTRUCT/Source)
target_link_libraries(shockDSTRUCT shockLG)

add_library(shockINPUT
	INPUT/Source/kbcook.c
	INPUT/Source/mouse.c
	INPUT/Source/sdl_events.c
)
target_include_directories(shockINPUT PUBLIC INPUT/Source)
# hella circular dependencies
target_include_directories(shockINPUT PRIVATE ../MacSrc ../GameSrc/Headers)
target_link_libraries(shockINPUT shockLG shock3D dep_SDL2)

add_library(shockUI
	UI/Source/curdrw.c
	UI/Source/cursors.c
	UI/Source/event.c
	UI/Source/hotkey.c
	UI/Source/region.c
	UI/Source/slab.c
	UI/Source/vmouse.c
)
target_include_directories(shockUI PUBLIC UI/Source)
target_link_libraries(shockUI PUBLIC shockLG shock2D shockDSTRUCT shockINPUT dep_SDL2)

add_library(shockSND INTERFACE)
# nothing but a header file
target_include_directories(shockSND INTERFACE SND/Source)

add_library(shockFIXPP
	FIXPP/Source/fixpp.cpp
)
target_include_directories(shockFIXPP PUBLIC FIXPP/Source)
target_link_libraries(shockFIXPP shockFIX)

add_library(shockEDMS
	EDMS/Source/interfac.cc
	EDMS/Source/collide.cc
	EDMS/Source/intrsect.cc
	EDMS/Source/globals.cc
	EDMS/Source/phy_tool.cc
	EDMS/Source/soliton.cc
	EDMS/Source/MODELS/robot.cc
	EDMS/Source/MODELS/pelface.cc
	EDMS/Source/MODELS/ftl.cc
	EDMS/Source/MODELS/pelvis.cc
	EDMS/Source/MODELS/d_frame.cc
	EDMS/Source/MODELS/d_f_face.cc
	EDMS/Source/MODELS/ftlface.cc
	EDMS/Source/MODELS/d_f_2.cc
)
target_include_directories(shockEDMS PUBLIC EDMS/Source EDMS/Source/MODELS)
target_link_libraries(shockEDMS shockFIXPP)

add_library(shockRND
	RND/Source/rnd.c
)
target_include_directories(shockRND PUBLIC RND/Source)
target_link_libraries(shockRND shockLG shockFIX)

add_library(shockPALETTE
	PALETTE/Source/palette.c
)
target_include_directories(shockPALETTE PUBLIC PALETTE/Source)
target_link_libraries(shockPALETTE shockLG shock2D shockFIX)

add_library(shockVOX
	VOX/Source/vox2d.c
	VOX/Source/vox3d.c
	VOX/Source/voxinit.c
)
target_include_directories(shockVOX PUBLIC VOX/Source)
target_link_libraries(shockVOX shockLG shock2D shock3D)

add_library(shockAFILE
	AFILE/Source/afile.c
	AFILE/Source/amov.c
	AFILE/Source/compose.c
	AFILE/Source/draw4x4.cpp
	AFILE/Source/huffde.cpp
	AFILE/Source/movie.c)
target_link_libraries(shockAFILE shock2D shockRES shockDSTRUCT)
target_include_directories(shockAFILE PUBLIC AFILE/Source)

add_library(shockADLMIDI
	adlmidi/adlmidi_sequencer.cpp
	adlmidi/adlmidi_bankmap.tcc
	adlmidi/cvt_mus2mid.hpp
	adlmidi/adlmidi_bankmap.h
	adlmidi/midi_sequencer.hpp
	adlmidi/chips/dosbox_opl3.h
	adlmidi/chips/opl_chip_base.h
	adlmidi/chips/nuked_opl3.h
	adlmidi/chips/dosbox/dbopl.h
	adlmidi/chips/dosbox/dbopl.cpp
	adlmidi/chips/dosbox_opl3.cpp
	adlmidi/chips/nuked_opl3_v174.cpp
	adlmidi/chips/nuked/nukedopl3_174.c
	adlmidi/chips/nuked/nukedopl3.c
	adlmidi/chips/nuked/nukedopl3_174.h
	adlmidi/chips/nuked/nukedopl3.h
	adlmidi/chips/opl_chip_base.tcc
	adlmidi/chips/nuked_opl3.cpp
	adlmidi/chips/nuked_opl3_v174.h
	adlmidi/wopl/wopl_file.c
	adlmidi/wopl/wopl_file.h
	adlmidi/include/adlmidi.hpp
	adlmidi/include/adlmidi.h
	adlmidi/adlmidi_private.hpp
	adlmidi/file_reader.hpp
	adlmidi/adlmidi_ptr.hpp
	adlmidi/adlmidi_opl3.cpp
	adlmidi/midi_sequencer_impl.hpp
	adlmidi/adldata.hh
	adlmidi/adlmidi_private.cpp
	adlmidi/adlmidi_midiplay.cpp
	adlmidi/adlmidi.cpp
	adlmidi/cvt_xmi2mid.hpp
	adlmidi/adlmidi_load.cpp
	adlmidi/adldata.cpp
	adlmidi/fraction.hpp
	adlmidi/midi_sequencer.h
)
target_include_directories(shockADLMIDI PUBLIC adlmidi/include)
