##########################################################################
# If not stated otherwise in this file or this component's Licenses.txt
# file the following copyright and licenses apply:
#
# Copyright 2018 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################
AUTOMAKE_OPTIONS = subdir-objects
# ACLOCAL_AMFLAGS = -I cfg

SUBDIRS =

AM_CXXFLAGS = -g \
	-Wall \
	-std=c++0x \
	-DGCC4_XXX \
	$(CURL_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GST_CFLAGS) \
	$(JANSON_CFLAGS) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/common \
	-I$(top_srcdir)/rmf \
	-I$(top_srcdir)/aamp \
	-I$(top_srcdir)/closedcaption/ccManager/utils \
	-I$(top_srcdir)/closedcaption/ccManager \
	-I$(top_srcdir)/closedcaption/include \
	-I$(top_srcdir)/closedcaption/ccGraphics \
	-I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/rdk/ds \
	-I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/rdk/ds-hal \
	-I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/rdk/iarmbus \
	-I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/rdk/iarmmgrs \
	-I=$(includedir)/directfb

AM_CXXFLAGS += -DRT_PLATFORM_LINUX

bin_PROGRAMS = rdkmediaplayer

rdkmediaplayer_SOURCES = \
	main.cpp \
	rdkmediaplayer.cpp \
	rdkmediaplayer.h \
	common/rdkmediaplayerimpl.h \
	common/rdkmediaplayerimpl.cpp \
	common/rtevents.h \
	common/rtevents.cpp \
	common/glib_tools.cpp \
	common/glib_tools.h \
	common/logger.cpp \
	common/logger.h \
	common/timer.cpp \
	common/timer.h \
	common/intrect.h \
	rmf/mediaplayer.h \
	rmf/rmfplayer.cpp \
	rmf/rmfplayer.h \
	rmf/mediaplayerdlna.cpp \
	rmf/mediaplayerdlna.h \
	rmf/mediaplayergeneric.cpp \
	rmf/mediaplayergeneric.h \
	aamp/aampplayer.h \
	aamp/aampplayer.cpp 

rdkmediaplayer_LDFLAGS = \
	-lrmfbase \
	-lhnsource \
	-lmediaplayersink \
	-ldumpfilesink \
    -lIARMBus 

if ENABLE_COMCAST_BUILD
rdkmediaplayer_SOURCES += \
	common/closedcaptions.h \
	common/closedcaptions.cpp
rdkmediaplayer_LDFLAGS += \
    -lAVEPlayer \
    -lrdkCCReader \
    -lrdkCCManager \
    -lrdkCCGfx \
    -llog4c \
    -ldirectfb
else
AM_CXXFLAGS += -DDISABLE_CLOSEDCAPTIONS
endif

rdkmediaplayer_LDFLAGS += -laamp

# TODO-test is static libs make a difference on performance or memory consumption
#rdkmediaplayer_LDFLAGS += -lrtRemote_s -lrtCore_s -lpthread -luuid -ldl
rdkmediaplayer_LDFLAGS += -lrtRemote -lrtCore

rdkmediaplayer_LDFLAGS += \
	$(SOC_LIBS) \
	$(CURL_LIBS) \
	$(GLIB_LIBS) \
	$(GST_LIBS) \
	$(JANSON_LIBS)

if ENABLE_DIRECT_QAM
AM_CXXFLAGS += -DENABLE_DIRECT_QAM
rdkmediaplayer_LDFLAGS += -lqamsrc
endif

if ENABLE_IPPV_CLIENT
AM_CXXFLAGS += -DIPPV_CLIENT_ENABLED
rdkmediaplayer_LDFLAGS += -lippvsource
endif

if ENABLE_VOD_SRC
AM_CXXFLAGS += -DUSE_VODSRC
rdkmediaplayer_LDFLAGS += -lvodsrc
endif

if ENABLE_RDK_LOGGER
AM_CXXFLAGS += -DUSE_RDK_LOGGER
rdkmediaplayer_LDFLAGS += -lrdkloggers -llog4c
endif

if ENABLE_BREAKPAD
AM_CXXFLAGS += -DENABLE_BREAKPAD $(breakpad_CFLAGS)
rdkmediaplayer_LDFLAGS += $(breakpad_LIBS)
endif

## IPK Generation Support
IPK_GEN_PATH = $(abs_top_builddir)/ipk
IPK_GEN_STAGING_DIR=$(abs_top_builddir)/staging_dir
IPK_GEN_INPUT  = debian-binary control.tar.gz data.tar.gz
IPK_GEN_OUTPUT = rdkmediaplayer.ipk

package_ipk:
	@echo "Generating IPK Package"
	$(MAKE) prefix=${abs_top_builddir}/staging_dir install
	$(mkdir_p) $(IPK_GEN_STAGING_DIR)/usr/bin/
	cp -af $(IPK_GEN_STAGING_DIR)/bin/* $(IPK_GEN_STAGING_DIR)/usr/bin/
	tar -czvf $(IPK_GEN_PATH)/data.tar.gz -C $(IPK_GEN_STAGING_DIR) usr
	tar -czvf $(IPK_GEN_PATH)/control.tar.gz -C $(IPK_GEN_PATH) control postinst prerm
	cd $(IPK_GEN_PATH) && ar cr $(IPK_GEN_OUTPUT) $(IPK_GEN_INPUT) && cd -
	rm -rf ${abs_top_builddir}/staging_dir
	rm -rf $(abs_top_builddir)/ipk/control.tar.gz
	rm -rf $(abs_top_builddir)/ipk/data.tar.gz
