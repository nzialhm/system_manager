# 상위 규칙 포함
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/package.mk

PKG_NAME:=system_manager
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_SRC_DIR:=$(TOPDIR)/package/nct11af/system_manager
PKG_MAINTAINER:=lucas <lucas@nzia.com>

# =========================
# Package info
# =========================
define Package/system_manager/Default
  SECTION:=nct11af
  CATEGORY:=nct11af
  TITLE:=system manager
  DEPENDS:=+libhpa +libgps +libtemp +libcheck_tx_power +libubox +libubus +libblobmsg-json +libuci
  URL:=nzia.net
endef

define Package/system_manager/Default/description
 This is just sample for "How to add new project to buildroot"
endef

define Package/system_manager
  $(call Package/system_manager/Default)
endef

define Package/system_manager/description
  $(call Package/system_manager/Default/description)
endef

# =========================
# Prepare: 소스 복사
# =========================
define Build/Prepare
	@echo "[system_manager] Prepare"
	rm -rf $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)

	# src 폴더 포함해서 전체 복사
	cp -r $(PKG_SRC_DIR)/* $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS += -g -O0
TARGET_CFLAGS += \
    -I$(PKG_BUILD_DIR)/src \
	-I$(PKG_BUILD_DIR)/src/common \
	-I$(PKG_BUILD_DIR)/src/config \
    -I$(PKG_BUILD_DIR)/src/slave_module \
    -I$(PKG_BUILD_DIR)/src/master_module \
    -I$(PKG_BUILD_DIR)/src/ubus

# =========================
# Compile: 완전 디버깅용
# =========================
# define Build/Compile
# 	@echo "[system_manager] Build/Compile (Debug mode)"

# 	# 디버그 심볼 포함, 최적화 제거
# 	# $(TARGET_CC)와 $(TARGET_CFLAGS)는 Buildroot 내부 변수
# 	$(TARGET_CFLAGS) += \
# 		-I$(PKG_BUILD_DIR)/system_manager \
# 		-I$(PKG_BUILD_DIR)/system_manager/slave_module \
# 		-I$(PKG_BUILD_DIR)/system_manager/master_module \
# 		-I$(PKG_BUILD_DIR)/system_manager/ubus

# 	$(TARGET_CC) -g -O0 \
# 		-I$(STAGING_DIR)/usr/include \
# 		-I$(STAGING_DIR)/usr/include/libubox \
# 		-I$(PKG_BUILD_DIR)/../../libgps/src \
# 		-I$(PKG_BUILD_DIR)/../../libcheck_tx_power/src \
# 		$(PKG_BUILD_DIR)/system_manager.c \
# 		$(PKG_BUILD_DIR)/master.c \
# 		$(PKG_BUILD_DIR)/slave.c \
# 		$(PKG_BUILD_DIR)/slave_module/alive_client.c \
# 		$(PKG_BUILD_DIR)/slave_module/image_client.c \
# 		$(PKG_BUILD_DIR)/master_module/image_server.c \
# 		$(PKG_BUILD_DIR)/ubus/ubus.c \
# 		-L$(STAGING_DIR)/usr/lib \
# 		-lhpa -lgps -ltemp -lcheck_tx_power -lubox -lubus -lblobmsg_json \
# 		-pthread \
# 		-o $(PKG_BUILD_DIR)/system_manager

# 	@echo "[system_manager] Build finished (Debug symbols included)"
# endef

define Build/Compile
	@echo "[system_manager] Compile"

	$(TARGET_CC) $(TARGET_CFLAGS) \
		-I$(STAGING_DIR)/usr/include \
		-I$(STAGING_DIR)/usr/include/libubox \
		$(PKG_BUILD_DIR)/src/system_manager.c \
		$(PKG_BUILD_DIR)/src/server_adress.c \
		$(PKG_BUILD_DIR)/src/apip.c \
		$(PKG_BUILD_DIR)/src/master.c \
		$(PKG_BUILD_DIR)/src/slave.c \
		$(PKG_BUILD_DIR)/src/config/config_reader.c \
		$(PKG_BUILD_DIR)/src/config/uci_reader.c \
		$(PKG_BUILD_DIR)/src/config/uci_cmd.c \
		$(PKG_BUILD_DIR)/src/slave_module/alive_client.c \
		$(PKG_BUILD_DIR)/src/slave_module/image_client.c \
		$(PKG_BUILD_DIR)/src/master_module/image_server.c \
		$(PKG_BUILD_DIR)/src/master_module/alive_server.c \
		$(PKG_BUILD_DIR)/src/ubus/ubus.c \
		-L$(STAGING_DIR)/usr/lib \
		-lubox -lubus -lblobmsg_json -luci \
		-pthread \
		-o $(PKG_BUILD_DIR)/system_manager
endef

# =========================
# Install
# =========================
define Package/system_manager/install
	@echo "[system_manager] Installing..."
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/system_manager $(1)/usr/bin/system_manager
	chmod 0755 $(1)/usr/bin/system_manager
	$(CP) ./src/config/systemmanager_config $(1)/usr/bin/systemmanager_config

	# $(INSTALL_DIR) $(1)/etc/init.d
	# $(INSTALL_BIN) ./files/system_manager.init $(1)/etc/init.d/system_manager
endef

$(eval $(call BuildPackage,system_manager))
