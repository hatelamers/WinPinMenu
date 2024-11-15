set(PRODUCT_VERSION_MAJOR 0)
set(PRODUCT_VERSION_MINOR 1)
set(PRODUCT_VERSION_PATCH 0)
set(PRODUCT_NAME WinPinMenu)
set(PRODUCT_VERSION ${PRODUCT_VERSION_MAJOR}.${PRODUCT_VERSION_MINOR}.${PRODUCT_VERSION_PATCH}.${BUILD_NUMBER})
set(PRODUCT_VENDOR "diVISION")
set(PRODUCT_DESCRIPTON "diVISION Pinnable Taskbar Menu For Windows")
set(PRODUCT_MAINTANER "dimamizou@users.sf.net")
set(PRODUCT_HOMEPAGE_URL "https://github.com/hatelamers/WinPinMenu")
set(PROJECT_LICENSE "GNU/GPL")
set(PROJECT_LICENSE_URL "https://www.gnu.org/licenses/gpl-3.0.en.html")
set(PROJECT_COPYRIGHT "© 2024, some rights reserved")
string(TOLOWER "${PROJECT_NAME}" PRODUCT_IDENTIFIER)
set(PACKAGE_IDENTIFIER "net.jasics.${PRODUCT_IDENTIFIER}")
