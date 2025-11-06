ROMNAME = Cypher
ROMTITLE = "CYPHER N64"
FINAL = 0
DEBUG = 1

BUILD_DIR = build
SRC_DIR = code

SRC = $(SRC_DIR)/main.c $(SRC_DIR)/startup.c $(SRC_DIR)/game.c $(SRC_DIR)/player.c $(SRC_DIR)/controls.c $(SRC_DIR)/debug_menu.c $(SRC_DIR)/animation.c
#SRC += $(SRC_DIR)/example.c

# Toolchain paths
include $(N64_INST)/include/n64.mk
include $(N64_INST)/include/t3d.mk

N64_ROM_SAVETYPE = eeprom4k

ifeq ($(FINAL),1)
  N64_ROM_ELFCOMPRESS = 3
  MKSPRITE_FLAGS = --compress 2
endif

ifeq ($(DEBUG), 1)
  N64_CFLAGS += -g -DDEBUG=$(DEBUG)
  N64_LDFLAGS += -g
endif

# Asset conversion rules
assets_png = $(wildcard assets/*.png)
assets_png_conv = $(addprefix filesystem/,$(notdir $(assets_png:%.png=%.sprite)))

assets_ttf = $(wildcard assets/*.ttf)
assets_ttf_conv = $(addprefix filesystem/,$(notdir $(assets_ttf:%.ttf=%.font64)))

assets_glb = $(wildcard assets/*.glb)
assets_gltf = $(wildcard assets/*.gltf)
assets_glb_conv = $(addprefix filesystem/,$(notdir $(assets_glb:%.glb=%.t3dm)))
assets_gltf_conv = $(addprefix filesystem/,$(notdir $(assets_gltf:%.gltf=%.t3dm)))

assets_mp3 = $(wildcard assets/*.mp3)
assets_mp3_conv = $(addprefix filesystem/,$(notdir $(assets_mp3:%.mp3=%.wav64)))

filesystem/%.sprite: assets/%.png
	@mkdir -p $(dir $@)
	@echo "    [SPRITE] $@"
	$(N64_MKSPRITE) $(MKSPRITE_FLAGS) --format CI4 --verbose -o $(dir $@) "$<"

filesystem/%.font64: assets/%.ttf
	@mkdir -p $(dir $@)
	@echo "    [FONT] $@"
	$(N64_MKFONT) $(MKFONT_FLAGS) -o $(dir $@) "$<"

# Specific font sizes
filesystem/NeuropolX.font64: MKFONT_FLAGS+=--size 28
filesystem/NeuropolX-small.font64: MKFONT_FLAGS+=--size 16

filesystem/%.t3dm: assets/%.glb
	@mkdir -p $(dir $@)
	@echo "    [3D-MODEL] $@"
	$(T3D_GLTF_TO_3D) $(T3DM_FLAGS) "$<" $@

filesystem/%.t3dm: assets/%.gltf
	@mkdir -p $(dir $@)
	@echo "    [3D-MODEL] $@"
	$(T3D_GLTF_TO_3D) $(T3DM_FLAGS) "$<" $@

filesystem/%.wav64: assets/%.wav
	@mkdir -p $(dir $@)
	@echo "    [AUDIO-WAV] $@"
	$(N64_AUDIOCONV) $(AUDIOCONV_FLAGS) -o $(dir $@) "$<"

filesystem/%.wav64: assets/%.mp3
	@mkdir -p $(dir $@)
	@echo "    [AUDIO-MP3] $@"
	$(N64_AUDIOCONV) $(AUDIOCONV_FLAGS) -o $(dir $@) "$<"

filesystem/%.xm64: assets/%.xm
	@mkdir -p $(dir $@)
	@echo "    [MUSIC] $@"
	$(N64_AUDIOCONV) $(AUDIOCONV_FLAGS) -o $(dir $@) "$<"

filesystem/%.desc: assets/%.txt
	@mkdir -p $(dir $@)
	@echo "    [DESCRIPTION] $@"
	cp "$<" $@

# Build rules
all: $(ROMNAME).z64

# Ensure sprites are built before models that may reference them
$(assets_glb_conv): $(assets_png_conv)
$(assets_gltf_conv): $(assets_png_conv)

$(BUILD_DIR)/$(ROMNAME).dfs: $(assets_png_conv) $(assets_ttf_conv) $(assets_glb_conv) $(assets_gltf_conv) $(assets_mp3_conv)
$(BUILD_DIR)/$(ROMNAME).elf: $(SRC:%.c=$(BUILD_DIR)/%.o)

$(ROMNAME).z64: N64_ROM_TITLE=$(ROMTITLE)
$(ROMNAME).z64: $(BUILD_DIR)/$(ROMNAME).dfs $(BUILD_DIR)/$(ROMNAME).msym

clean:
	rm -rf $(BUILD_DIR) filesystem/* $(ROMNAME).z64 

# Include dependency files
-include $(wildcard $(BUILD_DIR)/*.d)

.PHONY: all clean
