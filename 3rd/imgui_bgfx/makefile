#
# Copyright 2011-2023 Branimir Karadzic. All rights reserved.
# License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
#

include ../../../scripts/shader-embeded.mk

roboto_regular.ttf.h: ../../runtime/font/roboto-regular.ttf
	@bin2c -f $(<) -o $(@) -n s_robotoRegularTtf

robotomono_regular.ttf.h: ../../runtime/font/robotomono-regular.ttf
	@bin2c -f $(<) -o $(@) -n s_robotoMonoRegularTtf

icons_font_awesome.ttf.h: ../../runtime/font/fontawesome-webfont.ttf
	@bin2c -f $(<) -o $(@) -n s_iconsFontAwesomeTtf

icons_kenney.ttf.h: ../../runtime/font/kenney-icon-font.ttf
	@bin2c -f $(<) -o $(@) -n s_iconsKenneyTtf

fonts: droidsans.ttf.h \
	roboto_regular.ttf.h \
	robotomono_regular.ttf.h \
	icons_font_awesome.ttf.h \
	icons_kenney.ttf.h

rebuild: fonts
	@make -s --no-print-directory clean all
