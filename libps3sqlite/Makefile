#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(PSL1GHT)),)
$(error "Please set PSL1GHT in your environment. export PSL1GHT=<path>")
endif

#---------------------------------------------------------------------------------
#  TITLE, APPID, CONTENTID, ICON0 SFOXML before ppu_rules.
#---------------------------------------------------------------------------------
TITLE		:=	PS3 SQLite
APPID		:=	PS3SQLITE1
CONTENTID	:=	UP0001-$(APPID)_00-0000000000000000

include $(PSL1GHT)/ppu_rules

# aditional scetool flags (--self-ctrl-flags, --self-cap-flags...)
SCETOOL_FLAGS	+=	

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
TARGET		:=	libps3sqlite
BUILD		:=	build
SOURCES		:=	source
DATA		:=	data
SHADERS		:=	shaders
INCLUDES	:=	../include


#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS		:=	-lz -lsimdmath -lgcm_sys -lio -lsysutil -lrt -llv2 -lsysmodule -lm


#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------

CFLAGS		=	-std=gnu99 -mminimal-toc -DWORDS_BIGENDIAN -DPATH_MAX=512 -DPS3 -D_FILE_OFFSET_BITS=64 \
			-DSQLITE_OS_OTHER=1 -DSQLITE_DEFAULT_LOCKING_MODE=1 -DSQLITE_MUTEX_NOOP -DCONFIG_KVSTORE=1 -DENABLE_KVSTORE=1 \
			-DENABLE_SQLITE_INTERNAL=1 -DCONFIG_SQLITE_VFS=1 -D ENABLE_SQLITE_VFS=1 -DCONFIG_SQLITE_INTERNAL=1 \
			-DSQLITE_THREADSAFE=0 \
			-DSQLITE_OMIT_UTF16 \
			-DSQLITE_OMIT_AUTOINIT \
			-DSQLITE_OMIT_COMPLETE \
			-DSQLITE_OMIT_DECLTYPE \
			-DSQLITE_OMIT_DEPRECATED \
			-DSQLITE_OMIT_GET_TABLE \
			-DSQLITE_OMIT_TCL_VARIABLE \
			-DSQLITE_OMIT_LOAD_EXTENSION \
			-DSQLITE_DEFAULT_FOREIGN_KEYS=1 \
			-O2 -Wall -mcpu=cell $(MACHDEP) $(INCLUDE) \
			
CXXFLAGS	=	$(CFLAGS)

LDFLAGS		=	$(MACHDEP) -Wl,-Map,$(notdir $@).map


#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:=

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir)) \
					$(foreach dir,$(SHADERS),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

export BUILDDIR	:=	$(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# automatically build a list of object files for our project
#---------------------------------------------------------------------------------
CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
sFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))
BINFILES	:= $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.bin)))
VCGFILES	:=	$(foreach dir,$(SHADERS),$(notdir $(wildcard $(dir)/*.vcg)))
FCGFILES	:=	$(foreach dir,$(SHADERS),$(notdir $(wildcard $(dir)/*.fcg)))

VPOFILES	:=	$(VCGFILES:.vcg=.vpo)
FPOFILES	:=	$(FCGFILES:.fcg=.fpo)

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
					$(addsuffix .o,$(VPOFILES)) \
					$(addsuffix .o,$(FPOFILES)) \
					$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) \
					$(sFILES:.s=.o) $(SFILES:.S=.o)

#---------------------------------------------------------------------------------
# build a list of include paths
#---------------------------------------------------------------------------------
export INCLUDE	:=	$(foreach dir,$(INCLUDES), -I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					$(LIBPSL1GHT_INC) \
					-I$(CURDIR)/$(BUILD) -I$(PORTLIBS)/include

#---------------------------------------------------------------------------------
# build a list of library paths
#---------------------------------------------------------------------------------
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib) \
					$(LIBPSL1GHT_LIB) -L$(PORTLIBS)/lib

export OUTPUT	:=	$(CURDIR)/$(TARGET)
.PHONY: $(BUILD) clean


#---------------------------------------------------------------------------------
$(BUILD):
	[ -d $@ ] || mkdir -p $@
	$(MAKE) -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT).self  EBOOT.BIN
	@rm -fr *.a

#---------------------------------------------------------------------------------


else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT).a: base64.o db_support.o fa_data.o fa_fs.o fileaccess.o font_b.o ps3_main.o rstr.o sqlite3.o str.o vfs.o wd.o

#---------------------------------------------------------------------------------
# This rule links in binary data with the .bin extension
#---------------------------------------------------------------------------------
%.bin.o	:	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
%.vpo.o	:	%.vpo
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
%.fpo.o	:	%.fpo
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
